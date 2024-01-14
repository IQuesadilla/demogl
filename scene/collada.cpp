#include "collada.h"

COLLADAScene::COLLADAScene(std::string path) : GLScene(), basicxml()
{
  LoaderIT = 0;
  isInsideTriangles = false;
  //TrianglesInputCount = 0;

  std::cout << "COLLADAScene Size: " << sizeof(COLLADAScene) << std::endl;

  ifs.open(path);
  if (ifs.is_open())
  {
    basepath = path;
    //buffersize = 100;

    ifs.clear();  // Clears any error flags
    ifs.seekg(0, std::ios::beg);

    std::chrono::steady_clock::time_point ParseBegin = std::chrono::steady_clock::now();
    parse();
    std::cout << "Finished Parsing file " << path << ", Took " <<
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - ParseBegin).count()
      << " milliseconds" << std::endl;
  } else {
    std::cout << "Failed to open file " << path << std::endl;
  }
}

int COLLADAScene::loadcallback(char *buffer, int buffsize)
{
  ifs.read(buffer, buffsize);
  int s = ifs.gcount();
  //std::cout << "Requested " << buffsize << " bytes, Read " << s << " bytes" << std::endl;
  return s;
}

void COLLADAScene::parsecallback(element e)
{
  TagOptions LoadingTag;
  std::string name(e.name);//,e.namelen);
  std::string value(e.value);//,e.valuelen);
  LoadingTag = TagLUT[name];

  //std::cout << "--> Name: " << name << ", Enum: " << LoadingTag.Name << std::endl;
  //if (e.valuelen > 0) std::cout << "--> Value: {" << value << "}" << std::endl;
  //std::cout << "--> Closing: " << e.isClosing << ", Standalone: " << e.isStandalone << ", First: " << e.isFirst << std::endl;
  if (!e.isClosing) {
    std::unordered_map<std::string,std::string> Arguments;
    for (auto arg = e.atts; arg != nullptr; arg = arg->next)
      Arguments[std::string(arg->name)/*,arg->namelen)*/] = std::string(arg->value);//,arg->valuelen);

    switch (LoadingTag)
    {
    case Accessor:
      SourcesArray[CurrentLoadingArray].AccessorStride = std::stoi(Arguments["stride"]);
    case Asset:
      //LoadingTag.CurrentAsset = CurrentTags.top().CurrentAsset;
      break;
    case Author:
      CurrentAsset->Author.append(value);
      break;
    case AuthoringTool:
      CurrentAsset->AuthoringTool.append(value);
    case COLLADA:
      CurrentAsset = &Info;
      break;
    case Contributor:
      //LoadingTag.CurrentAsset = CurrentTags.top().CurrentAsset;
      break;
    case Coverage:
      //LoadingTag.CurrentAsset = CurrentTags.top().CurrentAsset;
      break;
    case Created:
      CurrentAsset->Created.append(value);
      break;
    case FloatArray:
      {
        std::string ArrayID = Arguments["id"];
        if (ArrayID.length() > 0) CurrentLoadingArray = ArrayID;

        auto *array = &SourcesArray[CurrentLoadingArray].FloatArray;
 
        std::string LengthString = Arguments["count"];
        if (LengthString.length() > 0)
        {
          int FloatCount = std::stoi(LengthString);
          std::cout << "Array: " << CurrentLoadingArray << ", FloatCount: " << FloatCount << std::endl;
          array->reserve(FloatCount);
        }

        std::istringstream ss(value);
        float number;
        while (ss >> number)
        {
          ++LoaderIT;
          array->push_back(number);
        }
        //std::cout << "Loaded Float Array." << std::endl;
      }
      break;
    case GeographicLocation:
      CurrentAsset->GeographicLocation.push_back(value);
      break;
    case Geometry:
      CurrentModel.reset(new Model());
      models[Arguments["id"]] = CurrentModel;
      CurrentAsset = &CurrentModel->Info;
      CurrentModel->isEnclosed = false;
      CurrentModel->Info.Title = Arguments["name"];
      CurrentModel->shader = nullptr;
    case Input:
      {
        if (isInsideTriangles) ++TrianglesInputCount;
        //std::cout << "TrianglesInputCount: " << TrianglesInputCount << std::endl;
        std::string semantic = Arguments["semantic"];
        if (semantic == "VERTEX")
        {
          std::string SourceStr = Arguments["source"];
          std::string SourceID = SourceStr.substr(1,SourceStr.length()-1);
          auto &Array = SourcesArray[SourceID].FloatArray;

          //std::cout << "Setting vertice from ID "<< SourceID << " to {" << std::flush;
          //for (auto &x : Array)
          //  std::cout << x << ',';
          //std::cout << "}" << std::endl;

          CurrentModel->setModel(Array);
          CurrentModel->CollisionVerts = Array;
          SourcesArray[SourceID].FloatArray.clear();
        } else if (semantic == "POSITION")
        {
          std::string SourceStr = Arguments["source"];
          std::string SourceID = SourceStr.substr(1,SourceStr.length()-1)+"-array";
          std::cout << "Copying " << VerticesID << " to " << SourceID << "." << std::endl;
          SourcesArray[VerticesID] = SourcesArray[SourceID];
        }
      }
      break;
    case InstanceGeometry:
    {
      auto NewNode = CurrentNode.top();
      std::string URLStr = Arguments["url"];
      std::string ModelID = URLStr.substr(1,URLStr.length()-1); 
      auto NewNodeModel = models[ModelID];
      NewNode->setModel(NewNodeModel);
    } break;
    case InstanceVisualScene:
      Info.Title.append(Arguments["url"]);
      break;
    case Keywords:
      CurrentAsset->Keywords.push_back(value);
      break;
    case Modified:
      CurrentAsset->Modified.append(value);
      break;
    case Matrix:
      if (Arguments["sid"] == "transform")
      {
        std::cout << "Transform: " << value << std::endl;
        int i = 0;
        float number;
        glm::mat4 transform;
        std::stringstream ss(value);
        while (ss >> number)
        {
          if (glm::abs(number) < 0.001f) number = 0.f;
          transform[i%4][i/4] = number;
          //std::cout << "Number [" << i/4 << "][" << i%4 << "] = " << number << std::endl;
          ++i;
        }
        CurrentNode.top()->Info.ImpliedTransform = transform;
      } break;
    case Node:
    {
      std::shared_ptr<Renderable> NewNode;
      auto ModelIT = models.find("blank");
      std::shared_ptr<Model> ModelPTR;
      if (ModelIT == models.end())
      {
        ModelPTR.reset(new Model());
        models.emplace("blank",ModelPTR);
      } else ModelPTR = ModelIT->second;
      NewNode.reset(new Renderable(ModelPTR));
      auto ParentNode = CurrentNode.top();
      if (ParentNode) renderables[ParentNode].push_back(NewNode);
      else SceneBase.push_back(NewNode);
      CurrentNode.push(NewNode);
      renderables.emplace(NewNode,std::vector<std::shared_ptr<Renderable>>());
    } break;
    case P:
    {
      uint number;
      std::istringstream data(value);
      //int i = LoaderIT;
      while (data >> number)
      {
        if (LoaderIT % TrianglesInputCount == 0)
          UIntVector.push_back(number);
        ++LoaderIT;
      }
      //LoaderIT = i;
    } break;
    case Scene:
      break;
    case Triangles:
    {
      isInsideTriangles = true;
      TrianglesInputCount = 0;
      ClaimedTriCount = std::stoi(Arguments["count"]);
      std::cout << "Clamed Triangle Count: " << ClaimedTriCount << std::endl;
    } break;
    case UpAxis:
      std::cout << "World UpAxis: \"" << value << "\", Author: " << CurrentAsset->Author << std::endl;
      if (value == "Z_UP") CurrentAsset->ImpliedTransform = glm::mat4(1.f) * glm::toMat4(glm::quat(glm::radians(glm::vec3(-90.f,0.f,0.f))));
      break;
    case Vertices:
      VerticesID = Arguments["id"];
      break;
    case VisualScene:
      for (;CurrentNode.size();CurrentNode.pop());
      CurrentNode.push(nullptr);
      break;
    default:
      // This is very important.
      // If this is run, a COLLADA feature is being used that isn't supported
      //std::cout << "Running default option for tag " << name << std::endl;
      break;
    };

    /*if (!e.isStandalone && e.isFirst)
    {
      //CurrentTags.push(LoadingTag);
      //std::cout << "Pushed, New size: " << CurrentTags.size() << std::endl;
    }*/
  } else { // ------------------------- Closing </example> --------------------------
    switch (LoadingTag)
    {
    case FloatArray:
      //std::cout << "here" << std::endl << std::endl << std::endl << std::endl;
      //std::cout << "DEBUG Float Array Print: \"" << CurrentLoadingArray << "\" = <" << std::flush;
      //for (auto &x : SourcesArray[CurrentLoadingArray].FloatArray)
      //  std::cout << x << ",";
      //std::cout << ">" << std::endl;
      std::cout << "Successfully loaded FloatArray \"" << CurrentLoadingArray << "\", Actual Size: " << LoaderIT << std::endl;
      //LoaderIT = 0;
      break;
    case Geometry:
      //CurrentModel->setModel(FloatVector);
      break;
    case Mesh:
      SourcesArray.clear();
      break;
    case Node:
      CurrentNode.pop();
      break;
    case P:
      std::cout << "P Loaded " << LoaderIT << " numbers" << std::endl;
      //LoaderIT = 0;

      //std::cout << "Setting indices to {" << std::flush;
      //for (auto &x : UIntVector)
      //  std::cout << x << ',';
      //std::cout << "}" << std::endl;
      CurrentModel->setIndices(UIntVector);
      CurrentModel->CollisionIndices = UIntVector;
      UIntVector.clear();
      break;
    case Triangles:
      //CurrentModel->TCount = FloatVector.size();
      //CurrentModel->CollisionVerts = FloatVector;
      std::cout << "Loadeded Triangles, Implied P Count: " << ClaimedTriCount*TrianglesInputCount*3 << std::endl;
      CurrentModel->Info.Author = "Me";
      //TrianglesInputCount = 0;5
      isInsideTriangles = false;
      break;
    default:
      break;
    };
    LoaderIT = 0;
    //CurrentTags.pop();
    //std::cout << "Popped, New Size: " << CurrentTags.size() << std::endl;
  }
}

std::map<std::string,COLLADAScene::TagOptions> COLLADAScene::TagLUT{
    {"acceleration", TagOptions::Acceleration},
    {"accessor", TagOptions::Accessor},
    {"active", TagOptions::Active},
    {"alpha_func", TagOptions::AlphaFunc},
    {"alpha_test_enable", TagOptions::AlphaTestEnable},
    {"alpha", TagOptions::Alpha},
    {"altitude", TagOptions::Altitude},
    {"ambient", TagOptions::Ambient}, // core & FX
    {"angle", TagOptions::Angle},
    {"angular_velocity", TagOptions::AngularVelocity},
    {"angular", TagOptions::Angular},
    {"animation_clip", TagOptions::AnimationClip},
    {"animation", TagOptions::Animation},
    {"annotate", TagOptions::Annotate},
    {"argument", TagOptions::Argument},
    {"array", TagOptions::Array},
    {"articulated_system", TagOptions::ArticulatedSystem},
    {"aspect_ratio", TagOptions::AspectRatio},
    {"asset", TagOptions::Asset},
    {"attachment_end", TagOptions::AttachmentEnd},
    {"attachment_full", TagOptions::AttachmentFull},
    {"attachment_start", TagOptions::AttachmentStart},
    {"attachment", TagOptions::Attachment},
    {"author_email", TagOptions::AuthorEmail},
    {"author_website", TagOptions::AuthorWebsite},
    {"author", TagOptions::Author},
    {"authoring_tool", TagOptions::AuthoringTool},
    {"auto_normal_enable", TagOptions::AutoNormalEnable},
    {"axis_info", TagOptions::AxisInfo},
    {"axis", TagOptions::Axis},
    {"binary", TagOptions::Binary},
    {"bind_attribute", TagOptions::BindAttribute},
    {"bind_joint_axis", TagOptions::BindJointAxis},
    {"bind_kinematics_model", TagOptions::BindKinematicsModel},
    {"bind_material", TagOptions::BindMaterial},
    {"bind_shape_matrix", TagOptions::BindShapeMatrix},
    {"bind_uniform", TagOptions::BindUniform},
    {"bind_vertex_input", TagOptions::BindVertexInput},
    {"bind", TagOptions::Bind}, // FX & kinematics
    {"blend_color", TagOptions::BlendColor},
    {"blend_enable", TagOptions::BlendEnable},
    {"blend_equation_separate", TagOptions::BlendEquationSeparate},
    {"blend_equation", TagOptions::BlendEquation},
    {"blend_func_separate", TagOptions::BlendFuncSeparate},
    {"blend_func", TagOptions::BlendFunc},
    {"blinn", TagOptions::Blinn},
    {"bool_array", TagOptions::BoolArray},
    {"border_color", TagOptions::BorderColor},
    {"box", TagOptions::Box},
    {"brep", TagOptions::Brep},
    {"camera", TagOptions::Camera},
    {"capsule", TagOptions::Capsule},
    {"channel", TagOptions::Channel},
    {"circle", TagOptions::Circle},
    {"clip_plane_enable", TagOptions::ClipPlaneEnable},
    {"clip_plane", TagOptions::ClipPlane},
    {"code", TagOptions::Code},
    {"COLLADA", TagOptions::COLLADA},
    {"color_clear", TagOptions::ColorClear},
    {"color_logic_op_enable", TagOptions::ColorLogicOpEnable},
    {"color_mask", TagOptions::ColorMask},
    {"color_material_enable", TagOptions::ColorMaterialEnable},
    {"color_material", TagOptions::ColorMaterial},
    {"color_target", TagOptions::ColorTarget},
    {"color", TagOptions::Color},
    {"comments", TagOptions::Comments},
    {"compiler", TagOptions::Compiler},
    {"cone", TagOptions::Cone},
    {"connect_param", TagOptions::ConnectParam},
    {"constant_attenuation", TagOptions::ConstantAttenuation},
    {"constant", TagOptions::Constant}, // combiner & FX
    {"contributor", TagOptions::Contributor},
    {"control_vertices", TagOptions::ControlVertices},
    {"controller", TagOptions::Controller},
    {"convex_mesh", TagOptions::ConvexMesh},
    {"copyright", TagOptions::Copyright},
    {"coverage", TagOptions::Coverage},
    {"create_2d", TagOptions::Create2D},
    {"create_3d", TagOptions::Create3D},
    {"create_cube", TagOptions::CreateCube},
    {"created", TagOptions::Created},
    {"cull_face_enable", TagOptions::CullFaceEnable},
    {"cull_face", TagOptions::CullFace},
    {"curve", TagOptions::Curve},
    {"curves", TagOptions::Curves},
    {"cylinder", TagOptions::Cylinder}, // uhh & BRep
    {"damping", TagOptions::Damping},
    {"deceleration", TagOptions::Deceleration},
    {"density", TagOptions::Density},
    {"depth_bounds_enable", TagOptions::DepthBoundsEnable},
    {"depth_bounds", TagOptions::DepthBounds},
    {"depth_clamp_enable", TagOptions::DepthClampEnable},
    {"depth_clear", TagOptions::DepthClear},
    {"depth_func", TagOptions::DepthFunc},
    {"depth_mask", TagOptions::DepthMask},
    {"depth_range", TagOptions::DepthRange},
    {"depth_target", TagOptions::DepthTarget},
    {"depth_test_enable", TagOptions::DepthTestEnable},
    {"diffuse", TagOptions::Diffuse},
    {"direction", TagOptions::Direction},
    {"directional", TagOptions::Directional},
    {"dither_enable", TagOptions::DitherEnable},
    {"draw", TagOptions::Draw},
    {"dynamic_friction", TagOptions::DynamicFriction},
    {"dynamic", TagOptions::Dynamic},
    {"edges", TagOptions::Edges},
    {"effect", TagOptions::Effect},
    {"effector_info", TagOptions::EffectorInfo},
    {"ellipse", TagOptions::Ellipse},
    {"emission", TagOptions::Emission},
    {"enabled", TagOptions::Enabled},
    {"equation", TagOptions::Equation},
    {"evaluate_scene", TagOptions::EvaluateScene},
    {"evaluate", TagOptions::Evaluate},
    {"exact", TagOptions::Exact},
    {"extra", TagOptions::Extra},
    {"faces", TagOptions::Faces},
    {"falloff_angle", TagOptions::FalloffAngle},
    {"falloff_exponent", TagOptions::FalloffExponent},
    {"float_array", TagOptions::FloatArray},
    {"float", TagOptions::Float},
    {"focal", TagOptions::Focal},
    {"fog_color", TagOptions::FogColor},
    {"fog_coord_src", TagOptions::FogCoordSrc},
    {"fog_density", TagOptions::FogDensity},
    {"fog_enable", TagOptions::FogEnable},
    {"fog_end", TagOptions::FogEnd},
    {"fog_mode", TagOptions::FogMode},
    {"fog_state", TagOptions::FogState},
    {"force_field", TagOptions::ForceField},
    {"format", TagOptions::Format},
    {"formula", TagOptions::Formula},
    {"frame_object", TagOptions::FrameObject},
    {"frame_origin", TagOptions::FrameOrigin},
    {"frame_tcp", TagOptions::FrameTCP},
    {"frame_tip", TagOptions::FrameTip},
    {"front_face", TagOptions::FrontFace},
    {"func", TagOptions::Func},
    {"geographic_location", TagOptions::GeographicLocation},
    {"geometry", TagOptions::Geometry},
    {"gravity", TagOptions::Gravity},
    {"h", TagOptions::H},
    {"half_extents", TagOptions::HalfExtents},
    {"height", TagOptions::Height},
    {"hex", TagOptions::Hex},
    {"hint", TagOptions::Hint},
    {"hollow", TagOptions::Hollow},
    {"hyperbola", TagOptions::Hyperbola},
    {"idref_array", TagOptions::IDREFArray},
    {"image", TagOptions::Image},
    {"imager", TagOptions::Imager},
    {"import", TagOptions::Import},
    {"include", TagOptions::Include},
    {"index_of_refraction", TagOptions::IndexOfRefraction},
    {"index", TagOptions::Index},
    {"inertia", TagOptions::Inertia},
    {"init_from", TagOptions::InitFrom},
    {"inline", TagOptions::Inline},
    {"input", TagOptions::Input}, // uhh & shared & unshared
    {"instance_animation", TagOptions::InstanceAnimation},
    {"instance_articulated_system", TagOptions::InstanceArticulatedSystem},
    {"instance_camera", TagOptions::InstanceCamera},
    {"instance_controller", TagOptions::InstanceController},
    {"instance_effect", TagOptions::InstanceEffect},
    {"instance_force_field", TagOptions::InstanceForceField},
    {"instance_formula", TagOptions::InstanceFormula},
    {"instance_geometry", TagOptions::InstanceGeometry},
    {"instance_image", TagOptions::InstanceImage},
    {"instance_joint", TagOptions::InstanceJoint},
    {"instance_kinematics_model", TagOptions::InstanceKinematicsModel},
    {"instance_kinematics_scene", TagOptions::InstanceKinematicsScene},
    {"instance_light", TagOptions::InstanceLight},
    {"instance_material", TagOptions::InstanceMaterial}, // geometry & rendering
    {"instance_node", TagOptions::InstanceNode},
    {"instance_physics_material", TagOptions::InstancePhysicsMaterial},
    {"instance_physics_model", TagOptions::InstancePhysicsModel},
    {"instance_physics_scene", TagOptions::InstancePhysicsScene},
    {"instance_rigid_body", TagOptions::InstanceRigidBody},
    {"instance_rigid_constraint", TagOptions::InstanceRigidConstraint},
    {"instance_visual_scene", TagOptions::InstanceVisualScene},
    {"int_array", TagOptions::IntArray},
    {"interpenetrate", TagOptions::Interpenetrate},
    {"jerk", TagOptions::Jerk},
    {"joint", TagOptions::Joint},
    {"joints", TagOptions::Joints},
    {"keywords", TagOptions::Keywords},
    {"kinematics_model", TagOptions::KinematicsModel},
    {"kinematics_scene", TagOptions::KinematicsScene},
    {"kinematics", TagOptions::Kinematics},
    {"lambert", TagOptions::Lambert},
    {"latitude", TagOptions::Latitude},
    {"layer", TagOptions::Layer},
    {"library_animation_clips", TagOptions::LibraryAnimationClips},
    {"library_animations", TagOptions::LibraryAnimations},
    {"library_articulated_systems", TagOptions::LibraryArticulatedSystems},
    {"library_cameras", TagOptions::LibraryCameras},
    {"library_controllers", TagOptions::LibraryControllers},
    {"library_effects", TagOptions::LibraryEffects},
    {"library_force_fields", TagOptions::LibraryForceFields},
    {"library_formulas", TagOptions::LibraryFormulas},
    {"library_geometries", TagOptions::LibraryGeometries},
    {"library_images", TagOptions::LibraryImages},
    {"library_joints", TagOptions::LibraryJoints},
    {"library_kinematics_models", TagOptions::LibraryKinematicsModels},
    {"library_kinematics_scenes", TagOptions::LibraryKinematicsScenes},
    {"library_lights", TagOptions::LibraryLights},
    {"library_materials", TagOptions::LibraryMaterials},
    {"library_nodes", TagOptions::LibraryNodes},
    {"library_physics_materials", TagOptions::LibraryPhysicsMaterials},
    {"library_physics_models", TagOptions::LibraryPhysicsModels},
    {"library_physics_scenes", TagOptions::LibraryPhysicsScenes},
    {"library_visual_scenes", TagOptions::LibraryVisualScenes},
    {"light_ambient", TagOptions::LightAmbient},
    {"light_constant_attenuation", TagOptions::LightConstantAttenuation},
    {"light_diffuse", TagOptions::LightDiffuse},
    {"light_enable", TagOptions::LightEnable},
    {"light_linear_attenuation", TagOptions::LightLinearAttenuation},
    {"light_model_ambient", TagOptions::LightModelAmbient},
    {"light_model_color_control", TagOptions::LightModelColorControl},
    {"light_model_local_viewer_enable", TagOptions::LightModelLocalViewerEnable},
    {"light_model_two_side_enable", TagOptions::LightModelTwoSideEnable},
    {"light_position", TagOptions::LightPosition},
    {"light_quadratic_attenuation", TagOptions::LightQuadraticAttenuation},
    {"light_specular", TagOptions::LightSpecular},
    {"light_spot_cutoff", TagOptions::LightSpotCutoff},
    {"light_spot_direction", TagOptions::LightSpotDirection},
    {"light_spot_exponent", TagOptions::LightSpotExponent},
    {"lighting_enable", TagOptions::LightingEnable},
    {"lights", TagOptions::Lights},
    {"limits", TagOptions::Limits},
    {"line_smooth_enable", TagOptions::LineSmoothEnable},
    {"line_stipple_enable", TagOptions::LineStippleEnable},
    {"line_stipple", TagOptions::LineStipple},
    {"line_width", TagOptions::LineWidth},
    {"line", TagOptions::Line},
    {"linear_attenuation", TagOptions::LinearAttenuation},
    {"linear", TagOptions::Linear},
    {"lines", TagOptions::Lines},
    {"linestrips", TagOptions::Linestrips},
    {"link", TagOptions::Link},
    {"linker", TagOptions::Linker},
    {"locked", TagOptions::Locked},
    {"logic_op_enable", TagOptions::LogicOpEnable},
    {"logic_op", TagOptions::LogicOp},
    {"longitude", TagOptions::Longitude},
    {"lookout", TagOptions::Lookout},
    {"magfilter", TagOptions::Magfilter},
    {"mass_frame", TagOptions::MassFrame},
    {"mass", TagOptions::Mass},
    {"material_ambient", TagOptions::MaterialAmbient},
    {"material_diffuse", TagOptions::MaterialDiffuse},
    {"material_emission", TagOptions::MaterialEmission},
    {"material_shininess", TagOptions::MaterialShininess},
    {"material_specular", TagOptions::MaterialSpecular},
    {"material", TagOptions::Material},
    {"matrix", TagOptions::Matrix},
    {"max_anisotropy", TagOptions::MaxAnisotropy},
    {"max", TagOptions::Max},
    {"mesh", TagOptions::Mesh},
    {"min", TagOptions::Min},
    {"minfilter", TagOptions::Minfilter},
    {"mip_bias", TagOptions::MipBias},
    {"mip_max_level", TagOptions::MipMaxLevel},
    {"mip_min_level", TagOptions::MipMinLevel},
    {"mipfilter", TagOptions::Mipfilter},
    {"mips", TagOptions::Mips},
    {"model_view_matrix", TagOptions::ModelViewMatrix},
    {"modified", TagOptions::Modified},
    {"modifier", TagOptions::Modifier},
    {"morph", TagOptions::Morph},
    {"motion", TagOptions::Motion},
    {"multisample_enable", TagOptions::MultisampleEnable},
    {"newparam", TagOptions::Newparam},
    {"node", TagOptions::Node},
    {"normalize_enable", TagOptions::NormalizeEnable},
    {"nurbs_surface", TagOptions::NurbsSurface},
    {"nurbs", TagOptions::Nurbs},
    {"optics", TagOptions::Optics},
    {"orient", TagOptions::Orient},
    {"origin", TagOptions::Origin},
    {"orthographic", TagOptions::Orthographic},
    {"p", TagOptions::P},
    {"parabola", TagOptions::Parabola},
    {"param", TagOptions::Param}, // uhh & dataflow & reference
    {"pass", TagOptions::Pass},
    {"pcurves", TagOptions::Pcurves},
    {"perspective", TagOptions::Perspective},
    {"ph", TagOptions::PH},
    {"phong", TagOptions::Phong},
    {"physics_material", TagOptions::PhysicsMaterial},
    {"physics_model", TagOptions::PhysicsModel},
    {"physics_scene", TagOptions::PhysicsScene},
    {"plane", TagOptions::Plane},
    {"point_distance_attenuation", TagOptions::PointDistanceAttenuation},
    {"point_fade_threshold_size", TagOptions::PointFadeThresholdSize},
    {"point_size_max", TagOptions::PointSizeMax},
    {"point_size_min", TagOptions::PointSizeMin},
    {"point_size", TagOptions::PointSize},
    {"point_smooth_enable", TagOptions::PointSmoothEnable},
    {"point", TagOptions::Point},
    {"polygon_mode", TagOptions::PolygonMode},
    {"polygon_offset_fill_enable", TagOptions::PolygonOffsetFillEnable},
    {"polygon_offset_line_enable", TagOptions::PolygonOffsetLineEnable},
    {"polygon_offset_point_enable", TagOptions::PolygonOffsetPointEnable},
    {"polygon_offset", TagOptions::PolygonOffset},
    {"polygon_smooth_enable", TagOptions::PolygonSmoothEnable},
    {"polygon_stipple_enable", TagOptions::PolygonStippleEnable},
    {"polygons", TagOptions::Polygons},
    {"polylist", TagOptions::Polylist},
    {"prismatic", TagOptions::Prismatic},
    {"profile_bridge", TagOptions::ProfileBRIDGE},
    {"profile_cg", TagOptions::ProfileCG},
    {"profile_common", TagOptions::ProfileCOMMON},
    {"profile_gles", TagOptions::ProfileGLES},
    {"profile_gles2", TagOptions::ProfileGLES2},
    {"profile_glsl", TagOptions::ProfileGLSL},
    {"program", TagOptions::Program},
    {"projection_matrix", TagOptions::ProjectionMatrix},
    {"quadratic_attenuation", TagOptions::QuadraticAttenuation},
    {"radius", TagOptions::Radius},
    {"ref_attachment", TagOptions::RefAttachment},
    {"ref", TagOptions::Ref},
    {"reflective", TagOptions::Reflective},
    {"reflectivity", TagOptions::Reflectivity},
    {"render", TagOptions::Render},
    {"renderable", TagOptions::RRenderable},
    {"rescale_normal_enable", TagOptions::RescaleNormalEnable},
    {"restitution", TagOptions::Restitution},
    {"revision", TagOptions::Revision},
    {"revolute", TagOptions::Revolute},
    {"rgb", TagOptions::RGB},
    {"rigid_body", TagOptions::RigidBody},
    {"rigid_constraint", TagOptions::RigidConstraint},
    {"rotate", TagOptions::Rotate},
    {"sample_alpha_to_converge_enable", TagOptions::SampleAlphaToConvergeEnable},
    {"sample_alpha_to_one_enable", TagOptions::SampleAlphaToOneEnable},
    {"sample_coverage_enable", TagOptions::SampleCoverageEnable},
    {"sample_coverage", TagOptions::SampleCoverage},
    {"sampler_image", TagOptions::SamplerImage},
    {"sampler_states", TagOptions::SamplerStates},
    {"sampler", TagOptions::Sampler},
    {"sampler_1d", TagOptions::Sampler1D},
    {"sampler_2d", TagOptions::Sampler2D},
    {"sampler_3d", TagOptions::Sampler3D},
    {"sampler_cube", TagOptions::SamplerCUBE},
    {"sampler_depth", TagOptions::SamplerDEPTH},
    {"sampler_rect", TagOptions::SamplerRECT},
    {"scale", TagOptions::Scale},
    {"scene", TagOptions::Scene},
    {"scissor_test_enable", TagOptions::ScissorTestEnable},
    {"scissor", TagOptions::Scissor},
    {"semantic", TagOptions::Semantic},
    {"setparam", TagOptions::Setparam},
    {"shade_model", TagOptions::ShadeModel},
    {"shader", TagOptions::Shader},
    {"shape", TagOptions::Shape},
    {"shells", TagOptions::Shells},
    {"shininess", TagOptions::Shininess},
    {"sidref_array", TagOptions::SIDREFArray},
    {"size_exact", TagOptions::SizeExact},
    {"size_ratio", TagOptions::SizeRatio},
    {"size", TagOptions::Size},
    {"skeleton", TagOptions::Skeleton},
    {"skew", TagOptions::Skew},
    {"skin", TagOptions::Skin},
    {"solids", TagOptions::Solids},
    {"source_data", TagOptions::SourceData},
    {"source", TagOptions::Source},
    {"sources", TagOptions::Sources},
    {"specular", TagOptions::Specular},
    {"speed", TagOptions::Speed},
    {"sphere", TagOptions::Sphere},
    {"spline", TagOptions::Spline},
    {"spot", TagOptions::Spot},
    {"spring", TagOptions::Spring},
    {"states", TagOptions::States},
    {"static_friction", TagOptions::StaticFriction},
    {"stencil_clear", TagOptions::StencilClear},
    {"stencil_func_separate", TagOptions::StencilFuncSeparate},
    {"stencil_func", TagOptions::StencilFunc},
    {"stencil_mask", TagOptions::StencilMask},
    {"stencil_op_separate", TagOptions::StencilOpSeparate},
    {"stencil_op", TagOptions::StencilOp},
    {"stencil_target", TagOptions::StencilTarget},
    {"stencil_test_enable", TagOptions::StencilTestEnable},
    {"stiffness", TagOptions::Stiffness},
    {"subject", TagOptions::Subject},
    {"surface_curves", TagOptions::SurfaceCurves},
    {"surface", TagOptions::Surface},
    {"surfaces", TagOptions::Surfaces},
    {"swept_surface", TagOptions::SweptSurface},
    {"swing_cone_and_twist", TagOptions::SwingConeAndTwist},
    {"target_value", TagOptions::TargetValue},
    {"target", TagOptions::Target},
    {"targets", TagOptions::Targets},
    {"technique_common", TagOptions::TechniqueCommon},
    {"technique_hint", TagOptions::TechniqueHint},
    {"technique_override", TagOptions::TechniqueOverride},
    {"technique", TagOptions::Technique}, // core & FX
    {"texcombiner", TagOptions::Texcombiner},
    {"texcoord", TagOptions::Texcoord},
    {"texenv", TagOptions::Texenv},
    {"texture_env_color", TagOptions::TextureEnvColor},
    {"texture_env_mode", TagOptions::TextureEnvMode},
    {"texture_pipeline", TagOptions::TexturePipeline}, // uhh & render state
    {"texture", TagOptions::Texture},
    {"texture_1d_enable", TagOptions::Texture1DEnable},
    {"texture_1d", TagOptions::Texture1D},
    {"texture_2d_enable", TagOptions::Texture2DEnable},
    {"texture_2d", TagOptions::Texture2D},
    {"texture_3d_enable", TagOptions::Texture3DEnable},
    {"texture_3d", TagOptions::Texture3D},
    {"texture_cube_enable", TagOptions::TextureCUBEEnable},
    {"texture_cube", TagOptions::TextureCUBE},
    {"texture_depth_enable", TagOptions::TextureDEPTHEnable},
    {"texture_depth", TagOptions::TextureDEPTH},
    {"texture_rect_enable", TagOptions::TextureRECTEnable},
    {"texture_rect", TagOptions::TextureRECT},
    {"time_step", TagOptions::TimeStep},
    {"title", TagOptions::Title},
    {"torus", TagOptions::Torus},
    {"translate", TagOptions::Translate},
    {"transparency", TagOptions::Transparency},
    {"transparent", TagOptions::Transparent},
    {"triangles", TagOptions::Triangles},
    {"trifans", TagOptions::Trifans},
    {"tristrips", TagOptions::Tristrips},
    {"unit", TagOptions::Unit},
    {"unnormalized", TagOptions::Unnormalized},
    {"up_axis", TagOptions::UpAxis},
    {"usertype", TagOptions::Usertype},
    {"v", TagOptions::V},
    {"value", TagOptions::Value}, // uhh & render state
    {"vcount", TagOptions::Vcount},
    {"velocity", TagOptions::Velocity},
    {"vertex_weights", TagOptions::VertexWeights},
    {"vertices", TagOptions::Vertices},
    {"visual_scene", TagOptions::VisualScene},
    {"wires", TagOptions::Wires},
    {"wrap_p", TagOptions::WrapP},
    {"wrap_s", TagOptions::WrapS},
    {"wrap_t", TagOptions::WrapT},
    {"xfov", TagOptions::Xfov},
    {"xmag", TagOptions::Xmag},
    {"yfov", TagOptions::Yfov},
    {"ymag", TagOptions::Ymag},
    {"zfar", TagOptions::Zfar},
    {"znear", TagOptions::Znear},
};
