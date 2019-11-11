#pragma once

// usage e.g. GLOW_VIEWER_INJECT_FUNCTION(glow::viewer::view);
//
// currently injects into:
//   glow::viewer::builder
//   glow
//   polymesh
//   glm
//   tg
#define GLOW_VIEWER_INJECT_FUNCTION(function)      \
    namespace glow                                 \
    {                                              \
    using function;                                \
    namespace vector                               \
    {                                              \
    using function;                                \
    }                                              \
    namespace viewer                               \
    {                                              \
    namespace builder                              \
    {                                              \
    using function;                                \
    }                                              \
    }                                              \
    }                                              \
    namespace polymesh                             \
    {                                              \
    using function;                                \
    }                                              \
    namespace tg                                   \
    {                                              \
    using function;                                \
    }                                              \
    GLOW_VIEWER_IMPL_INJECT_FUNCTION_GLM(function) \
    static_assert(true, "") // force ;

#ifdef GLOW_HAS_GLM
#define GLOW_VIEWER_IMPL_INJECT_FUNCTION_GLM(function) \
    namespace glm                                      \
    {                                                  \
    using function;                                    \
    }
#else
#define GLOW_VIEWER_IMPL_INJECT_FUNCTION_GLM(function)
#endif
