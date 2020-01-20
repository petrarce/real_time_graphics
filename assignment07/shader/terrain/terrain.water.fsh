#include "material.glsl"
#include "../common.glsl"
#include "../shading.glsl"

in vec3 vWorldPos;
in vec3 vViewPos;
in vec4 vScreenPos;
in vec3 vNormal;
in vec3 vTangent;
in vec2 vTexCoord;
in vec4 vAOs;
in vec2 vUV;

out vec4 fAccumA;
out float fAccumB;
out vec3 fDistortion;

void main()
{
    // out of distance
    float camDis = distance(uCamPos, vWorldPos);
    if (camDis > uRenderDistance)
        discard;

    // calc AOs
    float vAOx0 = mix(vAOs.x, vAOs.z, vUV.x);
    float vAOx1 = mix(vAOs.y, vAOs.w, vUV.x);
    float vAO = mix(vAOx0, vAOx1, vUV.y);
    vAO = vAO * vAO * (3 - 2 * vAO); // smoothstep

    // derive dirs
    vec3 V = normalize(uCamPos - vWorldPos);
    vec3 L = normalize(uLightDir);
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent);
    vec3 B = normalize(cross(T, N));
    mat3 tbn = mat3(T, B, N);
    vec3 ON = N;
    vec3 viewN = mat3(uView) * N;
    
    // material
    vec3 normalMapA = texture(uTexNormal, vTexCoord + uRuntime * vec2(0.01)).rgb;
    vec3 normalMapB = texture(uTexNormal, vTexCoord + uRuntime * vec2(0.003, -0.016)).rgb;
    vec3 specular = vec3(0.3);
    float roughness = 0.15;
    float reflectivity = uReflectivity;
    vec3 waterColor = vec3(0, 1, 1);
    float depthFalloff = 55.0;
    float cubeMapLOD = 3.0;
    float normalSmoothing = 1.0;

    // normal mapping
    normalMapA.z *= normalSmoothing; // make normal map more "moderate"
    normalMapA.xy = normalMapA.xy * 2 - 1;
    normalMapB.z *= normalSmoothing; // make normal map more "moderate"
    normalMapB.xy = normalMapB.xy * 2 - 1;
    vec3 NA = normalize(tbn * normalMapA);
    vec3 NB = normalize(tbn * normalMapB);
    N = normalize(NA + NB);

    // reflection
    vec3 R = reflect(-V, N);
    vec3 cubeMapRefl = textureLod(uTexCubeMap, R, cubeMapLOD).rgb;

    // F (Fresnel term)
    float dotNV = max(dot(ON, V), 0.0);
    float F_a = 1.0;
    float F_b = pow(1.0 - dotNV, 5);
    vec3 F = mix(vec3(F_b), vec3(F_a), specular);
    float alpha = max(F.x, max(F.y, F.z));

    // read opaque Framebuffer
    vec2 screenXY = vScreenPos.xy / vScreenPos.w;
    float opaqueDepth = texelFetch(uTexOpaqueDepth, ivec2(gl_FragCoord.xy)).x;
    vec4 opaquePos4 = uInvProj * vec4(screenXY, opaqueDepth * 2 - 1, 1.0);
    float opaqueDis = opaquePos4.z / opaquePos4.w;
    float waterDepth = vViewPos.z - opaqueDis;

    // reflection
    float shadowF = shadowing(vWorldPos);
    vec3 refl = shadingSpecularGGXNoF(N, V, L, roughness, specular) * shadowF * uLightColor;
    refl += cubeMapRefl * reflectivity * mix(1.0, shadowF, dot(R, L) * 0.5 + 0.5);

    // refraction
    vec2 tsize = vec2(textureSize(uTexOpaqueDepth));
    vec3 RF = refract(-V, N, 1 / 1.33);
    vec3 groundPosR = vWorldPos + RF * clamp(waterDepth / 2, 0.0, 1.0); // clamp due to stability!
    vec4 groundPosS = uProj * uView * vec4(groundPosR, 1.0);
    vec2 refrXY = groundPosS.xy / groundPosS.w;
    vec2 refrXYS = (refrXY * 0.5 + 0.5) * tsize;
    vec2 offset = refrXYS - gl_FragCoord.xy;
    float blurriness = 0.0;

    // depth modulation
    vec3 depthColor = waterColor * (1 - pow(0.5, waterDepth / depthFalloff));
    alpha = mix(1.0, alpha, pow(0.5, waterDepth / depthFalloff));

    // transparent color
    vec3 color = vec3(0.0);
    color += refl;               // reflection (emissive)
    color += depthColor * alpha; // depth modulation (partial coverage)

    // render distance transition
    const float transition = 3.0;
    alpha *= smoothstep(uRenderDistance, uRenderDistance - transition, camDis);

    // write T-Buffer
    float w = oitWeight(gl_FragCoord.z, alpha);
    fAccumA = tBufferAccumA(color, alpha, w);
    fAccumB = tBufferAccumB(alpha, w);
    fDistortion = tBufferDistortion(offset, blurriness);
}