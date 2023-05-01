#include "cube.h"

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

myCube::myCube()
{
    SDL_Surface *icon = SDL_LoadBMP("assets/uvtemplate.bmp");
    if (icon == NULL)
    {
        std::cout << "Failed to load image" << std::endl;
        return;
    }


    glGenTextures(1, &texbuff);
    glBindTexture(GL_TEXTURE_2D, texbuff);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    GLuint format, iformat;
    if (icon->format->BitsPerPixel == 32)
    {
        format = GL_BGRA;
        iformat = GL_RGBA;
    }
    else
    {
        format = GL_BGR;
        iformat = GL_RGB;
    }

    glTexImage2D( GL_TEXTURE_2D, 0, iformat, icon->w, icon->h, 0, format, GL_UNSIGNED_BYTE, icon->pixels );

    SDL_FreeSurface(icon);

    glGenerateMipmap(GL_TEXTURE_2D);

    // Generate a Vertex Buffer Object to represent the cube's vertices
    glGenBuffers(1,&vertbuff);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertData), vertData, GL_STATIC_DRAW);

    // Generate a Vertex Buffer Object to represent the cube's colors
    glGenBuffers(1, &uvbuff);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(imgUV), imgUV, GL_STATIC_DRAW);

            // Set vertices to location 0 - GLSL: layout(location = 0) in vec3 aPos;
    glBindBuffer(GL_ARRAY_BUFFER, vertbuff);
    glVertexAttribPointer(
        0,                  // location
        3,                  // size (per vertex)
        GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
        GL_FALSE,           // is normalized*
        0,                  // stride**
        (void*)0            // array buffer offset
    );

    // Set colors to location 1 - GLSL: layout(location = 1) in vec3 aColor;
    glBindBuffer(GL_ARRAY_BUFFER, uvbuff);
    glVertexAttribPointer(
        1,                  // location
        2,                  // size (per vertex)
        GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
        GL_FALSE,           // is normalized*
        0,                  // stride**
        (void*)0            // array buffer offset
    );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texbuff);

    shader.reset( new _shader() );
    // Load and compile the basic demo shaders, returns true if error
    if ( shader->load("assets/basic_textured.vert","assets/basic_textured.frag") )
    {
        std::cout << "Failed to load shaders!" << std::endl << shader->getErrors() << std::endl;
        return;
    }
}

myCube::myCube(myCube *temp)
{
    memcpy(this,temp,sizeof(myCube));
}

myCube::~myCube()
{
    ;
}
