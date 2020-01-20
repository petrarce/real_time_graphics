
uniform float uRuntime;

uniform vec3 uCamPos;
uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uViewProj;
uniform float uRenderDistance;

// Helper

float distance2(vec3 a, vec3 b)
{
    vec3 d = a - b;
    return dot(d, d);
}

// G Buffer fill functions

vec4 gBufferColor(vec3 albedo, float ao)
{
    return vec4(albedo, ao);
}

vec4 gBufferMatA(vec3 worldNormal, float metallic)
{
    return vec4(worldNormal.xyz * 0.5 + 0.5, metallic);
}

vec2 gBufferMatB(float roughness, float translucency)
{
    return vec2(roughness, translucency);
}

// T Buffer fill functions

float oitWeight(float fragZ, float alpha)
{
    float d = 1 - fragZ;
    return alpha * max(1e-2, 3e3 * d * d * d);
}


/// Task 2.b
///
/// Your job is to:
///     - write down the correct T-Buffer values for Weighted OIT
///
/// Notes:
///     - the color is already pre-multiplied by alpha
///     - remember that revelage and accum.b are swapped
///
/// ============= STUDENT CODE BEGIN =============
vec4 tBufferAccumA(vec3 premultColor, float alpha, float w)
{
    return vec4(0);
}

float tBufferAccumB(float alpha, float w)
{
    return 0;
}
/// ============= STUDENT CODE END =============

/// Task 2.c
///
/// Your job is to:
///     - write offset and blurriness into T-Buffer
///
/// ============= STUDENT CODE BEGIN =============
vec3 tBufferDistortion(vec2 offset, float blurriness)
{
    return vec3(0);
}
/// ============= STUDENT CODE END =============
