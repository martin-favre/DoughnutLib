#pragma once

#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <vector>
#include <mutex>
#include "GameObject.h"
#include "Logging.h"
#include "ISceneGenerator.h"
/*
        Usage:
        void foo()
        {
          Add gameobjects here
        }

        Engine::initialize();
        Engine::registerScene("scene1", foo);
        Engine::loadScene("scene1");
        Engine::start();
        Engine::teardown();

*/

class GameObject;
class Engine {
 public:
  static void initialize();
  static void teardown();
  static void start();
  static void stop();

  static void addGameObject(std::unique_ptr<GameObject> gameobject);

  static void loadScene(const std::string&);

  static void removeGameObject(GameObject* gObj);
  static void registerScene(const std::string& name, std::unique_ptr<ISceneGenerator> scenecreator);

  static GameObject* getGameObject(const Uuid& identifier);
  
 private:
  Engine();
  static void serialize();
  static void mainLoop();
  static void replaceScene();
  static void updateGameObjects();
  static void renderGameObjects();
  static void clearAllGameObjects();
  static void logicThreadMainLoop();
  /*Actually puts the changes in place*/
  static void putGameObjectsIntoWorld();
  static void removeGameObjectFromWorld();
  static void runSetups(std::vector<GameObject*>&);

  static std::map<std::string, std::unique_ptr<ISceneGenerator>> mScenes;

  static std::optional<std::string> mNameOfSceneToLoad;
  static bool mAboutToLoadScene;
  static bool mRunning;
  static bool mInitialized;
  static std::vector<std::unique_ptr<GameObject>> mGameobjects;
  static std::vector<std::unique_ptr<GameObject>> mGameobjectsToAdd;
  static std::set<GameObject*> mGameobjectsToRemove;
  static std::mutex mMutex;
  static std::unique_ptr<SerializedObj> mSavedState;
  static std::unique_ptr<SerializedObj> mLoadState;
};