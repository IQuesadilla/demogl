#include "scene.h"

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <list>
#include <chrono>

GLScene::GLScene()
{
    selectClosest = false;
}

void GLScene::Draw(float deltaTime, std::shared_ptr<Camera> camera)
{
    std::list<std::pair<std::shared_ptr<Renderable>, float> > sorted;
    std::pair<std::shared_ptr<Renderable>, float> closestHovered
        = std::make_pair(nullptr, 1000.0f);

    glm::mat4 projection = camera->GetProjectionMatrix(0.01f,100.0f);
    glm::mat4 view = camera->GetViewMatrix();

    for (auto &cube : renderables)
    {
        cube.second->flags.isHovered = cube.second->raycastAABB(camera->Position, camera->Front);
        if (cube.second->flags.isHovered)
        {
            float dis = cube.second->distance(camera->Position);
            if ( dis < closestHovered.second )
            {
                closestHovered = std::make_pair(cube.second,dis);
            }
        }
    }

    if (closestHovered.second < 1000.0f)
    {
        closestHovered.first->flags.isClosest = true;

        if ( selectClosest )
        {
            closestHovered.first->select();
            selectClosest = false;
        }
    }

    for (auto &cube : renderables)
    {
        if (cube.second->alpha == 1.0f && !cube.second->flags.isClosest)
            cube.second->render(projection,view,deltaTime);
        else
        {
            bool isInserted = false;
            float dis = cube.second->distance(camera->Position);
            for (auto it = sorted.begin(); it != sorted.end(); it++)
            {
                if ( dis > it->second )
                {
                    sorted.insert( it, std::make_pair(cube.second,dis) );
                    isInserted = true;
                    break;
                }
            }

            if ( !isInserted )
            {
                sorted.emplace_back(std::make_pair(cube.second,dis));
            }
        }
    }

    //glDepthMask( GL_FALSE );
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (auto &cube : sorted)
    {
        cube.first->render(projection,view,deltaTime);
    }
    glDisable(GL_BLEND);
}

GLScene::~GLScene()
{
    ;
}