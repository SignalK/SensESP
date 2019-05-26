#include "signalksource.h"

std::vector<SignalKSource*> SignalKSource::sources;

SignalKSource::SignalKSource(String sk_path) : sk_path{sk_path} {
  sources.push_back(this);
}
