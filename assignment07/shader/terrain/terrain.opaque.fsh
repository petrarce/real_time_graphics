#include "material.glsl"
#include "../common.glsl"

uniform sampler2DRect uTexOpaqueDepth;
uniform bool uShowWrongDepthPre;

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vTangent;
in vec2 vTexCoord;
in vec4 vAOs;
in vec2 vUV;
in vec4 vEdges;

out vec4 fColor;
out vec4 fMatA;
out vec2 fMatB;

void main()
{
    // calc AOs
    float vAOx0 = mix(vAOs.x, vAOs.z, vUV.x);
    float vAOx1 = mix(vAOs.y, vAOs.w, vUV.x);
    float vAO = mix(vAOx0, vAOx1, vUV.y);
    vAO = vAO * vAO * (3 - 2 * vAO); // smoothstep

    // derive edges

    const float edginess = 0.1;

    float edgeT = 0.0;
    float edgeB = 0.0;
    edgeT -= vEdges.x * smoothstep(1 - edginess, 1.0, vUV.x);
    edgeT += vEdges.y * smoothstep(edginess, 0.0, vUV.x);
    edgeB -= vEdges.z * smoothstep(1 - edginess, 1.0, vUV.y);
    edgeB += vEdges.w * smoothstep(edginess, 0.0, vUV.y);
    //edgeT = edgeB = 0;

    // derive dirs
    vec3 N = normalize(vNormal + edgeT * vTangent + edgeB * cross(vTangent, vNormal));
    vec3 T = normalize(vTangent);
    vec3 B = normalize(cross(T, N));
    mat3 tbn = mat3(T, B, N);

    // material
    vec3 albedo = texture(uTexAlbedo, vTexCoord).rgb;
    vec3 normalMap = texture(uTexNormal, vTexCoord).xyz;
    float AO = texture(uTexAO, vTexCoord).x;
    float metallic = uMetallic;
    float roughness = texture(uTexRoughness, vTexCoord).x;
    float translucency = uTranslucency;

    vec3 edgeColor = texture(uTexAlbedo, vec2(0.5), 10).rgb;

    // edginess
    float edgeF = max(abs(edgeT), abs(edgeB));
    albedo = mix(albedo, edgeColor, edgeF);
    normalMap = mix(normalMap, vec3(0.5, 0.5, 1.0), edgeF);
    AO = mix(AO, 1.0, edgeF);
    roughness = mix(roughness, 0.6, edgeF);

    // normal mapping
    normalMap.xy = normalMap.xy * 2 - 1;
    N = normalize(tbn * normalMap);


    // Debug: if wrong depth-pre pass, show it
    if (uShowWrongDepthPre && gl_FragCoord.z != texelFetch(uTexOpaqueDepth, ivec2(gl_FragCoord.xy)).x)
    {
        ivec3 ip = ivec3(floor(vWorldPos * 4 + 0.001));
        albedo = vec3(1,0,1) * float((ip.x + ip.y + ip.z) % 2 == 0);
    }

    // write G-Buffer
    fColor = gBufferColor(albedo, AO * vAO);
    fMatA = gBufferMatA(N, metallic);
    fMatB = gBufferMatB(roughness, translucency);

    // fColor.rgb = abs(vEdges.xyz);
    // fColor.rgb = vec3(edgeT * 0.5 + 0.5, edgeB * 0.5 + 0.5, 0.5);
}