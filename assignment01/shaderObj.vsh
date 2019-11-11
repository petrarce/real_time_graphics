in vec2 aPosition;

uniform vec2 uPosition;
uniform vec2 uSize;

void main()
{
    vec2 pos = uPosition + aPosition * uSize;
    gl_Position = vec4(pos * 2 - 1, 0, 1);
}