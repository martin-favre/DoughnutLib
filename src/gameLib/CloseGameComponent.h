#pragma once
#include "Component.h"
#include "Engine.h"
#include "InputManager.h"
#include "Serializer.h"

class GameObject;
struct CloseGameComponent : public Component {
  CloseGameComponent(GameObject& owner) : Component(owner) {}
  inline void update() final override {
    while (InputManager::hasKeyEvents(mInputHandle)) {
      KeyEvent keyEvent = InputManager::dequeueKeyEvent(mInputHandle);
      if (!keyEvent.mKeyDown) continue;
      switch (keyEvent.mKey) {
        case INPUT_KEY_QUIT:
        case SDL_SCANCODE_Q:
          Engine::stop();
          break;
        default:
          break;
      }
    }
  }

  SerializedObj serialize() const override { 
    SerializedObj out = createSerializedObj<CloseGameComponent>();
    out["parent"] = Component::serialize();
    return out;
  }

  CloseGameComponent(GameObject& owner, const SerializedObj& serObj)
      : Component(owner, serObj["parent"]) {}

  static std::string getTypeString() { return "CloseGameComponent"; }

 private:
  QueueHandle mInputHandle;
};