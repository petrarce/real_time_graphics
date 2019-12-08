uniform sampler2DRect uTexture;

out vec4 fColor;

void main() 
{
    fColor = texelFetch(uTexture, ivec2(gl_FragCoord.xy));
}