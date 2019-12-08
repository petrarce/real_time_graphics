out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vTangent;
out vec2 vTexCoord;
out vec3 vViewPos;
out vec4 vScreenPos;
out float vAO;

uniform mat4 uProj;
uniform mat4 uView;
uniform float uTextureScale;

///
/// Vertex shader code for all terrain materials
///
/// Your job is to:
///     - define vertex attribute(s) for the data you need
///     - compute tangent and texture coordinate
///     - write to all defined out-variables (vWorldPos, ...)
///
///     - Advanced: unpack position, ao value and normal from your optimized vertex format
///
/// Notes:
///     - don't forget to divide texture coords by uTextureScale
///
/// ============= STUDENT CODE BEGIN =============

in vec3 aPosition;

void main()
{
    vNormal = vec3(0, 1, 0);
    vTangent = vec3(1, 0, 0);
    vTexCoord = vec2(0, 0); // don't forget uTextureScale
    vAO = 1.0;

    vWorldPos = aPosition;
    vViewPos = vec3(uView * vec4(aPosition, 1.0));
    vScreenPos = uProj * vec4(vViewPos, 1.0);

    gl_Position = vScreenPos;
}

/// ============= STUDENT CODE END =============
