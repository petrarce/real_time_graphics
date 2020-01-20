#include "../common.glsl"

in vec3 aPosition;

in vec3 aLightPosition;
in float aLightRadius;
in vec3 aLightColor;
in int aLightSeed;

out vec3 vLightColor;
out float vLightRadius;
out vec3 vLightPosition;
out vec4 vScreenPos;

void main()
{
    vLightColor = aLightColor;
    vLightRadius = aLightRadius;
    vLightPosition = aLightPosition;

    // TODO: some early z calc

    vec3 worldPos = aLightPosition + aPosition * aLightRadius;
    vScreenPos = uProj * uView * vec4(worldPos, 1.0);
    gl_Position = vScreenPos;
}
