in vec2 vPosition;

uniform samplerCube uTexCubeMap;

uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform vec3 uLightDir;
uniform float uRuntime;

out vec3 fColor;

void main() 
{
    vec4 near = uInvProj * vec4(vPosition * 2 - 1, 0, 1);
    vec4 far = uInvProj * vec4(vPosition * 2 - 1, 0.5, 1);

    near /= near.w;
    far /= far.w;

    near = uInvView * near;
    far = uInvView * far;

    fColor = texture(uTexCubeMap, (far - near).xyz).rgb;

    vec3 sunColor = vec3(0.9,0.9,0.7);
    float dotVL = max(0, dot(uLightDir, normalize(far - near).xyz));
    // Sun
    fColor += 0.2*smoothstep(0.996, 0.9999, dotVL) * sunColor;
    // Corona
    fColor += mix(vec3(0), sunColor, pow(dotVL, 180));

    fColor = pow(fColor, vec3(1 / 2.2));
}
