uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform vec3 uCamPos;

uniform float uShadowStrength;
uniform float uShadowSamples;
uniform vec3 uGroundShadowMin;
uniform vec3 uGroundShadowMax;
uniform sampler2D uShadowMapSoft;

uniform float uGroundY;
uniform float uMeshDiag;
uniform vec3 uMeshCenter;
uniform bool uShowGrid;

uniform sampler2DRect uTexDepth;

in vec2 vPosition;

out vec4 fColor;
out vec4 fNormal;

void main()
{
    const float scale = 3;
    const float size0 = 0.015;
    const float size1 = 0.08;

    vec3 V;

    {
        vec4 p0 = uInvProj * vec4(vPosition * 2 - 1, 1, 1);
        vec4 p1 = uInvProj * vec4(vPosition * 2 - 1, 0.8, 1);
        p0 /= p0.w;
        p1 /= p1.w;
        V = normalize(mat3(uInvView) * (p1 - p0).xyz);
    }

    if ((V.y > 0) == (uCamPos.y > uGroundY))
        discard;

    fColor.rgb = vec3(0,0,0);
    fColor.a = 0;

    vec3 p = uCamPos - V * (uCamPos.y - uGroundY) / V.y;

    float centerDis = distance(p.xz, uMeshCenter.xz);
    if (centerDis > uMeshDiag)
        discard;

    float ref_depth = texture(uTexDepth, gl_FragCoord.xy).x;
    vec4 sp = uProj * uView * vec4(p, 1);
    sp /= sp.w;
    if (V.y > 0)
    {
        gl_FragDepth = ref_depth;
        fNormal = vec4(0,0,0,0);
    }
    else
    {
        if (sp.z <= ref_depth)
            discard;
        gl_FragDepth = sp.z;
        fNormal = vec4(0,0,0,1);
    }

    vec2 uv0 = fract(p.xz / uMeshDiag * scale);
    vec2 uv1 = fract(p.xz / uMeshDiag * scale * 10);

    uv0.x -= float(uv0.x > 0.5);
    uv0.y -= float(uv0.y > 0.5);
    uv1.x -= float(uv1.x > 0.5);
    uv1.y -= float(uv1.y > 0.5);

    if(uShowGrid)
    {
        fColor.a = max(fColor.a, int(abs(uv0.x) < size0 || abs(uv0.y) < size0));
        fColor.a = max(fColor.a, int(abs(uv1.x) < size1 || abs(uv1.y) < size1) * 0.5);
        fColor.a *= 0.5;
    }

    vec2 shadowPos = ((p - uGroundShadowMin) / (uGroundShadowMax - uGroundShadowMin)).xz;
    float shadow = texture(uShadowMapSoft, shadowPos).x / uShadowSamples;
    fColor.a = mix(fColor.a, 1, shadow * uShadowStrength);

    fColor.a *= smoothstep(uMeshDiag * 1, uMeshDiag * 0.7, centerDis);
}
