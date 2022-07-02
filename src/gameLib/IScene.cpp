#include "ISceneGenerator.h"
#include "CloseGameComponent.h"

std::vector<std::unique_ptr<GameObject>>
ISceneGenerator::internalGenerateGameObjects() const {
  auto gameobjects = generateGameObjects();
  auto closeGameObj = std::make_unique<GameObject>();
  closeGameObj->addComponent<CloseGameComponent>();
  gameobjects.emplace_back(std::move(closeGameObj));
  return gameobjects;
}
