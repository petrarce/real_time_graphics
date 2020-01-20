uniform mat4 uViewProj;

in vec3 aPosition;

void main()
{
    gl_Position = uViewProj * vec4(aPosition, 1.0);
}
