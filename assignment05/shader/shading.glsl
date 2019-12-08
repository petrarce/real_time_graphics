
uniform vec3 uLightDir;
uniform vec3 uAmbientLight;
uniform vec3 uLightColor;

uniform float uRuntime;

uniform vec3 uCamPos;
uniform mat4 uInvProj;
uniform mat4 uProj;
uniform mat4 uView;

uniform bool uIsShadowPass;
uniform bool uIsOpaquePass;
uniform bool uIsTransparentPass;

uniform bool uShowAmbientOcclusion;
uniform bool uShowNormal;

uniform samplerCube uCubeMap;

uniform mat4 uShadowViewProj;
uniform sampler2DShadow uShadowMap;

uniform float uMetallic;
uniform float uReflectivity;
uniform sampler2D uTexAlbedo;
uniform sampler2D uTexNormal;
uniform sampler2D uTexRoughness;
uniform sampler2D uTexHeight;
uniform sampler2D uTexAO;

float shadowing(vec3 worldPos, vec3 N, vec3 L)
{
    vec3 shadowDir = L;
    vec3 offset = shadowDir * 0.01 * 0; // to prevent shadow acne
    vec4 shadowPos = uShadowViewProj * vec4(worldPos + offset, 1.0f);
    shadowPos /= shadowPos.w;

    if (any(greaterThan(abs(shadowPos.xyz), vec3(0.99f))))
        return 1.0f; // no shadow outside

    float dotNL = dot(N, L);
    float f = sqrt(1 - dotNL * dotNL) / dotNL;
    float bias = 0.95 * f / textureSize(uShadowMap, 0).x; // calibrated for 2048^2
    bias = clamp(bias, 0.0, 1.0);

    shadowPos.xyz = shadowPos.xyz * 0.5 + 0.5;
    shadowPos.z -= bias;

    return texture(uShadowMap, shadowPos.xyz);
}

// DO NOT MULTIPLY BY COS THETA
vec3 shadingSpecularGGXNoF(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0)
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

    // G (remapped hotness, see Unreal Shading)
    float k = (alpha + 2 * roughness + 1) / 8.0;
    float G = dotNL / (mix(dotNL, 1, k) * mix(dotNV, 1, k));
    // '* dotNV' - canceled by normalization

    // '/ dotLN' - canceled by lambert
    // '/ dotNV' - canceled by G
    return vec3(1) * D * G / 4.0;
}

vec3 shadingDiffuseOrenNayar(vec3 N, vec3 V, vec3 L, float roughness, vec3 albedo) 
{
    float dotVL = dot(L, V);
    float dotLN = dot(L, N);
    float dotNV = dot(N, V);

    float s = dotVL - dotLN * dotNV;
    float t = mix(1.0, max(dotLN, dotNV), step(0.0, s));

    float sigma2 = roughness * roughness;
    vec3 A = 1.0 + sigma2 * (albedo / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));    
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    return albedo * max(0.0, dotLN) * (A + B * s / t);
}

// Does
//  - ambient
//  - GGX specular
//  - Oren nayar diffuse
//  - skybox reflection
//  - shadowing
vec3 shadingComplete(vec3 worldPos, vec3 origN, vec3 N, vec3 V, vec3 L, float AO, float roughness, vec3 diffuse, vec3 specular, float reflectivity)
{
    vec3 color = vec3(0.0);

    // reflection
    vec3 R = reflect(-V, N);
    float lod = roughness * 5.0;
    vec3 reflection = textureLod(uCubeMap, R, lod).rgb;

    // F (Fresnel term)
    vec3 H = normalize(V + L);
    float dotLH = max(dot(L, H), 0.0);
    float F_a = 1.0;
    float F_b = pow(1.0 - dotLH, 5);
    vec3 F = mix(vec3(F_b), vec3(F_a), specular);

    // add light
    {
        // ambient
        color += uAmbientLight * diffuse;

        // oren-nayar diffuse
        vec3 cDiffuse = shadingDiffuseOrenNayar(N, V, L, max(0.01, roughness), diffuse);
        // ggx specular
        vec3 cSpecular = shadingSpecularGGXNoF(N, V, L, max(0.01, roughness), specular);

        color += uLightColor * mix(cDiffuse, cSpecular, F) * shadowing(worldPos, origN, L);

        // skybox reflection
        color += reflectivity * reflection * specular;
    }

    return color * AO;
}
