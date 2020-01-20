uniform float uShadowExponent;

out float fShadow;

void main()
{
    fShadow = exp(gl_FragCoord.z * uShadowExponent);
}