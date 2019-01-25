#ifndef _observable_H_
#define _observable_H_

#include <vector>
#include <functional>
#include <forward_list>


///////////////////
// Observables are classes which allow observers
// to attach callbacks to themselves. The callbacks
// will be called when the observable needs to
// notify the observers about some state change.

class Observable {
    std::forward_list < std::function<void()> > observers;
  public:
    void notify();
    void attach(std::function<void()> observer);
};

#endif
