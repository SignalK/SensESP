#include "device.h"

std::set<Device*> Device::devices;

Device::Device(String id, String schema) : Configurable{id, schema} {
  devices.insert(this);
}



NumericDevice::NumericDevice(String id, String schema, uint8_t valueIndex) :
   Device(id, schema), NumericProducer(valueIndex) {

};


IntegerDevice::IntegerDevice(String id, String schema, uint8_t valueIndex) :
   Device(id, schema), IntegerProducer(valueIndex) {

};


StringDevice::StringDevice(String id, String schema, uint8_t valueIndex) :
   Device(id, schema), StringProducer(valueIndex) {

};
