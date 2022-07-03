
#include "Engine.h"
#include "CloseGameComponent.h"
#include "Component.h"
#include "GameObject.h"
#include "GraphicsManager.h"
#include <gtest/gtest.h>
#include <memory>

TEST(EngineTest, TestInitialize) {
  // will assert internally on failure.
  Engine::initialize();
  Engine::teardown();
}

class AutoCloseComponent : public Component {
public:
  AutoCloseComponent(GameObject &owner) : Component(owner) {}
  void update() override { Engine::stop(); }
};

TEST(EngineTest, TestInitializeMakingGameobject) {
  // will assert internally on failure.
  Engine::initialize();
  auto gObj = std::make_unique<GameObject>();
  gObj->addComponent<AutoCloseComponent>();
  Engine::addGameObject(std::move(gObj));
  Engine::start();
  Engine::teardown();
}