in vec2 aPosition;

out vec2 vPosition;

void main()
{
    vPosition = aPosition;
    gl_Position = vec4(aPosition * 2 - 1, 0, 1);
}