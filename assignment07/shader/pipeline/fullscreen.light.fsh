#include "../common.glsl"
#include "../shading.glsl"

in vec2 vPosition;

out vec3 fColor;

void main() 
{
    ivec2 coords = ivec2(gl_FragCoord.xy);

    // read G-Buffer
    vec4 gDepth = texelFetch(uTexOpaqueDepth, coords);
    vec4 gColor = texelFetch(uTexGBufferColor, coords);
    vec4 gMatA = texelFetch(uTexGBufferMatA, coords);
    vec4 gMatB = texelFetch(uTexGBufferMatB, coords);

    // unpack G-Buffer
    float depth = gDepth.x;
    vec3 albedo = gColor.rgb;
    float AO = gColor.a;
    vec3 N = gMatA.xyz * 2 - 1;
    float metallic = gMatA.w;
    float roughness = gMatB.x;
    float translucency = gMatB.y;

    // restore position
    vec4 screenPos = vec4(vPosition * 2 - 1, depth * 2 - 1, 1.0);
    vec4 viewPos = uInvProj * screenPos;
    viewPos /= viewPos.w;
    vec4 worldPos = uInvView * viewPos;

    // derive properties
    vec3 V = normalize(uCamPos - worldPos.xyz);
    vec3 L = normalize(uLightDir);
    float fragDis = length(viewPos);

    // drop everything after render distance
    if (fragDis >= uRenderDistance)
        discard;

    // diffuse/specular
    vec3 diffuse = albedo * (1 - metallic); // metals have no diffuse
    vec3 specular = mix(vec3(0.04), albedo, metallic); // fixed spec for non-metals

    float reflectivity = mix(0.0, 1.0, max(specular.x, max(specular.y, specular.z))); // TODO!

    // lighting
    fColor = shadingComplete(
        worldPos.xyz,
        N, V, L,
        AO,
        roughness,
        diffuse,
        specular,
        reflectivity,
        translucency
    );

    // add lambertian hemisphere up for better AO
    fColor += diffuse * (dot(N, L) * 0.5 + 0.5) * 0.05;
}
