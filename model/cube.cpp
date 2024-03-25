#include "cube.h"

#define I 1.0f

static const GLfloat CubeVerts[] = {
    // Top
    -I, I,-I, // 0
    -I, I, I, // 1
     I, I,-I, // 2
//     I, I,-I, // 2
//    -I, I, I, // 1
     I, I, I, // 3

    // North
     I, I,-I, // 4
     I,-I,-I, // 5
    -I, I,-I, // 6
//    -I, I,-I, // 6
//     I,-I,-I, // 5
    -I,-I,-I, // 7

    // East
    -I, I,-I, // 8
    -I,-I,-I, // 9
    -I, I, I, // 10
//    -I, I, I, // 10
//    -I,-I,-I, // 9
    -I,-I, I, // 11

    // South
    -I, I, I, // 12
    -I,-I, I, // 13
     I, I, I, // 14
//     I, I, I, // 14
//    -I,-I, I, // 13
     I, -I,I, // 15

    // West
     I, I, I, // 16
     I,-I, I, // 17
     I, I,-I, // 18
//     I, I,-I, // 18
//     I,-I, I, // 17
     I,-I,-I, // 19

    // Bottom
     I,-I,-I, // 20
     I,-I, I, // 21
    -I,-I,-I, // 22
//    -I,-I,-I, // 22
//     I,-I, I, // 21
    -I,-I, I, // 23
};

const std::vector<GLuint> CubeIndices = {
     0, 1, 2, 2, 1, 3,
     4, 5, 6, 6, 5, 7,
     8, 9,10,10, 9,11,
    12,13,14,14,13,15,
    16,17,18,18,17,19,
    20,21,22,22,21,23
};

static const GLfloat CubeUV[] = {
    // Top
    0.25f, 1.f/3,
    0.25f, 2.f/3,
    0.50f, 1.f/3,
//    0.50f, 1.f/3,
//    0.25f, 2.f/3,
    0.50f, 2.f/3,

    // North
    0.50f, 1.f/3,
    0.50f, 0.f/3,
    0.25f, 1.f/3,
//    0.25f, 1.f/3,
//    0.50f, 0.f/3,
    0.25f, 0.f/3,

    // East
    0.25f, 1.f/3,
    0.00f, 1.f/3,
    0.25f, 2.f/3,
//    0.25f, 2.f/3,
//    0.00f, 1.f/3,
    0.00f, 2.f/3,

    // South
    0.25f, 2.f/3,
    0.25f, 1.00f,
    0.50f, 2.f/3,
//    0.50f, 2.f/3,
//    0.25f, 1.00f,
    0.50f, 1.00f,

    // West
    0.50f, 2.f/3,
    0.75f, 2.f/3,
    0.50f, 1.f/3,
//    0.50f, 1.f/3,
//    0.75f, 2.f/3,
    0.75f, 1.f/3,

    // Bottom
    0.75f, 1.f/3,
    0.75f, 2.f/3,
    1.00f, 1.f/3,
//    1.00f, 1.f/3,
//    0.75f, 2.f/3,
    1.00f, 2.f/3,
};

myCube::myCube() : Model()
{
    std::vector<GLfloat> vertDataVec;
    vertDataVec.resize(sizeof(CubeVerts) / sizeof(GLfloat));
    memcpy(vertDataVec.data(),CubeVerts,sizeof(CubeVerts)); 

    std::vector<GLfloat> uvDataVec;
    uvDataVec.resize(sizeof(CubeUV) / sizeof(GLfloat));
    memcpy(uvDataVec.data(),CubeUV,sizeof(CubeUV));

    CollisionVerts = vertDataVec;
    CollisionIndices = CubeIndices;

    doGenerateMipmap = true;
    setIndices(CubeIndices);
    setModel(vertDataVec);
  auto image = cv::imread("assets/rubix.png");
  cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    setTex(image, uvDataVec);
    GLInit();

    //TCount = 12;
    isEnclosed = true;
    Info.Title = "cube";
}
/*
myCube::myCube(myCube *_new)
{
    *this = *_new;
}*/

/*bool RaycastRotatedCube(const glm::vec3& cubeExtents, const glm::mat4& cubeTransform, const glm::vec3& rayOrigin, const glm::vec3& rayDir)
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
}*/
