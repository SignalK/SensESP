#include "kelvintocelsius.h"

KelvinToCelsius::KelvinToCelsius(String sk_path, String id, String schema) :
   OneToOneNumericTransform(sk_path, id, schema) {
}


void KelvinToCelsius::set_input(float K, uint8_t inputChannel) {
    output = K - 273.15;
    notify();
}
