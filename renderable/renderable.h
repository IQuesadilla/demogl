#ifndef RENDERABLE_H
#define RENDERABLE_H

#ifndef GL_GLEXT_PROTOTYPES

#endif

#include "model/model.h"

#include "imgui.h"

//#include "SDL.h"
#include <SDL_opengl.h>
#include <glm.hpp>
#include <gtx/quaternion.hpp>

#include <memory>
#include <iomanip>

#include "camera.h"
#include "shader.h"

class Renderable
{
public:
    Renderable();
    Renderable(std::shared_ptr<Model> model);
    Renderable( Renderable *_new );
    ~Renderable();

    void Init();
    void setModel(std::shared_ptr<Model> model);

    void render(glm::mat4 view_projection);
    float distance(glm::vec3 pos);
    void select();
    void AnimationUpdate(float DeltaTime, glm::mat4 ParentModelMatrix);
    void Collide(std::shared_ptr<Renderable> k);
    static bool raycastTriangle(float *dis, glm::vec3 pos, glm::vec3 dir, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);
    float raycastAABB(glm::vec3 pos, glm::vec3 dir);

//private:
    void genModelMatrix(glm::mat4 inmodel);
    void updateAABB();

    static glm::mat4 ApplyTransforms(glm::mat4 inmodel, glm::vec3 inrot, glm::vec3 intrans, glm::vec3 inscale);

    std::shared_ptr<Model> _model;
    std::vector<glm::vec3> TransformedCollisionVerts; // Should be temporary
    std::vector<std::shared_ptr<Renderable>> Collisions;
    void *ModelDataPtr;

    struct {
		bool isHovered;
		bool isSelected;
        bool QueueDestruction;
        bool isCollisionUpdated;
		//bool isClosest;
	} flags;

    glm::mat4 _modelmatrix;
    glm::vec3 posAABB, negAABB;
    glm::vec3 trans, rotAxis, scale;
    glm::vec3 FrameTranslate, FrameRotation, spinAxis;
    float alpha;
    AssetData Info;
};

#endif
