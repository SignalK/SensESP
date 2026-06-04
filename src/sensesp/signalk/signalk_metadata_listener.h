#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_METADATA_LISTENER_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_METADATA_LISTENER_H_

#include <ArduinoJson.h>

#include <memory>
#include <vector>

#include "sensesp/system/observable.h"
#include "sensesp/system/valueproducer.h"
#include "signalk_listener.h"

namespace sensesp {

/**
 * @brief A single Signal K metadata zone.
 *
 * Zones describe value ranges and the alarm state associated with each (used
 * e.g. to colour a gauge). A path may define several. `has_lower`/`has_upper`
 * distinguish an open-ended zone (no bound) from one bounded at 0.
 *
 * @see https://signalk.org/specification/1.7.0/doc/data_model_metadata.html
 */
struct SKZone {
  bool has_lower = false;
  float lower = 0.0f;
  bool has_upper = false;
  float upper = 0.0f;
  String state;    ///< e.g. "nominal", "alarm", "warn", "alert", "emergency"
  String message;  ///< optional human-readable message
};

/**
 * @brief A typed, read-only view of Signal K metadata received for a path.
 *
 * This is the inbound counterpart to `SKMetadata` (which is an output-only
 * serializer used by `SKOutput`). It is the value emitted by
 * `SKMetadataListener`, so it must be cheaply copyable: it flows through
 * `ValueProducer` by value (copied into the producer's output and into each
 * observer). The commonly-used fields are exposed as typed members; the full
 * received meta object is retained (owned, refcounted) in `raw` so that fields
 * not modeled here (displayScale, vendor extensions, ...) are never lost.
 *
 * Construct from the `value` object of a `meta[]` delta entry.
 *
 * @see SKMetadataListener
 * @see SKMetadata
 */
struct SKMetaView {
  String display_name;
  String units;
  String description;
  String display_units;
  std::vector<SKZone> zones;

  /// The full received meta object, owned and refcounted (a copy is a refcount
  /// bump, not a deep copy). Carries any field not exposed as a typed member.
  /// Null on a default-constructed (no-data-yet) view.
  std::shared_ptr<const JsonDocument> raw;

  SKMetaView() = default;

  /// Parse a typed view from a received meta `value` object. Stores `owned` as
  /// the lossless `raw` backing; `meta` must point into `owned`.
  SKMetaView(JsonObjectConst meta, std::shared_ptr<const JsonDocument> owned)
      : display_name{meta["displayName"].as<String>()},
        units{meta["units"].as<String>()},
        description{meta["description"].as<String>()},
        display_units{meta["displayUnits"].as<String>()},
        raw{std::move(owned)} {
    for (JsonVariantConst zv : meta["zones"].as<JsonArrayConst>()) {
      JsonObjectConst z = zv.as<JsonObjectConst>();
      SKZone zone;
      if (!z["lower"].isNull()) {
        zone.has_lower = true;
        zone.lower = z["lower"].as<float>();
      }
      if (!z["upper"].isNull()) {
        zone.has_upper = true;
        zone.upper = z["upper"].as<float>();
      }
      zone.state = z["state"].as<String>();
      zone.message = z["message"].as<String>();
      zones.push_back(std::move(zone));
    }
  }
};

/**
 * @brief A ValueProducer that listens to the metadata stream for a single
 * Signal K path and emits a typed `SKMetaView` whenever it changes.
 *
 * Mirrors `SKValueListener` but consumes `updates[].meta[]` entries instead of
 * `updates[].values[]`. Metadata deltas are only pushed by the server when the
 * stream is subscribed with `sendMeta=all` (the SKWSClient default).
 *
 * The emitted `SKMetaView` exposes the common fields (units, displayName,
 * zones, ...) as typed members and retains the full meta object in `raw`, so it
 * is both ergonomic and lossless. It is safe for any deferred consumer (e.g. UI
 * work marshalled onto the event loop): the typed members are owned values and
 * `raw` is a refcounted owned document, so nothing aliases the transient
 * receive document.
 *
 * Because it is-a `SKListener`, its path is subscribed automatically by
 * `SKWSClient::subscribe_listeners()`, and `process_received_updates` routes
 * meta deltas to it via the out-of-band queue tag plus `wants_meta()`.
 *
 * Example:
 * @code
 * auto ml = std::make_shared<SKMetadataListener>("environment.depth.belowKeel");
 * ml->connect_to(new LambdaConsumer<SKMetaView>([](const SKMetaView& m) {
 *   gauge.set_units(m.units);
 *   gauge.set_zones(m.zones);
 * }));
 * @endcode
 *
 * @see SKValueListener
 * @see SKMetaView
 * @see SKWSClient
 */
class SKMetadataListener : public SKListener,
                           public ValueProducer<SKMetaView> {
 public:
  /**
   * @param sk_path The Signal K path to listen to for metadata changes
   * @param listen_delay The minimum interval between updates in ms
   * @param config_path Optional configuration path
   */
  SKMetadataListener(const String& sk_path, int listen_delay = 1000,
                     const String& config_path = "")
      : SKListener(sk_path, listen_delay, config_path) {
    if (sk_path == "") {
      ESP_LOGE(__FILENAME__,
               "SKMetadataListener: User has provided no sk_path.");
    }
  }

  bool wants_meta() const override { return true; }

  /**
   * Called on the main task by `SKWSClient::process_received_updates` with an
   * already-owned, read-only metadata document moved off the receive queue
   * (shape `{path, value: {...meta...}}`). Parses the typed view and emits it;
   * the owned document backs `SKMetaView::raw` so the lossless data outlives
   * any deferred consumer with no extra deep copy.
   */
  void parse_meta(
      const std::shared_ptr<const JsonDocument>& meta_doc) override {
    JsonObjectConst meta = (*meta_doc)["value"].as<JsonObjectConst>();
    this->emit(SKMetaView(meta, meta_doc));
  }

  // parse_value(const JsonObject&) is intentionally not overridden: metadata is
  // delivered via parse_meta() with an owned document, never via the value-path
  // JsonObject& (which would alias the receive-queue document).
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_METADATA_LISTENER_H_
