#include "rack.hpp"
#include <memory>
#include <functional>

using namespace rack;

class TextWidget : public TransparentWidget {
public:
  TextWidget(Vec bounds, std::function<std::string()>);

  template<typename  T>
  static T* create(const Rect& dim,
		   std::function<std::string()> const& getContents,
		   std::string fontpath,
		   int fontSize = 12,
		   NVGcolor color = nvgRGBA(0, 0, 0, 255)) {
    auto t = new T(dim.size, getContents);
    t->box.pos = dim.pos;
    t->setFont(fontpath);
    t->setFontSize(fontSize);
    t->setColor(color);
    return t;
  }

  template<typename T>
  static std::function<std::string()> text(T const& t) {
    return [&t](){return std::string(t);};
  }
  
  void draw(NVGcontext *vg) override;
  void setColor(NVGcolor const&);
  void setFontSize(int);
  void setFont(std::string fontpath);
  
protected:
  virtual void applyStyle(NVGcontext *vg);
  virtual void applyScissor(NVGcontext *vg);
  virtual Vec calculateTextOffset(NVGcontext *vg, std::string const&);
  virtual void displayText(NVGcontext *vg);
  virtual Vec scrollText(Vec offset);

  std::function<std::string ()> const getContents_;
  NVGcolor textColor_ = nvgRGBA(0, 0, 0, 255);
  int fontSize_ = 12;
  std::shared_ptr<Font> font_;
  Vec bounds_;
  float time_;
  float scrollSpeed_ = 2.0f;
};

class VerticalTextWidget : public TextWidget {
public:
  using TextWidget::TextWidget;
protected:
  virtual void applyStyle(NVGcontext* vg) override;
  virtual void applyScissor(NVGcontext *vg) override;
};
