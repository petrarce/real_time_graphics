uniform mat4 uView;
uniform mat4 uProj;

in float aPosition;

uniform vec3 uFrom;
uniform vec3 uTo;

void main() 
{
    vec3 pos = mix(uFrom, uTo, aPosition);
    gl_Position = uProj * uView * vec4(pos, 1.0);
}
