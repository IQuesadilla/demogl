#ifndef SCENE_H
#define SCENE_H
#pragma once

#include "glad/glad.h"

#include <unordered_map>
#include <stack>
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

  struct RendNode {
      RendNode(std::shared_ptr<Model> model) : Node(model) {};
      std::vector<std::shared_ptr<RendNode>> Children;
      std::deque<std::shared_ptr<RendNode>> Collisions;
      bool isInlineWithCamera = false;
      Renderable Node;
  };

  void ImportScene(GLScene *scene);
  void ImportModelsFrom(GLScene *scene);
  void ImportRenderablesFrom(GLScene *scene);
  void ImportRenderablesFromInto(GLScene *scene, std::vector<std::shared_ptr<RendNode>> *ChildVector);
  std::pair<
    std::vector<std::shared_ptr<RendNode>>*,
    std::vector<std::shared_ptr<RendNode>>::iterator>
  FindSiblingVectorOfChild(std::shared_ptr<RendNode> child);
  bool CheckCascadingChild(std::shared_ptr<RendNode> parent, std::shared_ptr<RendNode> child);

  void DebugSelectRenderable(std::shared_ptr<RendNode> renderable);
  void DebugDrawAABB(std::shared_ptr<RendNode> renderable, glm::mat4 view_projection, glm::vec3 CameraPos);

//private:
  //std::vector< std::pair< std::shared_ptr<Renderable>, std::vector<std::shared_ptr<Renderable> > > > renderables;
  std::unordered_map<std::string, std::shared_ptr<Model> > models;
  std::unordered_map<std::string, std::shared_ptr<_shader> > shaders;

  std::vector<std::pair<std::shared_ptr<RendNode>, float>> SceneSorted;
  std::vector<std::shared_ptr<RendNode>> SceneBase;
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
