#include "loader.h"
#include "collada.h"

GLSceneLoader::GLSceneLoader()
{
  QueueMutex.lock();
  Polling = true;
  ShouldExit = false;
  //_DefaultShader = DefaultShader;
  LoaderThread = std::thread(&GLSceneLoader::ThreadFunction, this);
}

GLSceneLoader::~GLSceneLoader()
{
  ShouldExit = true;
  QueueMutex.unlock();
  LoaderThread.join();
}

bool GLSceneLoader::QueueFile(std::filesystem::path FilePath)
{
  std::cout << "Queueing file" << std::endl;
  if (Polling || QueueMutex.try_lock())
  {
    FileQueue.push(FilePath);
    QueueMutex.unlock();
    std::cout << "Successfully Queued File" << std::endl;
    return true;
  }
  else return false;
}

std::pair<std::string,std::shared_ptr<GLScene>>
  GLSceneLoader::Retrieve()
{
  if (SceneQueue.size() == 0 || !SceneMutex.try_lock()) return std::make_pair("",nullptr);

  auto to_return = SceneQueue.front();
  SceneQueue.pop();
  SceneMutex.unlock();
  return to_return;
}

bool GLSceneLoader::isLoading()
{
  return !Polling;
}

void GLSceneLoader::ThreadFunction()
{
  while (!ShouldExit)
  {
    std::cout << "Attepting lock" << std::endl;
    QueueMutex.lock();
    if (FileQueue.size() == 0)
    {
      Polling = true;
    }
    else 
    {
      Polling = false;
      //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      std::filesystem::path FilePath = FileQueue.front();
      FileQueue.pop();
      QueueMutex.unlock();
      std::cout << "File Path: " << FilePath << std::endl;

      //std::filesystem::path FilePath = myFileBrowser.GetSelected();
      std::string RelPathString = std::filesystem::relative(FilePath,std::filesystem::current_path());
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
    }
  }
}
