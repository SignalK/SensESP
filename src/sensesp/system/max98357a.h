#ifndef SENSESP_SYSTEM_MAX98357A_H_
#define SENSESP_SYSTEM_MAX98357A_H_

#include <Arduino.h>
#include <ESP_I2S.h>
#include <cmath>

namespace sensesp {

/**
 * @brief Driver for the MAX98357A I2S mono class D amplifier.
 *
 * The MAX98357A has no control interface: it plays whatever arrives on its
 * I2S input. This class wraps the Arduino-ESP32 `ESP_I2S` library (Arduino
 * core 3.x and later) and outputs 16-bit mono PCM.
 *
 * Wiring: LRC is the word select (WS) line, BCLK the bit clock and DIN the
 * amplifier's data input, i.e. the data *output* of the ESP32.
 */
class MAX98357A {
 public:
  /**
   * @param bclk_pin GPIO connected to the amplifier BCLK pin.
   * @param lrc_pin GPIO connected to the amplifier LRC pin.
   * @param din_pin GPIO connected to the amplifier DIN pin.
   * @param sample_rate Sample rate in Hz.
   */
  MAX98357A(int bclk_pin, int lrc_pin, int din_pin,
            uint32_t sample_rate = 16000)
      : bclk_pin_{bclk_pin},
        lrc_pin_{lrc_pin},
        din_pin_{din_pin},
        sample_rate_{sample_rate} {}

  /**
   * @brief Start the I2S peripheral. Returns false on failure.
   */
  bool begin() {
    i2s_.setPins(bclk_pin_, lrc_pin_, din_pin_);
    // Send the mono samples in both the left and the right slot, so the
    // amplifier gets the full signal whichever channel SD_MODE selects.
    started_ = i2s_.begin(I2S_MODE_STD, sample_rate_, I2S_DATA_BIT_WIDTH_16BIT,
                          I2S_SLOT_MODE_MONO, I2S_STD_SLOT_BOTH);
    return started_;
  }

  void end() {
    i2s_.end();
    started_ = false;
  }

  /**
   * @brief Write signed 16-bit mono samples. Blocks until they are queued.
   * @return Number of samples written.
   */
  size_t write(const int16_t* samples, size_t count) {
    if (!started_) {
      return 0;
    }
    return i2s_.write(reinterpret_cast<const uint8_t*>(samples),
                      count * sizeof(int16_t)) /
           sizeof(int16_t);
  }

  /**
   * @brief Play a WAV file held in memory. Blocks until playback finishes.
   */
  void play_wav(const uint8_t* data, size_t len) {
    if (started_) {
      i2s_.playWAV(const_cast<uint8_t*>(data), len);
    }
  }

  /**
   * @brief Play a sine tone. Blocks for the duration of the tone.
   * @param frequency Tone frequency in Hz.
   * @param duration_ms Tone length in milliseconds.
   * @param volume Amplitude between 0.0 and 1.0.
   */
  void play_tone(float frequency, uint32_t duration_ms, float volume = 0.2f) {
    constexpr size_t kChunk = 256;
    int16_t buf[kChunk];
    const float amplitude = constrain(volume, 0.0f, 1.0f) * INT16_MAX;
    const float step = 2.0f * static_cast<float>(M_PI) * frequency /
                       static_cast<float>(sample_rate_);
    size_t remaining =
        static_cast<uint64_t>(sample_rate_) * duration_ms / 1000;
    float phase = 0;
    while (remaining > 0) {
      const size_t n = remaining < kChunk ? remaining : kChunk;
      for (size_t i = 0; i < n; i++) {
        buf[i] = static_cast<int16_t>(amplitude * sinf(phase));
        phase += step;
        if (phase > 2.0f * static_cast<float>(M_PI)) {
          phase -= 2.0f * static_cast<float>(M_PI);
        }
      }
      if (write(buf, n) != n) {
        ESP_LOGE(__FILENAME__, "I2S write failed");
        return;
      }
      remaining -= n;
    }
  }

  uint32_t get_sample_rate() const { return sample_rate_; }

 protected:
  const int bclk_pin_;
  const int lrc_pin_;
  const int din_pin_;
  const uint32_t sample_rate_;
  I2SClass i2s_;
  bool started_ = false;
};

}  // namespace sensesp

#endif  // SENSESP_SYSTEM_MAX98357A_H_
