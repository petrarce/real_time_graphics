uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

/// Task 1.a
/// Draw the line mesh
///
/// Your job is to:
///     - define the vertex attributes and uniforms that you need for the rendering
///     - compute the position, transform it to screen space and pass it on
///
/// Notes:
///     - gl_Position is four-dimensional
///
/// ============= STUDENT CODE BEGIN =============

in vec3 aPosition;

uniform vec3 uColor;

void main() 
{
    gl_Position = uProjectionMatrix * uViewMatrix * vec4(aPosition, 1.0);
}

/// ============= STUDENT CODE END =============
