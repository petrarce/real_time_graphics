#include "../common.glsl"
#include "../shading.glsl"

uniform sampler2D uTexLightSprites;

out vec4 fAccumA;
out float fAccumB;
out vec3 fDistortion;

in vec2 vTexCoord;
in vec3 vLightColor;
in vec2 vLocalCoord;
in vec3 vWorldPos;
in vec3 vWorldPosReal;

void main()
{
    vec3 color = texture(uTexLightSprites, vTexCoord).rgb;
    float distToQuadCenter = distance(vLocalCoord, vec2(0.5));

    // compute "circular" alpha with varying halo size
    float variation = 0.9 + 0.1 * cos(4*uRuntime + cos(5.3 * uRuntime));
    float alphaFromPosition = max(0, variation-2.0*distToQuadCenter);
    alphaFromPosition = smoothstep(0.0, 1.0, alphaFromPosition);
    float alphaFromColor = max(color.r, max(color.g, color.b));


    //float alpha = min(alphaFromColor, alphaFromPosition);
    float alpha = alphaFromPosition;

    // choose color
    color = mix(vLightColor * color, vLightColor * alphaFromPosition, 0.5);
    color *= 2.5; // brighter

    // Compute a sphere normal
    vec3 dir = normalize(uCamPos - vWorldPos);
    vec3 worldPos = vWorldPosReal + dir * 0.5 * (1 - 2 * distToQuadCenter);
    vec3 normal = normalize(worldPos - vWorldPos);

    // Create some 3D-ish impression
    color *= 0.5 + 0.5 * dot(abs(normal), uLightDir);

    // write T-Buffer
    float w = oitWeight(gl_FragCoord.z, alpha);
    fAccumA = tBufferAccumA(color, alpha, w);
    fAccumB = tBufferAccumB(alpha, w);
    fDistortion = tBufferDistortion(vec2(0.0), 0.0);
}
