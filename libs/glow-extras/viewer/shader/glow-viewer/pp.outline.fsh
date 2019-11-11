uniform sampler2DRect uTexDepth;
uniform sampler2DRect uTexNormal;

uniform float uNearPlane;
uniform vec3 uCamPos;
uniform mat4 uInvProj;
uniform mat4 uInvView;

uniform float uDepthThreshold;
uniform float uNormalThreshold;
uniform ivec2 uViewportOffset;

in vec2 vPosition;

out vec4 fColor;

float linearDepth(float d)
{
    return uNearPlane / d;
}

float linearDepthAt(vec2 t)
{
    return linearDepth(texture(uTexDepth, t).x);
}
float depthAt(vec2 t)
{
    return texture(uTexDepth, t).x;
}

void main()
{
    const vec2 fragCoord = gl_FragCoord.xy;
    float rd = depthAt(fragCoord);
    float d = linearDepth(rd);
    vec3 n = texture(uTexNormal, fragCoord).xyz;

    vec4 ss = vec4(vPosition * 2 - 1, rd, 1);
    vec4 vs = uInvProj * ss;
    vs /= vs.w;
    vec4 ws = uInvView * vs;
    vec3 worldPos = ws.xyz;

    vec3 camDir = normalize(worldPos - uCamPos);

    vec2[] offsets = vec2[](
        vec2(1, 0),
        vec2(0, 1),
        vec2(-1, 0),
        vec2(0, -1)
    );

    bool is_edge = false;

    float df = float(n != vec3(0)); // 0 if normal is 0

    float depthThreshold = uDepthThreshold / (abs(dot(camDir, n)) + 0.001);
    depthThreshold = mix(uDepthThreshold, depthThreshold, df);
    
    for (int od = 1; od <= 1; ++od)
    for (int i = 0; i < 4; ++i)
    {
        vec2 o = offsets[i] * od;
        float dd = linearDepthAt(fragCoord + o);
        vec3 nn = texture(uTexNormal, fragCoord + o).xyz;

        is_edge = is_edge || abs(d - dd) * df > depthThreshold;
        is_edge = is_edge || distance(n, nn) > uNormalThreshold;
    }

    if (is_edge)
        fColor = vec4(0,0,0,0.8);
    else
        fColor = vec4(0,0,0,0);
}
