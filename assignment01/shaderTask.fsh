in vec2 E;

out vec3 fColor;

uniform sampler2D uTexture;

void main()
{
    fColor = texture(uTexture, E).rgb;

    bool antialias = true;

    if(antialias)
    {
        float dx = 1.0/512;
        fColor += texture(uTexture, E + vec2(-dx, 0)).rgb;
        fColor += texture(uTexture, E + vec2(0, -dx)).rgb;
        fColor += texture(uTexture, E + vec2(dx, 0)).rgb;
        fColor += texture(uTexture, E + vec2(0, dx)).rgb;

        // Do not divide by 5.0 to compensate darker color due to averaging
        fColor /= 4.0;
    }

    if (fColor.r < 0.5) discard;
}
