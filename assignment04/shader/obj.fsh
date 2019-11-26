#version 330 core

uniform vec3 uLightPos = vec3(2, 2, 2);
uniform vec3 uCamPos;
uniform vec3 uColor;

uniform mat4 uShadowViewProj;
uniform sampler2D uShadowMap;

in vec3 vObjectPos;
in vec3 vWorldPos;
in vec3 vViewPos;
in vec3 vNormal;
in vec2 vTexCoord;

out vec3 fColor;

float shadowing()
{
    vec3 shadowDir = normalize(uLightPos - vWorldPos);
    vec3 offset = shadowDir * 0.08;
    vec4 shadowPos = uShadowViewProj * vec4(vWorldPos + offset, 1.0f);
    shadowPos /= shadowPos.w;

    if (abs(shadowPos.x) > 1.0f)
        return 1.0f; // no shadow outside
    if (abs(shadowPos.y) > 1.0f)
        return 1.0f; // no shadow outside
    if (abs(shadowPos.z) > 1.0f)
        return 1.0f; // no shadow outside

    shadowPos.xyz = shadowPos.xyz * 0.5 + 0.5;

    float refZ = texture(uShadowMap, shadowPos.xy).x;
    return refZ > shadowPos.z ? 1.0f : 0.3f;
}

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

    // shadowing
    color *= shadowing();

    return color;
}

void main()
{
    vec3 baseColor = uColor;

    // light vector
    vec3 L = normalize(uLightPos - vWorldPos);
    // view vector
    vec3 V = normalize(uCamPos - vWorldPos);
    // normal vector
    vec3 N = normalize(vNormal);

    // lighting
    fColor = shading(L, V,  N, baseColor);
}
