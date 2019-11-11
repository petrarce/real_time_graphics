#include "debugging_embed_shaders.hh"

// This file is generated upon running CMake, do not modify it!

namespace internal_embedded_files {

const std::pair<const char*, const char*> debugging_embed_shaders[] = {
{"glow-debugging/primitive.opaque.fsh", R"%%RES_EMBED%%(
uniform vec3 uColor;

out vec3 fColor;

void main()
{
    fColor = uColor;
}

)%%RES_EMBED%%"},
{"glow-debugging/primitive.vsh", R"%%RES_EMBED%%(
in vec3 aPosition;

uniform mat4 uModel;
uniform mat4 uVP;

void main()
{
    gl_Position = uVP * uModel * vec4(aPosition, 1.0);
}

)%%RES_EMBED%%"},

};
}
