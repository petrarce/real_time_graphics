#include "pipeline_embed_shaders.hh"

// This file is generated upon running CMake, do not modify it!

namespace internal_embedded_files {

const std::pair<const char*, const char*> pipeline_embed_shaders[] = {
{"glow-pipeline/internal/common/cszReconstruct.glsl", R"%%RES_EMBED%%(
#ifndef GLOW_PIPELINE_CSZ_RECONSTRUCT_GLSL
#define GLOW_PIPELINE_CSZ_RECONSTRUCT_GLSL

#include <glow-pipeline/internal/common/globals.hh>

#if GLOW_PIPELINE_ENABLE_REVERSE_Z == 1
#glow pipeline internal_projectionInfoUbo
#else
uniform vec3 uClipInfo;
#endif

#if GLOW_PIPELINE_ENABLE_REVERSE_Z == 1
float getViewSpaceZ()
{
    return -(uPipelineProjectionInfo.viewportNearFar.z / gl_FragCoord.z);
}
#endif

float reconstructCSZ(float d) {
#if GLOW_PIPELINE_ENABLE_REVERSE_Z == 1
    return uPipelineProjectionInfo.viewportNearFar.z / d;
#else
    return uClipInfo.x / (uClipInfo.y * d + uClipInfo.z);
#endif
}

#endif

)%%RES_EMBED%%"},
{"glow-pipeline/internal/common/globals.hh", R"%%RES_EMBED%%(
// NOTE: This file is used in both GLSL and C++
#ifndef GLOW_PIPELINE_GLOBALS_HH_
#define GLOW_PIPELINE_GLOBALS_HH_

// GLSL: pass/ao/sao/SAO.glsl
// C++: DepthPreStage.hh
#define GLOW_PIPELINE_LINEAR_DEPTH_MAX_MIP_LEVEL 5

// GLSL: pass/shadow/shadowCascades.gsh
// C++: ShadowStage.hh
#define GLOW_PIPELINE_SHADOW_CASCADE_AMOUNT 4

// GLSL: opaque stage shaders
// C++: ShadowStage.hh
#define GLOW_PIPELINE_SHADOW_MAP_SIZE 2048

// GLSL: all clustering shaders in internal/pass/depthPre
// C++: StageCamera.cc, DepthPreStage.cc
#define GLOW_PIPELINE_CLUSTER_PIXEL_SIZE_X 120
#define GLOW_PIPELINE_CLUSTER_PIXEL_SIZE_Y 120
#define GLOW_PIPELINE_CLUSTER_AMOUNT_Z 24
#define GLOW_PIPELINE_MAX_LIGHTS_PER_CLUSTER 150

// The local workgroup size of the light assignment compute shader
// GLSL: pass/depthPre/clusterLightAssignment.csh
// C++: StageCamera.cc
#define GLOW_PIPELINE_LASSIGN_COMPUTE_X 4
#define GLOW_PIPELINE_LASSIGN_COMPUTE_Y 3

#define GLOW_PIPELINE_ENABLE_REVERSE_Z 1

#if GLOW_PIPELINE_ENABLE_REVERSE_Z
#define GLOW_PIPELINE_HORIZON_DEPTH 0.f
#else
#define GLOW_PIPELINE_HORIZON_DEPTH 1.f
#endif

#define GLOW_GLSL_FLT_MAX 3.402823466e+38

#endif

)%%RES_EMBED%%"},
{"glow-pipeline/internal/common/math.glsl", R"%%RES_EMBED%%(
#ifndef MATH_GLSL
#define MATH_GLSL

const float PI = 3.14159265359;

// Reference: http://stackoverflow.com/a/3380723/554283
float acosApproximation( float x ) 
{
   return (-0.69813170079773212 * x * x - 0.87266462599716477) * x + 1.5707963267948966;
}

#endif

)%%RES_EMBED%%"},
{"glow-pipeline/internal/common/positionReconstruct.glsl", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/common/globals.hh>

vec4 reconstructClipSpacePosition(vec4 screenPosition, float fragmentDepth)
{
#if GLOW_PIPELINE_ENABLE_REVERSE_Z == 1
    return vec4(screenPosition.xy / screenPosition.w, fragmentDepth, 1.0);
#else
    return vec4(screenPosition.xy / screenPosition.w, fragmentDepth * 2 - 1, 1.0);
#endif
}

vec3 reconstructViewPosition(vec4 screenPosition, float fragmentDepth)
{
    vec4 clipSpacePosition = reconstructClipSpacePosition(screenPosition, fragmentDepth);
    vec4 viewPosition = uPipelineProjectionInfo.projectionInverse * clipSpacePosition; 
    viewPosition /= viewPosition.w;
    return viewPosition.xyz;
}

vec3 reconstructWorldPosition(vec4 screenPosition, float fragmentDepth)
{
    vec4 clipSpacePosition = reconstructClipSpacePosition(screenPosition, fragmentDepth);
    vec4 viewPosition = uPipelineProjectionInfo.projectionInverse * clipSpacePosition; 
    viewPosition /= viewPosition.w;
    return vec3(uPipelineProjectionInfo.viewInverse * viewPosition);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/common/util.glsl", R"%%RES_EMBED%%(
#ifndef UTIL_GLSL
#define UTIL_GLSL

float distance2(vec3 a, vec3 b)
{
    vec3 d = a - b;
    return dot(d, d);
}

float rgbToLuminance(vec3 rgb)
{
    // Alternative luma weight: vec3(0.299, 0.587, 0.114) (FXAA 3.11 version)
    return dot(rgb, vec3(0.2126, 0.7152, 0.0722));
}

// ---- DirectX ----

// -- Missing functions --

// DirectX: min
vec3 min3(vec3 a, vec3 b)
{
    return vec3(min(a.r, b.r), min(a.g, b.g), min(a.b, b.b));
}

// DirectX: max
vec3 max3(vec3 a, vec3 b)
{
    return vec3(max(a.r, b.r), max(a.g, b.g), max(a.b, b.b));
}

// DirectX: any
bool anyFloat(vec2 v)
{
    return (v.r != 0 || v.g != 0);
}

bool anyFloat(vec3 v)
{
    return (v.r != 0 || v.g != 0 || v.b != 0);
}


// -- Name mapping / Convenience --

float saturate(float v) { return clamp(v, 0, 1); }
vec2 saturate(vec2 v) { return clamp(v, 0, 1); }
vec3 saturate(vec3 v) { return clamp(v, 0, 1); }
vec4 saturate(vec4 v) { return clamp(v, 0, 1); }

vec4 ddy(vec4 v) { return dFdy(v); }
vec3 ddy(vec3 v) { return dFdy(v); }
vec2 ddy(vec2 v) { return dFdy(v); }

vec4 ddx(vec4 v) { return dFdx(v); }
vec3 ddx(vec3 v) { return dFdx(v); }
vec2 ddx(vec2 v) { return dFdx(v); }

#endif

)%%RES_EMBED%%"},
{"glow-pipeline/internal/fullscreen.vsh", R"%%RES_EMBED%%(
in vec2 aPosition;

out vec2 vPosition;
out vec4 vScreenPosition;

void main() {
    vPosition = aPosition;
    vScreenPosition = vec4(aPosition * 2 - 1, 0, 1);
    gl_Position = vScreenPosition;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/fullscreenTriangle.vsh", R"%%RES_EMBED%%(
out vec2 vTexCoord;
out vec4 vScreenPosition;

void main()
{
    vTexCoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vScreenPosition = vec4(vTexCoord * vec2( 2.0, 2.0 ) + vec2( -1.0, -1.0) , 0.0, 1.0 );
    gl_Position = vScreenPosition;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/cssao/blurCommon.glsl", R"%%RES_EMBED%%(
layout(std140) uniform uCSSAOGlobals {
    uvec4 GlobalConstantBuffer_0; // std140uvec4 buildVersion;

    vec2 GlobalConstantBuffer_1; // std140vec2 invQuarterResolution;
    vec2 GlobalConstantBuffer_2; // std140vec2 invFullResolution;
    vec2 GlobalConstantBuffer_3; // std140vec2 UVToViewA;
    vec2 GlobalConstantBuffer_4; // std140vec2 UVToViewB;

    float GlobalConstantBuffer_5; // std140float fRadiusToScreen;
    float GlobalConstantBuffer_6; // std140float fR2;
    float GlobalConstantBuffer_7; // std140float fNegInvR2;
    float GlobalConstantBuffer_8; // std140float fNDotVBias;

    float GlobalConstantBuffer_9; // std140float fSmallScaleAOAmount;
    float GlobalConstantBuffer_10; // std140float fLargeScaleAOAmount;
    float GlobalConstantBuffer_11; // std140float fPowExponent;
    int GlobalConstantBuffer_12; // std140int iUnused;

    float GlobalConstantBuffer_13; // std140float fBlurViewDepth0;
    float GlobalConstantBuffer_14; // std140float fBlurViewDepth1;
    float GlobalConstantBuffer_15; // std140float fBlurSharpness0;
    float GlobalConstantBuffer_16; // std140float fBlurSharpness1;

    float GlobalConstantBuffer_17; // std140float fLinearizeDepthA;
    float GlobalConstantBuffer_18; // std140float fLinearizeDepthB;
    float GlobalConstantBuffer_19; // std140float fInverseDepthRangeA;
    float GlobalConstantBuffer_20; // std140float fInverseDepthRangeB;

    vec2 GlobalConstantBuffer_21; // std140vec2 f2InputViewportTopLeft;
    float GlobalConstantBuffer_22; // std140float fViewDepthThresholdNegInv;
    float GlobalConstantBuffer_23; // std140float fViewDepthThresholdSharpness;

    float GlobalConstantBuffer_24; // std140float fBackgroundAORadiusPixels;
    float GlobalConstantBuffer_25; // std140float fForegroundAORadiusPixels;
    int GlobalConstantBuffer_26; // std140int iDebugNormalComponent;
};


uniform sampler2D uInputNearest;
uniform sampler2D uInputLinear;

#define KERNEL_RADIUS 4

//----------------------------------------------------------------------------------
vec2 PointSampleAODepth(vec2 UV)
{
    return texture(uInputNearest, UV).xy;
}
vec2 LinearSampleAODepth(vec2 UV)
{
    return texture(uInputLinear, UV).xy;
}

//----------------------------------------------------------------------------------
struct CenterPixelData
{
    vec2 UV;
    float Depth;
    float Sharpness;
    float Scale;
    float Bias;
};

//----------------------------------------------------------------------------------
float CrossBilateralWeight(float R, float SampleDepth, float DepthSlope, CenterPixelData Center)
{
    const float BlurSigma = (KERNEL_RADIUS+1.0) * 0.5;
    const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);

    SampleDepth -= DepthSlope * R;

    float DeltaZ = SampleDepth * Center.Scale + Center.Bias;

    return exp2(-R*R*BlurFalloff - DeltaZ*DeltaZ);
}

//-------------------------------------------------------------------------
void ProcessSample(vec2 AOZ,
                   float R,
                   float DepthSlope,
                   CenterPixelData Center,
                   inout float TotalAO,
                   inout float TotalW)
{
    float AO = AOZ.x;
    float Z = AOZ.y;

    float W = CrossBilateralWeight(R, Z, DepthSlope, Center);
    TotalAO += W * AO;
    TotalW += W;
}

//-------------------------------------------------------------------------
void ProcessRadius(float R0,
                   vec2 DeltaUV,
                   float DepthSlope,
                   CenterPixelData Center,
                   inout float TotalAO,
                   inout float TotalW)
{
    float R = R0;

    //[unroll]
    for (; R <= KERNEL_RADIUS/2; R += 1)
    {
        vec2 UV = R * DeltaUV + Center.UV;
        vec2 AOZ = PointSampleAODepth(UV);
        ProcessSample(AOZ, R, DepthSlope, Center, TotalAO, TotalW);
    }

    //[unroll]
    for (; R <= KERNEL_RADIUS; R += 2)
    {
        vec2 UV = (R + 0.5) * DeltaUV + Center.UV;
        vec2 AOZ = LinearSampleAODepth(UV);
        ProcessSample(AOZ, R, DepthSlope, Center, TotalAO, TotalW);
    }
}

//-------------------------------------------------------------------------
void ProcessRadius1(vec2 DeltaUV,
                    CenterPixelData Center,
                    inout float TotalAO,
                    inout float TotalW)
{
    vec2 AODepth = PointSampleAODepth(Center.UV + DeltaUV);
    float DepthSlope = AODepth.y - Center.Depth;

    ProcessSample(AODepth, 1, DepthSlope, Center, TotalAO, TotalW);
    ProcessRadius(2, DeltaUV, DepthSlope, Center, TotalAO, TotalW);
}

//-------------------------------------------------------------------------
float ComputeBlur(vec2 uv,
                  vec2 DeltaUV,
                  out float CenterDepth)
{
    // Normalize for oversized input textures (allocAtLeast)
    uv /= vec2(textureSize(uInputLinear, 0)) * GlobalConstantBuffer_2;

    vec2 AOZ = PointSampleAODepth(uv);
    CenterDepth = AOZ.y;

    CenterPixelData Center;
    Center.UV = uv;
    Center.Depth = CenterDepth;
    Center.Sharpness = GlobalConstantBuffer_16;

    Center.Scale = Center.Sharpness;
    Center.Bias = -Center.Depth * Center.Sharpness;

    float TotalAO = AOZ.x;
    float TotalW = 1.0;

    ProcessRadius1(DeltaUV, Center, TotalAO, TotalW);
    ProcessRadius1(-DeltaUV, Center, TotalAO, TotalW);

    return TotalAO / TotalW;
}
)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/cssao/blurX.fsh", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/pass/ao/cssao/blurCommon.glsl>

in vec2 vTexCoord;

out vec4 fOut;

void main()
{
    float CenterDepth;
    float AO = ComputeBlur(vTexCoord, vec2(GlobalConstantBuffer_2.x, 0), CenterDepth);

    fOut = vec4(AO, CenterDepth, 1, 1);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/cssao/blurY.fsh", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/pass/ao/cssao/blurCommon.glsl>

in vec2 vTexCoord;

out vec4 fOut;

void main()
{
    float CenterDepth;
    float AO = ComputeBlur(vTexCoord, vec2(0, GlobalConstantBuffer_2.y), CenterDepth);

    fOut.x = pow(clamp(AO, 0.0, 1.0), GlobalConstantBuffer_11);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/cssao/coarseAO.fsh", R"%%RES_EMBED%%(
layout(std140) uniform uCSSAOGlobals {
    uvec4 GlobalConstantBuffer_0; // std140uvec4 buildVersion;

    vec2 GlobalConstantBuffer_1; // std140vec2 invQuarterResolution;
    vec2 GlobalConstantBuffer_2; // std140vec2 invFullResolution;
    vec2 GlobalConstantBuffer_3; // std140vec2 UVToViewA;
    vec2 GlobalConstantBuffer_4; // std140vec2 UVToViewB;

    float GlobalConstantBuffer_5; // std140float fRadiusToScreen;
    float GlobalConstantBuffer_6; // std140float fR2;
    float GlobalConstantBuffer_7; // std140float fNegInvR2;
    float GlobalConstantBuffer_8; // std140float fNDotVBias;

    float GlobalConstantBuffer_9; // std140float fSmallScaleAOAmount;
    float GlobalConstantBuffer_10; // std140float fLargeScaleAOAmount;
    float GlobalConstantBuffer_11; // std140float fPowExponent;
    int GlobalConstantBuffer_12; // std140int iUnused;

    float GlobalConstantBuffer_13; // std140float fBlurViewDepth0;
    float GlobalConstantBuffer_14; // std140float fBlurViewDepth1;
    float GlobalConstantBuffer_15; // std140float fBlurSharpness0;
    float GlobalConstantBuffer_16; // std140float fBlurSharpness1;

    float GlobalConstantBuffer_17; // std140float fLinearizeDepthA;
    float GlobalConstantBuffer_18; // std140float fLinearizeDepthB;
    float GlobalConstantBuffer_19; // std140float fInverseDepthRangeA;
    float GlobalConstantBuffer_20; // std140float fInverseDepthRangeB;

    vec2 GlobalConstantBuffer_21; // std140vec2 f2InputViewportTopLeft;
    float GlobalConstantBuffer_22; // std140float fViewDepthThresholdNegInv;
    float GlobalConstantBuffer_23; // std140float fViewDepthThresholdSharpness;

    float GlobalConstantBuffer_24; // std140float fBackgroundAORadiusPixels;
    float GlobalConstantBuffer_25; // std140float fForegroundAORadiusPixels;
    int GlobalConstantBuffer_26; // std140int iDebugNormalComponent;
};

layout(std140) uniform uCSSAOPerPass {
    vec4 f4Jitter;
    vec2 f2Offset;
    float fSliceIndex;
    uint uSliceIndex;
} uPPCB;

uniform sampler2DArray uInputDepth;
uniform sampler2D uInputNormals;

out float fOut;

#define NUM_DIRECTIONS 8
#define NUM_STEPS 4

const vec2 uvRescaleDenom = vec2(textureSize(uInputNormals, 0)) * GlobalConstantBuffer_2;

vec3 UVToView(vec2 UV, float ViewDepth)
{
    UV = GlobalConstantBuffer_3 * UV + GlobalConstantBuffer_4;
    return vec3(UV * ViewDepth, ViewDepth);
}

vec3 FetchFullResViewNormal(vec2 UV)
{
    return texture(uInputNormals, UV / uvRescaleDenom).xyz * 2.0 - 1.0;
}

vec3 FetchQuarterResViewPos(vec2 UV)
{
    float ViewDepth = texture(uInputDepth, vec3(UV / uvRescaleDenom, 0), 0).x;
    return UVToView(UV, ViewDepth);
}

vec2 RotateDirection(vec2 V, vec2 RotationCosSin)
{
    return vec2(V.x*RotationCosSin.x - V.y*RotationCosSin.y,
                  V.x*RotationCosSin.y + V.y*RotationCosSin.x);
}

float DepthThresholdFactor(float ViewDepth)
{
    return clamp((ViewDepth * GlobalConstantBuffer_22 + 1.0) * GlobalConstantBuffer_23, 0.0, 1.0);
}

struct AORadiusParams
{
    float fRadiusPixels;
    float fNegInvR2;
};

void ScaleAORadius(inout AORadiusParams Params, float ScaleFactor)
{
    Params.fRadiusPixels *= ScaleFactor;
    Params.fNegInvR2 *= 1.0 / (ScaleFactor * ScaleFactor);
}

AORadiusParams GetAORadiusParams(float ViewDepth)
{
    AORadiusParams Params;
    Params.fRadiusPixels = GlobalConstantBuffer_5 / ViewDepth;
    Params.fNegInvR2 = GlobalConstantBuffer_7;

    if (GlobalConstantBuffer_24 != -1.f)
    {
        ScaleAORadius(Params, max(1.0, GlobalConstantBuffer_24 / Params.fRadiusPixels));
    }

    if (GlobalConstantBuffer_25 != -1.f)
    {
        ScaleAORadius(Params, min(1.0, GlobalConstantBuffer_25 / Params.fRadiusPixels));
    }

    return Params;
}

float Falloff(float DistanceSquare, AORadiusParams Params)
{
    return DistanceSquare * Params.fNegInvR2 + 1.0;
}

//----------------------------------------------------------------------------------
// P = view-space position at the kernel center
// N = view-space normal at the kernel center
// S = view-space position of the current sample
//----------------------------------------------------------------------------------
float ComputeAO(vec3 P, vec3 N, vec3 S, AORadiusParams Params)
{
    vec3 V = S - P;
    float VdotV = dot(V, V);
    float NdotV = dot(N, V) * inversesqrt(VdotV);

    return clamp(NdotV - GlobalConstantBuffer_8, 0.0, 1.0) * clamp(Falloff(VdotV, Params), 0.0, 1.0);
}

void AccumulateAO(
    inout float AO,
    inout float RayPixels,
    float StepSizePixels,
    vec2 Direction,
    vec2 FullResUV,
    vec3 ViewPosition,
    vec3 ViewNormal,
    AORadiusParams Params
)
{
    vec2 SnappedUV = round(RayPixels * Direction) * GlobalConstantBuffer_1 + FullResUV;
    vec3 S = FetchQuarterResViewPos(SnappedUV);

    RayPixels += StepSizePixels;

    AO += ComputeAO(ViewPosition, ViewNormal, S, Params);
}

float ComputeCoarseAO(vec2 FullResUV, vec3 ViewPosition, vec3 ViewNormal, AORadiusParams Params)
{
    // Divide by NUM_STEPS+1 so that the farthest samples are not fully attenuated
    float StepSizePixels = (Params.fRadiusPixels / 4.0) / (NUM_STEPS + 1);

    vec4 Rand = uPPCB.f4Jitter;

    const float Alpha = 2.0 * 3.14159265f / NUM_DIRECTIONS;
    float SmallScaleAO = 0;
    float LargeScaleAO = 0;

    #pragma optionNV (unroll all)
    for (float DirectionIndex = 0; DirectionIndex < NUM_DIRECTIONS; ++DirectionIndex)
    {
        float Angle = Alpha * DirectionIndex;

        // Compute normalized 2D direction
        vec2 Direction = RotateDirection(vec2(cos(Angle), sin(Angle)), Rand.xy);

        // Jitter starting sample within the first step
        float RayPixels = (Rand.z * StepSizePixels + 1.0);

        {
            AccumulateAO(SmallScaleAO, RayPixels, StepSizePixels, Direction, FullResUV, ViewPosition, ViewNormal, Params);
        }

        #pragma optionNV (unroll all)
        for (float StepIndex = 1; StepIndex < NUM_STEPS; ++StepIndex)
        {
            AccumulateAO(LargeScaleAO, RayPixels, StepSizePixels, Direction, FullResUV, ViewPosition, ViewNormal, Params);
        }
    }

    float AO = (SmallScaleAO * GlobalConstantBuffer_9) + (LargeScaleAO * GlobalConstantBuffer_10);

    AO /= (NUM_DIRECTIONS * NUM_STEPS);

    return AO;
}

void main()
{
    vec2 inPos = gl_FragCoord.xy * 4.0 + uPPCB.f2Offset;
    vec2 inUv = inPos * (GlobalConstantBuffer_1 / 2.0); // Orig 4

    vec3 ViewPosition = FetchQuarterResViewPos(inUv);
    vec3 ViewNormal = FetchFullResViewNormal(inUv);

    AORadiusParams Params = GetAORadiusParams(ViewPosition.z);

    // Early exit if the projected radius is smaller than 1 full-res pixel
    //[branch]
    if (Params.fRadiusPixels < 1.0)
    {
        fOut = 1.0;
        return;
    }

    float AO = ComputeCoarseAO(inUv, ViewPosition, ViewNormal, Params);

    //[branch]
    if (GlobalConstantBuffer_23 != -1.f)
    {
        AO *= DepthThresholdFactor(ViewPosition.z);
    }

    fOut = clamp(1.0 - AO * 2.0, 0.0, 1.0);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/cssao/coarseAO.gsh", R"%%RES_EMBED%%(
layout(std140) uniform uCSSAOPerPass {
    vec4 f4Jitter;
    vec2 f2Offset;
    float fSliceIndex;
    uint uSliceIndex;
} uPPCB;

layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

void main()
{
    gl_Layer = int(uPPCB.uSliceIndex);

    for (int VertexID = 0; VertexID < 3; VertexID++)
    {
        vec2 texCoords = vec2( (VertexID << 1) & 2, VertexID & 2 );
        gl_Position = vec4( texCoords * vec2( 2.0, 2.0 ) + vec2( -1.0, -1.0) , 0.0, 1.0 );
        EmitVertex();
    }
    EndPrimitive();
}
)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/cssao/coarseAO.vsh", R"%%RES_EMBED%%(
void main()
{
    gl_Position = vec4(0);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/cssao/deinterleavedDepth.fsh", R"%%RES_EMBED%%(
layout(std140) uniform uCSSAOGlobals {

    uvec4 GlobalConstantBuffer_0;

    vec2 GlobalConstantBuffer_1;
    vec2 GlobalConstantBuffer_2;

    vec2 GlobalConstantBuffer_3;
    vec2 GlobalConstantBuffer_4;

    float GlobalConstantBuffer_5;
    float GlobalConstantBuffer_6;
    float GlobalConstantBuffer_7;
    float GlobalConstantBuffer_8;

    float GlobalConstantBuffer_9;
    float GlobalConstantBuffer_10;
    float GlobalConstantBuffer_11;
    int GlobalConstantBuffer_12;

    float GlobalConstantBuffer_13;
    float GlobalConstantBuffer_14;
    float GlobalConstantBuffer_15;
    float GlobalConstantBuffer_16;

    float GlobalConstantBuffer_17;
    float GlobalConstantBuffer_18;
    float GlobalConstantBuffer_19;
    float GlobalConstantBuffer_20;

    vec2 GlobalConstantBuffer_21;
    float GlobalConstantBuffer_22;
    float GlobalConstantBuffer_23;

    float GlobalConstantBuffer_24;
    float GlobalConstantBuffer_25;
    int GlobalConstantBuffer_26;
};

layout(std140) uniform uCSSAOPerPass {
    vec4 f4Jitter;
    vec2 f2Offset;
    float fSliceIndex;
    uint uSliceIndex;
} PerPassConstantBuffer_0;

uniform sampler2DRect uInputLinDepth;

out float fOut0;
out float fOut1;
out float fOut2;
out float fOut3;
out float fOut4;
out float fOut5;
out float fOut6;
out float fOut7;

void main()
{
    vec2 uv = (floor(gl_FragCoord.xy) * 4.0 + PerPassConstantBuffer_0.f2Offset);

    fOut0 = texture(uInputLinDepth, uv).x;
    fOut1 = textureOffset(uInputLinDepth, uv, ivec2(1, 0)).x;
    fOut2 = textureOffset(uInputLinDepth, uv, ivec2(2, 0)).x;
    fOut3 = textureOffset(uInputLinDepth, uv, ivec2(3, 0)).x;
    fOut4 = textureOffset(uInputLinDepth, uv, ivec2(0, 1)).z;
    fOut5 = textureOffset(uInputLinDepth, uv, ivec2(1, 1)).x;
    fOut6 = textureOffset(uInputLinDepth, uv, ivec2(2, 1)).x;
    fOut7 = textureOffset(uInputLinDepth, uv, ivec2(3, 1)).x;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/cssao/linearizeDepth.fsh", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/common/globals.hh>

layout(std140) uniform uCSSAOGlobals {
    uvec4 GlobalConstantBuffer_0;
    vec2 GlobalConstantBuffer_1;
    vec2 GlobalConstantBuffer_2;
    vec2 GlobalConstantBuffer_3;
    vec2 GlobalConstantBuffer_4;
    float GlobalConstantBuffer_5;
    float GlobalConstantBuffer_6;
    float GlobalConstantBuffer_7;
    float GlobalConstantBuffer_8;
    float GlobalConstantBuffer_9;
    float GlobalConstantBuffer_10;
    float GlobalConstantBuffer_11;
    int GlobalConstantBuffer_12;
    float GlobalConstantBuffer_13;
    float GlobalConstantBuffer_14;
    float GlobalConstantBuffer_15;
    float GlobalConstantBuffer_16;
    float GlobalConstantBuffer_17;
    float GlobalConstantBuffer_18;
    float GlobalConstantBuffer_19;
    float GlobalConstantBuffer_20;
    vec2 GlobalConstantBuffer_21;
    float GlobalConstantBuffer_22;
    float GlobalConstantBuffer_23;
    float GlobalConstantBuffer_24;
    float GlobalConstantBuffer_25;
    int GlobalConstantBuffer_26;
};

uniform sampler2DRect uInputDepth;
#if GLOW_PIPELINE_ENABLE_REVERSE_Z == 1
uniform float uNearPlane;
#endif

out float fOut;

void main()
{
    float depth = texture(uInputDepth, ivec2(gl_FragCoord.xy)).x;
#if GLOW_PIPELINE_ENABLE_REVERSE_Z == 1
    // clamp to F32 max to avoid NaN at the (infinite) far plane
    fOut = clamp(uNearPlane / depth, 0.f, GLOW_GLSL_FLT_MAX); 
#else
    depth = GlobalConstantBuffer_19 * depth + GlobalConstantBuffer_20;
    depth = clamp(depth, 0.0, 1.0);
    depth = depth * GlobalConstantBuffer_17 + GlobalConstantBuffer_18;
    fOut = 1.0 / depth;
#endif
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/cssao/reconstructNormal.fsh", R"%%RES_EMBED%%(
layout(std140) uniform uCSSAOGlobals {
    uvec4 GlobalConstantBuffer_0; // std140uvec4 buildVersion;

    vec2 GlobalConstantBuffer_1; // std140vec2 invQuarterResolution;
    vec2 GlobalConstantBuffer_2; // std140vec2 invFullResolution;
    vec2 GlobalConstantBuffer_3; // std140vec2 UVToViewA;
    vec2 GlobalConstantBuffer_4; // std140vec2 UVToViewB;

    float GlobalConstantBuffer_5; // std140float fRadiusToScreen;
    float GlobalConstantBuffer_6; // std140float fR2;
    float GlobalConstantBuffer_7; // std140float fNegInvR2;
    float GlobalConstantBuffer_8; // std140float fNDotVBias;

    float GlobalConstantBuffer_9; // std140float fSmallScaleAOAmount;
    float GlobalConstantBuffer_10; // std140float fLargeScaleAOAmount;
    float GlobalConstantBuffer_11; // std140float fPowExponent;
    int GlobalConstantBuffer_12; // std140int iUnused;

    float GlobalConstantBuffer_13; // std140float fBlurViewDepth0;
    float GlobalConstantBuffer_14; // std140float fBlurViewDepth1;
    float GlobalConstantBuffer_15; // std140float fBlurSharpness0;
    float GlobalConstantBuffer_16; // std140float fBlurSharpness1;

    float GlobalConstantBuffer_17; // std140float fLinearizeDepthA;
    float GlobalConstantBuffer_18; // std140float fLinearizeDepthB;
    float GlobalConstantBuffer_19; // std140float fInverseDepthRangeA;
    float GlobalConstantBuffer_20; // std140float fInverseDepthRangeB;

    vec2 GlobalConstantBuffer_21; // std140vec2 f2InputViewportTopLeft;
    float GlobalConstantBuffer_22; // std140float fViewDepthThresholdNegInv;
    float GlobalConstantBuffer_23; // std140float fViewDepthThresholdSharpness;

    float GlobalConstantBuffer_24; // std140float fBackgroundAORadiusPixels;
    float GlobalConstantBuffer_25; // std140float fForegroundAORadiusPixels;
    int GlobalConstantBuffer_26; // std140int iDebugNormalComponent;
};

uniform sampler2DRect uInputDepth;

in vec2 vTexCoord;

out vec3 fOut;

vec3 UVToView(vec2 UV, float viewDepth)
{
    UV = GlobalConstantBuffer_3 * UV + GlobalConstantBuffer_4;
    return vec3(UV * viewDepth, viewDepth);
}

vec3 MinDiff(vec3 P, vec3 Pr, vec3 Pl)
{
    vec3 V1 = Pr - P;
    vec3 V2 = P - Pl;
    return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

vec3 ReconstructNormal(vec2 UV, vec3 P)
{
    vec3 Pr = UVToView(UV + vec2(GlobalConstantBuffer_2.x, 0), textureOffset(uInputDepth, gl_FragCoord.xy, ivec2(1, 0)).x);
    vec3 Pl = UVToView(UV + vec2(-GlobalConstantBuffer_2.x, 0), textureOffset(uInputDepth, gl_FragCoord.xy, ivec2(-1, 0)).x);
    vec3 Pt = UVToView(UV + vec2(0, GlobalConstantBuffer_2.y), textureOffset(uInputDepth, gl_FragCoord.xy, ivec2(0, 1)).x);
    vec3 Pb = UVToView(UV + vec2(0, -GlobalConstantBuffer_2.y), textureOffset(uInputDepth, gl_FragCoord.xy, ivec2(0, -1)).x);
    return normalize(cross(MinDiff(P, Pr, Pl), MinDiff(P, Pt, Pb)));
}

void main()
{
    vec2 uv = vTexCoord.xy;

    vec3 viewPosition = UVToView(uv, textureOffset(uInputDepth, gl_FragCoord.xy, ivec2(0, 0)).x);
    vec3 viewNormal = ReconstructNormal(uv, viewPosition);

    fOut = viewNormal * 0.5 + vec3(.5);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/cssao/reinterleavedAO.fsh", R"%%RES_EMBED%%(
uniform sampler2DArray uInputAO;
uniform sampler2DRect uInputDepth;

in vec2 vTexCoord;

out vec4 fOut;

vec4 Input0;
vec4 Temp[2];
ivec4 Temp_int[2];
uvec4 Temp_uint[2];

void main()
{
    Input0.xy = gl_FragCoord.xy;
    Temp[0].xy = vec4(floor(Input0.xyxx)).xy;
    Temp[0].zw = vec4(abs(Temp[0].yyyx) * vec4(0.000000, 0.000000, 0.250000, 0.250000)).zw;
    Temp[0].xy = vec4(Temp[0].xyxx * vec4(0.250000, 0.250000, 0.000000, 0.000000)).xy;
    Temp_int[1].xy = ivec4(Temp[0].xyxx).xy;
    Temp[0].xy = vec4(fract(Temp[0].zwzz)).xy;
    Temp[0].x = vec4(dot((Temp[0].xyxx).xy, (vec4(16.000000, 4.000000, 0.000000, 0.000000)).xy)).x;
    Temp_int[1].z = int(Temp[0].x);
    Temp[1].w = vec4(0.000000).w;
    Temp[0].x = texelFetch(uInputAO, ivec3((Temp_int[1]).xyz), 0).x;
    fOut.x = vec4(Temp[0].x).x;
    Temp[0].x = (texture(uInputDepth, gl_FragCoord.xy)).x;
    fOut.y = vec4(Temp[0].x).y;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/sao/SAO.glsl", R"%%RES_EMBED%%(
#extension GL_EXT_gpu_shader4 : require
#extension GL_ARB_gpu_shader5 : enable
#include <glow-pipeline/internal/pass/ao/sao/reconstructCS.glsl>
#include <glow-pipeline/internal/common/globals.hh>
/**         

    SCALABLE AMBIENT OBSCURANCE


    Technique and implementation from:
        http://research.nvidia.com/sites/default/files/pubs/2012-06_Scalable-Ambient-Obscurance/McGuire12SAO.pdf
    modified code found in files:
        SAO.glsl, aoBlur.fsh, aoCSZ.fsh, aoCSZMinify.fsh, cszReconstruct.glsl

    Original license below
*/

/**
 author: Morgan McGuire and Michael Mara, NVIDIA Research

 Reference implementation of the Scalable Ambient Obscurance (SAO) screen-space ambient obscurance algorithm. 
 
 The optimized algorithmic structure of SAO was published in McGuire, Mara, and Luebke, Scalable Ambient Obscurance,
 <i>HPG</i> 2012, and was developed at NVIDIA with support from Louis Bavoil.

 The mathematical ideas of AlchemyAO were first described in McGuire, Osman, Bukowski, and Hennessy, The 
 Alchemy Screen-Space Ambient Obscurance Algorithm, <i>HPG</i> 2011 and were developed at 
 Vicarious Visions.  
 
 DX11 HLSL port by Leonardo Zide of Treyarch

  Open Source under the "BSD" license: http://www.opensource.org/licenses/bsd-license.php

  Copyright (c) 2011-2012, NVIDIA
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// Total number of direct samples to take at each pixel
// Paper value: 11
#define NUM_SAMPLES 11

// If using depth mip levels, the log of the maximum pixel offset before we need to switch to a lower 
// miplevel to maintain reasonable spatial locality in the cache
// If this number is too small (< 3), too many taps will land in the same pixel, and we'll get bad variance that manifests as flashing.
// If it is too high (> 5), we'll get bad performance because we're not using the MIP levels effectively
#define LOG_MAX_OFFSET 3

// Used for preventing AO computation on the sky (at infinite depth) and defining the CS Z to bilateral depth key scaling. 
// Does not have to match the real far plane
// Paper value: -300 / 300
#define FAR_PLANE_Z 1000.0

// The number of turns around the circle that the spiral pattern makes.  Should be prime to prevent
// taps from lining up. Paper value: 7, tuned for NUM_SAMPLES = 9
#define NUM_SPIRAL_TURNS 7

uniform float           uProjScale;
uniform sampler2D       uCSZBuffer;
uniform float           uRadius;
uniform float           uBias;
uniform float           uIntensityDivR6; // intensity / radius^6


/** Returns a unit vector and a screen-space radius for the tap on a unit disk (the caller should scale by the actual disk radius) */
vec2 tapLocation(int sampleNumber, float spinAngle, out float ssR){
    // Radius relative to ssR
    float alpha = float(sampleNumber + 0.5) * (1.0 / NUM_SAMPLES);
    float angle = alpha * (NUM_SPIRAL_TURNS * 6.28) + spinAngle;

    ssR = alpha;
    return vec2(cos(angle), sin(angle));
}


/** Used for packing Z into the GB channels */
float CSZToKey(float z) {
    return clamp(z * (1.0 / FAR_PLANE_Z), 0.0, 1.0);
}


/** Used for packing Z into the GB channels */
void packKey(float key, out vec2 p) {
    // Round to the nearest 1/256.0
    float temp = floor(key * 256.0);

    // Integer part
    p.x = temp * (1.0 / 256.0);

    // Fractional part
    p.y = key * 256.0 - temp;
}

 
/** Read the camera-space position of the point at screen-space pixel ssP */
vec3 getPosition(ivec2 ssP) {
    vec3 P;
    P.z = texelFetch(uCSZBuffer, ssP, 0).r;

    // Offset to pixel center
    P = reconstructCSPosition(vec2(ssP) + vec2(0.5), P.z);
    return P;
}


/** Read the camera-space position of the point at screen-space pixel ssP + unitOffset * ssR.  Assumes length(unitOffset) == 1 */
vec3 getOffsetPosition(ivec2 ssC, vec2 unitOffset, float ssR) {
    // Derivation:
    //  mipLevel = floor(log(ssR / MAX_OFFSET));
#   ifdef GL_EXT_gpu_shader5
        int mipLevel = clamp(findMSB(int(ssR)) - LOG_MAX_OFFSET, 0, GLOW_PIPELINE_LINEAR_DEPTH_MAX_MIP_LEVEL);
#   else
        int mipLevel = clamp(int(floor(log2(ssR))) - LOG_MAX_OFFSET, 0, GLOW_PIPELINE_LINEAR_DEPTH_MAX_MIP_LEVEL);
#   endif

    ivec2 ssP = ivec2(ssR * unitOffset) + ssC;
    
    vec3 P;

    // We need to divide by 2^mipLevel to read the appropriately scaled coordinate from a MIP-map.  
    // Manually clamp to the texture size because texelFetch bypasses the texture unit
    ivec2 mipP = clamp(ssP >> mipLevel, ivec2(0), textureSize(uCSZBuffer, mipLevel) - ivec2(1));
    P.z = texelFetch(uCSZBuffer, mipP, mipLevel).r;

    // Offset to pixel center
    P = reconstructCSPosition(vec2(ssP) + vec2(0.5), P.z);

    return P;
}


float radius2 = uRadius * uRadius;

/** Compute the occlusion due to sample with index \a i about the pixel at \a ssC that corresponds
    to camera-space point \a C with unit normal \a n_C, using maximum screen-space sampling uRadius \a ssDiskRadius 

    Note that units of H() in the HPG12 paper are meters, not
    unitless.  The whole falloff/sampling function is therefore
    unitless.  In this implementation, we factor out (9 / uRadius).

    Four versions of the falloff function are implemented below
*/
float sampleAO(in ivec2 ssC, in vec3 C, in vec3 n_C, in float ssDiskRadius, in int tapIndex, in float randomPatternRotationAngle) {
    // Offset on the unit disk, spun for this pixel
    float ssR;
    vec2 unitOffset = tapLocation(tapIndex, randomPatternRotationAngle, ssR);
    ssR *= ssDiskRadius;
        
    // The occluding point in camera space
    vec3 Q = getOffsetPosition(ssC, unitOffset, ssR);

    vec3 v = Q - C;

    float vv = dot(v, v);
    float vn = dot(v, n_C);

    const float epsilon = 0.01;
    
    // A: From the HPG12 paper
    // Note large epsilon to avoid overdarkening within cracks
    // return float(vv < radius2) * max((vn - uBias) / (epsilon + vv), 0.0) * radius2 * 0.6;

    // B: Smoother transition to zero (lowers contrast, smoothing out corners). [Recommended]
    float f = max(radius2 - vv, 0.0); return f * f * f * max((vn - uBias) / (epsilon + vv), 0.0);

    // C: Medium contrast (which looks better at high radii), no division.  Note that the 
    // contribution still falls off with uRadius^2, but we've adjusted the rate in a way that is
    // more computationally efficient and happens to be aesthetically pleasing.
    // return 4.0 * max(1.0 - vv * invRadius2, 0.0) * max(vn - uBias, 0.0);

    // D: Low contrast, no division operation
    // return 2.0 * float(vv < uRadius * uRadius) * max(vn - uBias, 0.0);
}

vec3 scalableAmbientObscurance() {

    // Pixel being shaded 
    ivec2 ssC = ivec2(gl_FragCoord.xy);

    // World space point being shaded
    vec3 C = getPosition(ssC);

    vec2 key = vec2(0);
    packKey(CSZToKey(C.z), key);
    
    // Unneccessary with depth test.
    if (C.z > FAR_PLANE_Z) {
        // We're on the skybox
        discard;
    }

    // Hash function used in the HPG12 AlchemyAO paper
    float randomPatternRotationAngle = (3 * ssC.x ^ ssC.y + ssC.x * ssC.y) * 10;

    // Reconstruct normals from positions. These will lead to 1-pixel black lines
    // at depth discontinuities, however the blur will wipe those out so they are not visible
    // in the final image.
    vec3 n_C = reconstructCSFaceNormal(C);
    
    // Choose the screen-space sample radius
    // proportional to the projected area of the sphere
    float ssDiskRadius = -uProjScale * uRadius / C.z;
    
    float sum = 0.0;
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        sum += sampleAO(ssC, C, n_C, ssDiskRadius, i, randomPatternRotationAngle);
    }

    float A = max(0.0, 1.0 - sum * uIntensityDivR6 * (5.0 / NUM_SAMPLES));

    // Bilateral box-filter over a quad for free, respecting depth edges
    // (the difference that this makes is subtle)
    if (abs(dFdx(C.z)) < 0.02) {
        A -= dFdx(A) * ((ssC.x & 1) - 0.5);
    }
    if (abs(dFdy(C.z)) < 0.02) {
        A -= dFdy(A) * ((ssC.y & 1) - 0.5);
    }

    return vec3(A, key);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/sao/aoBlur.fsh", R"%%RES_EMBED%%(
#extension GL_EXT_gpu_shader4 : require

//////////////////////////////////////////////////////////////////////////////////////////////
// Tunable Parameters:

/** Increase to make depth edges crisper. Decrease to reduce flicker. */
#define EDGE_SHARPNESS     (1.0)

/** Step in 2-pixel intervals since we already blurred against neighbors in the
    first AO pass.  This constant can be increased while R decreases to improve
    performance at the expense of some dithering artifacts. 
    
    Morgan found that a scale of 3 left a 1-pixel checkerboard grid that was
    unobjectionable after shading was applied but eliminated most temporal incoherence
    from using small numbers of sample taps.
    */
#define SCALE               (2)

/** Filter radius in pixels. This will be multiplied by SCALE. */
#define R                   (4)


//////////////////////////////////////////////////////////////////////////////////////////////

/** Type of data to read from uInput.  This macro allows
    the same blur shader to be used on different kinds of input data. */
#define VALUE_TYPE        float

/** Swizzle to use to extract the channels of uInput. This macro allows
    the same blur shader to be used on different kinds of input data. */
#define VALUE_COMPONENTS   r

#define VALUE_IS_KEY       0

/** Channel encoding the bilateral key value (which must not be the same as VALUE_COMPONENTS) */
#define KEY_COMPONENTS     gb


// Gaussian coefficients
const float gaussian[R + 1] = 
# if R == 3
    float[](0.153170, 0.144893, 0.122649, 0.092902);
# elif R == 4
    //float[](0.398943, 0.241971, 0.053991, 0.004432, 0.000134);  // stddev = 1.0
    float[](0.153170, 0.144893, 0.122649, 0.092902, 0.062970);  // stddev = 2.0
# elif R == 6
    float[](0.111220, 0.107798, 0.098151, 0.083953, 0.067458, 0.050920, 0.036108); // stddev = 3.0
# endif

uniform sampler2D   uInput;

/** (1, 0) or (0, 1)*/
uniform ivec2       uAxis;

out vec3            fAOOut;

#define  result         fAOOut.VALUE_COMPONENTS
#define  keyPassThrough fAOOut.KEY_COMPONENTS

/** Returns a number on (0, 1) */
float unpackKey(vec2 p) {
    return p.x * (256.0 / 257.0) + p.y * (1.0 / 257.0);
}


void main() {
    ivec2 ssC = ivec2(gl_FragCoord.xy);

    vec4 temp = texelFetch(uInput, ssC, 0);
    
    keyPassThrough = temp.KEY_COMPONENTS;
    float key = unpackKey(keyPassThrough);

    VALUE_TYPE sum = temp.VALUE_COMPONENTS;

    if (key == 1.0) { 
        // Sky pixel (if you aren't using depth keying, disable this test)
        result = sum;
        return;
    }

    // Base weight for depth falloff.  Increase this for more blurriness,
    // decrease it for better edge discrimination
    float BASE = gaussian[0];
    float totalWeight = BASE;
    sum *= totalWeight;

    for (int r = -R; r <= R; ++r) {
        // We already handled the zero case above.  This loop should be unrolled and the static branch optimized out,
        // so the IF statement has no runtime cost
        if (r != 0) {
            temp = texelFetch(uInput, ssC + uAxis * (r * SCALE), 0);
            float      tapKey = unpackKey(temp.KEY_COMPONENTS);
            VALUE_TYPE value  = temp.VALUE_COMPONENTS;
            
            // spatial domain: offset gaussian tap
            float weight = 0.3 + gaussian[abs(r)];
            
            // range domain (the "bilateral" weight). As depth difference increases, decrease weight.
            weight *= max(0.0, 1.0
                - (EDGE_SHARPNESS * 2000.0) * abs(tapKey - key)
                );

            sum += value * weight;
            totalWeight += weight;
        }
    }
 
    const float epsilon = 0.0001;
    result = sum / (totalWeight + epsilon);	
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/sao/aoPass.fsh", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/pass/ao/sao/SAO.glsl>

out vec3 fAOOut;

void main()
{
    fAOOut = scalableAmbientObscurance();
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/sao/linearDepth.fsh", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/common/cszReconstruct.glsl>

out float fLinearDepth;

uniform sampler2DRect gDepth;

//uniform float uFocusDistance;
//uniform float uFocusRange;
//uniform float uBackgroundBlur;

void main() {
    float depth = texelFetch(gDepth, ivec2(gl_FragCoord.xy)).x;
    fLinearDepth = reconstructCSZ(depth);

//    float coc = (
//        (depth < 1) ?
//        (linDepth - uFocusDistance) / uFocusRange
//        : uBackgroundBlur
//    );

//    fLinearDepth = vec2(linDepth, coc);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/sao/linearDepthMinify.fsh", R"%%RES_EMBED%%(
#extension GL_EXT_gpu_shader4 : require

uniform sampler2D uInput;
uniform int uPreviousMIPLevel;

out vec2 fLinearDepth;

void main() {
    ivec2 ssP = ivec2(gl_FragCoord.xy);

    // Rotated grid subsampling to avoid XY directional bias or Z precision bias while downsampling.
    // On DX9, the bit-and can be implemented with floating-point modulo
    fLinearDepth = 
        vec2(
            texelFetch2D(uInput, clamp(ssP * 2 + ivec2(ssP.y & 1, ssP.x & 1), ivec2(0), textureSize(uInput, uPreviousMIPLevel) - ivec2(1)), uPreviousMIPLevel).r,
            0 // CoC is not properly downsampled
        );
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/ao/sao/reconstructCS.glsl", R"%%RES_EMBED%%(
/**  vec4(-2.0f / (width*P[0][0]), 
          -2.0f / (height*P[1][1]),
          ( 1.0f - P[0][2]) / P[0][0], 
          ( 1.0f + P[1][2]) / P[1][1])
    
        mCameraData.projInfo = glm::vec4(
            float(-2.f / (mCameraData.resolution.x * mCameraData.proj[0][0])), float(-2.f / (mCameraData.resolution.y * mCameraData.proj[1][1])),
            float((1.f - mCameraData.proj[0][2]) / mCameraData.proj[0][0]), float((1.f + mCameraData.proj[1][2]) / mCameraData.proj[1][1]));
            
    where P is the projection matrix that maps camera space points 
    to [-1, 1] x [-1, 1].  That is, GCamera::getProjectUnit(). */
uniform vec4 uProjInfo;

/** Reconstruct camera-space P.xyz from screen-space S = (x, y) in
    pixels and camera-space z < 0.  Assumes that the upper-left pixel center
    is at (0.5, 0.5) [but that need not be the location at which the sample tap 
    was placed!]

    Costs 3 MADD.  Error is on the order of 10^3 at the far plane, partly due to z precision.
  */
vec3 reconstructCSPosition(vec2 S, float z) {
    return vec3((S.xy * uProjInfo.xy + uProjInfo.zw) * z, z);
}

/** Reconstructs screen-space unit normal from screen-space position */
vec3 reconstructCSFaceNormal(vec3 C) {
    return normalize(cross(dFdy(C), dFdx(C)));
}

vec3 reconstructNonUnitCSFaceNormal(vec3 C) {
    return cross(dFdy(C), dFdx(C));
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/depthpre/clusterAabbAssignment.csh", R"%%RES_EMBED%%(
layout(local_size_x = 1, local_size_y = 1) in;

#include <glow-pipeline/internal/common/globals.hh>
#include <glow-pipeline/internal/pass/depthpre/clusteringStructs.glsl>

layout (std430) restrict writeonly buffer sClusterAABBs {
    AABB clusterAABBs[];
};

uniform mat4 uProjInv;
uniform float uNearPlane;
uniform float uFarPlane;
uniform ivec2 uResolution;

vec4 clipToView(vec4 clip);
vec4 screenToView(vec4 screen);
vec3 lineIntersectionToZPlane(vec3 B, float zDistance);

void main() {
    uint tileIndex = gl_WorkGroupID.x +
                     gl_WorkGroupID.y * gl_NumWorkGroups.x +
                     gl_WorkGroupID.z * (gl_NumWorkGroups.x * gl_NumWorkGroups.y);

    // Min and max point in screen space
    vec4 maxPointSS = vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y + 1) * GLOW_PIPELINE_CLUSTER_PIXEL_SIZE_X, -1.0, 1.0); // Top Right
    vec4 minPointSS = vec4(gl_WorkGroupID.xy * GLOW_PIPELINE_CLUSTER_PIXEL_SIZE_X, -1.0, 1.0); // Bottom left
    
    vec3 maxPointVS = screenToView(maxPointSS).xyz;
    vec3 minPointVS = screenToView(minPointSS).xyz;

    // Near and far values of the cluster in view space
    float tileNear  = -uNearPlane * pow(uFarPlane / uNearPlane, gl_WorkGroupID.z / float(gl_NumWorkGroups.z));
    float tileFar   = -uNearPlane * pow(uFarPlane / uNearPlane, (gl_WorkGroupID.z + 1) / float(gl_NumWorkGroups.z));

    // 4 intersection points to the cluster near/far plane
    vec3 minPointNear = lineIntersectionToZPlane(minPointVS, tileNear);
    vec3 minPointFar  = lineIntersectionToZPlane(minPointVS, tileFar);
    vec3 maxPointNear = lineIntersectionToZPlane(maxPointVS, tileNear);
    vec3 maxPointFar  = lineIntersectionToZPlane(maxPointVS, tileFar);

    vec3 minPointAABB = min(min(minPointNear, minPointFar),min(maxPointNear, maxPointFar));
    vec3 maxPointAABB = max(max(minPointNear, minPointFar),max(maxPointNear, maxPointFar));

    clusterAABBs[tileIndex].min = vec4(minPointAABB, 0);
    clusterAABBs[tileIndex].max = vec4(maxPointAABB, 0);
}

// Intersection point of a line from the camera to a point in screen space
// on a z oriented plane at the given distance
vec3 lineIntersectionToZPlane(vec3 B, float zDistance) {
    const vec3 normal = vec3(0, 0, 1);

    // Intersection length for the line and the plane
    float t = (zDistance - dot(normal, vec3(0))) / dot(normal, B);
    // Position of the point along the line
    return t * B;
}

vec4 clipToView(vec4 clip) {
    vec4 view = uProjInv * clip;
    return view / view.w;
}

vec4 screenToView(vec4 screen) {
    vec2 texCoord = screen.xy / uResolution;
    vec4 clip = vec4(vec2(texCoord.x, texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    return clipToView(clip);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/depthpre/clusterLightAssignment.csh", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/common/globals.hh>

layout(local_size_x = GLOW_PIPELINE_LASSIGN_COMPUTE_X, local_size_y = GLOW_PIPELINE_LASSIGN_COMPUTE_Y, local_size_z = GLOW_PIPELINE_CLUSTER_AMOUNT_Z) in;

#include <glow-pipeline/internal/pass/depthpre/clusteringStructs.glsl>

// Cluster AABBs
layout(std430) restrict readonly buffer sClusterAABBs {
    AABB clusterAABBs[]; // Size: clusters.x * clusters.y * clusters.z
};

// All lights
layout(std430) restrict readonly buffer sLightData {
    PackedLightData lights[];
} ssboLightData;

// Global continuous lightIndex index list, indices into sLightData
layout(std430) restrict writeonly buffer sClusterLightIndexList {
    uint lightIndexList[];
} ssboLightIndexList;

// Cluster visibilities, indices into sClusterLightIndexList
layout(std430) restrict writeonly buffer sClusterVisibilities {
    ClusterVisibility data[];
} ssboClusterVisibilities;

// Single global index count, added atomically, to calculate offsets into the lightIndex index list
layout(std430) restrict buffer sGlobalIndexCount {
    uint globalIndexCount;
};

#define THREAD_COUNT (gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z)
#define LIGHTS_PER_BATCH (THREAD_COUNT)

shared vec4 sharedLights[LIGHTS_PER_BATCH];

uniform mat4 uView;

bool doesLightIntersectAabb(vec4 lightData, const AABB b){
    const vec3 center = vec3(uView * vec4(lightData.xyz, 1));
    const float radius = lightData.w;
    
    float sqDist = 0.0;

    // If this loop isn't unrolled manually, it causes a compiler / linker hang
    float v = center[0];
    if( v < b.min[0] ) sqDist += (b.min[0] - v) * (b.min[0] - v);
    if( v > b.max[0] ) sqDist += (v - b.max[0]) * (v - b.max[0]);
    v = center[1];
    if( v < b.min[1] ) sqDist += (b.min[1] - v) * (b.min[1] - v);
    if( v > b.max[1] ) sqDist += (v - b.max[1]) * (v - b.max[1]);
    v = center[2];
    if( v < b.min[2] ) sqDist += (b.min[2] - v) * (b.min[2] - v);
    if( v > b.max[2] ) sqDist += (v - b.max[2]) * (v - b.max[2]);

    return sqDist <= (radius * radius);
}

vec4 getBoundingSphere(PackedLightData light)
{
    // center (xyz) and bounding sphere radius (w)
    return vec4(
        (light.aSize.xyz + light.bRadius.xyz) * 0.5, 
        length(light.aSize.xyz - light.bRadius.xyz) * 0.5 + light.aSize.w + light.bRadius.w 
    );
}

void main() {
    // Reset the global index count
    globalIndexCount = 0;

    const uint lightCount = ssboLightData.lights.length();
    const uint batchAmount = (lightCount + LIGHTS_PER_BATCH - 1) / LIGHTS_PER_BATCH;

    uint clusterIndex = gl_GlobalInvocationID.x +
                     gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x +
                     gl_GlobalInvocationID.z * (gl_NumWorkGroups.x * gl_WorkGroupSize.x * gl_NumWorkGroups.y * gl_WorkGroupSize.y);
    const AABB clusterAabb = clusterAABBs[clusterIndex];

    uint visibleLightCount = 0;
    uint visibleLightIndices[GLOW_PIPELINE_MAX_LIGHTS_PER_CLUSTER];

    // Simple, brute-force variant
    // for (int i = 0; i < lightCount; ++i)
    // {
    //     if (doesLightIntersectAabb(getBoundingSphere(ssboLightData.lights[i]), clusterAabb))
    //     {
    //         visibleLightIndices[visibleLightCount] = i;
    //         ++visibleLightCount;
    //     } 
    // }

    // Load lights in batches into shared workgroup memory
    for (uint batch = 0; batch < batchAmount; ++batch) {
        uint lightIndex = batch * LIGHTS_PER_BATCH + gl_LocalInvocationIndex;
        // Prevent fetching a light out of bounds
        lightIndex = min(lightIndex, lightCount);

        // Load this thread's light into the shared light storage, compressed to the bounding sphere
        sharedLights[gl_LocalInvocationIndex] = getBoundingSphere(ssboLightData.lights[lightIndex]);

        barrier();

        // Test all lights of the batch against this thread's cluster AABB
        const uint maxBatchLightIndex = min(lightCount - batch * LIGHTS_PER_BATCH, LIGHTS_PER_BATCH);
        for (uint lightIndex = 0; lightIndex < maxBatchLightIndex; ++lightIndex) {
            if (doesLightIntersectAabb(sharedLights[lightIndex], clusterAabb)) {
                visibleLightIndices[visibleLightCount] = batch * LIGHTS_PER_BATCH + lightIndex;
                ++visibleLightCount;
            }
        }
    }

    // Wait until all threads are done
    barrier();

    uint offset = atomicAdd(globalIndexCount, visibleLightCount);

    for (uint i = 0; i < visibleLightCount; ++i) {
        ssboLightIndexList.lightIndexList[offset + i] = visibleLightIndices[i];
    }

    ssboClusterVisibilities.data[clusterIndex].offset = offset;
    ssboClusterVisibilities.data[clusterIndex].count = visibleLightCount;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/depthpre/clustering.glsl", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/common/globals.hh>
#include <glow-pipeline/internal/common/cszReconstruct.glsl>

uniform uvec3 uClusterDimensions;
uniform vec2 uClusteringScaleBias;

// Returns the cluster coordinate for the current fragment
uvec3 getCurrentCluster()
{
    uint zTile = uint(max(log2(reconstructCSZ(gl_FragCoord.z)) * uClusteringScaleBias.x + uClusteringScaleBias.y, 0.0));
    return uvec3(uvec2(gl_FragCoord.xy / GLOW_PIPELINE_CLUSTER_PIXEL_SIZE_X), zTile);
}

// Returns the 1D cluster index for a given cluster coordinate
uint getClusterIndex(in uvec3 cluster)
{
	return cluster.x + uClusterDimensions.x * cluster.y + (uClusterDimensions.x * uClusterDimensions.y) * cluster.z;
}

// Returns the 1D cluster index for the current fragment
uint getCurrentClusterIndex()
{
	return getClusterIndex(getCurrentCluster());
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/depthpre/clusteringStructs.glsl", R"%%RES_EMBED%%(

// A packed Light, as it is uploaded to the GPU
struct PackedLightData {
    vec4 aSize;
    vec4 bRadius;
    vec4 colorMaskId;
};

// An unpacked, usable light
struct LightData {
    vec3 posA;
    vec3 posB;
    float radius;
    float size;
    vec3 color;
    int maskId;
};

// The offset and count of a cluster into the light index list
struct ClusterVisibility {
    uint offset;
    uint count;
};

// The AABB of a cluster in view space
struct AABB {
    vec4 min;
    vec4 max;
};

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/lightingcombination/atmosphericScattering.glsl", R"%%RES_EMBED%%(
// Directional inscattering height fog, reference: UE4 implementation
// See:
    // https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Shaders/Private/HeightFogCommon.ush
    // https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Shaders/Private/AtmosphericFogShader.usf
    // https://forums.unrealengine.com/development-discussion/rendering/108197-implementing-exponential-height-fog-in-a-custom-shader

vec3 getAtmosphericScattering(in float worldHeight, float fragDepth, in vec3 camPos, in vec3 V, in vec3 lightDirection, in float distanceToCamera, in vec3 lightColor, in float sunIntensity)
{
    // Distance in world units from the camera beyond which the sun starts bleeding into geometry
    const float inscatteringStartDistance = 100; 
    // Controls the sharpness of the sun
    const float inscatteringExponent = 100;
    // Controls the size of the sun
    const float inscatteringMultiplier = 1.02;

    float exponentialFog = 1.0 - exp(-distanceToCamera * gPipelineScene.fogDensity);

    // Correction multiplier to ensure inscattering is behind near objects
    // At the far plane, always show inscattering
    float fragDepthWorldUnits = fragDepth * uPipelineProjectionInfo.viewportNearFar.z;
    float distanceCorrection = (fragDepth == GLOW_PIPELINE_HORIZON_DEPTH ? 1.0 : clamp((fragDepthWorldUnits - inscatteringStartDistance) / uPipelineProjectionInfo.viewportNearFar.w, 0, 1));
    
    float inscatteringAmount = max(dot(-V, (normalize(lightDirection) * inscatteringMultiplier)), 0.0) * distanceCorrection;

    vec3 outputColor = mix(gPipelineScene.atmoScatterColor, lightColor, pow(inscatteringAmount, inscatteringExponent) * sunIntensity);
    float falloffCorrection = clamp(((((worldHeight - gPipelineScene.atmoScatterFalloffStart) / ( gPipelineScene.atmoScatterFalloffEnd - gPipelineScene.atmoScatterFalloffStart)) * (0 - 1)) + 1), 0, 1);

    return outputColor * exponentialFog * pow(falloffCorrection, gPipelineScene.atmoScatterFalloffExponent);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/lightingcombination/combination.fsh", R"%%RES_EMBED%%(
#glow pipeline internal_projectionInfoUbo
#glow pipeline sceneInfo

#include <glow-pipeline/internal/common/util.glsl>
#include <glow-pipeline/internal/common/globals.hh>
#include <glow-pipeline/internal/common/positionReconstruct.glsl>
#include <glow-pipeline/internal/pass/lightingcombination/atmosphericScattering.glsl>

in vec4 vScreenPosition;

// IBuffer-sized
uniform sampler2DRect uShadedOpaque;
uniform sampler2DRect uDepth;

// T/GBuffer-sized
uniform sampler2DRect uAccumulationA;
uniform sampler2DRect uAccumulationB;
uniform sampler2DRect uDistortion;

uniform vec3 uCamPos;

out vec3 fHDR;

void main() 
{
    ivec2 coords = ivec2(gl_FragCoord.xy);

    // Read TBuffer
    vec4 tAccuA = texelFetch(uAccumulationA, coords);
    float tAccuB = texelFetch(uAccumulationB, coords).x;

    // Assemble weighted OIT
    float alpha = 1 - tAccuA.a;
    vec4 accu = vec4(tAccuA.xyz, tAccuB);
    vec3 transparentColor = accu.rgb / clamp(accu.a, 1e-4, 5e4);

    //  OIT Offset (Refraction)
    vec2 offset = texelFetch(uDistortion, coords).xy;
    vec3 opaqueColor = texture(uShadedOpaque, gl_FragCoord.xy + offset).rgb;

    vec3 hdrColor = mix(opaqueColor, transparentColor, alpha);

    // Color debug
    //hdrColor = vec3(texelFetch(uAccumulationA, coords).rgb / texelFetch(uAccumulationB, coords).x);
    
    // Distortion debug
    //hdrColor = vec3(fract(texelFetch(uDistortion, coords).xy / 30), 0);


    float fragDepth = texelFetch(uDepth, coords).x;
    vec3 worldPos = reconstructWorldPosition(vScreenPosition, fragDepth);

    vec3 cameraDistanceVector = uCamPos - worldPos;
    vec3 V = normalize(cameraDistanceVector);

    if (gPipelineScene.atmoScatterIntensity > 0.0)
    {
        // Normalize distance and height at the far plane
        // Otherwise fog would drastically change based on far plane settings
        const float simulatedFarPlane = 2000.0;

#if GLOW_PIPELINE_ENABLE_REVERSE_Z == 1
        float cameraDistance = fragDepth == 0.0 ? simulatedFarPlane : sqrt(dot(cameraDistanceVector, cameraDistanceVector));
        float worldHeight = fragDepth == 0.0 ? 0 : worldPos.y;
#else
        float cameraDistance = fragDepth == 1.0 ? simulatedFarPlane : sqrt(dot(cameraDistanceVector, cameraDistanceVector));
        float worldHeight = fragDepth == 1.0 ? worldPos.y * (simulatedFarPlane / uPipelineProjectionInfo.viewportNearFar.w) : worldPos.y;
#endif
        
        vec3 atmoScatter = gPipelineScene.atmoScatterIntensity * getAtmosphericScattering(worldHeight, fragDepth, uCamPos, V, gPipelineScene.sunDirection, cameraDistance, gPipelineScene.sunColor, gPipelineScene.sunIntensity);
        hdrColor += atmoScatter;
    }

    // HDR output and Bloom extraction
    // TODO: This clamp should not be required, but sometimes colors become negative in the preceding passes
    fHDR = max(hdrColor, vec3(0));
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/lightingcombination/kawaseBlur.fsh", R"%%RES_EMBED%%(
uniform sampler2DRect uInput;

uniform float uDistance;

out vec3 fColor;

void main() 
{
    vec3 color = vec3(0);
    color += texture(uInput, gl_FragCoord.xy + (uDistance + 0.5) * vec2(+1, +1)).rgb;
    color += texture(uInput, gl_FragCoord.xy + (uDistance + 0.5) * vec2(+1, -1)).rgb;
    color += texture(uInput, gl_FragCoord.xy + (uDistance + 0.5) * vec2(-1, +1)).rgb;
    color += texture(uInput, gl_FragCoord.xy + (uDistance + 0.5) * vec2(-1, -1)).rgb;
    fColor = color / 4.0;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/lightingcombination/medianFilter.fsh", R"%%RES_EMBED%%(
/*
Modified version, source:
http://casual-effects.com/research/McGuire2008Median/index.html

3x3 Median
Morgan McGuire and Kyle Whitson
http://graphics.cs.williams.edu


Copyright (c) Morgan McGuire and Williams College, 2006
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

uniform sampler2DRect uInput;

#define s2(a, b)				temp = a; a = min(a, b); b = max(temp, b);
#define mn3(a, b, c)			s2(a, b); s2(a, c);
#define mx3(a, b, c)			s2(b, c); s2(a, c);

#define mnmx3(a, b, c)			mx3(a, b, c); s2(a, b);                                   // 3 exchanges
#define mnmx4(a, b, c, d)		s2(a, b); s2(c, d); s2(a, c); s2(b, d);                   // 4 exchanges
#define mnmx5(a, b, c, d, e)	s2(a, b); s2(c, d); mn3(a, c, e); mx3(b, d, e);           // 6 exchanges
#define mnmx6(a, b, c, d, e, f) s2(a, d); s2(b, e); s2(c, f); mn3(a, b, c); mx3(d, e, f); // 7 exchanges

out vec3 fColor;

void main() {

  vec3 v[9];

  // Add the pixels which make up our window to the pixel array.
  for(int dX = -1; dX <= 1; ++dX) {
    for(int dY = -1; dY <= 1; ++dY) {		
      vec2 offset = vec2(float(dX), float(dY));
		    
      // If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
      // pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
      // bottom right pixel of the window at pixel[N-1].
      v[(dX + 1) * 3 + (dY + 1)] = texture(uInput, gl_FragCoord.xy + offset).rgb;
    }
  }

  vec3 temp;

  // Starting with a subset of size 6, remove the min and max each time
  mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);
  mnmx5(v[1], v[2], v[3], v[4], v[6]);
  mnmx4(v[2], v[3], v[4], v[7]);
  mnmx3(v[3], v[4], v[8]);

  fColor = v[4];
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/opaque/shadowing.glsl", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/common/globals.hh>
#include <glow-pipeline/internal/common/cszReconstruct.glsl>

layout(std140) uniform uShadowLightVPUBO {
    mat4 lightVPs[GLOW_PIPELINE_SHADOW_CASCADE_AMOUNT];
};

layout(std140) uniform uShadowCascadeLimitUBO {
    vec4 splitDepths;
    // float array
};

uniform sampler2DArrayShadow uShadowMaps;

const uvec2 global_shadowMapDimensions = uvec2(GLOW_PIPELINE_SHADOW_MAP_SIZE, GLOW_PIPELINE_SHADOW_MAP_SIZE);

// Either 3, 5 or 7
#define FilterSize_ 5

// Set to 1 to enable filtering across cascades
#define FilterAcrossCascades_ 1

vec2 computeReceiverPlaneDepthBias(vec3 texCoordDX, vec3 texCoordDY)
{
    vec2 biasUV;
    biasUV.x = texCoordDY.y * texCoordDX.z - texCoordDX.y * texCoordDY.z;
    biasUV.y = texCoordDX.x * texCoordDY.z - texCoordDY.x * texCoordDX.z;
    biasUV *= 1.0 / ((texCoordDX.x * texCoordDY.y) - (texCoordDX.y * texCoordDY.x));
    return biasUV;
}

float sampleShadowMap(in vec2 base_uv, in float u, in float v, in vec2 shadowMapSizeInv,
                      in uint cascadeIdx,  in float depth, in vec2 receiverPlaneDepthBias) {
    const vec2 uv = base_uv + vec2(u, v) * shadowMapSizeInv;
    const float z = depth + dot(vec2(u, v) * shadowMapSizeInv, receiverPlaneDepthBias);
    return texture(uShadowMaps, vec4(uv, cascadeIdx, z));
}

// Optimized Percentage Closer Filtering
// [The Witness] algorithm
// Ported and adjusted from https://mynameismjp.wordpress.com/2013/09/10/shadow-maps/ 
float sampleShadowCascade(vec3 shadowPos, vec3 shadowPosDX, vec3 shadowPosDY, uint cascadeIdx)
{
    float lightDepth = shadowPos.z;

    // -- Receiver plane depth bias --
    const vec2 shadowMapSizeInv = vec2(1.0) / global_shadowMapDimensions;
    const vec2 receiverPlaneDepthBias = computeReceiverPlaneDepthBias(shadowPosDX, shadowPosDY);

    // Static depth biasing to make up for incorrect fractional sampling on the shadow map grid
    // NOTE: This causes rather obvious peter panning
    const float fractionalSamplingError = 2 * dot(vec2(1.0, 1.0) * shadowMapSizeInv, abs(receiverPlaneDepthBias));
    lightDepth -= min(fractionalSamplingError, 0.01f);

    const vec2 uv = shadowPos.xy * global_shadowMapDimensions; // 1 unit - 1 texel

    vec2 base_uv;
    base_uv.x = floor(uv.x + 0.5);
    base_uv.y = floor(uv.y + 0.5);

    const float s = (uv.x + 0.5 - base_uv.x);
    const float t = (uv.y + 0.5 - base_uv.y);

    base_uv -= vec2(0.5, 0.5);
    base_uv *= shadowMapSizeInv; // [0, 1]

    float sum = 0;

    #if  FilterSize_ == 3

        float uw0 = (3 - 2 * s);
        float uw1 = (1 + 2 * s);

        float u0 = (2 - s) / uw0 - 1;
        float u1 = s / uw1 + 1;

        float vw0 = (3 - 2 * t);
        float vw1 = (1 + 2 * t);

        float v0 = (2 - t) / vw0 - 1;
        float v1 = t / vw1 + 1;

        sum += uw0 * vw0 * sampleShadowMap(base_uv, u0, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw1 * vw0 * sampleShadowMap(base_uv, u1, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw0 * vw1 * sampleShadowMap(base_uv, u0, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw1 * vw1 * sampleShadowMap(base_uv, u1, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);

        return sum * 1.0 / 16;

    #elif FilterSize_ == 5

        float uw0 = (4 - 3 * s);
        float uw1 = 7;
        float uw2 = (1 + 3 * s);

        float u0 = (3 - 2 * s) / uw0 - 2;
        float u1 = (3 + s) / uw1;
        float u2 = s / uw2 + 2;

        float vw0 = (4 - 3 * t);
        float vw1 = 7;
        float vw2 = (1 + 3 * t);

        float v0 = (3 - 2 * t) / vw0 - 2;
        float v1 = (3 + t) / vw1;
        float v2 = t / vw2 + 2;

        sum += uw0 * vw0 * sampleShadowMap(base_uv, u0, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw1 * vw0 * sampleShadowMap(base_uv, u1, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw2 * vw0 * sampleShadowMap(base_uv, u2, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);

        sum += uw0 * vw1 * sampleShadowMap(base_uv, u0, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw1 * vw1 * sampleShadowMap(base_uv, u1, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw2 * vw1 * sampleShadowMap(base_uv, u2, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);

        sum += uw0 * vw2 * sampleShadowMap(base_uv, u0, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw1 * vw2 * sampleShadowMap(base_uv, u1, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw2 * vw2 * sampleShadowMap(base_uv, u2, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);

        return sum * 1.0 / 144.0;

    #else // FilterSize_ == 7

        float uw0 = (5 * s - 6);
        float uw1 = (11 * s - 28);
        float uw2 = -(11 * s + 17);
        float uw3 = -(5 * s + 1);

        float u0 = (4 * s - 5) / uw0 - 3;
        float u1 = (4 * s - 16) / uw1 - 1;
        float u2 = -(7 * s + 5) / uw2 + 1;
        float u3 = -s / uw3 + 3;

        float vw0 = (5 * t - 6);
        float vw1 = (11 * t - 28);
        float vw2 = -(11 * t + 17);
        float vw3 = -(5 * t + 1);

        float v0 = (4 * t - 5) / vw0 - 3;
        float v1 = (4 * t - 16) / vw1 - 1;
        float v2 = -(7 * t + 5) / vw2 + 1;
        float v3 = -t / vw3 + 3;

        sum += uw0 * vw0 * sampleShadowMap(base_uv, u0, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw1 * vw0 * sampleShadowMap(base_uv, u1, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw2 * vw0 * sampleShadowMap(base_uv, u2, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw3 * vw0 * sampleShadowMap(base_uv, u3, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);

        sum += uw0 * vw1 * sampleShadowMap(base_uv, u0, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw1 * vw1 * sampleShadowMap(base_uv, u1, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw2 * vw1 * sampleShadowMap(base_uv, u2, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw3 * vw1 * sampleShadowMap(base_uv, u3, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);

        sum += uw0 * vw2 * sampleShadowMap(base_uv, u0, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw1 * vw2 * sampleShadowMap(base_uv, u1, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw2 * vw2 * sampleShadowMap(base_uv, u2, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw3 * vw2 * sampleShadowMap(base_uv, u3, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);

        sum += uw0 * vw3 * sampleShadowMap(base_uv, u0, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw1 * vw3 * sampleShadowMap(base_uv, u1, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw2 * vw3 * sampleShadowMap(base_uv, u2, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);
        sum += uw3 * vw3 * sampleShadowMap(base_uv, u3, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias);

        return sum * 1.0 / 2704.0;

    #endif
}

uint getCascade(float viewSpaceZ)
{
    uint cascadeIndex = 0;
    for (uint i = 0; i < GLOW_PIPELINE_SHADOW_CASCADE_AMOUNT - 1; ++i) {
		if (viewSpaceZ < splitDepths[i])
			cascadeIndex = i + 1;
	}
    return cascadeIndex;
}

#if GLOW_PIPELINE_ENABLE_REVERSE_Z
uint getCascade() { return getCascade(getViewSpaceZ()); }
#endif

vec3 getShadowPosition(uint cascade, vec3 worldSpacePosition)
{
    vec4 lightSpacePosition = lightVPs[cascade] * vec4(worldSpacePosition, 1.0);
    vec3 fragPosProjected = lightSpacePosition.xyz / lightSpacePosition.w; // NDC
    return fragPosProjected * 0.5 + 0.5; // Within [0, 1]
}

#if GLOW_PIPELINE_ENABLE_REVERSE_Z
float getShadowVisibility(vec3 worldSpacePosition)
{
    float viewSpaceZ = getViewSpaceZ();
#else
float getShadowVisibility(vec3 worldSpacePosition, float viewSpaceZ)
{
#endif

    uint cascade = getCascade(viewSpaceZ);
    vec3 shadowPos = getShadowPosition(cascade, worldSpacePosition);
    vec3 shadowPosDX = dFdxFine(shadowPos);
    vec3 shadowPosDY = dFdyFine(shadowPos);

    float shadowVisibility = sampleShadowCascade(shadowPos, shadowPosDX, shadowPosDY, cascade);

    #if FilterAcrossCascades_
         // Filter across cascades: Sample the next cascade, and blend between the two results to smooth the transition
        if (cascade != GLOW_PIPELINE_SHADOW_CASCADE_AMOUNT - 1)
        {
            const float blendThreshold = 0.9;
            float splitRatio = viewSpaceZ / splitDepths[cascade];

            if (splitRatio >= blendThreshold)
            {
                vec3 nextShadowPos = getShadowPosition(cascade + 1, worldSpacePosition);
                float nextVisibility = sampleShadowCascade(nextShadowPos, shadowPosDX, shadowPosDY, cascade + 1);

                float alpha = smoothstep(blendThreshold, 1.0, splitRatio);
                shadowVisibility = mix(shadowVisibility, nextVisibility, alpha);
            }
        }
    #endif

    return shadowVisibility;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/opaque/velocityInit.fsh", R"%%RES_EMBED%%(
in vec4 vHDCPosition;
in vec4 vPrevHDCPosition;

out vec2 fVelocity;

void main()
{
    vec2 a = (vHDCPosition.xy / vHDCPosition.w) * 0.5 + 0.5;
	vec2 b = (vPrevHDCPosition.xy / vPrevHDCPosition.w) * 0.5 + 0.5;
	fVelocity = a - b;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/opaque/velocityInit.vsh", R"%%RES_EMBED%%(
in vec2 aPosition;

out vec4 vHDCPosition;
out vec4 vPrevHDCPosition;

uniform mat4 uCleanVp;
uniform mat4 uPrevCleanVp;

void main() {
    vec4 outPosition = vec4(aPosition * 2 - 1, 0, 1);
    vHDCPosition = uCleanVp * outPosition;
    vPrevHDCPosition = uPrevCleanVp * outPosition;

    gl_Position = outPosition;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/output/common/fxaa.glsl", R"%%RES_EMBED%%(
#define FXAA_REDUCE_MIN   (1.0/ 128.0)
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#define FXAA_SPAN_MAX     8.0
#define EDGE_THRESHOLD_MIN  0.0312
#define EDGE_THRESHOLD_MAX  0.125

float rgb2luma(vec3 rgb) {
    return dot(rgb, vec3(0.299, 0.587, 0.114));
}

vec4 fxaa(sampler2DRect tex, vec2 fragCoord) {
    vec4 color;
    vec3 rgbNW = texture(tex, fragCoord + vec2(-1.0, -1.0)).xyz;
    vec3 rgbNE = texture(tex, fragCoord + vec2(1.0, -1.0)).xyz;
    vec3 rgbSW = texture(tex, fragCoord + vec2(-1.0, 1.0)).xyz;
    vec3 rgbSE = texture(tex, fragCoord + vec2(1.0, 1.0)).xyz;
    vec4 texColor = texture(tex, fragCoord);
    vec3 rgbM  = texColor.xyz;
    float lumaNW = rgb2luma(rgbNW);
    float lumaNE = rgb2luma(rgbNE);
    float lumaSW = rgb2luma(rgbSW);
    float lumaSE = rgb2luma(rgbSE);
    float lumaM  = rgb2luma(rgbM);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    float lumaRange = lumaMax - lumaMin;

    // Early out
    if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX)){
        return texColor;
    }

    mediump vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
                          (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
              max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
              dir * rcpDirMin));

    vec3 rgbA = 0.5 * (
        texture(tex, fragCoord + dir * (1.0 / 3.0 - 0.5)).xyz +
        texture(tex, fragCoord + dir * (2.0 / 3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(tex, fragCoord + dir * -0.5).xyz +
        texture(tex, fragCoord + dir * 0.5).xyz);

    float lumaB = rgb2luma(rgbB);
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        color = vec4(rgbA, texColor.a);
    else
        color = vec4(rgbB, texColor.a);
    return color;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/output/common/sharpen.glsl", R"%%RES_EMBED%%(
vec3 sharpen(sampler2DRect tex, vec2 fragCoord, float scaleOffset, float intensity) {
    vec3 color = texture(tex, fragCoord).rgb;
    vec3 colorUp = texture(tex, fragCoord + vec2(0, -scaleOffset)).rgb;
    vec3 colorRight = texture(tex, fragCoord + vec2(scaleOffset, 0)).rgb;
    vec3 colorLeft = texture(tex, fragCoord + vec2(-scaleOffset, 0)).rgb;
    vec3 colorDown = texture(tex, fragCoord + vec2(0, scaleOffset)).rgb;
    return clamp(color + (4 * color - colorUp - colorRight - colorLeft - colorDown) * intensity, 0, 1);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/output/output.fsh", R"%%RES_EMBED%%(
//#include <glow-pipeline/internal/pass/output/common/fxaa.glsl>
//#include <glow-pipeline/internal/pass/output/common/sharpen.glsl>
#glow pipeline sceneInfo

uniform sampler2DRect uInput;
uniform sampler3D uColorLut;

uniform ivec2 uViewportOffset;

out vec3 fColor;

// Size of the color LUT in each dimensions
// This value has to coincide with OutputStage::colorLutDimensions
#define COLOR_LUT_DIMS 16
const float colorLutScale = (COLOR_LUT_DIMS - 1.0) / COLOR_LUT_DIMS;
const vec3 colorLutOffset = vec3(1.0 / (2.0 * COLOR_LUT_DIMS));

void main()
{
    //// Downscaling
    vec3 color = texture(uInput, gl_FragCoord.xy - uViewportOffset).rgb;

    // FXAA
    //vec3 color = fxaa(uInput, gl_FragCoord.xy).rgb;

    // Brightness and Contrast
    color = (color - 0.5) * gPipelineScene.contrast + 0.5 + (gPipelineScene.brightness - 1);

    // Color grading
    color = texture(uColorLut, colorLutScale * color + colorLutOffset).rgb;

    // Output
    fColor = color;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/postprocessing/common/dithering.glsl", R"%%RES_EMBED%%(
uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

float wang_float(uint hash)
{
    return hash / float(0x7FFFFFFF) / 2.0;
}

vec3 dither(in vec3 color, vec2 uv, int divisor) {
    uint seed = uint(uv.x) + uint(uv.y) * 8096;
    float r = wang_float(wang_hash(seed * 3 + 0));
    float g = wang_float(wang_hash(seed * 3 + 1));
    float b = wang_float(wang_hash(seed * 3 + 2));
    vec3 random = vec3(r, g, b);

    return color + (random - .5) / divisor;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/postprocessing/common/srgb.glsl", R"%%RES_EMBED%%(

vec3 sRGBtoLinear(vec3 color, float gamma)
{
    return pow(color, vec3(gamma));
}
vec3 linearToSRGB(vec3 color, float gamma)
{
    return pow(color, vec3(1 / gamma));
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/postprocessing/common/tonemapping.glsl", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/common/util.glsl>

// -- Basic Reinhard Tonemapping --

vec3 reinhardTonemap(vec3 rgb)
{
    return rgb / (rgb + vec3(1));
}

// -- Uncharted 2 Tonemapping --
// Reference: http://filmicworlds.com/blog/filmic-tonemapping-operators/

const float U2_A = 0.22;
	// from 0.00 to 1.00
	// Shoulder strength

const float U2_B = 0.30;
	// from 0.00 to 1.00
	// Linear strength

const float U2_C = 0.10;
	// from 0.00 to 1.00
	// Linear angle

const float U2_D = 0.20;
	// from 0.00 to 1.00
	// Toe strength

const float U2_E = 0.01;
	// from 0.00 to 1.00
	// Toe numerator

const float U2_F = 0.22;
	// from 0.00 to 1.00
        // Toe denominator

const float U2_W = 11.2;
	// from 0.00 to 20.00
	// Linear White Point Value

vec3 U2Tonemap(vec3 x)
{
   return ((x*(U2_A*x+U2_C*U2_B)+U2_D*U2_E)/(x*(U2_A*x+U2_B)+U2_D*U2_F))-U2_E/U2_F;
}

vec3 uncharted2Tonemap(vec3 inputColor, float exposure, float gamma)
{
   inputColor *= exposure;

   float ExposureBias = 2.0f;
   vec3 curr = U2Tonemap(ExposureBias * inputColor);

   vec3 whiteScale = 1.0 / U2Tonemap(vec3(U2_W));      
   return curr * whiteScale;
}

// -- Luminance based Uncharted 2 Tonemapping variation --
// Reference: https://github.com/Zackin5/Filmic-Tonemapping-ReShade/blob/master/Uncharted2.fx

vec3 uncharted2TonemapLuminance(vec3 inputColor, float exposure, float gamma)
{
	// Do inital de-gamma of the game image to ensure we're operating in the correct colour range.
	inputColor = pow(inputColor, vec3(gamma));
		
	inputColor *= exposure;

	const float ExposureBias = 2.0;
	vec3 curr;
	
    float lum = 0.2126 * inputColor.r + 0.7152 * inputColor.g + 0.0722 * inputColor.b;
    vec3 newLum = U2Tonemap(vec3(ExposureBias * lum));
    float lumScale = (newLum / lum).x;
    curr = inputColor*lumScale;

	vec3 whiteScale = 1.0 / U2Tonemap(vec3(U2_W));
	vec3 color = curr * whiteScale;
    
	// Do the post-tonemapping gamma correction
	color = pow(color, vec3(1 / gamma));
	
	return color;
}

// -- Optimized Hejl-Dawson Tonemap + Gamma correction --
// Reference: http://filmicworlds.com/blog/filmic-tonemapping-operators/

vec3 optimizedHejlDawsonTonemap(vec3 inputColor, float exposure, float gamma)
{
    inputColor *= exposure;
    vec3 x = max3(vec3(0), inputColor - 0.004);
    return (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
}

// -- Simple filmic ACES --
// Reference: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/

vec3 ACESFilmTonemap(vec3 inputColor, float exposure, float gamma)
{
    inputColor *= exposure;
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return saturate((inputColor*(a*inputColor+b))/(inputColor*(c*inputColor+d)+e));
}

vec3 lumaWeightTonemap(vec3 rgb)
{
    return rgb / vec3(1.0 + rgbToLuminance(rgb));
}

vec3 lumaWeightTonemapInv(vec3 rgb)
{
    return rgb / vec3(1.0 - rgbToLuminance(rgb));
}

// -- Filmic ACES with conversion --
// Reference: https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat =
mat3(
    //0.59719, 0.35458, 0.04823,
    //0.07600, 0.90834, 0.01566,
    //0.02840, 0.13383, 0.83777
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04824, 0.01566, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat =
mat3(
    //1.60475, -0.53108, -0.07367,
    //-0.10208,  1.10813, -0.00605,
    //-0.00327, -0.07276,  1.07602

    1.60475, -0.10208, -0.00327,
    -0.53108, 1.10813, -0.07276,
    -0.07367, -0.00605, 1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 ACESFittedTonemap(vec3 inputColor, float exposure, float gamma)
{
    inputColor *= exposure;

    inputColor = ACESInputMat * inputColor;

    // Apply RRT and ODT
    inputColor = RRTAndODTFit(inputColor);

    inputColor = ACESOutputMat * inputColor;

    // Clamp to [0, 1]
    inputColor = saturate(inputColor);

    return inputColor;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/postprocessing/edgeOutline.fsh", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/common/globals.hh>
#include <glow-pipeline/internal/common/util.glsl>

uniform sampler2DRect uDepth;
uniform sampler2D uNormals;

uniform float uDepthThreshold;
uniform float uNormalThreshold;
uniform vec3 uColor;
uniform vec2 uInverseResolution;

// LDR output
out vec4 fHDR;

float getDepthCondition(float c)
{
    float c1 = texture(uDepth, gl_FragCoord.xy + vec2(0, 1)).x;
    float c2 = texture(uDepth, gl_FragCoord.xy + vec2(1, 0)).x;
        
    float d = 0.0;
    d = max(d, abs(c - c1));
    d = max(d, abs(c - c2));

    return float(d > uDepthThreshold);
}

float getNormalCondition()
{
    // Sample normal
    vec2 du = vec2(uInverseResolution.x, 0);
    vec2 dv = vec2(0, uInverseResolution.y);
    vec2 uv = (gl_FragCoord.xy) / (vec2(textureSize(uNormals, 0)));
    vec3 n = texture(uNormals, uv).xyz;
    vec3 n1 = texture(uNormals, uv + du).xyz;
    vec3 n2 = texture(uNormals, uv + dv).xyz;

    float d = 0.0;
    d = max(d, distance(n, n1));
    d = max(d, distance(n, n2));

    return float(d > uNormalThreshold);
}

void main()
{
    float mainDepth = texture(uDepth, gl_FragCoord.xy).x;
    float depthCond = getDepthCondition(mainDepth);
    float normalCond = mainDepth == GLOW_GLSL_FLT_MAX ? 0.0 : getNormalCondition();
    fHDR = vec4(uColor, mix(0.0, 0.9, max(depthCond, normalCond)));
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/postprocessing/postprocessing.fsh", R"%%RES_EMBED%%(
#glow pipeline sceneInfo

#include <glow-pipeline/internal/pass/postprocessing/common/tonemapping.glsl>
#include <glow-pipeline/internal/pass/postprocessing/common/srgb.glsl>
#include <glow-pipeline/internal/pass/postprocessing/common/dithering.glsl>
#include <glow-pipeline/internal/pass/output/common/sharpen.glsl>

in vec2 vPosition;

// HDR input
uniform sampler2DRect uInput;
uniform sampler2DRect uBloom;

// LDR output
out vec3 fLdr;

vec3 sampleWithChromaticAbberation(sampler2DRect tex, float intensity)
{
    vec2 fragmentToCenter = vPosition - vec2(0.5);
    float distance = dot(fragmentToCenter, fragmentToCenter) * intensity;
    vec2 dir = normalize(vPosition - vec2(0.5)) * distance;

    return vec3(
        texture(tex, gl_FragCoord.xy - dir * 1.0).r,
        texture(tex, gl_FragCoord.xy - dir * 0.8).g,
        texture(tex, gl_FragCoord.xy - dir * 0.7).b
    );
}

void main()
{
    vec3 bloomColor = max(texture(uBloom, gl_FragCoord.xy).rgb, 0.0); // This buffer is sometimes negative

    // Sample HDR color and sharpen
    vec3 hdrColor = sharpen(uInput, gl_FragCoord.xy, 1, gPipelineScene.sharpenStrength).rgb;

    // Add Bloom
    vec3 color = hdrColor + bloomColor;

    // Tonemapping
    if (gPipelineScene.tonemapEnabled > 0)
    {
        //color = vec3(1.0) - exp(-color * gPipelineScene.exposure);
        //color = optimizedHejlDawsonTonemap(color, gPipelineScene.exposure, gPipelineScene.gamma);
        color = ACESFittedTonemap(color, gPipelineScene.exposure, gPipelineScene.gamma);
        //color = lumaWeightTonemap(color);
    }

    // Gamma correction
    color = linearToSRGB(color, gPipelineScene.gamma);

    // Dithering and output
    fLdr = dither(color, gl_FragCoord.xy, 100); // Divisor is originally 255, but this looks much better (on an 8bit screen)
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/shadow/geometryDepthOnly.vsh", R"%%RES_EMBED%%(
in vec3 aPosition;

uniform mat4 uModel;

void main()
{
	gl_Position = uModel * vec4(aPosition, 1.0);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/shadow/geometryDepthOnlyInstanced.vsh", R"%%RES_EMBED%%(
in vec3 aPosition;

// -- instancing data --
in vec4 aModelC0;
in vec4 aModelC1;
in vec4 aModelC2;
in vec4 aModelC3;

void main()
{
    mat4 model = mat4(aModelC0, aModelC1, aModelC2, aModelC3);
	gl_Position = model * vec4(aPosition, 1.0);
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/shadow/shadowCascades.gsh", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/common/globals.hh>

layout(triangles, invocations = GLOW_PIPELINE_SHADOW_CASCADE_AMOUNT) in;

layout(triangle_strip, max_vertices = 3) out;

layout(std140) uniform uShadowLightVPUBO {
    mat4 lightVPs[GLOW_PIPELINE_SHADOW_CASCADE_AMOUNT];
};

void main()
{
    for (int i = 0; i < 3; ++i) // Triangles assumed
    {
        gl_Layer = gl_InvocationID;
        gl_Position = lightVPs[gl_InvocationID] * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/pass/temporalresolve/taa.fsh", R"%%RES_EMBED%%(
#include <glow-pipeline/internal/common/util.glsl>
#include <glow-pipeline/internal/pass/postprocessing/common/tonemapping.glsl>

#glow pipeline internal_projectionInfoUbo
#glow pipeline sceneInfo

out vec3 fHDR;
out vec3 fBloom;

uniform sampler2DRect uShadedOpaque;
uniform sampler2DRect uVelocity;
uniform sampler2DRect uTemporalHistory;

uniform bool uEnableHistoryRejection;
uniform vec2 uJitter;

// Unjitter the current color sample, can improve or degrade stability
#define UNJITTER_COLOR_SAMPLE 0

// 4-Tap Varying only samples 4 times (vs 9) but is less stable 
#define MINMAX_3X3_ROUNDED 1
#define MINMAX_4TAP_VARYING !MINMAX_3X3_ROUNDED

// Reduces flickering at the cost of (signifcantly) slower convergence
// (fast movements decrease sharpness)
#define HIGH_STABILITY_MODE 0

vec3 clipAabb(vec3 aabbMin, vec3 aabbMax, vec3 p, vec3 q)
{
    // note: only clips towards aabb center (but fast!)
    vec3 pClip = 0.5 * (aabbMax + aabbMin);
    vec3 eClip = 0.5 * (aabbMax - aabbMin) + 0.001;

    vec3 vClip = q - pClip;
    vec3 vUnit = vClip / eClip;
    vec3 aUnit = abs(vUnit);
    float maUnit = max(aUnit.x, max(aUnit.y, aUnit.z));

    if (maUnit > 1.0)
        return pClip + vClip / maUnit;
    else
        return q; // point inside aabb
}

vec3 performTemporalAA(vec2 uv, vec2 velocity)
{
#if MINMAX_3X3_ROUNDED

    // Sample 3x3 neighborhood of current frame
    vec2 du = vec2(1, 0);
    vec2 dv = vec2(0, 1);
    vec3 ctl = texture(uShadedOpaque, uv - dv - du).rgb;
    vec3 ctc = texture(uShadedOpaque, uv - dv).rgb;
    vec3 ctr = texture(uShadedOpaque, uv - dv + du).rgb;
    vec3 cml = texture(uShadedOpaque, uv - du).rgb;
    vec3 cmc = texture(uShadedOpaque, uv).rgb;
    vec3 cmr = texture(uShadedOpaque, uv + du).rgb;
    vec3 cbl = texture(uShadedOpaque, uv + dv - du).rgb;
    vec3 cbc = texture(uShadedOpaque, uv + dv).rgb;
    vec3 cbr = texture(uShadedOpaque, uv + dv + du).rgb;
        
    // Calculate component-wise minimum, maximum and average
    vec3 cmin = min3(ctl, min3(ctc, min3(ctr, min3(cml, min3(cmc, min3(cmr, min3(cbl, min3(cbc, cbr))))))));
    vec3 cmax = max3(ctl, max3(ctc, max3(ctr, max3(cml, max3(cmc, max3(cmr, max3(cbl, max3(cbc, cbr))))))));
    vec3 cavg = (ctl + ctc + ctr + cml + cmc + cmr + cbl + cbc + cbr) / 9.0;

    // Min/Max Rounding
    vec3 cmin5 = min3(ctc, min3(cml, min3(cmc, min3(cmr, cbc))));
    vec3 cmax5 = max3(ctc, max3(cml, max3(cmc, max3(cmr, cbc))));
    vec3 cavg5 = (ctc + cml + cmc + cmr + cbc) / 5.0;
    cmin = 0.5 * (cmin + cmin5);
    cmax = 0.5 * (cmax + cmax5);
    cavg = 0.5 * (cavg + cavg5);

#elif MINMAX_4TAP_VARYING

    const float _SubpixelThreshold = 0.5;
    const float _GatherBase = 0.5;
    const float _GatherSubpixelMotion = 0.1666;

    vec2 texel_vel = velocity / vec2(1);
    float texel_vel_mag = length(texel_vel) * 1 /* vs_dist */;
    float k_subpixel_motion = saturate(_SubpixelThreshold / (0.0001f + texel_vel_mag));
    float k_min_max_support = _GatherBase + _GatherSubpixelMotion * k_subpixel_motion;

    vec2 ss_offset01 = k_min_max_support * vec2(-1, 1);
    vec2 ss_offset11 = k_min_max_support * vec2(1, 1);
    vec3 c00 = texture(uShadedOpaque, uv - ss_offset11).rgb;
    vec3 c10 = texture(uShadedOpaque, uv - ss_offset01).rgb;
    vec3 c01 = texture(uShadedOpaque, uv + ss_offset01).rgb;
    vec3 c11 = texture(uShadedOpaque, uv + ss_offset11).rgb;

    vec3 cmin = min(c00, min(c10, min(c01, c11)));
    vec3 cmax = max(c00, max(c10, max(c01, c11)));

    vec3 cavg = (c00 + c10 + c01 + c11) / 4.0;

#endif

    // Sample current and history color
#if UNJITTER_COLOR_SAMPLE
    vec3 currentColor = texture(uShadedOpaque, uv - uJitter).rgb; 
#else
    #if MINMAX_3X3_ROUNDED
    vec3 currentColor = cmc; // Same as cmc, already sampled
    #else
    vec3 currentColor = texture(uShadedOpaque, uv).rgb;
    #endif
#endif
    vec3 historyColor = texture(uTemporalHistory, uv - velocity).rgb;

    // Clip to neighborhood color-space AABB
    historyColor = clipAabb(cmin, cmax, clamp(cavg, cmin, cmax), historyColor);

    // UE4 weighting (sucks)
    //float contrast = distance(cavg, currentColor);
    //float weight = 0.05 * contrast;
    //return mix(historyColor, currentColor, weight);

    // Feedback weight from unbiased luminance delta (t.lottes)
    float lum0 = rgbToLuminance(currentColor);
    float lum1 = rgbToLuminance(historyColor);
    float unbiasedDiff = abs(lum0 - lum1) / max(lum0, max(lum1, 0.2));
    float unbiasedWeight = 1.0 - unbiasedDiff;
    float unbiasedWeight2 = unbiasedWeight * unbiasedWeight;

#if HIGH_STABILITY_MODE
    float kFeedback = mix(0.97f, 0.999f, unbiasedWeight2);
#else
    float kFeedback = mix(0.88f, 0.97f, unbiasedWeight2);
#endif
    // History blending based on feedback weight
    return mix(currentColor, historyColor, kFeedback);
}

vec3 performTAANoRejection(vec2 uv, vec2 velocity)
{
    vec3 currentColor = texture(uShadedOpaque, uv).rgb;
    vec3 historyColor = texture(uTemporalHistory, uv - velocity).rgb;
    return mix(currentColor, historyColor, 0.98);
}

void main()
{
    // TAA
    vec2 velocity = texture(uVelocity, gl_FragCoord.xy).rg * uPipelineProjectionInfo.viewportNearFar.xy;
    vec3 hdrColor = uEnableHistoryRejection ? 
        performTemporalAA(gl_FragCoord.xy, velocity) : 
        performTAANoRejection(gl_FragCoord.xy, velocity);
    
    // HDR output and Bloom extraction
    fHDR = hdrColor;
    //fHDR.g *= length(velocity);

    float hdrColorLuminance = rgbToLuminance(hdrColor);
    vec3 normalizedColor = hdrColor / max(hdrColorLuminance, 0.1);

    fBloom = mix(
        vec3(0),
        normalizedColor * gPipelineScene.bloomIntensity,
        float(hdrColorLuminance > gPipelineScene.bloomThreshold)
    );

}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/utility/blank.fsh", R"%%RES_EMBED%%(


void main()
{
    // gl_FragDepth = gl_FragCoord.z;
}

)%%RES_EMBED%%"},
{"glow-pipeline/internal/utility/copy.fsh", R"%%RES_EMBED%%(
uniform sampler2DRect uInput;
out vec3 fHDR;

void main()
{
    fHDR = texture(uInput, gl_FragCoord.xy).rgb;
}
)%%RES_EMBED%%"},
{"glow-pipeline/internal/utility/downsample.fsh", R"%%RES_EMBED%%(
uniform sampler2DRect uInput;
uniform float uInverseRatio; // e.g. 2 for a quarter-res buffer

out vec3 fHDR;

void main()
{
    fHDR = texture(uInput, gl_FragCoord.xy * uInverseRatio).rgb;
}

)%%RES_EMBED%%"},
{"glow-pipeline/pass/depthpre/depthPre.glsl", R"%%RES_EMBED%%(
void outputDepthPreGeometry()
{
    fOut = 1;
}

)%%RES_EMBED%%"},
{"glow-pipeline/pass/opaque/opaquePass.glsl", R"%%RES_EMBED%%(
#ifndef _OPAQUE_PASS_GLSL
#define _OPAQUE_PASS_GLSL

#include <glow-pipeline/internal/pass/depthpre/clustering.glsl>
#include <glow-pipeline/internal/pass/depthpre/clusteringStructs.glsl>
#include <glow-pipeline/internal/pass/opaque/shadowing.glsl>

uniform sampler2DRect uSceneAo;

layout(std430) restrict readonly buffer sClusterLightIndexList {
    uint lightIndexList[];
} ssboLightIndexList;

layout(std430) restrict readonly buffer sClusterVisibilities {
    ClusterVisibility data[];
} ssboClusterVisibilities;

layout(std430) restrict readonly buffer sLightData {
    PackedLightData lights[];
} ssboLightData;

// -- Clustered Shading --

ClusterVisibility getClusterVisibilty()
{
    return ssboClusterVisibilities.data[getCurrentClusterIndex()];
}

uint getLightIndex(in uint offset)
{
    return ssboLightIndexList.lightIndexList[offset];
}

#define FOREACH_LIGHT(global_macro_lightIndex)                                          \
    ClusterVisibility _internal_clusterData = getClusterVisibilty();                    \
    uint global_macro_lightIndex = getLightIndex(_internal_clusterData.offset);         \
    for (uint _internal_i = 0u; _internal_i < _internal_clusterData.count; ++_internal_i, global_macro_lightIndex = getLightIndex(_internal_clusterData.offset + _internal_i))

// -- Lighting --

LightData unpackLightData(PackedLightData data) {
    LightData res;
    res.posA = data.aSize.xyz;
    res.posB = data.bRadius.xyz;
    res.size = data.aSize.w;
    res.radius = data.bRadius.w;
    res.color = data.colorMaskId.xyz;
    res.maskId = int(data.colorMaskId.w);
    return res;
}

LightData getLight(uint index)
{
    // For some reason, this line does not work on AMD:
    // return unpackLightData(ssboLightData.lights[index]);

    const PackedLightData data = ssboLightData.lights[index];
    LightData res;
    res.posA = data.aSize.xyz;
    res.posB = data.bRadius.xyz;
    res.size = data.aSize.w;
    res.radius = data.bRadius.w;
    res.color = data.colorMaskId.xyz;
    res.maskId = int(data.colorMaskId.w);
    return res;
}

// -- Lighting helpers --

float getSceneAo() {
    return clamp(texture(uSceneAo, gl_FragCoord.xy).r, 0, 1);
}

vec3 applyAo(in vec3 color, in float materialAo) {
    // Combined material + scene AO
    float combinedAo = (1 - ((1 - materialAo) + (1 - getSceneAo())));
    return color * combinedAo;
}

vec3 applyAo(in vec3 color) {
    return color * getSceneAo();
}

vec3 getTubeLightL(in vec3 R, in vec3 posA, in vec3 posB, in vec3 worldPos)
{
    const vec3 L0 = posA - worldPos;
    const vec3 L1 = posB - worldPos;
    const vec3 Ld = L1 - L0;

    const float tnum = dot(R, L0) * dot(R, Ld) - dot(L0, Ld);
    const float tden = pow(length(Ld), 2) - pow(dot(R, Ld), 2);
    const float t = tnum / tden;
    return L0 + clamp(t, 0.0, 1.0) * Ld;
}

vec3 getSphereLightL(in vec3 R, in vec3 L, in float sphereLightSize)
{
    const vec3 centerToRay = dot(L, R) * R - L; 
    return L + centerToRay * clamp(sphereLightSize / length(centerToRay), 0.0, 1.0);
}

void getTubeLightInfo(
    in LightData data, in vec3 worldPos, in vec3 V, in vec3 N, in float roughness,
    out vec3 correctedL, out vec3 radiance
    )
{
    vec3 lightDistanceVector = data.posA - worldPos;
    float areaNormalization = 1;

    if (data.size == 0)
    {
        // Degenerate case, Point Light 
        // (Tube Lights require a size > 0)
    }
    else
    {
        const vec3 R = reflect(-V, N);

        if (data.posA != data.posB)
        {
            // Tube Light
            lightDistanceVector = getTubeLightL(R, data.posA, data.posB, worldPos);
        }
        else
        {
            // Sphere Light
        }

        lightDistanceVector = getSphereLightL(R, lightDistanceVector, data.size);

        const float a = sqrt(max(roughness, 0.001));
        const float a2 = min(1.0, a + data.size / (2 * length(data.posA - worldPos)));
        areaNormalization = pow(a / a2, 2);
    }
    
    float lightDistance = sqrt(dot(lightDistanceVector, lightDistanceVector));

    // Basic Attenuation
    //float attenuation = smoothstep(data.radius, 0.0, lightDistance);

    // Inverse-Square Falloff
    float attenuation = pow(clamp(1 - pow(lightDistance / data.radius, 4), 0.0, 1.0), 2.0) / (pow(lightDistance, 2.0) + 1.0);

    radiance = data.color * attenuation * areaNormalization;
    correctedL = normalize(lightDistanceVector);
}

// -- Velocity and Output --

// Returns the screen space velocity vector for a given fragment,
// can be fed directly into outputOpaqueGeometry
// Expects unjittered homogenous device coordinates
// cleanHDC = cleanProjection * View * Model * aPosition;
// prevCleanHDC = previousCleanProjection * previousView * previousModel * aPosition;
vec2 getFragmentVelocity(in vec4 cleanHDC, in vec4 prevCleanHDC)
{
    // Both positions are unjittered
	// Range [0, 1] using the * 0.5 + 0.5 operation
	// corresponds to the UV coords of the TAA history buffer
	vec2 a = (cleanHDC.xy / cleanHDC.w) * 0.5 + 0.5;
	vec2 b = (prevCleanHDC.xy / prevCleanHDC.w) * 0.5 + 0.5;
	return a - b;
}

void outputOpaqueGeometry(vec3 hdrColor, vec2 velocity)
{
    fHdr = hdrColor;
    fVelocity = velocity;
}

void outputOpaqueGeometry(vec3 hdrColor)
{
    fHdr = hdrColor;
    fVelocity = vec2(0);
}

#endif

)%%RES_EMBED%%"},
{"glow-pipeline/pass/shadow/shadow.glsl", R"%%RES_EMBED%%(
// NOTE: This extension allows setting gl_Layer in the vertex shader
// It is supported all the way back to the HD 5000 Series (2009)
// See https://opengl.gpuinfo.org/listreports.php?extension=GL_AMD_vertex_shader_layer
#extension GL_AMD_vertex_shader_layer : require

#include <glow-pipeline/internal/common/globals.hh>

uniform uShadowVPUBO {
    mat4 lightVPs[SHADOW_CASCADE_AMOUNT];
    float cascadeLimits[SHADOW_CASCADE_AMOUNT + 1];
};

void outputShadowVertex(vec3 position, int cascade)
{
    gl_Layer = cascade;
    gl_Position = lightVPs[cascade] * position;
}

)%%RES_EMBED%%"},
{"glow-pipeline/pass/transparent/oitPass.glsl", R"%%RES_EMBED%%(
#glow pipeline internal_projectionInfoUbo

uniform sampler2DRect uDepth;
uniform vec3 uSunDirection;
uniform vec3 uSunColor;

float global_fragmentDepth = texelFetch(uDepth, ivec2(gl_FragCoord.xy)).x;

float getOitMaterialAlpha(vec3 N, vec3 V, vec3 specular) {
	float dotNV = max(dot(N, V), 0.0);
    float F_a = 1.0;
    float F_b = pow(1.0 - dotNV, 5);
    vec3 F = mix(vec3(F_b), vec3(F_a), specular);
    return max(F.x, max(F.y, F.z));
}

float getOitWeight(float fragZ, float alpha)
{
    float d = 1 - fragZ;
    return alpha * max(1e-2, 3e3 * d * d * d);
}

float transparentSpecularGGX(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0)
{
    vec3 H = normalize(V + L);

    float LdotH = max(dot(L, H), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    float alphaG = roughness * roughness;

    // D
    float alphaG2 = alphaG * alphaG;
    float denom = NdotH * NdotH * (alphaG2 - 1.0) + 1.0;
    float D = alphaG2 / (denom * denom);

    // G
    float k = (alphaG + 2 * roughness + 1) / 8.0;
    float G = NdotL / (mix(NdotL, 1, k) * mix(NdotV, 1, k));
    
    return D * G / 4.0;
}

struct OITMaterial
{
    vec3 albedo;
    float roughness;
    float alpha;
    vec3 specular;
    float reflectivity;
};

struct OITFragmentInfo
{
    vec3 N;
    vec3 V;
    vec3 R;
    vec3 posWorldSpace;
    vec4 posViewSpace;
    vec4 posScreenSpace;
    mat4 proj;
    mat4 view;
};

struct OITResult
{
    vec3 color;
    vec2 offset;
};

void getOitResult(
    in OITMaterial material, in OITFragmentInfo fragInfo, out OITResult result
    ) {
	// Thickness calculation
    vec2 projectedScreenXY = fragInfo.posScreenSpace.xy / fragInfo.posScreenSpace.w;
    vec4 opaquePos4 = uPipelineProjectionInfo.projectionInverse * vec4(projectedScreenXY, global_fragmentDepth * 2 - 1, 1.0);
    float opaqueDis = opaquePos4.z / opaquePos4.w;
    // Thickness: Distance between opaque depth beneath, and depth of current fragment
    float thickness = abs(fragInfo.posViewSpace.z - opaqueDis);
    //thickness = .5;

    // OIT Offset (Refraction)
    vec2 tsize = vec2(uPipelineProjectionInfo.viewportNearFar.xy);
    vec3 RF = refract(-fragInfo.V, fragInfo.N, 1 / 1.33);
    vec3 groundPosR = fragInfo.posWorldSpace + RF * clamp(thickness / 2, 0.0, 1.0);
    vec4 groundPosS = fragInfo.proj * fragInfo.view * vec4(groundPosR, 1.0);
    vec2 refrXY = groundPosS.xy / groundPosS.w;
    vec2 refrXYS = (refrXY * 0.5 + 0.5) * tsize;
    result.offset = refrXYS - gl_FragCoord.xy;

    // Depth modulated color
    const float depthFalloff = 5.0;
    vec3 depthAdjustedColor = material.albedo * (1 - pow(0.5, thickness / depthFalloff));

    // Reflection component
    vec3 refl = transparentSpecularGGX(fragInfo.N, fragInfo.V, uSunDirection, material.roughness, material.specular) * uSunColor;
    //refl += getPrefilterColor(R, roughness) * material.reflectivity;// * mix(1.0, shadowF, dot(R, L) * 0.5 + 0.5);

    // OIT Color
    result.color = refl + (depthAdjustedColor * material.alpha);
}

void outputOitGeometry(float fragZ, vec3 color, float alpha, vec2 offset)
{
    float weight = getOitWeight(fragZ, alpha);
    fAccuA = vec4(color * weight, alpha);
    fAccuB = alpha * weight;
    fDistortion = vec3(offset, 0.f);
}

void outputOitGeometry(float fragZ, vec3 color, float alpha)
{
    float weight = getOitWeight(fragZ, alpha);
    fAccuA = vec4(color * weight, alpha);
    fAccuB = alpha * weight;
    fDistortion = vec3(0);
}

)%%RES_EMBED%%"},

};
}
