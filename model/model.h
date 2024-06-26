#ifndef MODEL_H
#define MODEL_H
#pragma once

#include "glad/glad.h"
//#include "model/model.h"
#include "shader.h"
#include "renderable/gldata.h"

//#include "SDL.h"
#include <SDL_opengl.h>
#include <glm.hpp>
#include <gtx/quaternion.hpp>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <string>
#include <memory>

struct AssetData
{
  std::string Author;
  std::string AuthoringTool;
  std::string Coverage;
  std::vector<std::string> GeographicLocation;
  std::string Created;
  std::vector<std::string> Keywords;
  std::string Modified;
  std::string Revision;
  std::string Subject;
  std::string Title;
  glm::mat4 ImpliedTransform;
};

class Model
{
public:
  Model();
  //Model( Model *_new );
  virtual ~Model();

  void GLInit();

  void bind();
  void draw();

  virtual void update(bool DoDebugDraw) {};

  std::shared_ptr<_shader> shader;
  int TCount;
  bool isEnclosed;

  std::vector<GLfloat> CollisionVerts;
  std::vector<GLuint> CollisionIndices;

  AssetData Info;

//protected:
  void Init();
  void setModel(std::vector<GLfloat> vertData);
  void setModel(std::vector<glm::vec3> vertData);
  void updateModel(std::vector<glm::vec3> vertData);
  void setIndices(std::vector<GLuint> indexData);
  void updateIndices(std::vector<GLuint> indexData);
  void setTex(cv::Mat image = cv::Mat(), std::vector<GLfloat> uvData = std::vector<GLfloat>());
  void setColors(std::vector<GLfloat> colorData);

  SharedVBO vertbuff, uvbuff, colorbuff;
  SharedTex texbuff;
  SharedVBO ibuff;
  SharedVAO VAO;
  bool doGenerateMipmap;

  struct UnloadedModel
  {
    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;
  };
  UnloadedModel *_UnloadedModel;
};

#endif
