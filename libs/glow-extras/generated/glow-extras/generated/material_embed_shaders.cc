#include "material_embed_shaders.hh"

// This file is generated upon running CMake, do not modify it!

namespace internal_embedded_files {

const std::pair<const char*, const char*> material_embed_shaders[] = {
{"glow-material/material-ggx.glsl", R"%%RES_EMBED%%(
/// Precalculated LUT for GGX lighting
uniform sampler2D uGlowMaterialEnvLutGGX;
/// Environment Map for specular GGX IBL
uniform samplerCube uGlowMaterialEnvMapGGX;

// F0 helper
vec3 getF0(vec3 albedo, float metallic)
{
    const vec3 dielectricF0 = vec3(0.04); // Constant for dielectric (non-metal) surfaces
    return mix(dielectricF0, albedo, metallic);
}

// Fresnel term
// Schlicks Approximation
vec3 F_Schlick(vec3 f0, float f90, float u)
{
    return f0 + (f90 - f0) * clamp(pow(1.0 - u, 5.0), 0, 1);
}

vec3 F_SchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - cosTheta, 5.0);
}

// DO NOT MULTIPLY BY COS THETA
vec3 shadingSpecularGGX(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0)
{
    // see http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/
    vec3 H = normalize(V + L);

    float dotLH = max(dot(L, H), 0.0);
    float dotNH = max(dot(N, H), 0.0);
    float dotNL = max(dot(N, L), 0.0);
    float dotNV = max(dot(N, V), 0.0);

    float alpha = roughness * roughness;

    // D (GGX normal distribution)
    float alphaSqr = alpha * alpha;
    float denom = dotNH * dotNH * (alphaSqr - 1.0) + 1.0;
    float D = alphaSqr / (denom * denom);
    // no pi because BRDF -> lighting

    // F (Fresnel term)
    vec3 F = F_SchlickRoughness(dotLH, F0, roughness);

    // G (remapped hotness, see Unreal Shading)
    float k = (alpha + 2 * roughness + 1) / 8.0;
    float G = dotNL / (mix(dotNL, 1, k) * mix(dotNV, 1, k));
    // '* dotNV' - canceled by normalization

    // orginal G:
    /*
    {
        float k = alpha / 2.0;
        float k2 = k * k;
        float invK2 = 1.0 - k2;
        float vis = 1 / (dotLH * dotLH * invK2 + k2);

        vec3 FV = mix(vec3(F_b), vec3(F_a), F0) * vis;
        vec3 specular = D * FV / 4.0f;
        return specular * dotNL;
    }
    */

    // '/ dotLN' - canceled by lambert
    // '/ dotNV' - canceled by G
    return D * F * G / 4.0;
}

// specular and diffuse contribution of a single light direction
// DOES NOT INCLUDE IBL
vec3 shadingGGX(vec3 N, vec3 V, vec3 L, vec3 albedo, float roughness, float metallic)
{
    vec3 diffuse = albedo * (1 - metallic); // metals have no diffuse
    vec3 F0 = getF0(albedo, metallic);

    float dotNL = max(dot(N, L), 0.0);

    return diffuse * dotNL + shadingSpecularGGX(N, V, L, max(0.01, roughness), F0);
}

// image based specular lighting for GGX
vec3 iblSpecularGGX(vec3 N, vec3 V, vec3 albedo, float roughness, float metallic)
{
    float dotNV = max(dot(N, V), 0.0);
    vec3 R = reflect(-V, N);

    vec3 F0 = getF0(albedo, metallic);

    // Derive mipmap levels of environment map
    // TODO: Uniform / Precalculate
    // In OpenGL 4.6, use textureQueryLevels()
    const float maxLevel = floor(log2(float(textureSize(uGlowMaterialEnvMapGGX, 0).x))); 

    vec2 brdf = texture2D(uGlowMaterialEnvLutGGX, vec2(roughness, dotNV)).xy;
    vec3 envcolor = textureLod(uGlowMaterialEnvMapGGX, R, roughness * maxLevel).rgb;

    return envcolor * (F0 * brdf.x + brdf.y);
}

// image based lighting (specular + diffuse) for GGX
vec3 iblightingGGX(vec3 N, vec3 V, vec3 albedo, float roughness, float metallic)
{
    float dotNV = max(dot(N, V), 0.0);
    vec3 F0 = getF0(albedo, metallic);

    vec3 F = F_SchlickRoughness(dotNV, F0, roughness);
    vec3 R = reflect(-V, N);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(uGlowMaterialEnvMapGGX, N).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 specular = iblSpecularGGX(N, V, albedo, roughness, metallic);

    return kD * diffuse + specular;
}

)%%RES_EMBED%%"},
{"glow-material/precalc-env-brdf-lut.csh", R"%%RES_EMBED%%(
// see http://www.gamedev.net/topic/655431-ibl-problem-with-consistency-using-ggx-anisotropy/

#include <glow-material/precalc.glsl>

// http://www.unrealengine.com/files/downloads/2013SiggraphPresentationsNotes.pdf
vec2 IntegrateBRDF( float Roughness, float dotNV )
{
    vec3 V = vec3( sqrt(1 - dotNV * dotNV), // sin
                   0.0,
                   dotNV ); // cos

    float A = 0;
    float B = 0;

    vec3 N = vec3(0, 0, 1);

    float k = Roughness / 2.0;
    k = k * k;

    const int samples = 1024;
    for (int i = 0; i < samples; ++i)
    {
        vec2 Xi = Hammersley(i, samples);
        vec3 H = ImportanceSampleGGX( Xi, Roughness, N );
        vec3 L = 2 * dot(V, H) * H - V;

        float dotNL = max(L.z, 0.0);
        float dotNH = max(H.z, 0.0);
        float dotVH = max(dot(V, H), 0.0);

        if (dotNL > 0)
        {
            // original:
            // float G = dotNL * dotNV / (mix(dotNV, 1, k) * mix(dotNL, 1, k));
            // float G_Vis = G * dotVH / (dotNH * dotNV);

            // slightly optimized
            float G = dotNL / (mix(dotNV, 1, k) * mix(dotNL, 1, k));
            float G_Vis = G * dotVH / dotNH;

            float Fc = pow(1 - dotVH, 5);

            A += (1 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    return vec2(A, B) / float(samples);
}


/// COMPUTE SHADER
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

uniform layout(rg16f, binding=0) writeonly image2DRect uLUT;

void main()
{
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;
    ivec2 s = imageSize(uLUT);

    if (x >= s.x || y >= s.y)
        return; // out of bounds

    vec2 lut = IntegrateBRDF((float(x) + .5) / float(s.x), (float(y) + .5) / float(s.y));

    imageStore(uLUT, ivec2(x, y), vec4(lut, 0, 0));
}

)%%RES_EMBED%%"},
{"glow-material/precalc-env-map.csh", R"%%RES_EMBED%%(
// see http://www.gamedev.net/topic/655431-ibl-problem-with-consistency-using-ggx-anisotropy/

#include <glow-material/precalc.glsl>

uniform samplerCube uEnvMap;
uniform float uRoughness;

// http://www.unrealengine.com/files/downloads/2013SiggraphPresentationsNotes.pdf
vec3 PrefilterEnvMap( float Roughness, vec3 R )
{
    vec3 N = R;
    vec3 V = R;

    vec3 color = vec3(0, 0, 0);
    float totalWeight = 0;

    const int samples = int(max(1.0, 1024.0 * pow(uRoughness, 0.3)));

    for (int i = 0; i < samples; ++i)
    {
        vec2 Xi = Hammersley(i, samples);
        vec3 H = ImportanceSampleGGX( Xi, Roughness, N );
        vec3 L = 2 * dot(V, H) * H - V;

        float dotNL = dot(N, L);
        if (dotNL > 0)
        {
            color += textureLod(uEnvMap, L, 0).rgb;
            totalWeight += dotNL;
        }
    }

    return color / totalWeight;
}


/// COMPUTE SHADER
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

uniform layout(rgba16f, binding=0) writeonly imageCube uCube;

vec3 direction(float x, float y, uint l)
{
  // see ogl spec 8.13. CUBE MAP TEXTURE SELECTION
  switch(l) {
    // +x
    case 0: return vec3(+1, -y, -x);
    // -x
    case 1: return vec3(-1, -y, +x);
    // +y
    case 2: return vec3(+x, +1, +y);
    // -y
    case 3: return vec3(+x, -1, -y);
    // +z
    case 4: return vec3(+x, -y, +1);
    // -z
    case 5: return vec3(-x, -y, -1);
  }
  return vec3(0, 1, 0);
}

void main()
{
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;
    uint l = gl_GlobalInvocationID.z;
    ivec2 s = imageSize(uCube);

    if (x >= s.x || y >= s.y)
        return; // out of bounds

    float fx = (float(x) + .5) / float(s.x);
    float fy = (float(y) + .5) / float(s.y);

    vec3 dir = normalize(direction(fx * 2 - 1, fy * 2 - 1, l));

    vec3 color = PrefilterEnvMap(uRoughness, dir);

    imageStore(uCube, ivec3(x, y, l), vec4(color, 1));
}

)%%RES_EMBED%%"},
{"glow-material/precalc.glsl", R"%%RES_EMBED%%(
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
vec2 Hammersley(uint i, uint N) {
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}

// Image-Based Lighting
// http://www.unrealengine.com/files/downloads/2013SiggraphPresentationsNotes.pdf
vec3 ImportanceSampleGGX( vec2 Xi, float Roughness, vec3 N )
{
    float PI = 3.1415926535897932384626433832795;

    float a = Roughness * Roughness;

    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );

    vec3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;

    vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
    vec3 TangentX = normalize( cross( UpVector, N ) );
    vec3 TangentY = cross( N, TangentX );

    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

)%%RES_EMBED%%"},

};
}
