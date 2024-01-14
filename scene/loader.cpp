#include "loader.h"
#include "collada.h"

GLSceneLoader::GLSceneLoader()
{
  //QueueMutex.lock();
  //Polling = true;
  ShouldExit = false;
  //_DefaultShader = DefaultShader;
  LoaderThread = std::thread(&GLSceneLoader::ThreadFunction, this);
}

GLSceneLoader::~GLSceneLoader()
{
  {
    std::cout << "Attemping lock to kill" << std::endl;
    std::lock_guard<std::mutex> lock(QueueMutex);
    ShouldExit = true;
    Condition.notify_one();
  }
  LoaderThread.join();
}

void GLSceneLoader::QueueFile(std::filesystem::path FilePath)
{
  std::cout << "Queueing file: " << FilePath.string() << std::endl;
  PreFileQueue.push(FilePath);
}

std::pair<std::string,std::shared_ptr<GLScene>>
  GLSceneLoader::Retrieve()
{
  std::unique_lock<std::mutex> QueueLock(QueueMutex, std::try_to_lock);
  if (QueueLock && !PreFileQueue.empty())//QueueMutex.try_lock())
  {
    auto FilePath = PreFileQueue.front();
    FileQueue.push(FilePath);
    PreFileQueue.pop();
    Condition.notify_one();
    std::cout << "Successfully Queued File: " << FilePath.string() << std::endl;
  }

  std::unique_lock<std::mutex> SceneLock(SceneMutex, std::try_to_lock);
  if (SceneLock && !SceneQueue.empty())
  {
    auto to_return = SceneQueue.front();
    SceneQueue.pop();
    return to_return;
  } else return std::make_pair("",nullptr);
}

bool GLSceneLoader::isLoading()
{
  return true;//!Polling;
}

void GLSceneLoader::ThreadFunction()
{
  while (true)
  {
    std::unique_lock<std::mutex> QueueLock(QueueMutex);
    std::cout << "Sleeping until triggered" << std::endl;
    Condition.wait(QueueLock, [this]{ return !FileQueue.empty() || ShouldExit; });
    std::cout << "--> Triggered" << std::endl;
    if (ShouldExit) return;

    while (FileQueue.size() > 0)
    {
      std::chrono::steady_clock::time_point LoadStart = std::chrono::steady_clock::now();
      //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      std::filesystem::path FilePath = FileQueue.front();
      FileQueue.pop();
      QueueLock.unlock();
      std::cout << "File Path: " << FilePath << std::endl;

      //std::filesystem::path FilePath = myFileBrowser.GetSelected();
      std::string RelPathString = std::filesystem::relative(FilePath,std::filesystem::current_path()).string();
      std::string ExtensionString = FilePath.extension().string();

      //std::cout << "File Relative Path: " << RelPathString << std::endl;
      //std::cout << "Current PWD: " << std::filesystem::current_path() << std::endl;
      //std::cout << ""

      std::shared_ptr<GLScene> ToImport = nullptr;
      if (ExtensionString == ".dae")
      {
        COLLADAScene *SceneImport = new COLLADAScene(FilePath.string());
        GLScene *NewScene = new GLScene();
        NewScene->ImportScene(SceneImport);
        delete SceneImport;
        ToImport.reset(NewScene);
        std::cout << FilePath << " import time: " <<
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - LoadStart).count()
          << " ms" << std::endl;
      }
      else
      {
        std::cout << "No support for file extension " << ExtensionString << std::endl;
        continue;
      }

      SceneMutex.lock();
      SceneQueue.push(std::make_pair(RelPathString,ToImport));
      //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      SceneMutex.unlock();
      QueueLock.lock();
    }
  }
}
