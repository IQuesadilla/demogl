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

#include <map>
#include <memory>
#include <vector>
#include <string>
#include <utility>
#include "shader.h"
#include "model/model.h"
#include "renderable/renderable.h"
#include "camera.h"

class GLScene
{
public:
    GLScene();
    virtual ~GLScene();

    void Draw(float deltaTime, std::shared_ptr<Camera> camera);

    void UpdateSkybox(cv::Mat skybox);

    void ImportScene(GLScene *scene);
    std::pair<
        std::vector<std::shared_ptr<Renderable>>*,
        std::vector<std::shared_ptr<Renderable>>::iterator>
        FindSiblingVectorOfChild(std::shared_ptr<Renderable> child);
    bool CheckCascadingChild(std::shared_ptr<Renderable> parent, std::shared_ptr<Renderable> child);

    void DebugSelectRenderable(std::shared_ptr<Renderable> renderable);
    void DebugDrawAABB(std::shared_ptr<Renderable> renderable, glm::mat4 view_projection, glm::vec3 CameraPos);

//private:
    std::map< std::shared_ptr<Renderable>, std::vector<std::shared_ptr<Renderable>> > renderables;
    std::map<std::string, std::shared_ptr<Model> > models;
    std::map<std::string, std::shared_ptr<_shader> > shaders;

    std::vector<std::shared_ptr<Renderable>> SceneBase;
    _shader SkyboxShader;
    std::shared_ptr<_shader> AABBShader;
    GLuint SkyboxTexID, SkyboxVAO, SkyboxVBO;
    GLuint AABBVAO, AABBVBO[2];
    bool selectClosest, itemWasToggledOpen;

    AssetData Info;
};

#endif
