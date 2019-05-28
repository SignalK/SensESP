#include "device.h"

std::set<Device*> Device::devices;

Device::Device(String config_path) : Configurable{config_path}, Enable(10) {
  devices.insert(this);
}



NumericDevice::NumericDevice(String config_path) :
   Device(config_path), NumericProducer() {

};


IntegerDevice::IntegerDevice(String config_path) :
   Device(config_path), IntegerProducer() {

};


StringDevice::StringDevice(String config_path) :
   Device(config_path), StringProducer() {

};
