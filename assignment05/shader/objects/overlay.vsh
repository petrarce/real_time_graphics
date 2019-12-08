in vec3 aPosition;
in vec3 aNormal;

out vec3 vWorldPos;
out vec3 vNormal;

uniform mat4 uProj;
uniform mat4 uView;
uniform vec3 uPosition;

void main()
{
    vNormal = aNormal;

    vec3 offsettedPos = aPosition + 1;
    vWorldPos = 1.01*0.5*offsettedPos + uPosition;
    gl_Position = uProj * uView * vec4(vWorldPos, 1.0);
}
