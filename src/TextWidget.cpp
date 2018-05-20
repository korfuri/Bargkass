#include "TextWidget.hpp"
#include "window.hpp"

TextWidget::TextWidget(Vec bounds, std::function<std::string()> getContents) : getContents_(getContents) {
  bounds_ = bounds;
  time_ = glfwGetTime();
}

void TextWidget::setColor(NVGcolor const& color) {
  textColor_ = color;
}

void TextWidget::setFontSize(int size) {
  fontSize_ = size;
}

void TextWidget::setFont(std::string fontpath) {
  font_ = Font::load(fontpath);
}

void TextWidget::applyStyle(NVGcontext *vg) {
  nvgFontSize(vg, fontSize_);
  nvgFontFaceId(vg, font_->handle);
  nvgFillColor(vg, textColor_);
}

void TextWidget::applyScissor(NVGcontext *vg) {
  nvgIntersectScissor(vg, 0.f, 0.0f, bounds_.x, bounds_.y);
}

void TextWidget::draw(NVGcontext *vg) {
  time_ += 0.01f;

  // We scissor before setting style, as style includes rotation which
  // is done by rotating the context.
  applyScissor(vg);
  applyStyle(vg);

  // nvgBeginPath(vg);
  // nvgRect(vg, -10000.0f, -10000.0f, 20000.0f, 20000.0f);
  // nvgFillColor(vg, nvgRGBA(0, 0, 255, 255));
  // nvgFill(vg);

  displayText(vg);
}

void TextWidget::displayText(NVGcontext *vg) {
  nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
  std::string contents = getContents_();
  Vec offset = calculateTextOffset(vg, contents);
  nvgText(vg, offset.x, offset.y, contents.c_str(), nullptr);
}

Vec TextWidget::scrollText(Vec offset) {
  offset.x *= (sin((glfwGetTime() - time_) * scrollSpeed_) + 1.0f) / 2.0f;
  return offset;
}

Vec TextWidget::calculateTextOffset(NVGcontext *vg, std::string const& contents) {
  float bounds[4];
  nvgTextBounds(vg, 0.0f, 0.0f, contents.c_str(), nullptr, bounds);
  float textWidth = bounds[2] - bounds[0]; // xmax - xmin
  if (textWidth <= bounds_.x) {
    return Vec(0.0f, 0.0f);
  } else {
    return scrollText(Vec(-(textWidth - bounds_.x), 0.0f));
  }
}

void VerticalTextWidget::applyScissor(NVGcontext *vg) {
  nvgScissor(vg, 0.f, -bounds_.x, bounds_.y, bounds_.x);
}

void VerticalTextWidget::applyStyle(NVGcontext *vg) {
  nvgRotate(vg, -M_PI/2.0f);
  TextWidget::applyStyle(vg);
}
