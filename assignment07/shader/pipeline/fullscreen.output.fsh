#include "../common.glsl"

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
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9u;
    seed = seed ^ (seed >> 4u);
    seed *= 0x27D4EB2Du;
    seed = seed ^ (seed >> 15u);
    return seed;
}

float wang_float(uint hash)
{
    return hash / float(0x7FFFFFFF) / 2.0;
}

uniform sampler2DRect uTexture;
uniform bool uUseFXAA;
uniform bool uUseDithering;

// rendering pipeline textures
uniform sampler2DArray uShadowMaps;
uniform float uShadowExponent;

uniform sampler2DRect uTexOpaqueDepth;
uniform sampler2DRect uTexShadedOpaque;

uniform sampler2DRect uTexGBufferColor;
uniform sampler2DRect uTexGBufferMatA;
uniform sampler2DRect uTexGBufferMatB;

uniform sampler2DRect uTexTBufferAccumA;
uniform sampler2DRect uTexTBufferAccumB;
uniform sampler2DRect uTexTBufferDistortion;

uniform int uDebugOutput;

in vec2 vPosition;

out vec3 fColor;

float linearDepth(float z)
{
    vec4 sp = vec4(vPosition * 2 - 1, z * 2 - 1, 1.0);
    vec4 vp = uInvProj * sp;
    vp /= vp.w;
    vec4 wp = uInvView * vp;
    return distance(uCamPos, wp.xyz) / uRenderDistance;
}

vec3 lin2sRGB(vec3 c)
{
    return pow(c, vec3(1 / 2.224));
}

void main() 
{
    ivec2 coords = ivec2(gl_FragCoord.xy);

    // FXAA
    vec3 color = uUseFXAA ? fxaa(uTexture, gl_FragCoord.xy).rgb :
                            texelFetch(uTexture, coords).rgb;

    // Linear to sRGB
    fColor = lin2sRGB(color);

    // Dithering
    if (uUseDithering)
    {
        uint seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * 8096u;
        float r = wang_float(wang_hash(seed * 3u + 0u));
        float g = wang_float(wang_hash(seed * 3u + 1u));
        float b = wang_float(wang_hash(seed * 3u + 2u));
        vec3 random = vec3(r, g, b);
        fColor += (random - 0.5) / 256.0;
    }

    // Pipeline Debug Output
    switch (uDebugOutput)
    {
    // Output,
    case 0:
        break;

    // OpaqueDepth,
    case 1:
        fColor = vec3(linearDepth(texelFetch(uTexOpaqueDepth, coords).x));
        break;
    // ShadedOpaque,
    case 2:
        fColor = lin2sRGB(texelFetch(uTexShadedOpaque, coords).rgb);
        break;

    // GBufferAlbedo,
    case 3:
        fColor = lin2sRGB(texelFetch(uTexGBufferColor, coords).rgb);
        break;
    // GBufferAO,
    case 4:
        fColor = vec3(texelFetch(uTexGBufferColor, coords).a);
        break;
    // GBufferNormal,
    case 5:
        fColor = vec3(texelFetch(uTexGBufferMatA, coords).xyz);
        break;
    // GBufferMetallic,
    case 6:
        fColor = vec3(texelFetch(uTexGBufferMatA, coords).w);
        break;
    // GBufferRoughness,
    case 7:
        fColor = vec3(texelFetch(uTexGBufferMatB, coords).x);
        break;
    // GBufferTranslucency,
    case 8:
        fColor = vec3(texelFetch(uTexGBufferMatB, coords).y);
        break;

    // TBufferColor,
    case 9:
        fColor = vec3(texelFetch(uTexTBufferAccumA, coords).rgb / texelFetch(uTexTBufferAccumB, coords).x);
        break;
    // TBufferAlpha,
    case 10:
        fColor = vec3(1 - texelFetch(uTexTBufferAccumA, coords).a);
        break;
    // TBufferDistortion,
    case 11:
        fColor = vec3(fract(texelFetch(uTexTBufferDistortion, coords).xy / 30), 0);
        break;
    // TBufferBlurriness,
    case 12:
        fColor = vec3(texelFetch(uTexTBufferDistortion, coords).z);
        break;

    // ShadowCascade0,
    case 13:
        fColor = vec3(log(texture(uShadowMaps, vec3(vPosition, 0)).x) / uShadowExponent);
        break;
    // ShadowCascade1,
    case 14:
        fColor = vec3(log(texture(uShadowMaps, vec3(vPosition, 1)).x) / uShadowExponent);
        break;
    // ShadowCascade2,
    case 15:
        fColor = vec3(log(texture(uShadowMaps, vec3(vPosition, 2)).x) / uShadowExponent);
        break;
    }
}
