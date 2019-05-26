#ifndef _kelvinToCelsius_H
#define _kelvinToCelsius_H

#include "transforms/transform.h"

class KelvinToCelsius : public OneToOneNumericTransform {

    public:
       KelvinToCelsius(String sk_path="", String id="", String schema="");

        virtual void set_input(float K, uint8_t inputChannel = 0) override;
};


#endif
