#include "../shading.glsl"

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vTangent;
in vec2 vTexCoord;
in float vAO;

out vec3 fColor;

void main()
{
    if (uIsShadowPass) // early out
    {
        fColor = vec3(1);
        return;
    }

    // derive dirs
    vec3 V = normalize(uCamPos - vWorldPos);
    vec3 L = normalize(uLightDir);
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent);
    vec3 B = normalize(cross(T, N));
    mat3 tbn = mat3(T, B, N);

    // material
    vec3 albedo = texture(uTexAlbedo, vTexCoord).rgb;
    vec3 normalMap = texture(uTexNormal, vTexCoord).xyz;
    float AO = texture(uTexAO, vTexCoord).x;
    float metallic = uMetallic;
    float roughness = texture(uTexRoughness, vTexCoord).x;
    float reflectivity = uReflectivity;

    // normal mapping
    normalMap.xy = normalMap.xy * 2 - 1;
    N = normalize(tbn * normalMap);
    
    // diffuse/specular
    vec3 diffuse = albedo * (1 - metallic); // metals have no diffuse
    vec3 specular = mix(vec3(0.04), albedo, metallic); // fixed spec for non-metals

    // lighting
    fColor = shadingComplete(
        vWorldPos, normalize(vNormal),
        N, V, L,
        AO,
        roughness,
        diffuse,
        specular,
        reflectivity
    ) * vAO; // vertex-AO hack

    // add lambertian hemisphere up for better AO
    fColor += diffuse * (dot(N, L) * 0.5 + 0.5) * 0.1;

    // DEBUG: render ambient occlusion
    if (uShowAmbientOcclusion)
        fColor = vec3(vAO);

    // DEBUG: render normals
    if (uShowNormal)
        fColor = N;
}