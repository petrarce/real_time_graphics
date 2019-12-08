#include "../shading.glsl"

uniform sampler2DRect uFramebufferOpaque;
uniform sampler2DRect uFramebufferDepth;

in vec3 vWorldPos;
in vec3 vViewPos;
in vec4 vScreenPos;
in vec3 vNormal;
in vec3 vTangent;
in vec2 vTexCoord;
in float vAO;

out vec3 fColor;

void main()
{
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
    vec3 normalMap = texture(uTexNormal, vTexCoord + uRuntime * 0.01).rgb;
    vec3 specular = vec3(0.3);
    float roughness = 0.15;
    float reflectivity = 1.0;
    vec3 waterColor = vec3(0, 1, 1);
    float depthFalloff = 5.0;
    float cubeMapLOD = 3.0;
    float normalSmoothing = 1.0;

    // normal mapping
    normalMap.z *= normalSmoothing; // make normal map more "moderate"
    normalMap.xy = normalMap.xy * 2 - 1;
    N = normalize(tbn * normalMap);

    // reflection
    vec3 R = reflect(-V, N);
    vec3 cubeMapRefl = textureLod(uCubeMap, R, cubeMapLOD).rgb;

    // F (Fresnel term)
    float dotNV = max(dot(ON, V), 0.0);
    float F_a = 1.0;
    float F_b = pow(1.0 - dotNV, 5);
    vec3 F = mix(vec3(F_b), vec3(F_a), specular);

    // read opaque Framebuffer
    vec2 screenXY = vScreenPos.xy / vScreenPos.w;
    float opaqueDepth = texelFetch(uFramebufferDepth, ivec2(gl_FragCoord.xy)).x;
    vec4 opaquePos4 = uInvProj * vec4(screenXY, opaqueDepth * 2 - 1, 1.0);
    float opaqueDis = opaquePos4.z / opaquePos4.w;
    float waterDepth = clamp(vViewPos.z - opaqueDis, 0.0, 3.0); // clamp due to stability!

    // custom z test
    if (gl_FragCoord.z > opaqueDepth)
        discard;

    // water lighting
    //  - reflection:
    //      - GGX
    //      - Skybox
    //  - refraction:
    //      - opaque sample (depth-modulated, tinted)
    vec3 color = vec3(0);

    // reflection
    vec3 refl = shadingSpecularGGXNoF(N, V, L, roughness, specular) * shadowing(vWorldPos, normalize(vNormal), L) * uLightColor;
    refl += cubeMapRefl * reflectivity;

    // refraction
    vec2 tsize = vec2(textureSize(uFramebufferOpaque));
    vec3 RF = refract(-V, N, 1 / 1.33);
    vec3 groundPosR = vWorldPos + RF * waterDepth;
    vec4 groundPosS = uProj * uView * vec4(groundPosR, 1.0);
    vec2 refrXY = groundPosS.xy / groundPosS.w;
    vec2 refrXYS = (refrXY * 0.5 + 0.5) * tsize;
    vec3 opaqueColor = texture(uFramebufferOpaque, refrXYS).rgb;

    // depth modulation
    vec3 refr = mix(waterColor, opaqueColor, pow(0.5, waterDepth / depthFalloff));

    // return mix of refl and refr
    fColor = mix(refr, refl, F);
}