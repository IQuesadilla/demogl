#pragma once
#include "scene.h"
//#include "shader.h"
#include <thread>
#include <queue>
#include <mutex>
#include <memory>
#include <filesystem>
#include <utility>
#include <condition_variable>

class GLSceneLoader
{
public:
  GLSceneLoader();
  ~GLSceneLoader();
  void QueueFile(std::filesystem::path FilePath);
  std::pair<std::string,std::shared_ptr<GLScene>> Retrieve();
  bool isLoading();
private:
  void ThreadFunction();
  std::mutex QueueMutex, SceneMutex;
  std::queue<std::filesystem::path> PreFileQueue;  
  std::queue<std::filesystem::path> FileQueue;
  std::queue<std::pair<std::string,std::shared_ptr<GLScene>>> SceneQueue;
  std::thread LoaderThread;
  std::condition_variable Condition;
  //std::shared_ptr<_shader> _DefaultShader;
  bool ShouldExit;
};
