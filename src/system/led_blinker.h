#ifndef _led_blinker_H_
#define _led_blinker_H_

#include <ReactESP.h>

#include "enable.h"
#include "net/networking.h"
#include "net/ws_client.h"

#define PATTERN_END (-1)

/**
 * A base class for LED blinker classes.
 */
class BaseBlinker : public Enable {
 public:
  BaseBlinker(int pin);
  void set_state(bool state);
  void flip_state();
  void set_enabled(bool state);
  /**
   * Tick is called whenever the blinker is enabled or when it's time to
   * change the LED state.
   */
  virtual void tick() = 0;
  void enable() override;

 protected:
  int pin;
  bool enabled = true;
  bool state = false;
  Reaction* reaction = NULL;
};

/**
 * A base class for periodic blinkers.
 */
class PeriodicBlinker : public BaseBlinker {
 public:
  PeriodicBlinker(int pin, unsigned int period);
  void set_period(unsigned int period) { this->period = period; }

 protected:
  unsigned int period;
};

/**
 * EvenBlinker is a LED blinker class that blinks the LED 50% off, 50% on,
 * at a given period.
 */
class EvenBlinker : public PeriodicBlinker {
 public:
  EvenBlinker(int pin, unsigned int period);
  void tick() override final;
};

/**
 * RatioBlinker is a periodic blinker that defines both the on-ratio
 * and the period length.
 */
class RatioBlinker : public PeriodicBlinker {
 public:
  RatioBlinker(int pin, unsigned int period, float ratio = 0.);
  void tick() override final;
  void set_ratio(unsigned int ratio) { this->ratio = ratio; }

 protected:
  float ratio;
};

/**
 * PatternBlinker is a blinker that blinks the LED according to a defined
 * repeating pattern.
 */
class PatternBlinker : public BaseBlinker {
 public:
  PatternBlinker(int pin, int pattern[]);
  void tick() override final;
  void set_pattern(int pattern[]);
  void restart();

 protected:
  int* pattern;
  unsigned int pattern_ptr = 0;
};

#endif
