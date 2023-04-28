#ifndef RENDERABLE_H
#define RENDERABLE_H

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include "model/model.h"

#include "SDL.h"
#include <SDL_opengl.h>
#include <glm.hpp>
#include <gtx/quaternion.hpp>

#include <memory>

#include "camera/camera.h"
#include "shader/shader.h"

class Renderable
{
public:
    Renderable(std::shared_ptr<Camera> camera);
    ~Renderable();

    void render(glm::mat4 projection, glm::mat4 view, float deltaTime);

private:
    GLuint VAO;
    std::shared_ptr<Model> _model;
    std::shared_ptr<Camera> _camera;

    struct {
		bool isHovered;
		bool isSelected;
		bool isClosest;
	} flags;

    glm::vec3 trans, rotAxis, spinAxis, spin;
    float alpha;
};

#endif