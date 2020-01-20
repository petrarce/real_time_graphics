uniform sampler2DArray uTexture;
uniform int uCascade;

out float fShadow;

void main()
{
    // fShadow = texelFetch(uTexture, ivec3(gl_FragCoord.xy, uCascade), 0).x;
    // return;

    // see https://github.com/Jam3/glsl-fast-gaussian-blur/blob/master/9.glsl

    vec3 uv = vec3(gl_FragCoord.xy / vec2(textureSize(uTexture, 0).xy), uCascade);
    float resolution = float(textureSize(uTexture, 0).x);
    vec2 direction = vec2(1, 0);

    float s = 0.0;
    vec3 off1 = vec3(vec2(1.3846153846) * direction, 0);
    vec3 off2 = vec3(vec2(3.2307692308) * direction, 0);
    s += texture(uTexture, uv).x * 0.2270270270;
    s += texture(uTexture, uv + (off1 / resolution)).x * 0.3162162162;
    s += texture(uTexture, uv - (off1 / resolution)).x * 0.3162162162;
    s += texture(uTexture, uv + (off2 / resolution)).x * 0.0702702703;
    s += texture(uTexture, uv - (off2 / resolution)).x * 0.0702702703;

    fShadow = s;
}