#ifndef SENSESP_SRC_SENSESP_SYSTEM_SYSTEM_STATUS_LED_H_
#define SENSESP_SRC_SENSESP_SYSTEM_SYSTEM_STATUS_LED_H_

#include <memory>
#include <vector>
#include <vector>

#include "lambda_consumer.h"
#include "led_blinker.h"
#include "pwm_output.h"
#include "sensesp/controllers/system_status_controller.h"

#include "esp_arduino_version.h"

#if ESP_ARDUINO_VERSION_MAJOR < 3
#define rgbLedWrite neopixelWrite
#endif


namespace sensesp {

// LED patterns. It isn't strictly speaking necessary to have these public,
// but it makes it easier to test the LED functionality.
extern LEDPattern no_ap_pattern;
extern LEDPattern wifi_disconnected_pattern;
extern LEDPattern wifi_connected_pattern;
extern LEDPattern ws_connecting_pattern;
extern LEDPattern ws_connected_pattern;
extern LEDPattern ws_disconnected_pattern;
extern LEDPattern ws_authorizing_pattern;

/**
 * @brief Consumes the networking and websocket states and delta counts
 * and updates the device LED accordingly.
 *
 * This is an abstract class that can be implemented for different types of
 * LEDs.
 */
class BaseSystemStatusLed {
 protected:
  CRGB leds_[1] = {CRGB::Black};

  std::unique_ptr<LEDBlinker> blinker_;

  virtual void show() = 0;

  virtual void set_brightness(uint8_t brightness) = 0;

  virtual void set_wifi_no_ap();
  virtual void set_wifi_disconnected();
  virtual void set_ws_disconnected();
  virtual void set_ws_authorizing();
  virtual void set_ws_connecting();
  virtual void set_ws_connected();

 public:
  BaseSystemStatusLed() {
    blinker_ = std::unique_ptr<LEDBlinker>(
        new LEDBlinker(leds_[0], no_ap_pattern, [this]() { this->show(); }));

    event_loop()->onRepeat(5, [this]() {
      this->blinker_->tick();
      this->show();
    });
  }

  LambdaConsumer<SystemStatus> system_status_consumer_{
      [this](SystemStatus status) {
        switch (status) {
          case SystemStatus::kWifiNoAP:
            this->set_wifi_no_ap();
            break;
          case SystemStatus::kWifiDisconnected:
            this->set_wifi_disconnected();
            break;
          case SystemStatus::kSKWSDisconnected:
            this->set_ws_disconnected();
            break;
          case SystemStatus::kSKWSConnecting:
            this->set_ws_connecting();
            break;
          case SystemStatus::kSKWSAuthorizing:
            this->set_ws_authorizing();
            break;
          case SystemStatus::kSKWSConnected:
            this->set_ws_connected();
            break;
          default:
            break;
        }
      }};

  ValueConsumer<SystemStatus>& get_system_status_consumer() {
    return system_status_consumer_;
  }

  ValueConsumer<int>& get_delta_tx_count_consumer();
};

// TODO: Instead of setting PWM output manually, it would be great to
// instead implement a single-LED FastLED controller class. That would
// allow supporting features like dithering etc.

/**
 * @brief Monochromatic system status LED.
 *
 */
class SystemStatusLed : public BaseSystemStatusLed {
 public:
  SystemStatusLed(uint8_t pin, uint8_t brightness = 255)
      : pwm_output_{PWMOutput(pin, 2000, 8)},
        BaseSystemStatusLed(),
        brightness_{brightness} {}

 protected:
  PWMOutput pwm_output_;
  uint8_t brightness_;
  void show() override {
    // Convert the RGB color to a single brightness value using the
    // perceptual luminance formula.
    float value = ((0.2126 * leds_[0].r +  // Red contribution
                    0.7152 * leds_[0].g +  // Green contribution
                    0.0722 * leds_[0].b    // Blue contribution
                    ) *
                   brightness_ / (255.0 * 256.0));
    pwm_output_.set(value);
  }

  void set_brightness(uint8_t brightness) override { brightness_ = brightness; }
};

// Direct use of FastLED breaks when the WiFi client is enabled. Thus,
// use the Arduino ESP32 Core native rgbLedWrite function instead. That
// seems to work with WiFi as well.

class RGBSystemStatusLed : public BaseSystemStatusLed {
 public:
  RGBSystemStatusLed(uint8_t pin, uint8_t brightness = 40)
      : BaseSystemStatusLed(), pin_{pin}, brightness_{brightness} {}

 protected:
  uint8_t pin_;
  uint8_t brightness_;
  void show() override {
    rgbLedWrite(pin_, brightness_ * leds_[0].r / 255,
                brightness_ * leds_[0].g / 255, brightness_ * leds_[0].b / 255);
  }

  void set_brightness(uint8_t brightness) override { brightness_ = brightness; }
};

}  // namespace sensesp

#endif
