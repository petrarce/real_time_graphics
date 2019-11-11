#version 330 core

in vec3 aPosition;
in vec3 aNormal;
in vec3 aColor;

uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uProjectionMatrix;

out vec3 vObjectPos;
out vec3 vWorldPos;
out vec3 vViewPos;
out vec3 vNormal;
out vec3 vColor;

void main() {
    vObjectPos = aPosition;
    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vec4 viewPos = uViewMatrix * worldPos;
    gl_Position = uProjectionMatrix * viewPos;

    vViewPos = viewPos.xyz;
    vWorldPos = worldPos.xyz;
    vColor = aColor;

    // assuming non-non-uniform scaling
    vNormal = mat3(uModelMatrix) * aNormal;
}
