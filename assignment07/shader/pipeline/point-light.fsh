#include "../common.glsl"
#include "../shading.glsl"

uniform bool uDebugLights;

in vec3 vLightColor;
in float vLightRadius;
in vec3 vLightPosition;
in vec4 vScreenPos;

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
    vec4 screenPos = vec4(vScreenPos.xy / vScreenPos.w, depth * 2 - 1, 1.0);
    vec4 viewPos = uInvProj * screenPos;
    viewPos /= viewPos.w;
    vec3 worldPos = vec3(uInvView * viewPos);

    // early out
    float lightDis2 = distance2(worldPos, vLightPosition);
    if (lightDis2 > vLightRadius * vLightRadius)
        discard;

    // derive properties
    vec3 V = normalize(uCamPos - worldPos);
    vec3 L = normalize(uLightDir);
    float fragDis = length(viewPos);
    
    // drop everything after render distance
    if (fragDis >= uRenderDistance)
        discard;
        
    // diffuse/specular
    vec3 diffuse = albedo * (1 - metallic); // metals have no diffuse
    vec3 specular = mix(vec3(0.04), albedo, metallic); // fixed spec for non-metals

    float reflectivity = mix(0.0, 0.5, max(specular.x, max(specular.y, specular.z))); // TODO!

    float attenuation = smoothstep(vLightRadius, 0.0, sqrt(lightDis2));

    // lighting
    fColor = shadingLightOnly(
        worldPos.xyz,
        N, V, L,
        AO,
        roughness,
        diffuse,
        specular,
        vLightColor
    ) * attenuation;

    // Debug: light color
    if (uDebugLights)
        fColor = vLightColor;
}
