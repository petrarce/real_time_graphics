#pragma once

#include <glow/fwd.hh>

namespace glow
{
namespace material
{
/// Static helper class for pre-calculating IBL-related maps
///
/// Requires access to material shaders, e.g.:
///   DefaultShaderParser::addIncludePath("PATH/TO/glow-extras/material/shader");
///
class IBL
{
public:
    /**
     * @brief creates a 2D look-up table for the GGX environment BRDF
     *
     * Parametrization:
     *   x: roughness in [0,1]
     *   y: dot(N, V) in [0,1]
     *
     * Linear filtering, no mipmaps, clamp to edge
     *
     * NOTE: normally you don't need this function as initShaderGGX will use an internal version
     */
    static SharedTexture2D createEnvLutGGX(int width = 512, int height = 512);

    /**
     * @brief Returns the default cube map, loading it if necessary
     */
    static SharedTextureCubeMap const& getDefaultCubeMap();

    /**
     * @brief creates a pre-filtered environment map for use with GGX IBL
     */
    static SharedTextureCubeMap createEnvMapGGX(SharedTextureCubeMap const& envMap, int size = 512);

    /**
     * @brief prepates a shader for use with GGX-based IBL
     *
     * Must be called at least once for every shader that wants to use this type of IBL
     *
     * If no custom LUT is provided, an internal 64x64 one will be used.
     */
    static void initShaderGGX(UsedProgram& shader, SharedTexture2D const& customLUT = nullptr);

    /**
     * @brief prepares a shader for actual IBL use
     *
     * initShaderGGX must be called once per shader and is independent of environment map
     * prepareShaderGGX must be called whenever the env map changes
     *
     * NOTE: don't use a normal environment map here. Use a prefiltered one using 'createEnvMapGGX'
     */
    static void prepareShaderGGX(UsedProgram& shader, SharedTextureCubeMap const& envMapGGX);

    /**
     * @brief Initializes glow-extras-material shaders
     *
     * Either adds embedded shaders as virtual files, or adds the required include path to DefaultShaderParser
     */
    static void GlobalInit();

private:
    IBL() = delete;

    static SharedTexture2D sEnvLutGGX;
    static SharedTextureCubeMap sDefaultEnvMap;
};
}
}
