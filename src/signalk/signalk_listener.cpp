#include "signalk_listener.h"

std::vector<SKListener*> SKListener::listeners;

SKListener::SKListener(String sk_path, int listen_delay) : sk_path{sk_path}, listen_delay{listen_delay} {
  listeners.push_back(this);
}