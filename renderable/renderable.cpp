#include "renderable.h"

Renderable::Renderable(std::shared_ptr<Camera> camera)
{
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    rotAxis = glm::vec3(0.f);
    spinAxis = glm::vec3(0.f);
    alpha = 1.0f;
    trans = glm::vec3(0.f);

    flags.isHovered = false;
    flags.isSelected = false;
    flags.isClosest = false;

    _camera = camera;
}

Renderable::~Renderable()
{
    glDeleteVertexArrays(1,&VAO);
}

void Renderable::render(glm::mat4 projection, glm::mat4 view, float deltaTime)
{
    //if ( !(speed > -0.01f && speed < 0.01f) )
    rotAxis += deltaTime*spinAxis;

    if (rotAxis.x > 360.f)
        rotAxis.x -= 360.f;
    if (rotAxis.y > 360.f)
        rotAxis.y -= 360.f;
    if (rotAxis.z > 360.f)
        rotAxis.z -= 360.f;

    if (rotAxis.x < 0.f)
        rotAxis.x += 360.f;
    if (rotAxis.y < 0.f)
        rotAxis.y += 360.f;
    if (rotAxis.z < 0.f)
        rotAxis.z += 360.f;

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, trans);

    model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
        glm::angleAxis(							// Angle axis has two arguments - angle and axis
            glm::radians(rotAxis.x),					// The angle to rotate all the vertices, converts deg to rad
            glm::vec3(1.f,0.f,0.f) ));							// Which axis to apply the rotation to and how much - (x,y,z)
    model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
        glm::angleAxis(							// Angle axis has two arguments - angle and axis
            glm::radians(rotAxis.y),					// The angle to rotate all the vertices, converts deg to rad
            glm::vec3(0.f,1.f,0.f) ));							// Which axis to apply the rotation to and how much - (x,y,z)
    model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
        glm::angleAxis(							// Angle axis has two arguments - angle and axis
            glm::radians(rotAxis.z),					// The angle to rotate all the vertices, converts deg to rad
            glm::vec3(0.f,0.f,1.f) ));							// Which axis to apply the rotation to and how much - (x,y,z)

    //glm::vec3 lookingDirection = glm::vec3(cam->Front)
    flags.isHovered = RaycastRotatedCube(glm::vec3(1.0f, 1.0f, 1.0f), model, _camera->Position, _camera->Front);

    glBindVertexArray(VAO);

    if ( !flags.isSelected )
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glEnable(GL_CULL_FACE);
    }

    //if ( flags.isHovered )
    //	alpha = 0.3f;
    //else
    //	alpha = 1.0f;

    // Use shader "shader" and give it all 3 uniforms
    _model->shader->use();
    _model->shader->setMat4("model",model);				// GLSL: uniform mat4 model;
    _model->shader->setMat4("view",view);				// GLSL: uniform mat4 view;
    _model->shader->setMat4("projection",projection);	// GLSL: uniform mat4 projection;

    if (flags.isClosest)    _model->shader->setFloat("alpha",alpha/2);
    else                    _model->shader->setFloat("alpha",alpha);

    flags.isClosest = false;

    // Enable location 0 and location 1 in the shader
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    _model->shader->setInt("myTextureSampler", 0);

    // * if normalized is set to GL_TRUE, it indicates that values stored in an integer format are to be mapped
    // * to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and converted
    // * to floating point. Otherwise, values will be converted to floats directly without normalization. 

    // ** 0 means tightly packed, in this case 0 means OpenGL should automatically calculate size * sizeof(GLFloat) = 12
    // ** no distance from "how many bytes it is from the start of one element to the start of another"

    // Draw the cube
    glDrawArrays(GL_TRIANGLES, 0, 12*3);

    // Disable location 0 and location 1
    glDisableVertexArrayAttrib(VAO, 0);
    glDisableVertexArrayAttrib(VAO, 1);
}