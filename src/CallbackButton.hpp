#ifndef BARGKASS_CALLBACKBUTTON_HPP__
#define BARGKASS_CALLBACKBUTTON_HPP__

#include "rack.hpp"
#include <functional>

class CallbackButton : public Button {
public:
  CallbackButton(std::string label, std::function<void()> const& callback) : f_(callback) {
    this->text = label;
  }

  void onAction(EventAction &e) override {
    f_();
  }

  static CallbackButton* create(Vec const& pos, Vec const& size, std::string label, std::function<void()> const& callback) {
    CallbackButton* cb = new CallbackButton(label, callback);
    cb->box.pos = pos;
    cb->box.size = size;
    return cb;
  }
  
private:
  const std::function<void()> f_;
};

#endif
