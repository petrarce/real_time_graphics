out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vTangent;
out vec2 vTexCoord;
out vec3 vViewPos;
out vec4 vScreenPos;
out vec4 vAOs;
out vec2 vUV;
out vec4 vEdges;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uViewProj;
uniform float uTextureScale;

in vec3 aPosition;
in int aFlags;
// Flags:
//  4 values     - vIdx
//  6 values     - pDir
//  4 x 4 values - ao for all sides
// TODO: edges

void main()
{
    // unpack flags
    int flags = aFlags;

    // .. vertex idx
    int vIdx = flags % 4;
    flags /= 4;

    // .. packed direction
    int pdir = flags % 6;
    int s = pdir / 3 * 2 - 1;
    int dir = pdir % 3;
    flags /= 6;

    // .. AOs
    vAOs.x = float(flags % 4) / 3.0;
    flags /= 4;
    vAOs.y = float(flags % 4) / 3.0;
    flags /= 4;
    vAOs.z = float(flags % 4) / 3.0;
    flags /= 4;
    vAOs.w = float(flags % 4) / 3.0;
    flags /= 4;

    // .. edges
    vEdges.x = float(flags % 3) - 1.0;
    flags /= 3;
    vEdges.y = float(flags % 3) - 1.0;
    flags /= 3;
    vEdges.z = float(flags % 3) - 1.0;
    flags /= 3;
    vEdges.w = float(flags % 3) - 1.0;
    flags /= 3;
    
    // derive TBN
    vec3 N = vec3(float(dir == 0), float(dir == 1), float(dir == 2)) * float(s);
    vec3 T = cross(N, vec3(float(dir == 1), float(dir == 2), float(dir == 0)));
    vec3 B = cross(T, N);

    // derive UV
    vUV = vec2(
        float(vIdx / 2),
        float(vIdx % 2)
    );
    vTexCoord = vec2(
        dot(aPosition, T),
        dot(aPosition, B)
    ) / uTextureScale;
    
    vNormal = N;
    vTangent = T;

    vWorldPos = aPosition;
    vViewPos = vec3(uView * vec4(aPosition, 1.0));
    vScreenPos = uViewProj * vec4(aPosition, 1.0);

    gl_Position = vScreenPos;
}
