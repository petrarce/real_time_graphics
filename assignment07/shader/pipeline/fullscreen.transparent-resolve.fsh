#include "../common.glsl"
#include "../shading.glsl"

// defined in shading: uniform sampler2DRect uTexOpaqueDepth;
uniform sampler2DRect uTexShadedOpaque;
uniform sampler2DRect uTexTBufferAccumA;
uniform sampler2DRect uTexTBufferAccumB;
uniform sampler2DRect uTexTBufferDistortion;

uniform bool uDrawBackground;

in vec2 vPosition;

out vec3 fColor;

void main()
{
    ivec2 coords = ivec2(gl_FragCoord.xy);

    vec3 opaqueColor = vec3(0);
    vec3 transparentColor = vec3(0);
    float alpha = 0.0;

    /// Task 2.b
    /// Weighted, Blended Order-Independent Transparency
    ///
    /// Your job is to:
    ///     - read the T-Buffer
    ///     - calculate "transparentColor" and "alpha" based on the T-Buffer
    ///
    /// Notes:
    ///     - remember that revealage and accum.a are swapped
    ///     - you may want to clamp the accumulated alpha between 1e-4 and 5e4
    ///
    /// ============= STUDENT CODE BEGIN =============

    /// ============= STUDENT CODE END =============

    /// Task 2.c
    /// opaque color with refraction
    ///
    /// Your job is to:
    ///     - use the distortion part of the T-Buffer to implement fake refractions
    ///
    /// Notes:
    ///     - the distortion is saved in pixel coordinates
    ///     - be sure to use bi-linear interpolation
    ///
    /// ============= STUDENT CODE BEGIN =============
    opaqueColor = texelFetch(uTexShadedOpaque, coords).rgb;
    float opaqueDepth = texelFetch(uTexOpaqueDepth, coords).x;
    /// ============= STUDENT CODE END =============

    // background
    if (uDrawBackground)
    {
        /// Task 2.a
        ///
        /// Your job is to:
        ///     - draw the background of the scene by changing opaqueColor
        ///     - the transition should be smooth (e.g. blend the last 3m)
        ///
        /// Notes:
        ///     - the maximal render distance is stored in uRenderDistance (do not use far plane only!)
        ///     - the skybox is stored in uTexCubeMap
        ///     - uInvProj and uInvView contain inverse projection and view matrix
        ///     - vPosition contains screen coordinates from 0..1
        ///
        /// ============= STUDENT CODE BEGIN =============

        /// ============= STUDENT CODE END =============
    }

    fColor = mix(opaqueColor, transparentColor, alpha);
}
