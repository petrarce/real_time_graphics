#pragma once

#include <vector>

#include <typed-geometry/tg-lean.hh>

#include <glow/fwd.hh>

#include <glow/objects/ArrayBufferAttribute.hh>

#include <glow-extras/glfw/GlfwApp.hh>

#include "Character.hh"
#include "Chunk.hh"
#include "Material.hh"
#include "World.hh"

enum class RenderPass
{
    Shadow,
    DepthPre,
    Opaque,
    Transparent
};

enum class DebugTarget
{
    Output,

    OpaqueDepth,
    ShadedOpaque,

    GBufferAlbedo,
    GBufferAO,
    GBufferNormal,
    GBufferMetallic,
    GBufferRoughness,
    GBufferTranslucency,

    TBufferColor,
    TBufferAlpha,
    TBufferDistortion,
    TBufferBlurriness,

    ShadowCascade0,
    ShadowCascade1,
    ShadowCascade2
};

/**
 * Assignment07: A minecraft clone (part 2)
 */
class Assignment07 : public glow::glfw::GlfwApp
{
private:
    /// our block world
    World mWorld;

    /// our character moving through the world
    Character mCharacter;

private: // input and camera
    tg::vec3 mLightDir = normalize(tg::vec3(.25f, .66f, .75f));
    tg::color3 mLightColor = tg::color3::white;
    tg::color3 mAmbientLight = 0.05f * tg::color3::white;

    // "Player"
    tg::pos3 mPlayerPos = {0, 0, 0};

    // Interaction
    RayHit mMouseHit;
    bool mShiftPressed = false;
    bool mCtrlPressed = false;
    int8_t mCurrentMaterial = 1;
    bool mFreeFlightCamera = false;
    bool mDoJump = true;

    // Lights
    float mLightSpawnCountdown = 0.0f;

    struct LightSource
    {
        tg::pos3 position;
        float radius;
        tg::vec3 velocity;
        float intensity; // keep real unit a secret
        tg::color3 color;
        int seed;
    };
    std::vector<LightSource> mLightSources;

private: // object gfx
    // terrain
    std::map<std::string, glow::SharedProgram> mShadersTerrain;
    glow::SharedProgram mShaderTerrainShadow;
    glow::SharedProgram mShaderTerrainDepthPre;

    // background
    glow::SharedTextureCubeMap mTexSkybox;

    // objects
    glow::SharedProgram mShaderLineTransparent;

    glow::SharedVertexArray mMeshCube;
    glow::SharedVertexArray mMeshLine;

    // lights
    glow::SharedProgram mShaderLightSprites;
    glow::SharedVertexArray mMeshLightSpheres;
    glow::SharedVertexArray mMeshLightSprites;
    glow::SharedArrayBuffer mLightArrayBuffer;
    glow::SharedTexture2D mTexLightSprites;

private: // rendering pipeline
    std::vector<glow::SharedTextureRectangle> mFramebufferTargets;

    // meshes
    glow::SharedVertexArray mMeshQuad;

    // shaders
    glow::SharedProgram mShaderOutput;
    glow::SharedProgram mShaderClear;
    glow::SharedProgram mShaderShadowBlurX;
    glow::SharedProgram mShaderShadowBlurY;
    glow::SharedProgram mShaderFullscreenLight;
    glow::SharedProgram mShaderPointLight;
    glow::SharedProgram mShaderTransparentResolve;

    // Shadow Pass
    int mShadowMapSize = 1024;

    struct ShadowCamera : glow::camera::CameraBase
    {
        tg::pos3 mPosition;
        tg::mat4 mViewMatrix;
        tg::mat4 mProjectionMatrix;
        tg::isize2 mViewportSize;

        void setPosition(tg::pos3 const& position) { mPosition = position; }
        void setViewMatrix(tg::mat4 const& viewMatrix) { mViewMatrix = viewMatrix; }
        void setProjectionMatrix(tg::mat4 const& projectionMatrix) { mProjectionMatrix = projectionMatrix; }
        void setViewportSize(tg::isize2 const& viewportSize) { mViewportSize = viewportSize; }

        tg::pos3 getPosition() const override { return mPosition; }
        tg::mat4 getViewMatrix() const override { return mViewMatrix; }
        tg::mat4 getProjectionMatrix() const override { return mProjectionMatrix; }

        // (actually not needed in this assignment)
        tg::isize2 getViewportSize() const override { return {}; };
        float getNearClippingPlane() const override { return {}; };
        float getFarClippingPlane() const override { return {}; };
        tg::angle getVerticalFieldOfView() const override { return {}; };
        tg::angle getHorizontalFieldOfView() const override { return {}; };
    };

    struct ShadowCascade
    {
        glow::SharedFramebuffer framebuffer;
        ShadowCamera camera;
        float minRange = -1;
        float maxRange = -1;
    };
    glow::SharedTexture2DArray mShadowMaps;
    std::vector<ShadowCascade> mShadowCascades;
    tg::array<tg::mat4, SHADOW_CASCADES> mShadowViewProjs;
    tg::pos3 mShadowPos;
    float mShadowExponent = 80.0f;
    float mShadowRange = 200.0f;
    glow::SharedFramebuffer mFramebufferShadowBlur;
    glow::SharedTexture2D mShadowBlurTarget;

    // Depth Pre-Pass
    glow::SharedFramebuffer mFramebufferDepthPre;
    glow::SharedTextureRectangle mTexOpaqueDepth;

    // Opaque Pass
    glow::SharedFramebuffer mFramebufferGBuffer;
    glow::SharedTextureRectangle mTexGBufferMatA;  // normals, metallic
    glow::SharedTextureRectangle mTexGBufferMatB;  // roughness, translucency SSS
    glow::SharedTextureRectangle mTexGBufferColor; // albedo, AO

    // Light Pass
    glow::SharedFramebuffer mFramebufferShadedOpaque;
    glow::SharedTextureRectangle mTexShadedOpaque;

    // Transparent Pass
    glow::SharedFramebuffer mFramebufferTBuffer;
    glow::SharedTextureRectangle mTexTBufferAccumA;
    glow::SharedTextureRectangle mTexTBufferAccumB;
    glow::SharedTextureRectangle mTexTBufferDistortion; // offset + LOD bias

    // Transparent Resolve
    glow::SharedFramebuffer mFramebufferTransparentResolve;
    glow::SharedTextureRectangle mTexHDRColor;

    // Output Stage
    // .. no FBO (to screen)
    DebugTarget mDebugOutput = DebugTarget::Output;

    // gfx options
    bool mUseFXAA = true;
    bool mUseDithering = true;
    bool mEnableShadows = true;
    bool mSoftShadows = true;
    bool mShowWireframeOpaque = false;
    bool mShowWireframeTransparent = false;
    bool mShowDebugLights = false;
    bool mDrawBackground = true;

    // pipeline
    bool mEnablePointLights = true;
    bool mPassDepthPre = true;
    bool mPassOpaque = true;
    bool mPassTransparent = true;

    // culling
    float mRenderDistance = 128;
    bool mEnableCustomBFC = true;
    bool mEnableFrustumCulling = true;

    // debug
    bool mBackFaceCulling = true;
    bool mShowWrongDepthPre = false;

    // stats
    int mStatsChunksGenerated = -1;
    int mStatsMeshesRendered[4] = {};
    int mStatsVerticesRendered[4] = {};
    float mStatsVerticesPerMesh[4] = {};

private: // gfx options
    /// accumulated time
    double mRuntime = 0.0;

    constexpr static int accumframes = 30;
    int64_t mCurframeidx = 0;
    float mLastFrameTimes[accumframes] = {};
    float mAvgFrameTime = 0.0f;
    double mLastStatUpdate = 0.0;

    bool mVSync = true;

private: // helper
    /// sets common uniforms such as 3D transformation matrices
    void setUpShader(glow::Program* program, glow::camera::CameraBase* cam, RenderPass pass);
    void setUpLightShader(glow::Program* program, glow::camera::CameraBase* cam);

    /// get a ray from current mouse pos into the scene
    void getMouseRay(tg::pos3& pos, tg::vec3& dir) const;

    /// performs a raycast and updates selected block
    void updateViewRay();

    /// Creates a shader for terrain (with terrain.vsh)
    /// Also registers it with mShadersTerrain
    /// Does not create the same shader twice
    void createTerrainShader(std::string const& name);

    /// Updates shadow map texture if size changed
    void updateShadowMapTexture();

    // line drawing
    void buildLineMesh();
    void drawLine(tg::pos3 from, tg::pos3 to, tg::color3 color, RenderPass pass);
    void drawAABB(tg::pos3 min, tg::pos3 max, tg::color3 color, RenderPass pass);

    /// spawn a light source close to the player
    void spawnLightSource(const tg::pos3& origin);
    /// update all light sources
    void updateLightSources(float elapsedSeconds);

private: // rendering
    /// renders the scene for a render pass
    void renderScene(glow::camera::CameraBase* cam, RenderPass pass);

    // pipeline passes
    void renderShadowPass();
    void renderDepthPrePass();
    void renderOpaquePass();
    void renderLightPass();
    void renderTransparentPass();
    void renderTransparentResolve();
    void renderOutputStage();

public:
    void init() override;
    void update(float elapsedSeconds) override;
    void render(float elapsedSeconds) override;
    void onGui() override;

    // Input/Event handling
    bool onMouseButton(double x, double y, int button, int action, int mods, int clickCount) override;
    bool onMousePosition(double x, double y) override;
    bool onKey(int key, int scancode, int action, int mods) override;
    void onResize(int w, int h) override;
};
