#pragma once
#include "Component.h"
#include "Serializer.h"
#include "Timer.h"
#include <memory>
class TextComponent;

class FpsCounter : public Component {
 public:
  FpsCounter(GameObject& gObj);
  FpsCounter(GameObject& gObj, const SerializedObj& serObj);
  SerializedObj serialize() const override;
  static std::string getTypeString();
  void setup() override;
  void update() override;
  void render() override;
 private:
  Timer mUpdateTimer;
  Timer mRenderTimer;
  int64_t mMsPerRender{0};
  std::weak_ptr<TextComponent> mText;
};