#pragma once

#include "Uuid.h"
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <vector>
#include <mutex>

class GameObject;
class ISceneGenerator;
class Uuid;
class Engine {
 public:
  static void initialize();
  static void teardown();
  static void start();
  static void stop();

  static void addGameObject(std::unique_ptr<GameObject> gameobject);

  static void loadScene(const std::string&);

  static void removeGameObject(const Uuid& identifier);
  static void registerScene(const std::string& name, std::unique_ptr<ISceneGenerator> scenecreator);

  static std::weak_ptr<GameObject> getGameObject(const Uuid& identifier);
  
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
  static void runSetups(std::vector<std::shared_ptr<GameObject>>&);

  static std::map<std::string, std::unique_ptr<ISceneGenerator>> mScenes;

  static std::optional<std::string> mNameOfSceneToLoad;
  static bool mAboutToLoadScene;
  static bool mRunning;
  static bool mInitialized;
  static std::vector<std::shared_ptr<GameObject>> mGameobjects;
  static std::vector<std::shared_ptr<GameObject>> mGameobjectsToAdd;
  static std::set<Uuid> mGameobjectsToRemove;
  static std::mutex mMutex;
  static std::unique_ptr<SerializedObj> mSavedState;
  static std::unique_ptr<SerializedObj> mLoadState;
};