#include "cube.h"

#include "assets/rawcube.h"

bool RaycastRotatedCube(const glm::vec3& cubeExtents, const glm::mat4& cubeTransform, const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
    // Compute the inverse transformation matrix for the cube
    glm::mat4 inverseTransform = glm::inverse(cubeTransform);

    // Transform the ray into the coordinate space of the cube
    glm::vec3 localRayOrigin = glm::vec3(inverseTransform * glm::vec4(rayOrigin, 1.0));
    glm::vec3 localRayDir = glm::normalize(glm::vec3(inverseTransform * glm::vec4(rayDir, 0.0)));

    // Compute the inverse of the cube's extents
    glm::vec3 inverseExtents = glm::vec3(1.0) / cubeExtents;

    // Compute the minimum and maximum intersection times for the x-axis
    float txMin = (inverseExtents.x * (-cubeExtents.x - localRayOrigin.x)) / localRayDir.x;
    float txMax = (inverseExtents.x * ( cubeExtents.x - localRayOrigin.x)) / localRayDir.x;

    // Compute the minimum and maximum intersection times for the y-axis
    float tyMin = (inverseExtents.y * (-cubeExtents.y - localRayOrigin.y)) / localRayDir.y;
    float tyMax = (inverseExtents.y * ( cubeExtents.y - localRayOrigin.y)) / localRayDir.y;

    // Compute the minimum and maximum intersection times for the z-axis
    float tzMin = (inverseExtents.z * (-cubeExtents.z - localRayOrigin.z)) / localRayDir.z;
    float tzMax = (inverseExtents.z * ( cubeExtents.z - localRayOrigin.z)) / localRayDir.z;

    // Compute the maximum and minimum intersection times for the whole cube
    float tMin = glm::max(glm::max(glm::min(txMin, txMax), glm::min(tyMin, tyMax)), glm::min(tzMin, tzMax));
    float tMax = glm::min(glm::min(glm::max(txMin, txMax), glm::max(tyMin, tyMax)), glm::max(tzMin, tzMax));

	//std::cout << "tMin: " << tMin << ", tMax: " << tMax << std::endl;

    // Check if the ray intersects the cube
    if (tMin > tMax || tMax < 0.0f)
    {
        // The ray missed the cube
        return false;
    }

    // The ray intersects the cube
    return true;
}

myCube::myCube() : Model()
{
    std::vector<GLfloat> vertDataVec;
    vertDataVec.resize(sizeof(vertData) / sizeof(GLfloat));
    memcpy(vertDataVec.data(),vertData,sizeof(vertData));

    std::vector<GLfloat> uvDataVec;
    uvDataVec.resize(sizeof(colorData) / sizeof(GLfloat));
    memcpy(uvDataVec.data(),colorData,sizeof(colorData));

    setModel(vertDataVec, uvDataVec);
}

myCube::myCube(myCube *_new)
{
    *this = *_new;
}

myCube::~myCube()
{
    ;
}
