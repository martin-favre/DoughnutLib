#include "MainGame.h"

#include "Component.h"
#include "Engine.h"
#include "FpsCounter.h"
#include "GameObject.h"
#include "Paths.h"
#include "Vector2DInt.h"
#include "Serializer.h"
#include "Component.h"
#include <memory>
#include "CloseGameComponent.h"

void MainGame::run() {
  Engine::initialize();
  auto gObj = std::make_unique<GameObject>();
  gObj->addComponent<CloseGameComponent>();
  Engine::addGameObject(std::move(gObj));
  Engine::start();
  Engine::teardown();
}