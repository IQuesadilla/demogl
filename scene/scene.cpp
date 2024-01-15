#include "scene.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <list>
#include <stack>
#include <chrono>

#include "assets/rawcube.h"

void ImPrintMat(glm::mat4 matrix)
{
  ImGui::Text("%6.3f,%6.3f,%6.3f,%6.3f",matrix[0][0],matrix[1][0],matrix[2][0],matrix[3][0]);
  ImGui::Text("%6.3f,%6.3f,%6.3f,%6.3f",matrix[0][1],matrix[1][1],matrix[2][1],matrix[3][1]);
  ImGui::Text("%6.3f,%6.3f,%6.3f,%6.3f",matrix[0][2],matrix[1][2],matrix[2][2],matrix[3][2]);
  ImGui::Text("%6.3f,%6.3f,%6.3f,%6.3f",matrix[0][3],matrix[1][3],matrix[2][3],matrix[3][3]);
}

GLScene::GLScene(libQ::log _logobj)
{
  logobj = _logobj;
  logobj.setClass("GLScene");
  logobj("GLScene");
  Init();
  //if (run_init) GLInit();
}

void GLScene::Init()
{
  logobj("Init");
  selectClosest = false;
  //SkyboxTexID = 0;
  //SkyboxVAO = 0;
  //SkyboxVAO();
  //SkyboxVBO = 0;
  //AABBVAO = 0;
  //AABBVAO();
  //AABBVBO = 0;
  DefaultShader = nullptr;
  SkyboxShader = nullptr;
  Info.ImpliedTransform = glm::mat4(1.f);
}

void GLScene::ImportWorldOptions(
  SharedVAO ImportSkyboxVAO, SharedVAO ImportAABBVAO,
  SharedTex ImportSkyboxTex,
//  SharedVBO ImportSkyboxVBO, SharedVBO ImportAABBVBO,
  std::shared_ptr<_shader> ImportSkyboxShader, std::shared_ptr<_shader> ImportDefaultShader)
{
  logobj("ImportWorldOptions");
  if (!SkyboxVAO) SkyboxVAO = ImportSkyboxVAO;
  if (!SkyboxTexID) SkyboxTexID = ImportSkyboxTex;
//  SkyboxVBO = ImportSkyboxVBO;
  if (!SkyboxShader) SkyboxShader = ImportSkyboxShader;
  //if (!AABBVAO) AABBVAO = ImportAABBVAO;
//  AABBVBO = ImportAABBVBO;
  if (!DefaultShader) DefaultShader = ImportDefaultShader;
  //DefaultShader.reset(new _shader);
  //SkyboxShader.reset(new _shader);
  //DefaultShader->load("assets/solid_color.vert","assets/solid_color.frag");
  //SkyboxShader->load("assets/skybox.vert","assets/skybox.frag");
}

void GLScene::GLInit()
{
  auto log = logobj("GLInit");
  std::chrono::steady_clock::time_point BeginInit = std::chrono::steady_clock::now(); 
  if (SkyboxShader)
  {
    SkyboxShader->compile();
    log << "Skybox shader compile took " <<
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginInit).count()
      << " milliseconds" << libQ::VALUEVV;
    BeginInit = std::chrono::steady_clock::now();
  }

  if (!SkyboxVAO)
  {
    SkyboxVAO.Generate();
    auto image = cv::imread("assets/skybox.png");
    log << "Skybox image load took " <<
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginInit).count()
      << " milliseconds" << libQ::VALUEVV;
    BeginInit = std::chrono::steady_clock::now();
 
    UpdateSkybox(cv::imread("assets/skybox.png"));
    log << "UpdateSkybox took " <<
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginInit).count()
      << " milliseconds" << libQ::VALUEVV;
    BeginInit = std::chrono::steady_clock::now();
  }

  if (DefaultShader)
  {
    DefaultShader->compile();
    log << "AABB shader compile took " <<
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginInit).count()
      << " milliseconds" << libQ::VALUEVV;
    BeginInit = std::chrono::steady_clock::now();
    //std:cout << DefaultShader.getErrors();
  }

  if (!AABBVAO)
  {
    AABBVAO.Generate();
    log << "AABB VAO generated" << libQ::NOTEVV;
  }

  for (auto &x : models)
  {
    //x.second->shader = ImportedScene.second->DefaultShader;
    if (!x.second->shader) x.second->shader = DefaultShader;
    x.second->GLInit();
  }
  log << "All models GLInit() took " <<
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginInit).count()
    << " milliseconds" << libQ::VALUEV;
}

void GLScene::Draw(float deltaTime, std::shared_ptr<Camera> camera)
{
  auto log = logobj("Draw",libQ::DELAYPRINTFUNCTION);
  std::list<std::pair<std::shared_ptr<Renderable>, float> > sorted;
  std::pair<std::shared_ptr<Renderable>, float> closestHovered
    = std::make_pair(nullptr, 1000.0f);
  bool FoundClosestHovered = false;

  glm::mat4 projection = camera->ProjectionMatrix;
  glm::mat4 view = camera->GetViewMatrix();
  glm::mat4 view_projection = projection * view;
    
  enum
  {
    None,
    SceneEditor,
    //ModelManager,
    ModelInfo,
    SceneInfo
  } CurrentDebugSceneTab;
  CurrentDebugSceneTab = None;

  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
  if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
  {
    if (ImGui::BeginTabItem("Scene Editor"))
    {
      CurrentDebugSceneTab = SceneEditor;
      ImGui::EndTabItem();
    }
    /*if (ImGui::BeginTabItem("Model Manager"))
    {
      CurrentDebugSceneTab = ModelManager;
      ImGui::EndTabItem();
    }*/
    if (ImGui::BeginTabItem("Model Info"))
    {
      CurrentDebugSceneTab = ModelInfo;
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Scene Info"))
    {
      CurrentDebugSceneTab = SceneInfo;
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
/*
    if (CurrentDebugSceneTab == ModelInfo)
    {
        ImGui::BeginChild("ModelInfoPane", ImVec2(ImGui::GetContentRegionAvail().x, 0), 0, 0);
        for (auto &model : models)
        {
            ImGui::SeparatorText(model.first.c_str());
            ImGui::Text("Name: \"%s\"", model.second->name.c_str());
            ImGui::Text("Visual Triangle Count: %d", model.second->TCount);
            ImGui::Text("Hitbox Vertex Count: %zu", model.second->CollisionVerts.size() / 3);
            ImGui::Text("Hitbox Index Count: %zu", model.second->CollisionIndices.size());
            ImGui::Text("Is Enclosed? %s", model.second->isEnclosed ? "true" : "false");
            ImGui::Text("Uses Mipmaps? %s", model.second->doGenerateMipmap ? "true" : "false");
            ImGui::Text("Vertex Array Object: %d", model.second->VAO);
            ImGui::Text("Vertex Buffer Object: %d", model.second->vertbuff);
            ImGui::Text("Element Buffer Object: %d", model.second->ibuff);
            ImGui::Text("Shader ID: %p", model.second->shader.get());
        }
        ImGui::EndChild();
    }*/

  if (CurrentDebugSceneTab == SceneInfo)
  {
    ImGui::BeginChild("SceneInfoPane", ImVec2(ImGui::GetContentRegionAvail().x, 0), 0, 0);
    ImGui::Text("Title: %s", Info.Title.c_str());
    ImGui::Text("Revision: %s", Info.Revision.c_str());
    ImGui::Text("Author: %s", Info.Author.c_str());
    ImGui::Text("Authoring Tool: %s", Info.AuthoringTool.c_str());
    ImGui::Text("Created: %s", Info.Created.c_str());
    ImGui::Text("Modified: %s", Info.Modified.c_str());
    ImGui::Text("Skybox VAO: %d", (int)SkyboxVAO);
    ImGui::Text("Skybox VBO: %d", (int)SkyboxVBO);
    ImGui::Text("Skybox Texture ID: %d", (int)SkyboxTexID);
    ImGui::Text("Skybox Shader ID: %p", SkyboxShader.get());
    ImGui::Text("AABB VAO: %d", (int)AABBVAO);
    ImGui::Text("AABB VBO: %d", (int)AABBVBO);
    ImGui::Text("AABB Shader ID: %p", DefaultShader.get());
    ImGui::Text("Default World Transform:");
    ImPrintMat(Info.ImpliedTransform);
    ImGui::EndChild();
  }

  // Update all of the models in the scene once
  if (CurrentDebugSceneTab == ModelInfo) ImGui::BeginChild("ModelInfoPane");
  for (auto &model : models)
  { 
    if (CurrentDebugSceneTab == ModelInfo)
    {
      //ImGui::BeginChild("ModelInfoPane");
      ImGui::SeparatorText(model.first.c_str());
      ImGui::Text("Name: \"%s\"", model.second->Info.Title.c_str());
      ImGui::Text("Model ID: %p", model.second.get());
      ImGui::Text("Visual Triangle Count: %d", model.second->TCount);
      ImGui::Text("Hitbox Vertex Count: %zu", model.second->CollisionVerts.size() / 3);
      ImGui::Text("Hitbox Index Count: %zu", model.second->CollisionIndices.size()); 
      ImGui::Text("Is Enclosed? %s", model.second->isEnclosed ? "true" : "false");
      ImGui::Text("Uses Mipmaps? %s", model.second->doGenerateMipmap ? "true" : "false");
      ImGui::Text("Vertex Array Object: %d", (int)model.second->VAO);
      ImGui::Text("Vertex Buffer Object: %d", (int)model.second->vertbuff);
      ImGui::Text("Element Buffer Object: %d", (int)model.second->ibuff);
      ImGui::Text("Shader ID: %p", model.second->shader.get()); 
      //ImGui::EndChild();
    }
    for (auto &cube : renderables)
      if (cube.first->_model == model.second)
      {
        model.second->update(CurrentDebugSceneTab == ModelInfo);
        break;
      }
  }
  if (CurrentDebugSceneTab == ModelInfo) ImGui::EndChild();

  if (CurrentDebugSceneTab == SceneEditor)
  {
    if (ImGui::Button("New Object:"))
      ImGui::OpenPopup("NewModelPopup");
    ImGui::SameLine();
    ImGui::Text("Object Count: %zu",renderables.size());

    if (ImGui::BeginPopup("NewModelPopup"))
    {
      for (auto &model : models)
      {
        if ( ImGui::Selectable( model.first.c_str() ) )
        {
          std::shared_ptr<Renderable> newnode(new Renderable( model.second ));
          renderables[newnode] = std::vector<std::shared_ptr<Renderable>>();
          SceneBase.push_back(newnode);
        }
      }
      ImGui::EndPopup();
    }
  }

  int CollisionCount = 0;
  for (auto j : renderables)
    j.first->Collisions.clear();

  std::stack<std::shared_ptr<Renderable>> UpcomingNodes;
  std::stack<bool> OpenedStack;
  std::stack<std::shared_ptr<Renderable>> CurrentParents;
    
  static ImGuiTableFlags tflags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
  static ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_OpenOnArrow;// | ImGuiTreeNodeFlags_SpanFullWidth;
  if (CurrentDebugSceneTab == SceneEditor && ImGui::BeginTable("3ways", 3, tflags))
  {
    // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoResize);
    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::CalcTextSize("0x000000000000").x);
    ImGui::TableSetupColumn("Sel", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::CalcTextSize("false").x);
    ImGui::TableHeadersRow();
    OpenedStack.push(true);
  }
  else OpenedStack.push(false);

  std::shared_ptr<Renderable> dragpnode, dragnode = nullptr;
  bool isMouseOnLeftHalf = false, draghovered = false;

  for (auto &node : SceneBase)
    UpcomingNodes.push(node);

  while (!UpcomingNodes.empty())
  {
    auto node = UpcomingNodes.top();
    UpcomingNodes.pop();
    if (node == nullptr)
    {
      if (OpenedStack.top()) ImGui::TreePop();
      OpenedStack.pop();
      CurrentParents.pop();
      continue;
    }

    ImGui::PushID(node.get());

    glm::mat4 ParentMatrix;
    if (CurrentParents.size() == 0) ParentMatrix = Info.ImpliedTransform;
    else ParentMatrix = CurrentParents.top()->_modelmatrix;

    node->AnimationUpdate(deltaTime, ParentMatrix);

    for (auto k : renderables)
      if (node != k.first && k.first->flags.isCollisionUpdated && 
        node->negAABB.x < k.first->posAABB.x && k.first->negAABB.x < node->posAABB.x &&
        node->negAABB.y < k.first->posAABB.y && k.first->negAABB.y < node->posAABB.y &&
        node->negAABB.z < k.first->posAABB.z && k.first->negAABB.z < node->posAABB.z)
      {
        bool found = false;
        for (auto &x : node->Collisions)
          if (x == k.first) found = true;
        if (found) continue;
        for (auto &x : k.first->Collisions)
          if (x == node) found = true;
        if (found) continue;

        // This only checks is AABB's overlap
        // All Hitbox checks should happen during "Collide"

        CollisionCount++;
        node->Collisions.push_back(k.first);
        node->Collide(k.first);
          //if (CurrentDebugSceneTab == SceneEditor)
          //    ImGui::Text("Collision: %p and %p", j.first.get(), k.first.get());
      }

    node->genModelMatrix(ParentMatrix);

    // Find the nearest object in line with the camera
    node->flags.isHovered = false;
    float dis = node->raycastAABB(camera->Position, camera->Front);
    if (dis > 0.0f && dis < closestHovered.second)
    {
      closestHovered = std::make_pair(node,dis);
      FoundClosestHovered = true;
    }

    auto children = renderables[node];

    if (children.size() > 0)
    {
      UpcomingNodes.push(nullptr);
      CurrentParents.push(node);
      for (auto &child_n : children)
        UpcomingNodes.push(child_n);
      if (!OpenedStack.top()) OpenedStack.push(false);
    }

    if (CurrentDebugSceneTab == SceneEditor && OpenedStack.top())
    {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      if (children.size() > 0)
      {
        if (OpenedStack.top())
        {
          bool open = ImGui::TreeNodeEx(node->_model->Info.Title.c_str(), tree_node_flags);
          OpenedStack.push(open);
        }
      }
      else
      {
        ImGui::TreeNodeEx(node->_model->Info.Title.c_str(), tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen );
      }

      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MyTree"))
        {
          dragnode = *((std::shared_ptr<Renderable>*)payload->Data);
          dragpnode = node;
        }
        draghovered = true;
        ImGui::EndDragDropTarget();
      }

      if (ImGui::BeginDragDropSource())
      {
        ImGui::SetDragDropPayload("MyTree", &node, sizeof(void*));
        ImGui::Text("%p",node.get());
        ImGui::EndDragDropSource();
      }
      else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsItemHovered() && !itemWasToggledOpen && !draghovered)
      {
        DebugSelectRenderable(node);
      }

      if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        itemWasToggledOpen = ImGui::IsItemToggledOpen();

      if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
        itemWasToggledOpen = false;

      ImGui::TableNextColumn();
      ImGui::Text("%p",node.get());
      ImGui::TableNextColumn();
      ImGui::Text(node->flags.isSelected ? "true" : "false");

      if (draghovered)
      {
        ImGui::BeginDisabled();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("->");
        ImGui::TableNextColumn();

        isMouseOnLeftHalf = ImGui::GetMousePos().x < ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x * 0.5f;
        if (isMouseOnLeftHalf)  ImGui::TextUnformatted("Sibling");
        else                    ImGui::TextUnformatted("Child");

        draghovered = false;
        ImGui::EndDisabled();
      }
    }

    ImGui::PopID();
  }

  if (dragnode && !CheckCascadingChild(dragnode,dragpnode))
  {
    auto oldpnode = FindSiblingVectorOfChild(dragnode);
    oldpnode.first->erase(oldpnode.second);
    if (isMouseOnLeftHalf)
    {
      auto newpnode = FindSiblingVectorOfChild(dragpnode);
      newpnode.first->push_back(dragnode);
    }
    else
    {
      renderables[dragpnode].push_back(dragnode);
    }
  }

  if (CurrentDebugSceneTab == SceneEditor)
  {
    ImGui::EndTable();
    if (CollisionCount > 0)
      ImGui::Text("Collision Count: %d", CollisionCount);
  }

  // Mark the nearest object in line with the camera as hovered and optionally select
  if (FoundClosestHovered)
  {
    closestHovered.first->flags.isHovered = true;
    if ( selectClosest ) DebugSelectRenderable(closestHovered.first);
  }
  selectClosest = false;

  bool DoSelectionDraw = (CurrentDebugSceneTab == SceneEditor) || (CurrentDebugSceneTab == None);
  if (DoSelectionDraw) ImGui::BeginChild("SelectedObjects", ImVec2(ImGui::GetContentRegionAvail().x, 0), 0,0);//ImGuiChildFlags_ResizeY, 0);

  // Either render an object or sort it by distance
  for (auto &cube : renderables)
  {
    if (DoSelectionDraw && cube.first->flags.isSelected)
    {
      DebugDrawAABB(cube.first, view_projection, camera->Position);
      //ImGui::Text("Implied Transform:");
      //ImPrintMat(cube.first->Info.ImpliedTransform);
      //ImGui::Text("Model Matrix:");
      //ImPrintMat(cube.first->_modelmatrix);
    }
    else cube.first->flags.isSelected = false;
 
    if (cube.first->alpha == 1.0f)
      cube.first->render(view_projection);
    else
    {
      bool isInserted = false;
      float dis = cube.first->distance(camera->Position);
      for (auto it = sorted.begin(); it != sorted.end(); it++)
      {
        if ( dis > it->second )
        {
          sorted.insert( it, std::make_pair(cube.first,dis) );
          isInserted = true;
          break;
        }
      }

      if ( !isInserted )
      {
        sorted.emplace_back(std::make_pair(cube.first,dis));
      }
    }
  }

  // Draw the skybox after the solid objects
  if (SkyboxShader && SkyboxVAO)
  {
    glBindVertexArray(SkyboxVAO);
    SkyboxShader->use();
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxTexID);
    SkyboxShader->setMat4("view", glm::mat4(glm::mat3(view)));
    SkyboxShader->setMat4("projection", projection);
    SkyboxShader->setInt("samplerCube", 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  // Draw objects with alpha
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (auto &cube : sorted)
    cube.first->render(view_projection);
  glDisable(GL_BLEND);

  if (DoSelectionDraw) ImGui::EndChild();
}

void GLScene::UpdateSkybox(cv::Mat skybox)
{
  auto log = logobj("UpdateSkybox");
  //if (SkyboxVAO == 0)
  //{
  //  glGenVertexArrays(1, &SkyboxVAO);
  //  std:cout << "Generating VAO" << std::endl;
  //}
  glBindVertexArray(SkyboxVAO);

  log << "SKYBOX VAO: " << (int)SkyboxVAO << libQ::VALUEDEBUG;

  if (SkyboxVBO == 0)
  {
    SkyboxVBO.Generate();
    // Generate a Vertex Buffer Object to represent the cube's vertices
    //glGenBuffers(1,&SkyboxVBO);
    // Set vertices to location 0 - GLSL: layout(location = 0) in vec3 aPos;
    glBindBuffer(GL_ARRAY_BUFFER, SkyboxVBO);
    glVertexAttribPointer(
      0,                  // location
      3,                  // size (per vertex)
      GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
      GL_FALSE,           // is normalized*
      0,                  // stride**
      (void*)0            // array buffer offset
    );
    glEnableVertexAttribArray(0);

    std::vector<GLfloat> vertDataVec;
    vertDataVec.resize(sizeof(vertData) / sizeof(GLfloat));
    memcpy(vertDataVec.data(),vertData,sizeof(vertData)); 

    for (auto &v : vertDataVec)
      v *= 500.0f;

    glBindBuffer(GL_ARRAY_BUFFER, SkyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, vertDataVec.size() * sizeof(GLfloat), vertDataVec.data(), GL_STATIC_DRAW);
  }

  if (SkyboxTexID == 0)
  {
    SkyboxTexID.Generate();
    //glGenTextures(1, &SkyboxTexID);
  }
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxTexID);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  cv::cvtColor(skybox, skybox, cv::COLOR_BGR2RGB);

  float w = skybox.cols, h = skybox.rows;

  std::vector<cv::Mat> faces(6);
  faces[0] = cv::Mat(skybox, cv::Rect(w*0.50f,h*(1.f/3),w*0.25f,h*(1.f/3)));
  faces[1] = cv::Mat(skybox, cv::Rect(w*0.00f,h*(1.f/3),w*0.25f,h*(1.f/3)));
  faces[2] = cv::Mat(skybox, cv::Rect(w*0.25f,h*(0.f/3),w*0.25f,h*(1.f/3)));
  faces[3] = cv::Mat(skybox, cv::Rect(w*0.25f,h*(2.f/3),w*0.25f,h*(1.f/3)));
  faces[5] = cv::Mat(skybox, cv::Rect(w*0.75f,h*(1.f/3),w*0.25f,h*(1.f/3)));
  faces[4] = cv::Mat(skybox, cv::Rect(w*0.25f,h*(1.f/3),w*0.25f,h*(1.f/3)));

  for(unsigned int i = 0; i < 6; i++)
  {
    cv::Mat face = faces[i];
    if (!face.isContinuous())
      face = face.clone(); // Ensure the data is continuous

    //cv::imshow("SkyboxFace" + std::to_string(i),face);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data );
    glTexImage2D(
      GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
      0, GL_RGB, face.cols, face.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, face.data
    );
  }
}

GLScene::~GLScene()
{
  auto log = logobj("~GLScene");
}

void GLScene::ImportScene(GLScene *scene)
{
  auto log = logobj("ImportScene");
  ImportModelsFrom(scene);
  if (SceneBase.size() > 0)
  {
    std::shared_ptr<Renderable> TopNode;
    auto ModelIT = models.find("blank");
    std::shared_ptr<Model> ModelPTR;
    if (ModelIT == models.end())
    {
      ModelPTR.reset(new Model());
      models.emplace("blank",ModelPTR);
    } else ModelPTR = ModelIT->second;
    TopNode.reset(new Renderable(ModelPTR));
    std::vector<std::shared_ptr<Renderable>> ChildVector;
    ImportRenderablesFromInto(scene,&ChildVector);
    renderables.emplace(TopNode,ChildVector);
    SceneBase.push_back(TopNode);
  }
  else ImportRenderablesFrom(scene);
  shaders.insert(scene->shaders.begin(),scene->shaders.end());
  //AABBVAO = scene->AABBVAO;
  //SkyboxVAO = scene->SkyboxVAO;
  if (Info.Author.length() > 0) Info.Author.append(" ; ");
  Info.Author.append(scene->Info.Author);
  Info.ImpliedTransform *= scene->Info.ImpliedTransform;
  log << "Successfully Imported Scene" << libQ::NOTEV;
}

void GLScene::ImportModelsFrom(GLScene *scene)
{
  auto log = logobj("ImportModelsFrom");
  models.insert(scene->models.begin(), scene->models.end());
  log << "Successfully imported all models from scene" << libQ::NOTEVV;
}

void GLScene::ImportRenderablesFrom(GLScene *scene)
{
  ImportRenderablesFromInto(scene, &SceneBase);
}

void GLScene::ImportRenderablesFromInto(GLScene *scene, std::vector<std::shared_ptr<Renderable>> *ChildVector)
{
  auto log = logobj("ImportRenderablesFromInto");
  renderables.insert(scene->renderables.begin(),scene->renderables.end());
  ChildVector->insert(ChildVector->end(),scene->SceneBase.begin(),scene->SceneBase.end());
  log << "Successfully imported all renderables from scene" << libQ::NOTEVV;
} 

std::pair<
  std::vector<std::shared_ptr<Renderable>>*,
  std::vector<std::shared_ptr<Renderable>>::iterator>
  GLScene::FindSiblingVectorOfChild(std::shared_ptr<Renderable> child)
{
  auto log = logobj("FindSiblingVectorOfChild",libQ::DELAYPRINTFUNCTION);
  bool isChild = false;
  std::pair<
    std::vector<std::shared_ptr<Renderable>>*,
    std::vector<std::shared_ptr<Renderable>>::iterator>
  siblingvector;

  for (auto &potential : renderables)
  {
    auto itt = std::find(potential.second.begin(),potential.second.end(),child);
    if (itt != potential.second.end())
    {
      isChild = true;
      siblingvector.first = &potential.second;
      siblingvector.second = itt;
      break;
    }
  }

  if (!isChild)
  {
    auto pos = std::find(SceneBase.begin(),SceneBase.end(),child);
    if (pos != SceneBase.end())
    {
      siblingvector.first = &SceneBase;
      siblingvector.second = pos;
    }
  }

  return siblingvector;
}

bool GLScene::CheckCascadingChild(std::shared_ptr<Renderable> parent, std::shared_ptr<Renderable> child)
{
  for (auto &node : renderables[parent])
  {
    if (node == child || CheckCascadingChild(node,child))
      return true;
  }
  return false;
}

void GLScene::DebugSelectRenderable(std::shared_ptr<Renderable> renderable)
{
  bool tempSelect = renderable->flags.isSelected;
  for (auto &cube : renderables)
    cube.first->flags.isSelected = false;
  renderable->flags.isSelected = !tempSelect;
}

void GLScene::DebugDrawAABB(std::shared_ptr<Renderable> renderable, glm::mat4 view_projection, glm::vec3 CameraPos)
{
  auto log = logobj("DebugDrawAABB",libQ::DELAYPRINTFUNCTION);
  std::array<std::array<glm::vec3,2>,12> lines = {{
    // Front face
    {glm::vec3
    {renderable->negAABB.x, renderable->negAABB.y, renderable->posAABB.z},
    {renderable->posAABB.x, renderable->negAABB.y, renderable->posAABB.z},
    },

    {glm::vec3
    {renderable->posAABB.x, renderable->negAABB.y, renderable->posAABB.z},
    {renderable->posAABB.x, renderable->posAABB.y, renderable->posAABB.z},
    },

    {glm::vec3
    {renderable->posAABB.x, renderable->posAABB.y, renderable->posAABB.z},
    {renderable->negAABB.x, renderable->posAABB.y, renderable->posAABB.z},
    },

    {glm::vec3
    {renderable->negAABB.x, renderable->posAABB.y, renderable->posAABB.z},
    {renderable->negAABB.x, renderable->negAABB.y, renderable->posAABB.z},
    },

    // Back face
    {glm::vec3
    {renderable->negAABB.x, renderable->negAABB.y, renderable->negAABB.z},
    {renderable->posAABB.x, renderable->negAABB.y, renderable->negAABB.z},
    },

    {glm::vec3
    {renderable->posAABB.x, renderable->negAABB.y, renderable->negAABB.z},
    {renderable->posAABB.x, renderable->posAABB.y, renderable->negAABB.z},
    },

    {glm::vec3
    {renderable->posAABB.x, renderable->posAABB.y, renderable->negAABB.z},
    {renderable->negAABB.x, renderable->posAABB.y, renderable->negAABB.z},
    },

    {glm::vec3
    {renderable->negAABB.x, renderable->posAABB.y, renderable->negAABB.z},
    {renderable->negAABB.x, renderable->negAABB.y, renderable->negAABB.z},
    },

    // Connecting lines
    {glm::vec3
    {renderable->negAABB.x, renderable->negAABB.y, renderable->posAABB.z},
    {renderable->negAABB.x, renderable->negAABB.y, renderable->negAABB.z},
    },

    {glm::vec3
    {renderable->posAABB.x, renderable->negAABB.y, renderable->posAABB.z},
    {renderable->posAABB.x, renderable->negAABB.y, renderable->negAABB.z},
    },

    {glm::vec3
    {renderable->posAABB.x, renderable->posAABB.y, renderable->posAABB.z},
    {renderable->posAABB.x, renderable->posAABB.y, renderable->negAABB.z},
    },

    {glm::vec3
    {renderable->negAABB.x, renderable->posAABB.y, renderable->posAABB.z},
    {renderable->negAABB.x, renderable->posAABB.y, renderable->negAABB.z},
    },
  }};

    // Sort the lines using a lambda function
    /*
    std::sort(lines.begin(), lines.end(), [&CameraPos](const std::array<glm::vec3, 2>& line1, const std::array<glm::vec3, 2>& line2) {
        auto distanceToCamera = [&CameraPos](const std::array<glm::vec3, 2>& line) {
            //return glm::max(glm::distance(line[0], CameraPos),glm::distance(line[1], CameraPos));
            return glm::distance(line[0], CameraPos) + glm::distance(line[1], CameraPos);
        };

        return distanceToCamera(line1) > distanceToCamera(line2);
    });*/

  glm::vec3 AABBCenter = (renderable->posAABB + renderable->negAABB) / 2.0f;

  std::vector<glm::vec3> trianglelines;
  for (auto it = lines.begin(); it < lines.end(); ++it)
  {
    // Given a line segment from point A to point B
    glm::vec3 A = (*it)[0]; // Starting point of the line
    glm::vec3 B = (*it)[1]; // Ending point of the line
    float lineWidth = 0.1f; // Desired line width
    float HalfLineWidth = lineWidth / 2.0f;

    // Compute the direction vector of the line and a perpendicular vector
    glm::vec3 CameraDir = glm::normalize(glm::vec3(glm::vec3(B + A) * 0.5f) - (CameraPos));
    glm::vec3 direction = glm::normalize(B - A);
    glm::vec3 perp = glm::normalize(glm::cross(direction, CameraDir)); // Assuming Z is up
    perp *= HalfLineWidth;

    A = A + glm::normalize(A - AABBCenter) * HalfLineWidth;// - CameraDir * lineWidth;
    B = B + glm::normalize(B - AABBCenter) * HalfLineWidth;// - CameraDir * lineWidth;

    // Calculate the vertices of the quad
    std::array<glm::vec3,6> vertices = {
      A - perp, B - perp, B + perp,
      B + perp, A + perp, A - perp
    };

    trianglelines.insert(trianglelines.end(),vertices.begin(),vertices.end());
    //fadeindex.insert(fadeindex.end(),SingleFadeIndex.begin(),SingleFadeIndex.end());
  }

  //if (!AABBVAO)
  //{
  //  glGenVertexArrays(1, &AABBVAO);
  //}
  glBindVertexArray(AABBVAO);

  if (!AABBVBO)
  {
    AABBVBO.Generate();
    // Generate a Vertex Buffer Object to represent the cube's vertices
    //glGenBuffers(1,&AABBVBO);
    // Set vertices to location 0 - GLSL: layout(location = 0) in vec3 aPos;
    glBindBuffer(GL_ARRAY_BUFFER, AABBVBO);
    glVertexAttribPointer(
      0,                  // location
      3,                  // size (per vertex)
      GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
      GL_FALSE,           // is normalized*
      0,                  // stride**
      (void*)0            // array buffer offset
    );
/*
        glBindBuffer(GL_ARRAY_BUFFER, AABBVBO[1]);
        glVertexAttribPointer(
            1,                  // location
            2,                  // size (per vertex)
            GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
            GL_FALSE,           // is normalized*
            0,                  // stride**
            (void*)0            // array buffer offset
        );*/

    glEnableVertexAttribArray(0);
    //glEnableVertexAttribArray(1);
  }
  glBindBuffer(GL_ARRAY_BUFFER, AABBVBO);
  glBufferData(GL_ARRAY_BUFFER, trianglelines.size() * sizeof(glm::vec3), trianglelines.data(), GL_STATIC_DRAW);
  //glBindBuffer(GL_ARRAY_BUFFER, AABBVBO[1]);
  //glBufferData(GL_ARRAY_BUFFER, fadeindex.size() * sizeof(glm::vec2), fadeindex.data(), GL_STATIC_DRAW);

  DefaultShader->use();
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_CULL_FACE);
  DefaultShader->setMat4("mvp", view_projection);//renderable->_modelmatrix * glm::toMat4(glm::quat(-glm::radians(renderable->rotAxis))));// glm::scale(renderable->_modelmatrix,glm::vec3(1.01f)));
  DefaultShader->setVec4("color", glm::vec4(0.0f,0.0f,0.0f,1.0f));

  glDrawArrays(GL_TRIANGLES, 0, trianglelines.size());

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    log << "AABB Wireframe Error:" << (int)err << libQ::ERROR;
  }
}
