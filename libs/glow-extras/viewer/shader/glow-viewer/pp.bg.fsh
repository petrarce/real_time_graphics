uniform bool uPrintMode;
uniform vec3 uInnerColor;
uniform vec3 uOuterColor;

in vec2 vPosition;

out vec4 fColor;
out vec3 fNormal;

void main()
{
    float d = smoothstep(0.0, 0.5, distance(vPosition, vec2(0.5)));
    fColor.a = 0;
    fColor.rgb = mix(uInnerColor, uOuterColor, d);
    fColor.rgb = pow(fColor.rgb, vec3(2.224));

    if (uPrintMode)
        fColor = vec4(1);

    fNormal = vec3(0,0,0);
}
