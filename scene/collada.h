#pragma once
#include "libQ/include/xml.h"
#include "scene.h"

#include <stack>
#include <map>

class COLLADAScene : public GLScene, public basicxml
{
public:
  COLLADAScene(std::string path, libQ::log _logobj);

  int loadcallback(char *buffer, int buffsize);
  void parsecallback(element e);

  std::ifstream ifs;
  std::string basepath;

  std::string CurrentLoadingArray; // Index for SourcesArray
  std::string POSITIONSemantic; //
  std::string VerticesID;
  int LoaderIT, TrianglesInputCount, ClaimedTriCount;

  bool isInsideTriangles;
  AssetData *CurrentAsset;

  struct SourceTag {
    std::vector<float> FloatArray;
    int AccessorStride;
    // Accessor definition
  };
  std::unordered_map<std::string,SourceTag> SourcesArray;
  std::vector<float> FloatVector; // For loading 
  std::vector<uint> UIntVector; // For loading indices
  std::shared_ptr<Model> CurrentModel;
  std::stack<std::shared_ptr<Renderable>> CurrentNode;

  enum TagOptions // Every tag, per the COLLADA v1.5 spec
  {
        None = 0,
        Acceleration,
        Accessor,
        Active,
        AlphaFunc,
        AlphaTestEnable,
        Alpha,
        Altitude,
        Ambient, // core & FX
        Angle,
        AngularVelocity,
        Angular,
        AnimationClip,
        Animation,
        Annotate,
        Argument,
        Array,
        ArticulatedSystem,
        AspectRatio,
        Asset,
        AttachmentEnd,
        AttachmentFull,
        AttachmentStart,
        Attachment,
        AuthorEmail,
        AuthorWebsite,
        Author,
        AuthoringTool,
        AutoNormalEnable,
        AxisInfo,
        Axis,
        Binary,
        BindAttribute,
        BindJointAxis,
        BindKinematicsModel,
        BindMaterial,
        BindShapeMatrix,
        BindUniform,
        BindVertexInput,
        Bind, // FX & kinematis
        BlendColor,
        BlendEnable,
        BlendEquationSeparate,
        BlendEquation,
        BlendFuncSeparate,
        BlendFunc,
        Blinn,
        BoolArray,
        BorderColor,
        Box,
        Brep,
        Camera,
        Capsule,
        Channel,
        Circle,
        ClipPlaneEnable,
        ClipPlane,
        Code,
        COLLADA,
        ColorClear,
        ColorLogicOpEnable,
        ColorMask,
        ColorMaterialEnable,
        ColorMaterial,
        ColorTarget,
        Color,
        Comments,
        Compiler,
        Cone,
        ConnectParam,
        ConstantAttenuation,
        Constant, // combiner & FX
        Contributor,
        ControlVertices,
        Controller,
        ConvexMesh,
        Copyright,
        Coverage,
        Create2D,
        Create3D,
        CreateCube,
        Created,
        CullFaceEnable,
        CullFace,
        Curve,
        Curves,
        Cylinder, // uhh & BRep
        Damping,
        Deceleration,
        Density,
        DepthBoundsEnable,
        DepthBounds,
        DepthClampEnable,
        DepthClear,
        DepthFunc,
        DepthMask,
        DepthRange,
        DepthTarget,
        DepthTestEnable,
        Diffuse,
        Direction,
        Directional,
        DitherEnable,
        Draw,
        DynamicFriction,
        Dynamic,
        Edges,
        Effect,
        EffectorInfo,
        Ellipse,
        Emission,
        Enabled,
        Equation,
        EvaluateScene,
        Evaluate,
        Exact,
        Extra,
        Faces,
        FalloffAngle,
        FalloffExponent,
        FloatArray,
        Float,
        Focal,
        FogColor,
        FogCoordSrc,
        FogDensity,
        FogEnable,
        FogEnd,
        FogMode,
        FogState,
        ForceField,
        Format,
        Formula,
        FrameObject,
        FrameOrigin,
        FrameTCP,
        FrameTip,
        FrontFace,
        Func,
        GeographicLocation,
        Geometry,
        Gravity,
        H,
        HalfExtents,
        Height,
        Hex,
        Hint,
        Hollow,
        Hyperbola,
        IDREFArray,
        Image,
        Imager,
        Import,
        Include,
        IndexOfRefraction,
        Index,
        Inertia,
        InitFrom,
        Inline,
        Input, // uhh & shared & unshared
        InstanceAnimation,
        InstanceArticulatedSystem,
        InstanceCamera,
        InstanceController,
        InstanceEffect,
        InstanceForceField,
        InstanceFormula,
        InstanceGeometry,
        InstanceImage,
        InstanceJoint,
        InstanceKinematicsModel,
        InstanceKinematicsScene,
        InstanceLight,
        InstanceMaterial, // geometry & rendering
        InstanceNode,
        InstancePhysicsMaterial,
        InstancePhysicsModel,
        InstancePhysicsScene,
        InstanceRigidBody,
        InstanceRigidConstraint,
        InstanceVisualScene,
        IntArray,
        Interpenetrate,
        Jerk,
        Joint,
        Joints,
        Keywords,
        KinematicsModel,
        KinematicsScene,
        Kinematics,
        Lambert,
        Latitude,
        Layer,
        LibraryAnimationClips,
        LibraryAnimations,
        LibraryArticulatedSystems,
        LibraryCameras,
        LibraryControllers,
        LibraryEffects,
        LibraryForceFields,
        LibraryFormulas,
        LibraryGeometries,
        LibraryImages,
        LibraryJoints,
        LibraryKinematicsModels,
        LibraryKinematicsScenes,
        LibraryLights,
        LibraryMaterials,
        LibraryNodes,
        LibraryPhysicsMaterials,
        LibraryPhysicsModels,
        LibraryPhysicsScenes,
        LibraryVisualScenes,
        LightAmbient,
        LightConstantAttenuation,
        LightDiffuse,
        LightEnable,
        LightLinearAttenuation,
        LightModelAmbient,
        LightModelColorControl,
        LightModelLocalViewerEnable,
        LightModelTwoSideEnable,
        LightPosition,
        LightQuadraticAttenuation,
        LightSpecular,
        LightSpotCutoff,
        LightSpotDirection,
        LightSpotExponent,
        LightingEnable,
        Lights,
        Limits,
        LineSmoothEnable,
        LineStippleEnable,
        LineStipple,
        LineWidth,
        Line,
        LinearAttenuation,
        Linear,
        Lines,
        Linestrips,
        Link,
        Linker,
        Locked,
        LogicOpEnable,
        LogicOp,
        Longitude,
        Lookout,
        Magfilter,
        MassFrame,
        Mass,
        MaterialAmbient,
        MaterialDiffuse,
        MaterialEmission,
        MaterialShininess,
        MaterialSpecular,
        Material,
        Matrix,
        MaxAnisotropy,
        Max,
        Mesh,
        Min,
        Minfilter,
        MipBias,
        MipMaxLevel,
        MipMinLevel,
        Mipfilter,
        Mips,
        ModelViewMatrix,
        Modified,
        Modifier,
        Morph,
        Motion,
        MultisampleEnable,
        // NameArray - I don't know what this means...
        Newparam,
        Node,
        NormalizeEnable,
        NurbsSurface,
        Nurbs,
        Optics,
        Orient,
        Origin,
        Orthographic,
        P,
        Parabola,
        Param, // uhh & dataflow & reference
        Pass,
        Pcurves,
        Perspective,
        PH,
        Phong,
        PhysicsMaterial,
        PhysicsModel,
        PhysicsScene,
        Plane,
        PointDistanceAttenuation,
        PointFadeThresholdSize,
        PointSizeMax,
        PointSizeMin,
        PointSize,
        PointSmoothEnable,
        Point,
        PolygonMode,
        PolygonOffsetFillEnable,
        PolygonOffsetLineEnable,
        PolygonOffsetPointEnable,
        PolygonOffset,
        PolygonSmoothEnable,
        PolygonStippleEnable,
        Polygons,
        Polylist,
        Prismatic,
        ProfileBRIDGE,
        ProfileCG,
        ProfileCOMMON,
        ProfileGLES,
        ProfileGLES2,
        ProfileGLSL,
        Program,
        ProjectionMatrix,
        QuadraticAttenuation,
        Radius,
        RefAttachment,
        Ref,
        Reflective,
        Reflectivity,
        Render,
        RRenderable,
        RescaleNormalEnable,
        Restitution,
        Revision,
        Revolute,
        RGB,
        RigidBody,
        RigidConstraint,
        Rotate,
        SampleAlphaToConvergeEnable,
        SampleAlphaToOneEnable,
        SampleCoverageEnable,
        SampleCoverage,
        SamplerImage,
        SamplerStates,
        Sampler,
        Sampler1D,
        Sampler2D,
        Sampler3D,
        SamplerCUBE,
        SamplerDEPTH,
        SamplerRECT,
        Scale,
        Scene,
        ScissorTestEnable,
        Scissor,
        Semantic,
        Setparam,
        ShadeModel,
        Shader,
        Shape,
        Shells,
        Shininess,
        SIDREFArray,
        SizeExact,
        SizeRatio,
        Size,
        Skeleton,
        Skew,
        Skin,
        Solids,
        SourceData,
        Source,
        Sources,
        Specular,
        Speed,
        Sphere,
        Spline,
        Spot,
        Spring,
        States,
        StaticFriction,
        StencilClear,
        StencilFuncSeparate,
        StencilFunc,
        StencilMask,
        StencilOpSeparate,
        StencilOp,
        StencilTarget,
        StencilTestEnable,
        Stiffness,
        Subject,
        SurfaceCurves,
        Surface,
        Surfaces,
        SweptSurface,
        SwingConeAndTwist,
        TargetValue,
        Target,
        Targets,
        TechniqueCommon,
        TechniqueHint,
        TechniqueOverride,
        Technique, // core & FX
        Texcombiner,
        Texcoord,
        Texenv,
        TextureEnvColor,
        TextureEnvMode,
        TexturePipeline, // uhh & render state
        Texture,
        Texture1DEnable,
        Texture1D,
        Texture2DEnable,
        Texture2D,
        Texture3DEnable,
        Texture3D,
        TextureCUBEEnable,
        TextureCUBE,
        TextureDEPTHEnable,
        TextureDEPTH,
        TextureRECTEnable,
        TextureRECT,
        TimeStep,
        Title,
        Torus,
        Translate,
        Transparency,
        Transparent,
        Triangles,
        Trifans,
        Tristrips,
        Unit,
        Unnormalized,
        UpAxis,
        Usertype,
        V,
        Value, // uhh & render state
        Vcount,
        Velocity,
        VertexWeights,
        Vertices,
        VisualScene,
        Wires,
        WrapP,
        WrapS,
        WrapT,
        Xfov,
        Xmag,
        Yfov,
        Ymag,
        Zfar,
        Znear,
  };

  //struct Tag
  //{
  //  TagOptions Name;
  //  AssetData *CurrentAsset;
  //};
  //std::stack<Tag> CurrentTags;

  static std::map<std::string,TagOptions> TagLUT;
};
