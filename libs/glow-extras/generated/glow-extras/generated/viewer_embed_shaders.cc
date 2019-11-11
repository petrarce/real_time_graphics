#include "viewer_embed_shaders.hh"

// This file is generated upon running CMake, do not modify it!

namespace internal_embedded_files {

const std::pair<const char*, const char*> viewer_embed_shaders[] = {
{"glow-viewer/mesh.fsh", R"%%RES_EMBED%%(
uniform vec3 uSunPos;
uniform vec3 uCamPos;

uniform int uRenderMode;
uniform int uShadingMode;

uniform sampler2D uTexture;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vColor;

out vec4 fColor;
out vec3 fNormal;

void main()
{
    vec3 V = normalize(uCamPos - vPosition);
    vec3 L = normalize(uSunPos - vPosition);
    vec3 N = normalize(vNormal);
    vec3 H = normalize(L + V);

    if (!gl_FrontFacing)
        N = -N;

    fColor = vec4(1,0,1,1);

    switch (uRenderMode) {
        case 0:
            fColor.rgb = vec3(1);
            break;
        case 1:
            fColor.rgb = vColor;
            break;
        case 2:
            fColor.rgb = N;
            break;
    }

    switch (uShadingMode) {
        case 0:
            // unlit
            break;
        case 1:
            fColor.rgb *= N.y * .5 + .5;
            break;
    }

    fNormal = N;
}

)%%RES_EMBED%%"},
{"glow-viewer/mesh.shadow.fsh", R"%%RES_EMBED%%(
void main()
{
}

)%%RES_EMBED%%"},
{"glow-viewer/mesh.vsh", R"%%RES_EMBED%%(
uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

in vec3 aPosition;
in vec3 aNormal;
in vec3 aTangent;
in vec3 aColor;

out vec3 vPosition;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vColor;

void main()
{
    vPosition = vec3(uModel * vec4(aPosition, 1.0));
    vNormal = mat3(uModel) * aNormal;
    vTangent = mat3(uModel) * aTangent;
    vColor = aColor;

    gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
}

)%%RES_EMBED%%"},
{"glow-viewer/pp.accum.fsh", R"%%RES_EMBED%%(
uniform sampler2DRect uTexSSAO;
uniform sampler2DRect uTexColor;
uniform sampler2DRect uTexDepth;
uniform sampler2DRect uTexColorTransparent;
uniform sampler2DRect uTexDepthTransparent;

uniform layout(binding=0, rgba32f) image2DRect uTexAccum;

uniform int uAccumCnt;
uniform int uSSAOSamples;
uniform float uSSAOPower;
uniform bool uEnableSSAO;
uniform bool uEnableTonemap;
uniform float uTonemapExposure;

out vec4 fColor;

const mat3 ACESInputMat =
mat3(
    //0.59719, 0.35458, 0.04823,
    //0.07600, 0.90834, 0.01566,
    //0.02840, 0.13383, 0.83777
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04824, 0.01566, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat =
mat3(
    //1.60475, -0.53108, -0.07367,
    //-0.10208,  1.10813, -0.00605,
    //-0.00327, -0.07276,  1.07602

    1.60475, -0.10208, -0.00327,
    -0.53108, 1.10813, -0.07276,
    -0.07367, -0.00605, 1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 ACESFittedTonemap(vec3 inputColor, float exposure, float gamma)
{
    inputColor *= exposure;

    inputColor = ACESInputMat * inputColor;

    // Apply RRT and ODT
    inputColor = RRTAndODTFit(inputColor);

    inputColor = ACESOutputMat * inputColor;
    inputColor = clamp(inputColor, 0.0, 1.0);

    return inputColor;
}

vec3 reinhardTonemap(vec3 rgb)
{
    return rgb / (rgb + vec3(1));
}

void main()
{
    const vec2 fragCoord = gl_FragCoord.xy;
    float d = texture(uTexDepth, fragCoord).x;

    // compose color
    vec4 color = texture(uTexColor, fragCoord);
    vec4 colorTrans = texture(uTexColorTransparent, fragCoord);
    float dTrans = texture(uTexDepthTransparent, fragCoord).x;
    if (dTrans > d)
        color = colorTrans;

    // accum buffer
    if (uAccumCnt > 0)
    {
        vec4 accum = imageLoad(uTexAccum, ivec2(fragCoord));
        color = mix(accum, color, 1.0 / (uAccumCnt + 1));
    }
    imageStore(uTexAccum, ivec2(fragCoord), color);

    // ssao
    if (uEnableSSAO && d > 0)
    {
        float ssao = texture(uTexSSAO, fragCoord).x;
        ssao /= uSSAOSamples;
        ssao = pow(ssao, uSSAOPower);
        color.rgb *= ssao;
    }

    if (d > 0 && uEnableTonemap) color = vec4(ACESFittedTonemap(color.xyz, uTonemapExposure, 2.224), 1.0);
    else color = clamp(color, 0.0, 1.0);

    // gamma correction
    color.rgb = pow(color.rgb, vec3(1 / 2.224));

    fColor = color;
}

)%%RES_EMBED%%"},
{"glow-viewer/pp.bg.fsh", R"%%RES_EMBED%%(
uniform bool uPrintMode;
uniform vec3 uInnerColor;
uniform vec3 uOuterColor;

in vec2 vPosition;

out vec4 fColor;
out vec3 fNormal;

void main()
{
    float d = smoothstep(0.0, 0.5, distance(vPosition, vec2(0.5)));
    fColor.a = 0;
    fColor.rgb = mix(uInnerColor, uOuterColor, d);
    fColor.rgb = pow(fColor.rgb, vec3(2.224));

    if (uPrintMode)
        fColor = vec4(1);

    fNormal = vec3(0,0,0);
}

)%%RES_EMBED%%"},
{"glow-viewer/pp.ground.fsh", R"%%RES_EMBED%%(
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

)%%RES_EMBED%%"},
{"glow-viewer/pp.outline.fsh", R"%%RES_EMBED%%(
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

)%%RES_EMBED%%"},
{"glow-viewer/pp.output.fsh", R"%%RES_EMBED%%(
uniform sampler2DRect uTexOutput;

uniform int uAccumCnt;
uniform ivec2 uViewportOffset;
uniform bool uDebugPixels;

out vec4 fColor;

/**
Basic FXAA implementation based on the code on geeks3d.com with the
modification that the texture2DLod stuff was removed since it's
unsupported by WebGL.
--
From:
https://github.com/mitsuhiko/webgl-meincraft
Copyright (c) 2011 by Armin Ronacher.
Some rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.
    * The names of the contributors may not be used to endorse or
    promote products derived from this software without specific
    prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FXAA_REDUCE_MIN
    #define FXAA_REDUCE_MIN   (1.0/ 128.0)
#endif
#ifndef FXAA_REDUCE_MUL
    #define FXAA_REDUCE_MUL   (1.0 / 8.0)
#endif
#ifndef FXAA_SPAN_MAX
    #define FXAA_SPAN_MAX     8.0
#endif

//optimized version for mobile, where dependent
//texture reads can be a bottleneck
vec4 fxaa(sampler2DRect tex, vec2 fragCoord) {
    vec4 color;
    vec3 rgbNW = texture(tex, fragCoord + vec2(-1.0, -1.0)).xyz;
    vec3 rgbNE = texture(tex, fragCoord + vec2(1.0, -1.0)).xyz;
    vec3 rgbSW = texture(tex, fragCoord + vec2(-1.0, 1.0)).xyz;
    vec3 rgbSE = texture(tex, fragCoord + vec2(1.0, 1.0)).xyz;
    vec4 texColor = texture(tex, fragCoord);
    vec3 rgbM  = texColor.xyz;
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    mediump vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
                        (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
            max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
            dir * rcpDirMin));

    vec3 rgbA = 0.5 * (
        texture(tex, fragCoord + dir * (1.0 / 3.0 - 0.5)).xyz +
        texture(tex, fragCoord + dir * (2.0 / 3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(tex, fragCoord + dir * -0.5).xyz +
        texture(tex, fragCoord + dir * 0.5).xyz);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        color = vec4(rgbA, texColor.a);
    else
        color = vec4(rgbB, texColor.a);
    return color;
}

uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

float wang_float(uint hash)
{
    return hash / float(0x7FFFFFFF) / 2.0;
}

void main()
{
    const float fxaaMin = 1;
    const float fxaaMax = 8;
    const vec2 fragCoord = gl_FragCoord.xy - uViewportOffset;

    vec4 color = texture(uTexOutput, fragCoord);
    float ca = color.a;

    if (uAccumCnt < fxaaMax)
    {
        vec4 fcolor = fxaa(uTexOutput, fragCoord);
        color = mix(fcolor, color, smoothstep(fxaaMin, fxaaMax, float(uAccumCnt)));
    }

    // dithering
    uint seed = uint(fragCoord.x) + uint(fragCoord.y) * 8196;
    float r = wang_float(wang_hash(seed * 3 + 0));
    float g = wang_float(wang_hash(seed * 3 + 1));
    float b = wang_float(wang_hash(seed * 3 + 2));
    vec3 random = vec3(r, g, b);
    color.rgb += (random - .5) * (1 / 256.);

    // transparency debug
    if (uDebugPixels)
    {
        bool t = ca > 0;
        bool t0 = texture(uTexOutput, fragCoord + vec2(1, 0)).a > 0;
        bool t1 = texture(uTexOutput, fragCoord + vec2(-1, 0)).a > 0;
        bool t2 = texture(uTexOutput, fragCoord + vec2(0, 1)).a > 0;
        bool t3 = texture(uTexOutput, fragCoord + vec2(0, -1)).a > 0;

        if (t)
        {
            float a = 1;
            if (t0 && t1 && t2 && t3)
                a = .3;
            color.rgb = mix(color.rgb, vec3(1,0,1), a);
        }
    }

    fColor = color;
}

)%%RES_EMBED%%"},
{"glow-viewer/pp.shadow.fsh", R"%%RES_EMBED%%(
uniform sampler2DRect uShadowMap;
uniform mat4 uSunView;
uniform mat4 uSunProj;
uniform vec3 uGroundShadowMin;
uniform vec3 uGroundShadowMax;

in vec2 vPosition;

out float fShadow;

void main()
{
    vec3 p = uGroundShadowMin + vec3(vPosition.x, 0, vPosition.y) * (uGroundShadowMax - uGroundShadowMin);

    fShadow = 0;

    // shadow
    vec4 sp = uSunProj * uSunView * vec4(p, 1);
    sp /= sp.w;
    if (sp.x > -1 && sp.x < 1 && sp.y > -1 && sp.y < 1)
    {
        float shadow_d = texture(uShadowMap, (sp.xy * .5 + .5) * textureSize(uShadowMap)).x;
        float ref_d = sp.z;

        fShadow = int(shadow_d > ref_d);
    }
}
)%%RES_EMBED%%"},
{"glow-viewer/pp.ssao.fsh", R"%%RES_EMBED%%(
uniform sampler2DRect uTexDepth;
uniform sampler2DRect uTexNormal;

uniform vec2 uScreenSize;
uniform float uRadius;
uniform uint uSeed;

uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uInvProj;

uniform int uSamples;
uniform ivec2 uViewportOffset;

out float fSSAO;

uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

float wang_float(uint hash)
{
    return hash / float(0x7FFFFFFF) / 2.0;
}

vec3 viewPosAt(vec2 sp)
{
    float d = texture(uTexDepth, sp).x;
    vec4 vp = uInvProj * vec4(sp / uScreenSize * 2 - 1, d, 1);
    return vec3(vp) / vp.w;
}

void main()
{
    if (uRadius <= 0)
    {
        fSSAO = uSamples;
        return;
    }

    float visible_samples = 0;
    const float pi = 3.14159265359;
    const vec2 fragCoord = gl_FragCoord.xy;
    
    vec3 n = texture(uTexNormal, fragCoord).xyz;
    if (n == vec3(0))
    {
        fSSAO = uSamples;
        return;
    }

    n = normalize(mat3(uView) * n);
    vec3 p = viewPosAt(fragCoord);

    vec3 t0 = normalize(cross(abs(n.x) > abs(n.y) ? vec3(0,1,0) : vec3(1,0,0), n));
    vec3 t1 = normalize(cross(n, t0));

    uint seed = uint(fragCoord.x) + (uint(fragCoord.y) << 16);
    seed ^= uSeed;

    for (int _ = 0; _ < uSamples; ++_)
    {
        seed = wang_hash(seed);
        float r = sqrt(wang_float(seed));
        seed = wang_hash(seed);
        float a = wang_float(seed) * pi * 2;
        seed = wang_hash(seed);
        float rr = wang_float(seed);
        
        float ca = cos(a);
        float sa = sin(a);

        float x = r * ca;
        float y = r * sa;
        float z = sqrt(1 - x * x - y * y);

        vec3 pp = p + (x * t0 + y * t1 + z * n) * (uRadius * rr);
        vec4 sp = uProj * vec4(pp, 1);
        sp.xyz /= sp.w;

        if (sp.w < 0)
            visible_samples++;
        else 
        {
            float test_d = sp.z;
            float ref_d = texture(uTexDepth, (sp.xy * .5 + .5) * uScreenSize).x;

            visible_samples += float(test_d > ref_d);
        }
    }

    fSSAO = visible_samples;    
}

)%%RES_EMBED%%"},
{"glow-viewer/pp.vsh", R"%%RES_EMBED%%(
in vec2 aPosition;

out vec2 vPosition;

void main()
{
    vPosition = aPosition;
    gl_Position = vec4(aPosition * 2 - 1, 0, 1);
}
)%%RES_EMBED%%"},

};
}
