#include "signalk_listener.h"

std::vector<SKListener*> SKListener::listeners;

SKListener::SKListener(String sk_path, int listen_Delay) : sk_path{sk_path} {
  listeners.push_back(this);
  this->listen_delay = listen_Delay;
}