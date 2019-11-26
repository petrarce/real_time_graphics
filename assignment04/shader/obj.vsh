#version 330 core

in vec3 aPosition;
in vec3 aNormal;
in vec2 aTexCoord;

uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uProjectionMatrix;

out vec3 vObjectPos;
out vec3 vWorldPos;
out vec3 vViewPos;
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    vTexCoord = aTexCoord;

    vObjectPos = aPosition;
    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vec4 viewPos = uViewMatrix * worldPos;
    gl_Position = uProjectionMatrix * viewPos;

    vViewPos = viewPos.xyz;
    vWorldPos = worldPos.xyz;

    vNormal = inverse(transpose(mat3(uModelMatrix))) * aNormal;
}
