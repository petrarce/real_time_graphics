uniform sampler2D uTexture;

out float fShadow;

void main()
{
    // ivec2 tc = ivec2(gl_FragCoord.xy);
    // float s = texelFetch(uTexture, tc, 0).x;
    // fShadow = s;

    // see https://github.com/Jam3/glsl-fast-gaussian-blur/blob/master/9.glsl

    vec2 uv = gl_FragCoord.xy / vec2(textureSize(uTexture, 0).xy);
    float resolution = float(textureSize(uTexture, 0).x);
    vec2 direction = vec2(0, 1);

    float s = 0.0;
    vec2 off1 = vec2(1.3846153846) * direction;
    vec2 off2 = vec2(3.2307692308) * direction;
    s += texture(uTexture, uv).x * 0.2270270270;
    s += texture(uTexture, uv + (off1 / resolution)).x * 0.3162162162;
    s += texture(uTexture, uv - (off1 / resolution)).x * 0.3162162162;
    s += texture(uTexture, uv + (off2 / resolution)).x * 0.0702702703;
    s += texture(uTexture, uv - (off2 / resolution)).x * 0.0702702703;

    fShadow = s;
}