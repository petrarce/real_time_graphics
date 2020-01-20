uniform vec3 uLightDir;
uniform vec3 uAmbientLight;
uniform vec3 uLightColor;

uniform samplerCube uTexCubeMap;

uniform sampler2DRect uTexOpaqueDepth;
uniform sampler2DRect uTexGBufferColor;
uniform sampler2DRect uTexGBufferMatA;
uniform sampler2DRect uTexGBufferMatB;

const int SHADOW_CASCADES = 3;
uniform mat4 uShadowViewProjs[SHADOW_CASCADES];
uniform sampler2DArray uShadowMaps;
uniform float uShadowExponent;
uniform vec3 uShadowPos;
uniform float uShadowRange;

vec3 hsv2rgb(vec3 c) 
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float shadowPenDepth(vec3 worldPos)
{
    int casc = clamp(int(distance(uCamPos, worldPos) / uRenderDistance * SHADOW_CASCADES), 0, SHADOW_CASCADES - 1);

    vec4 shadowPos = uShadowViewProjs[casc] * vec4(worldPos, 1.0);
    shadowPos.xyz = shadowPos.xyz * 0.5 + 0.5;

    float sm = texture(uShadowMaps, vec3(shadowPos.xy, casc)).x;

    float refZ = shadowPos.z;

    float s = sm * exp(-refZ * uShadowExponent);

    return -log(s) / uShadowExponent * uShadowRange;
}

float shadowing(vec3 worldPos)
{
    return smoothstep(0.1, 0.0, shadowPenDepth(worldPos));
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

// for point lights
vec3 shadingLightOnly(vec3 worldPos, vec3 N, vec3 V, vec3 L, 
    float AO, float roughness, 
    vec3 diffuse, vec3 specular,
    vec3 lightColor
)
{
    // F (Fresnel term)
    vec3 H = normalize(V + L);
    float dotLH = max(dot(L, H), 0.0);
    float F_a = 1.0;
    float F_b = pow(1.0 - dotLH, 5);
    vec3 F = mix(vec3(F_b), vec3(F_a), specular);

    // oren-nayar diffuse
    vec3 cDiffuse = shadingDiffuseOrenNayar(N, V, L, max(0.01, roughness), diffuse);
    // ggx specular
    vec3 cSpecular = shadingSpecularGGXNoF(N, V, L, max(0.01, roughness), specular);

    vec3 color = lightColor * mix(cDiffuse, cSpecular, F);

    return color * AO;
}

// Does
//  - ambient
//  - GGX specular
//  - Oren nayar diffuse
//  - skybox reflection
//  - shadowing
vec3 shadingComplete(vec3 worldPos, vec3 N, vec3 V, vec3 L, 
    float AO, float roughness, 
    vec3 diffuse, vec3 specular, 
    float reflectivity, float translucency
)
{
    vec3 color = vec3(0.0);

    // reflection
    vec3 R = reflect(-V, N);
    float lod = roughness * 5.0;
    vec3 reflection = textureLod(uTexCubeMap, R, lod).rgb;

    // F (Fresnel term)
    vec3 H = normalize(V + L);
    float dotLH = max(dot(L, H), 0.0);
    float F_a = 1.0;
    float F_b = pow(1.0 - dotLH, 5);
    vec3 F = mix(vec3(F_b), vec3(F_a), specular);

    // shadow
    float shadowPen = max(0.0, shadowPenDepth(worldPos));
    float shadowF = smoothstep(0.1, 0.0, shadowPen);

    // add light
    {
        // ambient
        color += uAmbientLight * diffuse;
        
        // single scattering approximation
        if (translucency > 0)
        {
            vec3 cDiffuse = shadingDiffuseOrenNayar(-N, -V, L, max(0.01, roughness), diffuse);

            vec3 sssMod = exp(-shadowPen / (translucency * vec3(1.0, 0.9, 0.8)));
            color += uLightColor * cDiffuse * (1 - F) * sssMod;
        }

        // normal GGX
        {
            // oren-nayar diffuse
            vec3 cDiffuse = shadingDiffuseOrenNayar(N, V, L, max(0.01, roughness), diffuse);
            // ggx specular
            vec3 cSpecular = shadingSpecularGGXNoF(N, V, L, max(0.01, roughness), specular);

            color += uLightColor * mix(cDiffuse, cSpecular, F) * shadowF;
        }

        // skybox reflection
        color += reflectivity * reflection * specular;
    }

    return color * AO;
}
