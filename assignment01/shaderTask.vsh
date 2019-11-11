in vec2 aPosition;
in vec2 A;
in vec2 B;
in vec2 C;
in vec2 D;
out vec2 E;

uniform float uWidth;
uniform float uHeight;

void main()
{
    vec2 p = A + D * aPosition;
    p.y = 1 - p.y * uWidth / uHeight;
    E = B + C * aPosition;
    gl_Position = vec4(p * 2 - 1, 0, 1);
}
