#include <N2kMsg.h>
#include <NMEA2000.h>
#include <NMEA2000_esp32.h>
#include <map>
#include <memory>
#include <vector>

#include "sensesp/sensors/digital_input.h"
#include "sensesp/signalk/signalk_prefix_listener.h"
#include "sensesp/signalk/signalk_put_request.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/system/max98357a.h"
#include "sensesp/system/pico_tts.h"
#include "sensesp/system/system_status_led.h"
#include "sensesp/transforms/debounce.h"
#include "sensesp/ui/config_item.h"
#include "sensesp_app.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

// Pin defaults match the ESP32-S3 board configuration in platformio.ini.
#ifndef MAX98357A_LRC_PIN
#define MAX98357A_LRC_PIN 4
#endif
#ifndef MAX98357A_BCLK_PIN
#define MAX98357A_BCLK_PIN 5
#endif
#ifndef MAX98357A_DIN_PIN
#define MAX98357A_DIN_PIN 6
#endif
#ifndef N2K_CAN_TX_PIN
#define N2K_CAN_TX_PIN 17
#endif
#ifndef N2K_CAN_RX_PIN
#define N2K_CAN_RX_PIN 18
#endif
// Push buttons, wired between the pin and ground.
#ifndef ACK_BUTTON_PIN
#define ACK_BUTTON_PIN 15
#endif
#ifndef SNOOZE_BUTTON_PIN
#define SNOOZE_BUTTON_PIN 16
#endif
#ifndef PIN_RGB_LED
#define PIN_RGB_LED 48
#endif

// Signal K Outlook Guardian: speaks vessel alarms through a MAX98357A I2S
// amplifier using PicoTTS text-to-speech and shows them on the on-board RGB
// LED.
//
// - Signal K: subscribes to `notifications.*` and speaks the `message` of
//   every raised notification.
// - NMEA 2000: listens to the Alert (PGN 126983) and Alert Text (PGN 126985)
//   messages through a CAN transceiver, composes a sentence out of the alert
//   type, category, description and location, and speaks it. This is a
//   fallback: NMEA 2000 is ignored while the Signal K server is connected.
// - An alarm is repeated until it clears or is acknowledged or snoozed with
//   the two push buttons. For Signal K notifications the button press is
//   sent to the server as a PUT on the notification's `method`.
// - The LED color follows the most severe alarm: blue for alert/caution,
//   yellow for warning, red for alarm and emergency. It blinks until the
//   alarm is acknowledged or snoozed, and is off when there is no alarm.
// - The language, voice, volume, repeat interval and snooze time are set in
//   the web UI.

enum class Severity { kNone, kAlert, kWarn, kAlarm, kEmergency };

///////////////////////////////////////////////////////////////////////////////
// Configuration

// PicoTTS has a single speaker per language, so the voices are pitch
// variations of it.
struct Voice {
  const char* name;
  int pitch;
};
const Voice kVoices[] = {{"Standard", 100}, {"Deep", 80}, {"High", 120}};
// All the languages PicoTTS supports.
const char* const kLanguages[] = {"en-US", "en-GB", "it-IT",
                                  "de-DE", "es-ES", "fr-FR"};

class GuardianConfig : public FileSystemSaveable {
 public:
  GuardianConfig(const String& config_path) : FileSystemSaveable(config_path) {
    // Only the languages listed in `custom_picotts_languages` are built in.
    for (const char* language : kLanguages) {
      if (picotts_set_language(language)) {
        languages_.push_back(language);
      }
    }
    if (!languages_.empty()) {
      language_ = languages_.front();
    }
    load();
  }

  bool to_json(JsonObject& root) override {
    root["language"] = language_;
    root["voice"] = voice_;
    root["speed"] = speed_;
    root["volume"] = volume_;
    root["repeat_interval"] = repeat_interval_s_;
    root["snooze_time"] = snooze_time_min_;
    return true;
  }

  bool from_json(const JsonObject& config) override {
    const String language = config["language"] | language_;
    for (const String& available : languages_) {
      if (available == language) {
        language_ = language;
      }
    }
    voice_ = config["voice"] | voice_;
    speed_ = constrain(config["speed"] | speed_, 50, 200);
    volume_ = constrain(config["volume"] | volume_, 0, 100);
    repeat_interval_s_ = max(config["repeat_interval"] | repeat_interval_s_, 0);
    snooze_time_min_ = max(config["snooze_time"] | snooze_time_min_, 1);
    if (on_change_) {
      on_change_();
    }
    return true;
  }

  const String& language() const { return language_; }
  const std::vector<String>& languages() const { return languages_; }
  float volume() const { return volume_ / 100.0f; }
  uint32_t repeat_interval_ms() const { return repeat_interval_s_ * 1000UL; }
  uint32_t snooze_time_ms() const { return snooze_time_min_ * 60000UL; }

  // Wrap the text in the PicoTTS markup that selects the voice.
  String markup(String text) const {
    // Anything that looks like a tag would be taken as markup.
    text.replace('<', ' ');
    text.replace('>', ' ');
    int pitch = 100;
    for (const Voice& voice : kVoices) {
      if (voice_ == voice.name) {
        pitch = voice.pitch;
      }
    }
    if (pitch != 100) {
      text = "<pitch level=\"" + String(pitch) + "\">" + text + "</pitch>";
    }
    if (speed_ != 100) {
      text = "<speed level=\"" + String(speed_) + "\">" + text + "</speed>";
    }
    return text;
  }

  void on_change(std::function<void()> callback) { on_change_ = callback; }

 protected:
  std::vector<String> languages_;
  String language_ = "en-US";
  String voice_ = kVoices[0].name;
  int speed_ = 100;
  int volume_ = 50;
  int repeat_interval_s_ = 30;
  int snooze_time_min_ = 5;
  std::function<void()> on_change_;
};

const String ConfigSchema(const GuardianConfig& obj) {
  const auto select = [](const String& title, const String& options) {
    return String(R"###({"title":")###") + title +
           R"###(","type":"array","format":"select","uniqueItems":true,)###"
           R"###("items":{"type":"string","enum":[)###" +
           options + "]}}";
  };
  String languages;
  for (const String& language : obj.languages()) {
    languages += (languages.length() > 0 ? ",\"" : "\"") + language + "\"";
  }
  String voices;
  for (const Voice& voice : kVoices) {
    voices += String(voices.length() > 0 ? ",\"" : "\"") + voice.name + "\"";
  }
  return String(R"###({"type":"object","properties":{"language":)###") +
         select("Language", languages) + R"###(,"voice":)###" +
         select("Voice", voices) +
         R"###(,"speed":{"title":"Speech speed (%, 50-200)","type":"integer"})###"
         R"###(,"volume":{"title":"Volume (%)","type":"integer"})###"
         R"###(,"repeat_interval":{"title":"Repeat an unacknowledged alarm every (s, 0 = speak once)","type":"integer"})###"
         R"###(,"snooze_time":{"title":"Snooze time (min)","type":"integer"}}})###";
}

///////////////////////////////////////////////////////////////////////////////
// RGB LED

// Shows the alarm state instead of the SensESP system status.
class AlarmLed : public BaseSystemStatusLed {
 public:
  AlarmLed(uint8_t pin, uint8_t brightness = 40)
      : BaseSystemStatusLed(), pin_{pin}, brightness_{brightness} {}

  void set_alarm(Severity severity, bool blinking) {
    severity_ = severity;
    blinking_ = blinking;
  }

 protected:
  // Called every few milliseconds by the base class.
  void show() override {
    uint32_t color = 0x000000;
    switch (severity_) {
      case Severity::kAlert:
        color = 0x0000ff;
        break;
      case Severity::kWarn:
        color = 0xffa000;
        break;
      case Severity::kAlarm:
      case Severity::kEmergency:
        color = 0xff0000;
        break;
      default:
        break;
    }
    const uint32_t half_period_ms =
        severity_ == Severity::kEmergency ? 125 : 500;
    if (blinking_ && (millis() / half_period_ms) % 2 == 1) {
      color = 0x000000;
    }
    if (color == shown_color_) {
      return;
    }
    shown_color_ = color;
    rgbLedWrite(pin_, apply_brightness(color >> 16, brightness_),
                apply_brightness((color >> 8) & 0xff, brightness_),
                apply_brightness(color & 0xff, brightness_));
  }

  void set_brightness(uint8_t brightness) override { brightness_ = brightness; }

  uint8_t pin_;
  uint8_t brightness_;
  Severity severity_ = Severity::kNone;
  bool blinking_ = false;
  // Not a color, so that the LED is switched off on the first call.
  uint32_t shown_color_ = 0xffffffff;
};

///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<GuardianConfig> config;
std::shared_ptr<AlarmLed> alarm_led;
std::shared_ptr<MAX98357A> amplifier;
std::shared_ptr<PicoTTS> tts;
std::shared_ptr<SKPrefixListener> notification_listener;
tNMEA2000* nmea2000;

void say(const String& text) {
  tts->say(config->markup(text), config->language());
}

bool signalk_connected() {
  return sensesp_app->get_ws_client()->is_connected();
}

///////////////////////////////////////////////////////////////////////////////
// Alarms, from either Signal K or NMEA 2000

constexpr size_t kMaxAlarms = 32;

struct Alarm {
  bool from_signalk = false;
  Severity severity = Severity::kNone;
  String message;
  // Whether the source wants the alarm to sound.
  bool audible = true;
  bool remote_acknowledged = false;
  // Acknowledged with the button
  bool acknowledged = false;
  bool snoozed = false;
  uint32_t snoozed_at_ms = 0;
  // Signal K `method` of the notification, as JSON, and its value before
  // snoozing, which is restored when the snooze time is over.
  String method;
  String method_before_snooze;
  bool spoken = false;
  uint32_t spoken_at_ms = 0;

  bool is_acknowledged() const { return acknowledged || remote_acknowledged; }
  bool is_pending() const { return !is_acknowledged() && !snoozed; }
};

// Keyed by the Signal K path, or by "n2k:" and the NMEA 2000 alert identity.
std::map<String, Alarm> alarms;

void raise_alarm(const String& key, bool from_signalk, Severity severity,
                 const String& message, bool audible, bool remote_acknowledged,
                 const String& method = "") {
  auto it = alarms.find(key);
  if (it == alarms.end()) {
    if (alarms.size() >= kMaxAlarms) {
      return;
    }
    it = alarms.emplace(key, Alarm{}).first;
  }
  Alarm& alarm = it->second;
  if (alarm.severity != severity || alarm.message != message) {
    // New or changed: announce it again, whatever was done about it before.
    ESP_LOGI(__FILENAME__, "Alarm %s: %s", key.c_str(), message.c_str());
    alarm.spoken = false;
    alarm.acknowledged = false;
    alarm.snoozed = false;
  }
  alarm.from_signalk = from_signalk;
  alarm.severity = severity;
  alarm.message = message;
  alarm.audible = audible;
  alarm.remote_acknowledged = remote_acknowledged;
  alarm.method = method;
}

void clear_alarms(bool from_signalk) {
  for (auto it = alarms.begin(); it != alarms.end();) {
    it = it->second.from_signalk == from_signalk ? alarms.erase(it) : ++it;
  }
}

// Tell the Signal K server how a notification should be indicated from now
// on. `method` is a JSON array, e.g. `["visual"]`.
void put_notification_method(const String& path, const String& method) {
  if (!signalk_connected()) {
    return;
  }
  JsonDocument request;
  request["context"] = "vessels.self";
  request["put"]["path"] = path + ".method";
  request["put"]["value"] = serialized(method);
  SKRequest::send_request(request, [path](JsonDocument& response) {
    const int status = response["statusCode"] | 0;
    if (status != 0 && status != 200 && status != 202) {
      ESP_LOGW(__FILENAME__, "Server refused the update of %s: %d",
               path.c_str(), status);
    }
  });
}

// Acknowledge button: the pending alarms stay on the LED but are no longer
// spoken.
void acknowledge_alarms() {
  for (auto& [key, alarm] : alarms) {
    if (alarm.is_acknowledged()) {
      continue;
    }
    ESP_LOGI(__FILENAME__, "Acknowledged %s", key.c_str());
    alarm.acknowledged = true;
    alarm.snoozed = false;
    if (alarm.from_signalk) {
      put_notification_method(key, "[]");
    }
  }
}

// Snooze button: the pending alarms keep quiet for the snooze time.
void snooze_alarms() {
  for (auto& [key, alarm] : alarms) {
    if (!alarm.is_pending()) {
      continue;
    }
    ESP_LOGI(__FILENAME__, "Snoozed %s", key.c_str());
    alarm.snoozed = true;
    alarm.snoozed_at_ms = millis();
    if (alarm.from_signalk) {
      alarm.method_before_snooze = alarm.method;
      put_notification_method(key, "[\"visual\"]");
    }
  }
}

// Wake up the snoozed alarms, speak the next alarm that is due and update
// the LED.
void process_alarms() {
  const uint32_t now = millis();
  const uint32_t repeat_interval_ms = config->repeat_interval_ms();
  Alarm* next = nullptr;
  Severity led_severity = Severity::kNone;
  bool pending = false;

  for (auto& [key, alarm] : alarms) {
    if (alarm.snoozed && now - alarm.snoozed_at_ms > config->snooze_time_ms()) {
      alarm.snoozed = false;
      alarm.spoken = false;
      if (alarm.from_signalk) {
        put_notification_method(key, alarm.method_before_snooze.length() > 0
                                         ? alarm.method_before_snooze
                                         : "[\"visual\",\"sound\"]");
      }
    }
    led_severity = max(led_severity, alarm.severity);
    pending = pending || alarm.is_pending();

    const bool due =
        !alarm.spoken ||
        (repeat_interval_ms > 0 && now - alarm.spoken_at_ms > repeat_interval_ms);
    if (!alarm.is_pending() || !alarm.audible || !due ||
        alarm.message.length() == 0) {
      continue;
    }
    // The most severe first, then the one that has waited the longest.
    if (next == nullptr || alarm.severity > next->severity ||
        (alarm.severity == next->severity && next->spoken &&
         (!alarm.spoken || now - alarm.spoken_at_ms > now - next->spoken_at_ms))) {
      next = &alarm;
    }
  }

  alarm_led->set_alarm(led_severity, pending);
  if (next != nullptr && !tts->is_speaking()) {
    next->spoken = true;
    next->spoken_at_ms = now;
    say(next->message);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Signal K notifications

bool json_array_contains(JsonVariantConst array, const char* text) {
  for (JsonVariantConst item : array.as<JsonArrayConst>()) {
    if (item.as<String>() == text) {
      return true;
    }
  }
  return false;
}

void handle_sk_notification(const SKPathValue& notification) {
  const JsonVariantConst value = notification.value;
  const String state = value["state"] | "";
  Severity severity = Severity::kNone;
  if (state == "alert") {
    severity = Severity::kAlert;
  } else if (state == "warn") {
    severity = Severity::kWarn;
  } else if (state == "alarm") {
    severity = Severity::kAlarm;
  } else if (state == "emergency") {
    severity = Severity::kEmergency;
  }
  if (severity == Severity::kNone) {
    // Removed (null value), or back to "normal" or "nominal".
    alarms.erase(notification.path);
    return;
  }
  // `method` says how to indicate the alarm. Other devices acknowledge it
  // by emptying the list, and silence it by removing "sound" from it. Newer
  // servers also report these in `status`.
  const JsonVariantConst method = value["method"];
  const bool has_method = method.is<JsonArrayConst>();
  const bool acknowledged = (has_method && method.size() == 0) ||
                            (value["status"]["acknowledged"] | false);
  const bool audible = (!has_method || json_array_contains(method, "sound")) &&
                       !(value["status"]["silenced"] | false);
  String method_json;
  if (has_method) {
    serializeJson(method, method_json);
  }
  raise_alarm(notification.path, true, severity, value["message"] | "",
              audible, acknowledged, method_json);
}

///////////////////////////////////////////////////////////////////////////////
// NMEA 2000 alerts

constexpr unsigned long kN2kAlertPgn = 126983L;
constexpr unsigned long kN2kAlertTextPgn = 126985L;
// Alert State values of PGN 126983
constexpr uint8_t kN2kAlertStateDisabled = 0;
constexpr uint8_t kN2kAlertStateNormal = 1;
constexpr uint8_t kN2kAlertStateSilenced = 3;
constexpr uint8_t kN2kAlertStateAcknowledged = 4;
// How long to wait for the Alert Text of a newly raised alert before
// announcing it without a description.
constexpr uint32_t kN2kAlertTextWaitMs = 2000;
constexpr size_t kN2kMaxAlerts = 32;

struct N2kAlert {
  uint8_t type = 0;
  uint8_t category = 0;
  uint16_t id = 0;
  String text;
  String location;
  // Raised according to an Alert message; the text alone does not raise it.
  bool raised = false;
  uint32_t raised_at_ms = 0;
  bool silenced = false;
  bool acknowledged = false;
};

std::map<String, N2kAlert> n2k_alerts;

// Both alert PGNs start with the same fields identifying the alert.
String parse_n2k_alert_header(const tN2kMsg& msg, int& index, uint8_t& type,
                              uint8_t& category, uint16_t& id) {
  const uint8_t type_and_category = msg.GetByte(index);
  type = type_and_category & 0x0f;
  category = type_and_category >> 4;
  const uint8_t system = msg.GetByte(index);
  const uint8_t subsystem = msg.GetByte(index);
  id = msg.Get2ByteUInt(index);
  const uint64_t source_name = msg.GetUInt64(index);
  const uint8_t instance = msg.GetByte(index);
  const uint8_t source_index = msg.GetByte(index);
  const uint8_t occurrence = msg.GetByte(index);
  char key[64];
  snprintf(key, sizeof(key), "n2k:%u/%u/%u/%u/%016llx/%u/%u/%u", type, system,
           subsystem, id, static_cast<unsigned long long>(source_name),
           instance, source_index, occurrence);
  return key;
}

N2kAlert* find_n2k_alert(const String& key) {
  auto it = n2k_alerts.find(key);
  if (it != n2k_alerts.end()) {
    return &it->second;
  }
  if (n2k_alerts.size() >= kN2kMaxAlerts) {
    return nullptr;
  }
  return &n2k_alerts[key];
}

String read_n2k_string(const tN2kMsg& msg, int& index) {
  char buffer[128] = "";
  size_t size = sizeof(buffer);
  msg.GetVarStr(size, buffer, index);
  String text{buffer};
  text.trim();
  return text;
}

Severity n2k_alert_severity(const N2kAlert& alert) {
  switch (alert.type) {
    case 1:
      return Severity::kEmergency;
    case 2:
      return Severity::kAlarm;
    case 5:
      return Severity::kWarn;
    default:
      return Severity::kAlert;
  }
}

// The words used to compose the NMEA 2000 alert sentences and the startup
// announcement, by language.
struct Phrases {
  const char* language;  // Two-letter prefix of the language code
  const char* emergency_alarm;
  const char* alarm;
  const char* warning;
  const char* caution;
  const char* alert;
  // Category, with the separator that joins it to the alert type.
  const char* navigational;
  const char* technical;
  // The category goes before the type, as in "Technical warning".
  bool category_first;
  const char* number;
  const char* active;
};

const Phrases kPhrases[] = {
    {"en", "emergency alarm", "alarm", "warning", "caution", "alert",
     "Navigational ", "Technical ", true, "number", "active"},
    {"it", "Allarme di emergenza", "Allarme", "Avviso", "Avviso di cautela",
     "Avviso", " di navigazione", " tecnico", false, "numero", "attivo"},
    {"de", "Notfallalarm", "Alarm", "Warnung", "Vorsicht", "Meldung",
     "Navigation, ", "Technik, ", true, "Nummer", "aktiv"},
    {"es", "Alarma de emergencia", "Alarma", "Advertencia", "Precaución",
     "Alerta", " de navegación", " técnica", false, "número", "activo"},
    {"fr", "Alarme d'urgence", "Alarme", "Avertissement", "Attention",
     "Alerte", " de navigation", " technique", false, "numéro", "actif"},
};

// The phrases of the configured language, or the English ones.
const Phrases& phrases() {
  for (const Phrases& candidate : kPhrases) {
    if (config->language().startsWith(candidate.language)) {
      return candidate;
    }
  }
  return kPhrases[0];
}

// Compose the sentence to speak in the configured language, e.g.
// "Technical warning. Low oil pressure. Engine room."
String compose_n2k_alert_message(const N2kAlert& alert) {
  const Phrases& words = phrases();
  const char* type;
  switch (alert.type) {
    case 1:
      type = words.emergency_alarm;
      break;
    case 2:
      type = words.alarm;
      break;
    case 5:
      type = words.warning;
      break;
    case 8:
      type = words.caution;
      break;
    default:
      type = words.alert;
      break;
  }
  const char* category = alert.category == 0   ? words.navigational
                         : alert.category == 1 ? words.technical
                                               : "";
  String message = words.category_first ? String(category) + type
                                        : String(type) + category;
  if (alert.text.length() == 0) {
    return message + " " + words.number + " " + String(alert.id) + ".";
  }
  message += ". " + alert.text + ".";
  if (alert.location.length() > 0) {
    message += " " + alert.location + ".";
  }
  return message;
}

void handle_n2k_alert(const tN2kMsg& msg) {
  int index = 0;
  uint8_t type;
  uint8_t category;
  uint16_t id;
  const String key = parse_n2k_alert_header(msg, index, type, category, id);
  // Skip the status flags, the acknowledge source NAME, the trigger
  // condition, the threshold status and the priority.
  index += 11;
  const uint8_t state = msg.GetByte(index);

  if (state == kN2kAlertStateDisabled || state == kN2kAlertStateNormal) {
    // Cleared: forget it so that it is announced again the next time.
    n2k_alerts.erase(key);
    alarms.erase(key);
    return;
  }
  N2kAlert* alert = find_n2k_alert(key);
  if (alert == nullptr) {
    return;
  }
  alert->type = type;
  alert->category = category;
  alert->id = id;
  alert->silenced = state == kN2kAlertStateSilenced;
  alert->acknowledged = state == kN2kAlertStateAcknowledged;
  if (!alert->raised) {
    alert->raised = true;
    alert->raised_at_ms = millis();
  }
}

void handle_n2k_alert_text(const tN2kMsg& msg) {
  int index = 0;
  uint8_t type;
  uint8_t category;
  uint16_t id;
  const String key = parse_n2k_alert_header(msg, index, type, category, id);
  msg.GetByte(index);  // Language ID
  N2kAlert* alert = find_n2k_alert(key);
  if (alert == nullptr) {
    return;
  }
  alert->type = type;
  alert->category = category;
  alert->id = id;
  alert->text = read_n2k_string(msg, index);
  alert->location = read_n2k_string(msg, index);
}

void handle_n2k_message(const tN2kMsg& msg) {
  // NMEA 2000 is the fallback: the Signal K server reports the alarms while
  // it is connected.
  if (signalk_connected()) {
    return;
  }
  if (msg.PGN == kN2kAlertPgn) {
    handle_n2k_alert(msg);
  } else if (msg.PGN == kN2kAlertTextPgn) {
    handle_n2k_alert_text(msg);
  }
}

// The alert and its text come in separate messages: raise the alarm once the
// text is known, or without it if the text does not show up in time. A text
// arriving after that changes the message, which is then spoken again.
void raise_n2k_alarms() {
  for (auto& [key, alert] : n2k_alerts) {
    if (alert.raised &&
        (alert.text.length() > 0 ||
         millis() - alert.raised_at_ms > kN2kAlertTextWaitMs)) {
      raise_alarm(key, false, n2k_alert_severity(alert),
                  compose_n2k_alert_message(alert), !alert.silenced,
                  alert.acknowledged);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

// Switch between the two alarm sources as the Signal K server comes and goes.
// What the other source reported is dropped: it is no longer kept up to date.
void check_alarm_source() {
  static bool was_connected = false;
  const bool connected = signalk_connected();
  if (connected == was_connected) {
    return;
  }
  was_connected = connected;
  ESP_LOGI(__FILENAME__, "Alarms now come from %s",
           connected ? "Signal K" : "NMEA 2000");
  if (connected) {
    n2k_alerts.clear();
  }
  clear_alarms(!connected);
}

// Call `on_press` when the push button wired between `pin` and ground is
// pressed.
void setup_button(uint8_t pin, void (*on_press)()) {
  auto* input = new DigitalInputChange(pin, INPUT_PULLUP, CHANGE);
  input->connect_to(new Debounce<bool>(20))
      ->connect_to(new LambdaConsumer<bool>([on_press](bool level) {
        if (level == LOW) {
          on_press();
        }
      }));
}

void setup() {
  SetupLogging();

  // Create the global SensESPApp() object. The on-board RGB LED shows the
  // alarms instead of the system status.
  SensESPAppBuilder builder;
  alarm_led = std::make_shared<AlarmLed>(PIN_RGB_LED);
  sensesp_app = builder.set_hostname("outlook-guardian")
                    ->set_system_status_led(alarm_led)
                    ->get_app();

  config = std::make_shared<GuardianConfig>("/Outlook Guardian/Speech");
  ConfigItem(config)
      ->set_title("Speech and alarms")
      ->set_description(
          "Language and voice of the spoken alarms, and how they are "
          "repeated and snoozed.");

  amplifier = std::make_shared<MAX98357A>(
      MAX98357A_BCLK_PIN, MAX98357A_LRC_PIN, MAX98357A_DIN_PIN);
  if (!amplifier->begin()) {
    ESP_LOGE(__FILENAME__, "Failed to start the I2S output");
    return;
  }

  tts = std::make_shared<PicoTTS>(amplifier, config->language(),
                                  config->volume());
  if (!tts->begin()) {
    return;
  }
  config->on_change([]() { tts->set_volume(config->volume()); });
  say(String("Signal K Outlook Guardian ") + phrases().active);

  notification_listener = std::make_shared<SKPrefixListener>("notifications.");
  notification_listener->connect_to(
      std::make_shared<LambdaConsumer<SKPathValue>>(handle_sk_notification));

  nmea2000 = new tNMEA2000_esp32(static_cast<gpio_num_t>(N2K_CAN_TX_PIN),
                                 static_cast<gpio_num_t>(N2K_CAN_RX_PIN));
  nmea2000->SetMode(tNMEA2000::N2km_ListenOnly);
  nmea2000->EnableForward(false);
  nmea2000->SetMsgHandler(handle_n2k_message);
  nmea2000->Open();
  event_loop()->onRepeat(1, []() { nmea2000->ParseMessages(); });

  setup_button(ACK_BUTTON_PIN, acknowledge_alarms);
  setup_button(SNOOZE_BUTTON_PIN, snooze_alarms);

  event_loop()->onRepeat(100, []() {
    check_alarm_source();
    raise_n2k_alarms();
    process_alarms();
  });
}

void loop() { event_loop()->tick(); }
