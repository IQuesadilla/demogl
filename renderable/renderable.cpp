#include "renderable.h"
/*
glm::vec3 ApplyMatrix(glm::mat4 inmodel, glm::vec3 invert)
{

}*/

Renderable::Renderable(std::shared_ptr<Model> model)
{
    rotAxis = glm::vec3(0.f);
    spinAxis = glm::vec3(0.f);
    alpha = 1.0f;
    trans = glm::vec3(0.f);
    scale = glm::vec3(1.f);

    flags.isSelected = false;
    flags.isHovered = false;
    flags.QueueDestruction = false;
    flags.isCollisionUpdated = false;

    //_model->InitDataPtr(ModelDataPtr);

    _model = model;
}

Renderable::Renderable( Renderable *_new )
{
    *this = *_new;
}

Renderable::~Renderable()
{
    ;
}

void Renderable::render(glm::mat4 view_projection)
{
    flags.isCollisionUpdated = false;
    if (_model->TCount > 0)
    {
        _model->bind();
        if ( _model->isEnclosed )
        {
            glEnable(GL_CULL_FACE);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

        _model->shader->use();
        _model->shader->setMat4("mvp",view_projection * _modelmatrix);

        //if (flags.isHovered)    _model->shader->setFloat("alpha",alpha/2);
        _model->shader->setFloat("alpha",alpha);

        _model->shader->setInt("myTextureSampler", 0);

        // Draw the cube
        _model->draw();
        //glDrawArrays(GL_TRIANGLES, 0, 3 * _model->TCount);
    }

    if (flags.isSelected)
    {
        ImGui::PushID(this);
        ImGui::Text("ID: %p",this);
        ImGui::SameLine();
        if (ImGui::Button("Destroy")) flags.QueueDestruction = true;

        ImGui::Text("AABB Positive (%.3f,%.3f,%.3f)",posAABB.x,posAABB.y,posAABB.z);
        ImGui::Text("AABB Negative (%.3f,%.3f,%.3f)",negAABB.x,negAABB.y,negAABB.z);

        ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f);

        ImGui::DragFloat("Spin X", &spinAxis.x, 0.01f);
        ImGui::DragFloat("Spin Y", &spinAxis.y, 0.01f);
        ImGui::DragFloat("Spin Z", &spinAxis.z, 0.01f);

        ImGui::DragFloat("Rot X", &rotAxis.x);
        ImGui::DragFloat("Rot Y", &rotAxis.y);
        ImGui::DragFloat("Rot Z", &rotAxis.z);

        ImGui::DragFloat("X", &trans.x, 0.01f);
        ImGui::DragFloat("Y", &trans.y, 0.01f);
        ImGui::DragFloat("Z", &trans.z, 0.01f);

        ImGui::DragFloat("Scale X", &scale.x, 0.01f);
        ImGui::DragFloat("Scale Y", &scale.y, 0.01f);
        ImGui::DragFloat("Scale Z", &scale.z, 0.01f);
/*
        std::string PointsString;
        for (auto &x : TransformedCollisionVerts)
          PointsString.append("<" + std::to_string(x.x) + "," + std::to_string(x.y) + "," + std::to_string(x.z) + ">\n");
        ImGui::Text("%s",PointsString.c_str());
*/
        ImGui::PopID();
    }
}

float Renderable::distance(glm::vec3 pos)
{
    return glm::abs(glm::distance(pos,trans));
}

void Renderable::select()
{
    flags.isSelected = ! flags.isSelected;
}

bool Renderable::raycastTriangle(float *dis, glm::vec3 pos, glm::vec3 dir, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2)
{
    const float EPSILON = 0.0000001f; // A small number to represent floating point error
    glm::vec3 edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = v1 - v0;
    edge2 = v2 - v0;
    h = glm::cross(dir, edge2);
    a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON) // This means the ray is parallel to the triangle.
        return false;

    f = 1.0 / a;
    s = pos - v0;
    u = f * glm::dot(s, h);

    if (u < 0.0 || u > 1.0)
        return false;

    q = glm::cross(s, edge1);
    v = f * glm::dot(dir, q);

    if (v < 0.0 || u + v > 1.0)
        return false;

    // At this stage, we can compute t to find out where the intersection point is on the line.
    float t = f * glm::dot(edge2, q);

    if (t > EPSILON)
    {
        *dis = t;
        return true;
    } else {
        *dis = -1.f;
        return false;
    }
}

float Renderable::raycastAABB(glm::vec3 pos, glm::vec3 dir)
{
    glm::vec3 rayDir = glm::normalize(dir);

    // Compute the minimum and maximum intersection times for the x-axis
    float txMin = ( negAABB.x - pos.x) / rayDir.x;
    float txMax = ( posAABB.x - pos.x) / rayDir.x;

    // Compute the minimum and maximum intersection times for the y-axis
    float tyMin = ( negAABB.y - pos.y) / rayDir.y;
    float tyMax = ( posAABB.y - pos.y) / rayDir.y;

    // Compute the minimum and maximum intersection times for the z-axis
    float tzMin = ( negAABB.z - pos.z) / rayDir.z;
    float tzMax = ( posAABB.z - pos.z) / rayDir.z;

    // Compute the maximum and minimum intersection times for the whole cube
    float tMin = glm::max(glm::max(glm::min(txMin, txMax), glm::min(tyMin, tyMax)), glm::min(tzMin, tzMax));
    float tMax = glm::min(glm::min(glm::max(txMin, txMax), glm::max(tyMin, tyMax)), glm::max(tzMin, tzMax));

    // When the largest minimum becomes smaller than the smallest maximum

    // Check if the ray intersects the cube
    if (tMin > tMax || tMax < 0.0f)
    {
        // The ray missed the cube
        return -1.f;
    }

    float oIntDis = -1.f;
    if (_model->CollisionIndices.size() > 0)
        for (auto i = _model->CollisionIndices.begin(); i < _model->CollisionIndices.end(); i+=3)
        {
            if (raycastTriangle(&oIntDis,pos,rayDir,TransformedCollisionVerts[i[0]],TransformedCollisionVerts[i[1]],TransformedCollisionVerts[i[2]]))
                return oIntDis;
        }
    else
        for (auto i = TransformedCollisionVerts.begin(); i < TransformedCollisionVerts.end(); i+=3)
        {
            if (raycastTriangle(&oIntDis,pos,rayDir,i[0],i[1],i[2]))
                return oIntDis;
        }

    // The ray intersects the cube
    return oIntDis;
}

glm::mat4 Renderable::ApplyTransforms(glm::mat4 inmodel, glm::vec3 inrot, glm::vec3 intrans, glm::vec3 inscale)
{
    glm::mat4 model = inmodel;

    model = glm::translate(model, intrans);

    model = glm::scale(model,inscale);

/*
    model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
        glm::angleAxis(							// Angle axis has two arguments - angle and axis
            glm::radians(inrot.x),					// The angle to rotate all the vertices, converts deg to rad
            glm::vec3(1.f,0.f,0.f) ));							// Which axis to apply the rotation to and how much - (x,y,z)
    model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
        glm::angleAxis(							// Angle axis has two arguments - angle and axis
            glm::radians(inrot.y),					// The angle to rotate all the vertices, converts deg to rad
            glm::vec3(0.f,1.f,0.f) ));							// Which axis to apply the rotation to and how much - (x,y,z)
    model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
        glm::angleAxis(							// Angle axis has two arguments - angle and axis
            glm::radians(inrot.z),					// The angle to rotate all the vertices, converts deg to rad
            glm::vec3(0.f,0.f,1.f) ));							// Which axis to apply the rotation to and how much - (x,y,z)
*/

    model = model * glm::toMat4(glm::quat(glm::radians(inrot)));

    return model;
}

void Renderable::AnimationUpdate(float DeltaTime, glm::mat4 ParentModelMatrix)
{
    glm::vec3 PotentialRotation = DeltaTime*spinAxis;
    glm::vec3 PotentialTranslation = glm::vec3(0.0f);
    glm::vec3 PotentialScale = glm::vec3(0.0f);

    if (PotentialRotation.x >= 360.f)
        PotentialRotation.x -= 360.f;
    if (PotentialRotation.y > 360.f)
        PotentialRotation.y -= 360.f;
    if (PotentialRotation.z > 360.f)
        PotentialRotation.z -= 360.f;

    if (PotentialRotation.x < 0.f)
        PotentialRotation.x += 360.f;
    if (PotentialRotation.y < 0.f)
        PotentialRotation.y += 360.f;
    if (PotentialRotation.z < 0.f)
        PotentialRotation.z += 360.f;

    rotAxis += PotentialRotation;
    trans += PotentialTranslation;
    scale += PotentialScale;

    glm::mat4 PotentialTransform = ApplyTransforms(ParentModelMatrix,rotAxis,trans,scale);

    TransformedCollisionVerts.clear();
    //TransformedCollisionVerts.resize (_model->CollisionVerts.size() / 3, glm::vec3(0.0f));
    //std::vector<glm::vec3> verts;// verts.resize (_model->CollisionVerts.size() / 3);
    //int i = 0;
    for (auto it = _model->CollisionVerts.begin(); it < _model->CollisionVerts.end(); it+=3)
    {
        glm::vec4 positionVec4 = PotentialTransform * glm::vec4(it[0],it[1],it[2],1.0f);
        TransformedCollisionVerts.push_back(glm::vec3(positionVec4.x,positionVec4.y,positionVec4.z) / positionVec4.w);
        //verts.push_back(glm::vec3(it[0],it[1],it[2]));
        //++i;
    }

    //_modelmatrix = PotentialTransform;

    updateAABB();
    flags.isCollisionUpdated = true;
}

void Renderable::Collide(std::shared_ptr<Renderable> k)
{
    ;
}

void Renderable::genModelMatrix(glm::mat4 inmodel)
{
    _modelmatrix = ApplyTransforms(inmodel,rotAxis,trans,scale);
}

void Renderable::updateAABB()
{
    glm::vec3 cubeExtents = TransformedCollisionVerts.front();//trans;//glm::vec3(1000.0f, 1000.0f, 1000.0f);
    glm::vec3 negvExtents = TransformedCollisionVerts.front();//trans;//glm::vec3(0.0f, 0.0f, 0.0f);

    if (_model->CollisionIndices.size() > 0)
        for (auto i = _model->CollisionIndices.begin(); i < _model->CollisionIndices.end(); ++i)
        {
            cubeExtents = glm::max(cubeExtents,TransformedCollisionVerts[*i]);
            negvExtents = glm::min(negvExtents,TransformedCollisionVerts[*i]);
        }
    else
        for (auto i = TransformedCollisionVerts.begin(); i < TransformedCollisionVerts.end(); ++i)
        {
            cubeExtents = glm::max(cubeExtents,*i);
            negvExtents = glm::min(negvExtents,*i);
        }

    if (glm::abs(cubeExtents.x - negvExtents.x) < 0.001f)
    {
        cubeExtents.x += 0.001f;
        negvExtents.x -= 0.001f;
    }

    if (glm::abs(cubeExtents.y - negvExtents.y) < 0.001f)
    {
        cubeExtents.y += 0.001f;
        negvExtents.y -= 0.001f;
    }

    if (glm::abs(cubeExtents.z - negvExtents.z) < 0.001f)
    {
        cubeExtents.z += 0.001f;
        negvExtents.z -= 0.001f;
    }

    posAABB = cubeExtents;
    negAABB = negvExtents;
}
