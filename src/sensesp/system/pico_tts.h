#ifndef SENSESP_SYSTEM_PICO_TTS_H_
#define SENSESP_SYSTEM_PICO_TTS_H_

#include <Arduino.h>
#include <picotts.h>
#include <picotts_lang.h>
#include <deque>
#include <memory>
#include <utility>

#include "max98357a.h"
#include "sensesp.h"

namespace sensesp {

/**
 * @brief Text-to-speech output using the PicoTTS engine.
 *
 * Speech is synthesized in a separate task and played through a MAX98357A
 * amplifier, so `say()` returns immediately. Utterances are queued and spoken
 * one after the other, each in its own language. The engine allocates about
 * 1.1 MB of RAM while running, which in practice requires PSRAM.
 *
 * Requires the `picotts` library (lib/picotts). The available languages are
 * chosen at build time with `custom_picotts_languages` in platformio.ini.
 * PicoTTS holds one voice at a time, so changing language restarts the
 * engine, which adds a short pause between the two utterances.
 *
 * `say()` must be called from the main event loop. The PicoTTS callbacks
 * carry no context pointer, so only one instance can be active at a time.
 */
class PicoTTS {
 public:
  /**
   * @param amplifier Started amplifier running at PICOTTS_SAMPLE_FREQ_HZ.
   * @param language Language used by `say()` when none is given.
   * @param volume Output volume between 0.0 and 1.0.
   * @param task_priority Priority of the synthesis task.
   * @param core Core to run the synthesis task on, or -1 for any.
   */
  PicoTTS(std::shared_ptr<MAX98357A> amplifier, const String& language,
          float volume = 0.5f, unsigned task_priority = 2, int core = 0)
      : amplifier_{amplifier},
        default_language_{language},
        task_priority_{task_priority},
        core_{core} {
    set_volume(volume);
  }

  ~PicoTTS() { end(); }

  /**
   * @brief Start the TTS engine. Returns false on failure.
   */
  bool begin() {
    if (started_) {
      return true;
    }
    if (amplifier_->get_sample_rate() != PICOTTS_SAMPLE_FREQ_HZ) {
      ESP_LOGE(__FILENAME__, "Amplifier sample rate must be %d Hz",
               PICOTTS_SAMPLE_FREQ_HZ);
      return false;
    }
    instance() = this;
    picotts_set_idle_notify(&PicoTTS::on_idle);
    picotts_set_error_notify(&PicoTTS::on_error);
    if (!start_engine(default_language_)) {
      instance() = nullptr;
      return false;
    }
    started_ = true;
    poll_event_ = event_loop()->onRepeat(kPollIntervalMs,
                                         [this]() { this->process_queue(); });
    return true;
  }

  /**
   * @brief Stop the TTS engine and free its memory. Pending text is dropped.
   */
  void end() {
    if (!started_) {
      return;
    }
    poll_event_->remove(event_loop());
    stop_engine();
    queue_.clear();
    started_ = false;
    instance() = nullptr;
  }

  /**
   * @brief Queue text to be spoken in the default language.
   */
  void say(const String& text) { say(text, default_language_); }

  /**
   * @brief Queue text to be spoken in the given language, e.g. "it-IT".
   */
  void say(const String& text, const String& language) {
    if (started_ && text.length() > 0) {
      queue_.emplace_back(language, text);
    }
  }

  /**
   * @brief True while there is speech playing or waiting to be played.
   */
  bool is_speaking() const { return speaking_ || !queue_.empty(); }

  void set_volume(float volume) { volume_ = constrain(volume, 0.0f, 1.0f); }

 protected:
  static constexpr uint32_t kPollIntervalMs = 50;
  // Give up waiting for an utterance that produces no audio for this long.
  static constexpr uint32_t kSilenceTimeoutMs = 10000;

  static PicoTTS*& instance() {
    static PicoTTS* instance = nullptr;
    return instance;
  }

  // Called from the PicoTTS task with a buffer of synthesized samples.
  static void on_samples(int16_t* samples, unsigned count) {
    PicoTTS* self = instance();
    if (self == nullptr) {
      return;
    }
    self->got_samples_ = true;
    self->last_activity_ms_ = millis();
    const int32_t gain = static_cast<int32_t>(self->volume_ * 256);
    for (unsigned i = 0; i < count; i++) {
      samples[i] = static_cast<int16_t>((samples[i] * gain) >> 8);
    }
    self->amplifier_->write(samples, count);
  }

  // Called from the PicoTTS task when the engine has nothing left to do.
  // The engine also reports idle before it has seen any text, so only take
  // it as the end of the utterance once audio has been produced.
  static void on_idle() {
    PicoTTS* self = instance();
    if (self != nullptr && self->got_samples_) {
      self->speaking_ = false;
    }
  }

  // Called from the PicoTTS task right before it exits on an error.
  static void on_error() {
    PicoTTS* self = instance();
    if (self != nullptr) {
      self->engine_failed_ = true;
    }
  }

  bool start_engine(const String& language) {
    if (!picotts_set_language(language.c_str())) {
      return false;
    }
    if (!picotts_init(task_priority_, &PicoTTS::on_samples, core_)) {
      ESP_LOGE(__FILENAME__, "Failed to start PicoTTS");
      return false;
    }
    engine_language_ = language;
    engine_running_ = true;
    engine_failed_ = false;
    return true;
  }

  void stop_engine() {
    if (engine_running_) {
      picotts_shutdown();
      engine_running_ = false;
    }
    speaking_ = false;
  }

  // Runs in the main event loop: starts the next utterance once the previous
  // one has finished, restarting the engine if the language changes.
  void process_queue() {
    if (engine_failed_) {
      ESP_LOGE(__FILENAME__, "PicoTTS stopped on an error, restarting");
      stop_engine();
    }
    if (speaking_ && millis() - last_activity_ms_ > kSilenceTimeoutMs) {
      ESP_LOGW(__FILENAME__, "No audio from PicoTTS, skipping utterance");
      speaking_ = false;
    }
    if (speaking_ || queue_.empty()) {
      return;
    }
    auto [language, text] = std::move(queue_.front());
    queue_.pop_front();
    if (!engine_running_ || language != engine_language_) {
      stop_engine();
      if (!start_engine(language)) {
        return;
      }
    }
    got_samples_ = false;
    last_activity_ms_ = millis();
    speaking_ = true;
    // The terminating null tells the engine to start speaking.
    picotts_add(text.c_str(), text.length() + 1);
  }

  std::shared_ptr<MAX98357A> amplifier_;
  const String default_language_;
  const unsigned task_priority_;
  const int core_;
  String engine_language_;
  std::deque<std::pair<String, String>> queue_;
  reactesp::RepeatEvent* poll_event_ = nullptr;
  volatile float volume_ = 0.5f;
  volatile bool speaking_ = false;
  volatile bool got_samples_ = false;
  volatile bool engine_failed_ = false;
  volatile uint32_t last_activity_ms_ = 0;
  bool engine_running_ = false;
  bool started_ = false;
};

}  // namespace sensesp

#endif  // SENSESP_SYSTEM_PICO_TTS_H_
