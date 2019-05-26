#ifndef _kelvinToFahrenheit_H
#define _kelvinToFahrenheit_H

#include "transforms/transform.h"

class KelvinToFahrenheit : public OneToOneNumericTransform {

    public:
       KelvinToFahrenheit(String sk_path="", String id="", String schema="");

        virtual void set_input(float K, uint8_t inputChannel = 0) override;
};


#endif
