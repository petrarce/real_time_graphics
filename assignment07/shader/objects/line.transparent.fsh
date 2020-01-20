#include "../common.glsl"
#include "../shading.glsl"

uniform vec3 uColor;

out vec4 fAccumA;
out float fAccumB;
out vec3 fDistortion;

void main()
{
    vec3 color = uColor;
    float alpha = 1.0;    

    // write T-Buffer
    float w = oitWeight(gl_FragCoord.z, alpha);
    fAccumA = tBufferAccumA(color, alpha, w);
    fAccumB = tBufferAccumB(alpha, w);
    fDistortion = tBufferDistortion(vec2(0.0), 0.0);
}
