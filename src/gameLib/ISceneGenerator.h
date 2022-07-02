#pragma once

#include "Component.h"
#include "GameObject.h"
#include <memory>

class ISceneGenerator {
public:
  virtual ~ISceneGenerator() = default;
  virtual std::vector<std::unique_ptr<GameObject>>
  generateGameObjects() const = 0;
  std::vector<std::unique_ptr<GameObject>> internalGenerateGameObjects() const;
};