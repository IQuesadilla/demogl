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

#include "log.h"

class GLSceneLoader
{
public:
  /*struct Operation {
    enum {
      FILE_COLLADA,
    } op;
    union U {
      std::filesystem::path FILE_COLLADA;
      ~U(){}
    } data;
  };*/
  // TODO: This is commented out for now. Ultimately,
  // this should be used to create a Queue for any tasks
  // that simply require a lot of time. Another similar
  // strucure should exist for the responses.

  GLSceneLoader(libQ::log _logobj);
  ~GLSceneLoader();
  void QueueFile(std::filesystem::path FilePath);
  std::pair<std::string,std::shared_ptr<GLScene>> Retrieve();
  bool isLoading();
private:
  void ThreadFunction(libQ::log *tlogobj);
  std::mutex QueueMutex, SceneMutex;
  std::queue<std::filesystem::path> PreFileQueue;  
  std::queue<std::filesystem::path> FileQueue;
  std::queue<std::pair<std::string,std::shared_ptr<GLScene>>> SceneQueue;
  std::thread LoaderThread;
  std::condition_variable Condition;
  libQ::log logobj;
  //std::shared_ptr<_shader> _DefaultShader;
  bool ShouldExit;
};
