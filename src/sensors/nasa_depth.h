#ifndef _NASA_DEPTH_H_
#define _NASA_DEPTH_H_

#include "sensesp.h"
#include "sensor.h"
#include "Wire.h"

class NasaDepth : public NumericSensor {
  public:
    NasaDepth(int8_t sda_pin, int8_t scl_pin, uint read_delay, String config_path = "");
    void enable() override final;
    void set_led_pin(int8_t pin);

  private:
    bool data_available = false;
    char localCopy[11];
    float DecodeData ();

    uint read_delay;
    virtual JsonObject& get_configuration(JsonBuffer& buf) override;
    virtual bool set_configuration(const JsonObject& config) override;
    virtual String get_config_schema() override;

    char i2C_predata[5] = {0xce, 0x80, 0xe0, 0xf8, 0x70};
    char depth_mask[6] = {0x01, 0, 0, 0, 0, 0};
    char decpoint_mask[6] = {0, 0, 0, 0x80, 0x0, 0x0};
    char meters_mask[6] = {0, 0, 0, 0x40, 0x0, 0x0};
    char digit3_mask[6] = {0, 0xbf, 0, 0, 0, 0};
    char digit3[10][6] = {// from https://en.wikipedia.org/wiki/Seven-segment_display
                   {0, 0xbb, 0, 0, 0, 0}, // zero, a, b, c, d, e, f, / g
                   {0, 0x11, 0, 0, 0, 0}, // one / a, b, c, / d, / e, / f, / g
                   {0, 0x9e, 0, 0, 0, 0}, // two a, b, / c, d, e, / f, g
                   {0, 0x97, 0, 0, 0, 0}, // three a, b, c, d, / e, / f, g
                   {0, 0x35, 0, 0, 0, 0}, // four / a, b, c, / d, / e, f, g
                   {0, 0xa7, 0, 0, 0, 0}, // five a, / b, c, d, / e, f, g
                   {0, 0xaf, 0, 0, 0, 0}, // six a, / b, c, d, e, f, g
                   {0, 0x91, 0, 0, 0, 0}, // seven a, b, c, / d, / e, / f, / g
                   {0, 0xbf, 0, 0, 0, 0}, // eight a, b, c, d, e, f, g
                   {0, 0xb7, 0, 0, 0, 0}, // nine a, b, c, d, / e, f, g
                 };
                 
    char digit2_mask[6] = {0xfe, 0, 0, 0, 0, 0};
    char digit2[10][6] = {// from https://en.wikipedia.org/wiki/Seven-segment_display
                   {0xee, 0, 0, 0, 0, 0}, // zero, a, b, c, d, e, f, / g
                   {0x44, 0, 0, 0, 0, 0}, // one / a, b, c, / d, / e, / f, / g
                   {0xb6, 0, 0, 0, 0, 0}, // two a, b, / c, d, e, / f, g
                   {0xd6, 0, 0, 0, 0, 0}, // three a, b, c, d, / e, / f, g
                   {0x5c, 0, 0, 0, 0, 0}, // four / a, b, c, / d, / e, f, g
                   {0xda, 0, 0, 0, 0, 0}, // five a, / b, c, d, / e, f, g
                   {0xfa, 0, 0, 0, 0, 0}, // six a, / b, c, d, e, f, g
                   {0x46, 0, 0, 0, 0, 0}, // seven a, b, c, / d, / e, / f, / g
                   {0xfe, 0, 0, 0, 0, 0}, // eight a, b, c, d, e, f, g
                   {0xde, 0, 0, 0, 0, 0}, // nine a, b, c, d, / e, f, g
                 };
                 

    char digit1_mask[6] = {0, 0, 0, 0, 0x2f, 0xc0};
    char digit1[10][6] = {// from https://en.wikipedia.org/wiki/Seven-segment_display
                   {0, 0, 0, 0, 0x2e, 0xc0}, // zero, a, b, c, d, e, f, / g
                   {0, 0, 0, 0, 0x04, 0x40}, // one / a, b, c, / d, / e, / f, / g
                   {0, 0, 0, 0, 0x27, 0x80}, // two a, b, / c, d, e, / f, g
                   {0, 0, 0, 0, 0x25, 0xC0}, // three a, b, c, d, / e, / f, g
                   {0, 0, 0, 0, 0x0d, 0x40}, // four / a, b, c, / d, / e, f, g
                   {0, 0, 0, 0, 0x29, 0xC0}, // five a, / b, c, d, / e, f, g
                   {0, 0, 0, 0, 0x2b, 0xC0}, // six a, / b, c, d, e, f, g
                   {0, 0, 0, 0, 0x24, 0x40}, // seven a, b, c, / d, / e, / f, / g
                   {0, 0, 0, 0, 0x2f, 0xc0}, // eight a, b, c, d, e, f, g
                   {0, 0, 0, 0, 0x2d, 0xc0}, // nine a, b, c, d, / e, f, g
                 };


};

#endif
