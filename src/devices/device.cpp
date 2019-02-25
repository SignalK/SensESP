#include "device.h"

std::set<Device*> Device::devices;

Device::Device(String id, String schema) : Configurable{id, schema} {
  devices.insert(this);
}
