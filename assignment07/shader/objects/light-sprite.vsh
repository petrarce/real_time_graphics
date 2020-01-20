#include "../common.glsl"

in vec2 aPosition;

in vec3 aLightPosition;
in float aLightRadius;
in vec3 aLightColor;
in int aLightSeed;

out vec2 vTexCoord;
out vec3 vLightColor;
out vec2 vLocalCoord;
out vec3 vWorldPos;
out vec3 vWorldPosReal;

void main()
{
    int rnd = (aLightSeed + int(uRuntime*20)) % 16;
    int x = rnd % 4;
    int y = rnd / 4;

    vTexCoord = (vec2(x, y) + aPosition) / 4;
    vLightColor = aLightColor;
    vLocalCoord = aPosition;
    vWorldPos = aLightPosition;

    vec4 viewPos = uView * vec4(aLightPosition, 1.0);
    viewPos.xy += (aPosition * 2 - 1) * 0.2 * aLightRadius;

    vWorldPosReal = vec3(uInvView * viewPos);

    gl_Position = uProj * viewPos;
}
