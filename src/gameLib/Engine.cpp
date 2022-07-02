#include "Engine.h"

#include <algorithm>
#include <string>
#include <thread>

#include "GraphicsManager.h"
#include "Helpers.h"
#include "InputManager.h"
#include "Timer.h"
#include "CloseGameComponent.h"
std::map<std::string, std::unique_ptr<ISceneGenerator>> Engine::mScenes;
std::optional<std::string> Engine::mNameOfSceneToLoad;
bool Engine::mAboutToLoadScene = false;
bool Engine::mRunning = false;
bool Engine::mInitialized = false;
std::vector<std::unique_ptr<GameObject>> Engine::mGameobjects;
std::vector<std::unique_ptr<GameObject>> Engine::mGameobjectsToAdd;
std::set<GameObject*> Engine::mGameobjectsToRemove;
std::mutex Engine::mMutex;

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

void Engine::removeGameObject(GameObject* gObj) {
  if(!Engine::mRunning) return;
  std::scoped_lock lock(mMutex);
  LOG("Removing GameObject " << gObj->name() << " " << gObj);
  Engine::mGameobjectsToRemove.insert(gObj);
}

void Engine::registerScene(const std::string& name, std::unique_ptr<ISceneGenerator> scenecreator) {
  Engine::mScenes[name] = std::move(scenecreator);
}

void Engine::addGameObject(std::unique_ptr<GameObject> gameobject) {
  GameObject* out = gameobject.get();
  {
    std::scoped_lock lock(mMutex);
    Engine::mGameobjectsToAdd.emplace_back(std::move(gameobject));
  }
  LOG("Added gameobject" << out);
  if (!mRunning) putGameObjectsIntoWorld();
}


void Engine::loadScene(const std::string& name) {
  ASSERT(mScenes.count(name), "No scene named " + name);
  mNameOfSceneToLoad->assign(name);
  mAboutToLoadScene = true;
}
/* Private routines*/

void Engine::logicThreadMainLoop() {
  Timer timer;
  timer.start();
  int maxFps = 60;
  int msPerFrame = 1000 * 1 / maxFps;
  while (Engine::mRunning) {
    if (mAboutToLoadScene) {
      mAboutToLoadScene = false;
      replaceScene();
    }
    Engine::putGameObjectsIntoWorld();
    Engine::removeGameObjectFromWorld();
    Engine::updateGameObjects();

    auto elapsedMs = timer.getElapsedMilliseconds();
    auto timeLeft = msPerFrame - elapsedMs;
    if (timeLeft > 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(timeLeft));
    timer.start();
  }
}

void Engine::mainLoop() {
  std::thread logicThread(logicThreadMainLoop);
  Timer timer;
  timer.start();
  int maxFps = 100;
  int msPerFrame = 1000 * 1 / maxFps;
  while (Engine::mRunning) {
    InputManager::readInputs();
    GraphicsManager::prepareRendering();
    Engine::renderGameObjects();
    auto elapsedMs = timer.getElapsedMilliseconds();
    auto timeLeft = msPerFrame - elapsedMs;
    if (timeLeft > 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(timeLeft));
    timer.start();
  }
  logicThread.join();
}

void Engine::replaceScene() {
  /*
          Removes all gameobjects
          calls the sceneloader function
          this function will add the
          intial gameobjects to the engine.
  */
  clearAllGameObjects();
  auto gameobjects = mScenes[mNameOfSceneToLoad.value()]->internalGenerateGameObjects();
  for(auto& gameobject : gameobjects) {
    addGameObject(std::move(gameobject));
  }
}

void Engine::updateGameObjects() {
  for (auto& go : mGameobjects) {
    go->updateComponents();
  }
}

bool debug_render_colliders = true;

void Engine::renderGameObjects() {
  std::scoped_lock lock(mMutex);
  for (auto& go : mGameobjects) {
    go->render();
  }

  GraphicsManager::executeRendering();
}

void Engine::clearAllGameObjects() {
  {
    std::scoped_lock lock(mMutex);
    mGameobjects.clear();
    mGameobjectsToAdd.clear();
    mGameobjectsToRemove.clear();
  }
}

void Engine::putGameObjectsIntoWorld() {
  std::vector<GameObject*> addedItems;
  {
    std::scoped_lock lock(mMutex);
    for (auto it = mGameobjectsToAdd.begin(); it != mGameobjectsToAdd.end();
         it++) {
      std::unique_ptr<GameObject> newItem = std::move(*it);
      // store away a ptr because we have to run setups afterwards
      addedItems.emplace_back(newItem.get());
      size_t indx = 0;
      /* Does insert sort based on renderdepth
         to decide render order.
         large renderdepth = rendered later = "on top" / "closer to camera"
      */
      for (; indx != mGameobjects.size(); ++indx) {
        if (newItem->getRenderDepth() <= mGameobjects[indx]->getRenderDepth()) {
          mGameobjects.insert(mGameobjects.begin() + indx, std::move(newItem));
          break;
        }
      }
      if (indx == mGameobjects.size()) {
        mGameobjects.push_back(std::move(newItem));
      }
    }
  }
  mGameobjectsToAdd.clear();
  Engine::runSetups(addedItems);
}
void Engine::removeGameObjectFromWorld() {
  std::vector<std::unique_ptr<GameObject>> removedObjects;
  {
    std::scoped_lock lock(mMutex);
    for (auto gObj_to_remove : mGameobjectsToRemove) {
      auto gObj_in_world = mGameobjects.begin();
      while (gObj_in_world != mGameobjects.end()) {
        if ((gObj_in_world->get()) == (gObj_to_remove)) {
          removedObjects.emplace_back(std::move(*gObj_in_world));
          gObj_in_world = mGameobjects.erase(gObj_in_world);
          continue;
        }

        ++gObj_in_world;
      }
    }
    mGameobjectsToRemove.clear();
  }
  for (auto& obj : removedObjects) {
    obj->teardown();
  }
}

void Engine::runSetups(std::vector<GameObject*>& gameobjects) {
  for (auto& go : gameobjects) {
    go->setup();
  }
}

GameObject* Engine::getGameObject(const Uuid& identifier) {
  for (auto& go : mGameobjects) {
    // When a component wants to get
    // the gameobjects it's being deleted from
    // go may be null.
    if (go && go->getIdentifier() == identifier) {
      return go.get();
    }
  }
  return nullptr;
}