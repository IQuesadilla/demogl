#include "loader.h"
#include "collada.h"

GLSceneLoader::GLSceneLoader(libQ::log _logobj)
{
  logobj = _logobj;
  logobj.setClass("GLSceneLoader");
  auto log = logobj("GLSceneLoader");
  //QueueMutex.lock();
  //Polling = true;
  ShouldExit = false;
  //_DefaultShader = DefaultShader;
  LoaderThread = std::thread(&GLSceneLoader::ThreadFunction, this, _logobj.split("GLSceneLoader"));
}

GLSceneLoader::~GLSceneLoader()
{
  auto log = logobj("~GLSceneLoader");
  {
    log << "Attemping lock to kill" << libQ::NOTEV;
    std::lock_guard<std::mutex> lock(QueueMutex);
    ShouldExit = true;
    Condition.notify_one();
  }
  LoaderThread.join();
}

void GLSceneLoader::QueueFile(std::filesystem::path FilePath)
{
  auto log = logobj("QueueFile");
  log << "Queueing file: " << FilePath.string() << libQ::VALUEV;
  PreFileQueue.push(FilePath);
}

std::pair<std::string,std::shared_ptr<GLScene>>
  GLSceneLoader::Retrieve()
{
  auto log = logobj("Retrieve",libQ::DELAYPRINTFUNCTION);
  std::unique_lock<std::mutex> QueueLock(QueueMutex, std::try_to_lock);
  if (QueueLock && !PreFileQueue.empty())//QueueMutex.try_lock())
  {
    auto FilePath = PreFileQueue.front();
    FileQueue.push(FilePath);
    PreFileQueue.pop();
    Condition.notify_one();
    log << "Successfully Queued File: " << FilePath.string() << libQ::VALUEV;
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

void GLSceneLoader::ThreadFunction(libQ::log *tlogobj)
{
  std::shared_ptr<libQ::log> _logobj; _logobj.reset(tlogobj);
  _logobj->setClass("GLSceneLoader");
  auto log = _logobj->function("ThreadFunction");
  while (true)
  {
    std::unique_lock<std::mutex> QueueLock(QueueMutex);
    log << "Sleeping until triggered" << libQ::NOTEDEBUG;
    Condition.wait(QueueLock, [this]{ return !FileQueue.empty() || ShouldExit; });
    log << "Condition Triggered" << libQ::NOTEDEBUG;
    if (ShouldExit) return;

    while (FileQueue.size() > 0)
    {
      std::chrono::steady_clock::time_point LoadStart = std::chrono::steady_clock::now();
      //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      std::filesystem::path FilePath = FileQueue.front();
      FileQueue.pop();
      QueueLock.unlock();
      log << "File Path: " << FilePath << libQ::VALUEV;

      //std::filesystem::path FilePath = myFileBrowser.GetSelected();
      std::string RelPathString = std::filesystem::relative(FilePath,std::filesystem::current_path()).string();
      std::string ExtensionString = FilePath.extension().string();

      //std::cout << "File Relative Path: " << RelPathString << std::endl;
      //std::cout << "Current PWD: " << std::filesystem::current_path() << std::endl;
      //std::cout << ""

      std::shared_ptr<GLScene> ToImport = nullptr;
      if (ExtensionString == ".dae")
      {
        COLLADAScene *SceneImport = new COLLADAScene(FilePath.string(),logobj);
        GLScene *NewScene = new GLScene(logobj);
        NewScene->ImportScene(SceneImport);
        delete SceneImport;
        ToImport.reset(NewScene);
        log << FilePath << " import time: " <<
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - LoadStart).count()
          << " ms" << libQ::VALUEV;
      }
      else
      {
        log << "No support for file extension " << ExtensionString << libQ::ERROR;
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
