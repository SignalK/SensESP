#include "device.h"

std::set<Device*> Device::devices;

Device::Device(String id, String schema) : Configurable{id, schema}, WantsEnable(0) {
  devices.insert(this);
}



NumericDevice::NumericDevice(String id, String schema) :
   Device(id, schema), NumericProducer() {

};


IntegerDevice::IntegerDevice(String id, String schema) :
   Device(id, schema), IntegerProducer() {

};


StringDevice::StringDevice(String id, String schema) :
   Device(id, schema), StringProducer() {

};
