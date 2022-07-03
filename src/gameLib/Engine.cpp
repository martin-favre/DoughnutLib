#include "Engine.h"

#include <algorithm>
#include <memory>
#include <string>
#include <thread>

#include "CloseGameComponent.h"
#include "GameObject.h"
#include "GraphicsManager.h"
#include "Helpers.h"
#include "ISceneGenerator.h"
#include "InputManager.h"
#include "Logging.h"
#include "Timer.h"
#include "Uuid.h"

std::map<std::string, std::unique_ptr<ISceneGenerator>> Engine::mScenes;
std::optional<std::string> Engine::mNameOfSceneToLoad;
bool Engine::mAboutToLoadScene = false;
bool Engine::mRunning = false;
bool Engine::mInitialized = false;
std::vector<std::shared_ptr<GameObject>> Engine::mGameobjects;
std::vector<std::shared_ptr<GameObject>> Engine::mGameobjectsToAdd;
std::set<Uuid> Engine::mGameobjectsToRemove;

std::unique_ptr<SerializedObj> Engine::mSavedState;
std::unique_ptr<SerializedObj> Engine::mLoadState;

/* Public routines */
void Engine::initialize() {
  ASSERT(!Engine::mInitialized, "You can't initialize engine twice!");
  GraphicsManager::initialize();
  InputManager::initialize();
  Logging::initialize();
  Engine::mInitialized = true;
  LOGL("Finished initialize Engine", Logging::info);
}

void Engine::teardown() {
  ASSERT(!mRunning, "You need to stop the engine first");
  clearAllGameObjects();
  GraphicsManager::teardown();
  LOGL("Finished teardown Engine", Logging::info);
  Logging::teardown();
  Engine::mInitialized = false;
}

void Engine::start() {
  ASSERT(Engine::mInitialized, "You need to initialize engine first!");
  Engine::mRunning = true;
  LOGL("Starting Engine", Logging::info);
  Engine::mainLoop();
}

void Engine::stop() { Engine::mRunning = false; }

void Engine::removeGameObject(const Uuid &gObjIdentifier) {
  if (!Engine::mRunning)
    return;
  Engine::mGameobjectsToRemove.insert(gObjIdentifier);
}

void Engine::registerScene(const std::string &name,
                           std::unique_ptr<ISceneGenerator> scenecreator) {
  Engine::mScenes[name] = std::move(scenecreator);
}

void Engine::addGameObject(std::unique_ptr<GameObject> gameobject) {
  GameObject *out = gameobject.get();
  Engine::mGameobjectsToAdd.emplace_back(std::move(gameobject));
  LOG("Added gameobject" << out);
  if (!mRunning)
    putGameObjectsIntoWorld();
}

void Engine::loadScene(const std::string &name) {
  ASSERT(mScenes.count(name), "No scene named " + name);
  mNameOfSceneToLoad->assign(name);
  mAboutToLoadScene = true;
}

void Engine::mainLoop() {
  while (Engine::mRunning) {
    if (mAboutToLoadScene) {
      mAboutToLoadScene = false;
      replaceScene();
    }
    InputManager::readInputs();
    Engine::putGameObjectsIntoWorld();
    Engine::removeGameObjectFromWorld();
    Engine::updateGameObjects();
    GraphicsManager::prepareRendering();
    Engine::renderGameObjects();
  }
}

void Engine::replaceScene() {
  /*
          Removes all gameobjects
          calls the sceneloader function
          this function will add the
          intial gameobjects to the engine.
  */
  clearAllGameObjects();
  auto gameobjects =
      mScenes[mNameOfSceneToLoad.value()]->internalGenerateGameObjects();
  for (auto &gameobject : gameobjects) {
    addGameObject(std::move(gameobject));
  }
}

void Engine::updateGameObjects() {
  for (auto &go : mGameobjects) {
    go->updateComponents();
  }
}

bool debug_render_colliders = true;

void Engine::renderGameObjects() {
  for (auto &go : mGameobjects) {
    go->render();
  }

  GraphicsManager::executeRendering();
}

void Engine::clearAllGameObjects() {
  mGameobjects.clear();
  mGameobjectsToAdd.clear();
  mGameobjectsToRemove.clear();
}

void Engine::putGameObjectsIntoWorld() {
  std::vector<std::shared_ptr<GameObject>> addedItems;
  for (auto newGobj : mGameobjectsToAdd) {
    addedItems.emplace_back(newGobj);
    size_t indx = 0;
    for (; indx != mGameobjects.size(); ++indx) {
      if (newGobj->getRenderDepth() <= mGameobjects[indx]->getRenderDepth()) {
        mGameobjects.insert(mGameobjects.begin() + indx, newGobj);
        break;
      }
    }
    if (indx == mGameobjects.size()) {
      mGameobjects.push_back(std::move(newGobj));
    }
  }
  mGameobjectsToAdd.clear();

  Engine::runSetups(addedItems);
}
void Engine::removeGameObjectFromWorld() {
  std::vector<std::shared_ptr<GameObject>> removedObjects;
  for (auto identifierToRemove : mGameobjectsToRemove) {
    auto gObjInWorld = mGameobjects.begin();
    while (gObjInWorld != mGameobjects.end()) {
      if (gObjInWorld->get()->getIdentifier() == identifierToRemove) {
        removedObjects.emplace_back(std::move(*gObjInWorld));
        gObjInWorld = mGameobjects.erase(gObjInWorld);
        continue;
      }

      ++gObjInWorld;
    }
  }
  mGameobjectsToRemove.clear();

  for (auto &obj : removedObjects) {
    ASSERT(
        obj.use_count() == 1,
        "More than one owning reference exists of gameobject at time of erase");
    obj->teardown();
  }
}

void Engine::runSetups(std::vector<std::shared_ptr<GameObject>> &gameobjects) {
  for (auto &go : gameobjects) {
    go->setup();
  }
}

std::weak_ptr<GameObject> Engine::getGameObject(const Uuid &identifier) {
  for (auto &gObj : mGameobjects) {
    // When a component wants to get
    // the gameobjects it's being deleted from
    // go may be null.
    if (gObj && gObj->getIdentifier() == identifier) {
      return gObj;
    }
  }
  return std::weak_ptr<GameObject>();
}