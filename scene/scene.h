#ifndef SCENE_H
#define SCENE_H
#pragma once

#include "glad/glad.h"
#if defined __has_include
  #if __has_include (<SDL_opengl.h>)
    #include <SDL_opengl.h>
  #else
    #include <GL/gl.h>
  #endif
#endif

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <utility>
#include "shader.h"
#include "model/model.h"
#include "renderable/renderable.h"
#include "renderable/gldata.h"
#include "camera.h"
#include "log.h"

class GLScene
{
public:
  GLScene(libQ::log _logobj);
  virtual ~GLScene();

  void Init();
  void ImportWorldOptions(
      SharedVAO ImportSkyboxVAO, SharedVAO ImportAABBVAO,
      SharedTex ImportSkyboxTex,
//      SharedVBO ImportSkyboxVBO, SharedVBO ImportAABBVBO,
      std::shared_ptr<_shader> ImportSkyboxShader, std::shared_ptr<_shader> ImportDefaultShader);
  void GLInit();
  void Draw(float deltaTime);

  void UpdateSkybox(cv::Mat skybox);

  void ImportScene(GLScene *scene);
  void ImportModelsFrom(GLScene *scene);
  void ImportRenderablesFrom(GLScene *scene);
  void ImportRenderablesFromInto(GLScene *scene, std::vector<std::shared_ptr<Renderable>> *ChildVector);
  std::pair<
    std::vector<std::shared_ptr<Renderable>>*,
    std::vector<std::shared_ptr<Renderable>>::iterator>
  FindSiblingVectorOfChild(std::shared_ptr<Renderable> child);
  bool CheckCascadingChild(std::shared_ptr<Renderable> parent, std::shared_ptr<Renderable> child);

  void DebugSelectRenderable(std::shared_ptr<Renderable> renderable);
  void DebugDrawAABB(std::shared_ptr<Renderable> renderable, glm::mat4 view_projection, glm::vec3 CameraPos);

//private:
  std::unordered_map< std::shared_ptr<Renderable>, std::vector<std::shared_ptr<Renderable>> > renderables;
  std::unordered_map<std::string, std::shared_ptr<Model> > models;
  std::unordered_map<std::string, std::shared_ptr<_shader> > shaders;

  std::vector<std::shared_ptr<Renderable>> SceneBase;
  std::shared_ptr<_shader> SkyboxShader, DefaultShader;
  SharedTex SkyboxTexID;
  SharedVBO AABBVBO, SkyboxVBO;
  SharedVAO AABBVAO, SkyboxVAO;
  bool selectClosest, itemWasToggledOpen;

	std::shared_ptr<Camera> camera;
  libQ::log logobj;
  AssetData Info;
};

#endif
