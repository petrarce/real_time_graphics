in vec2 aPosition;

void main() 
{
    gl_Position = vec4(aPosition * 2 - 1, 0, 1);
}