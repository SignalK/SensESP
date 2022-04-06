#ifndef _led_blinker_H_
#define _led_blinker_H_

#include <ReactESP.h>

#include "sensesp/net/ws_client.h"
#include "startable.h"

namespace sensesp {

#define PATTERN_END (-1)

/**
 * @brief A base class for LED blinker classes.
 */
class BaseBlinker : public Startable {
 public:
  BaseBlinker(int pin);
  void set_state(bool state);
  void flip_state();
  void blip(int duration = 20);
  void set_enabled(bool state);
  /**
   * Tick is called whenever the blinker is enabled or when it's time to
   * change the LED state.
   */
  virtual void tick() = 0;
  void start() override;

 protected:
  int pin_;
  bool enabled_ = true;
  bool state_ = false;
  int update_counter_ = 0;
  Reaction* reaction_ = NULL;
};

/**
 * @brief A base class for periodic blinkers.
 */
class PeriodicBlinker : public BaseBlinker {
 public:
  PeriodicBlinker(int pin, unsigned int period);
  void set_period(unsigned int period) { this->period_ = period; }

 protected:
  unsigned int period_;
};

/**
 * @brief  An LED blinker class that blinks the LED 50% off, 50% on,
 * at a given period.
 */
class EvenBlinker : public PeriodicBlinker {
 public:
  EvenBlinker(int pin, unsigned int period);
  void tick() override final;
};

/**
 * @brief A periodic blinker that defines both the on-ratio
 * and the period length.
 */
class RatioBlinker : public PeriodicBlinker {
 public:
  RatioBlinker(int pin, unsigned int period, float ratio = 0.);
  void tick() override final;
  void set_ratio(unsigned int ratio) { this->ratio_ = ratio; }

 protected:
  float ratio_;
};

/**
 * @brief A blinker that blinks the LED according to a defined
 * repeating pattern.
 */
class PatternBlinker : public BaseBlinker {
 public:
  PatternBlinker(int pin, int pattern[]);
  void tick() override final;
  void set_pattern(int pattern[]);
  void restart();

 protected:
  int* pattern_;
  unsigned int pattern_ptr_ = 0;
};

}  // namespace sensesp

#endif
