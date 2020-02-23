#include "signalk_listener.h"

std::vector<SKListener*> SKListener::listeners;

SKListener::SKListener(String sk_path, int period) : sk_path{sk_path} {
  listeners.push_back(this);
  this->period = period;
}