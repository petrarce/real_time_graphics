#version 330 core

uniform vec3 uLightPos = vec3(2, 2, 2);
uniform vec3 uCamPos;

uniform float uTranslucency;
uniform bool uShowNormals;

in vec3 vObjectPos;
in vec3 vWorldPos;
in vec3 vViewPos;
in vec3 vNormal;
in vec3 vColor;

out vec3 fColor;

vec3 shading(vec3 L, vec3 V, vec3 N, vec3 baseColor)
{
    vec3 color = vec3(0);

    vec3 H = normalize(V + L);
    float dotNH = dot(N, H);
    if (dotNH >= 0.0)
    {
        // Lambertian diffuse
        color += max(0.0, dotNH) * baseColor;

        // Only positive specular (if at all)
        vec3 R = reflect(-V, N);
        float dotRL = max(0.0, dot(R, L));
        color += pow(dotRL, 8.0) * baseColor * 0.5 * vec3(1, 1, 1);
    }

    return color;
}

void main()
{
    vec3 baseColor = vColor;

    // light vector
    vec3 L = normalize(uLightPos - vWorldPos);
    // view vector
    vec3 V = normalize(uCamPos - vWorldPos);
    // normal vector
    vec3 N = normalize(vNormal);

    // swap if wrong dir
    if (!gl_FrontFacing)
        N *= -1.0f;

    if (uShowNormals)
    {
        fColor = N;
        return;
    }

    // front and back lighting
    fColor = mix(
        shading(L, V,  N, baseColor),
        shading(L, V, -N, baseColor),
        uTranslucency
    );
}
