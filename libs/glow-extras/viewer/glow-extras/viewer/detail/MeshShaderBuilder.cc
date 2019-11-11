#include "MeshShaderBuilder.hh"

#include <glow/common/log.hh>

#include <glow/objects/Program.hh>
#include <glow/objects/Shader.hh>

using namespace glow;
using namespace glow::viewer;
using namespace glow::viewer::detail;

void MeshShaderBuilder::addUniform(const std::string& type, const std::string& name)
{
    const Variable v = {type, name};
    if (findMatchingAttrOrUniform(v) == nullptr)
        mUniforms.push_back(v);
    else
        glow::warning() << "uniform " << name << " not added because already defined";
}

void MeshShaderBuilder::addAttribute(const std::string& type, const std::string& name)
{
    const Variable v = {type, name};
    if (findMatchingAttrOrUniform(v) == nullptr)
        mAttributes.push_back(v);
    else
        glow::warning() << "attribute " << name << " not added because already defined";
}

void MeshShaderBuilder::addFragmentLocation(const std::string& type, const std::string& name)
{
    mFragmentLocations += "out " + type + " " + name + ";\n";
}

void MeshShaderBuilder::addVertexShaderCode(const std::string& code)
{
    mVertexShaderBody += code;
    mVertexShaderBody += "\n";
}

void MeshShaderBuilder::addGeometryShaderCode(const std::string& code)
{
    mGeometryShaderBody += code;
    mGeometryShaderBody += "\n";
    mGeometryShaderUsed = true;
}

void MeshShaderBuilder::addFragmentShaderCode(const std::string& code)
{
    mFragmentShaderBody += code;
    mFragmentShaderBody += "\n";
}

void MeshShaderBuilder::addVertexShaderDecl(const std::string& code)
{
    mVertexShaderDecl += code;
    mVertexShaderDecl += "\n";
}

void MeshShaderBuilder::addGeometryShaderDecl(const std::string& code)
{
    mGeometryShaderDecl += code;
    mGeometryShaderDecl += "\n";
    mGeometryShaderUsed = true;
}

void MeshShaderBuilder::addFragmentShaderDecl(const std::string& code)
{
    mFragmentShaderDecl += code;
    mFragmentShaderDecl += "\n";
}

void MeshShaderBuilder::addPassthrough(std::string const& type, std::string const& nameWithoutPrefix)
{
    mInterfaceBlockVars.push_back({type, nameWithoutPrefix});
}

MeshShaderBuilder::Variable const* MeshShaderBuilder::findMatchingAttrOrUniform(Variable const& toSearch)
{
    for (auto const& a : mAttributes)
        if (a.name == toSearch.name || a.name == "a" + toSearch.name)
            return &a;

    for (auto const& u : mUniforms)
        if (u.name == toSearch.name || u.name == "u" + toSearch.name || u.name == "a" + toSearch.name) // a can be a uniform prefix for ConstantMeshAttributes
            return &u;

    return nullptr;
}

glow::SharedProgram MeshShaderBuilder::createProgram()
{
    // Passthroughs
    std::string vsPassthroughCode;
    std::string gsPassthroughFunc = "void passthrough(int vInIndex) {\n"; // Function to call to pass the data for the vertex with the given index through
    std::string gsPassthroughMixFunc = "void passthroughMix(int index1, int index2, float alpha) {\n"; // Function to call to mix the data of the two vertices for passthrough
    std::string fsPassthroughCode;
    std::string interfaceBlockContent = "VertexData {\n";
    for (auto const& v : mInterfaceBlockVars)
    {
        // Add declaration to interface block of all shaders
        interfaceBlockContent += "    " + v.type + " " + v.name + ";\n";

        // Pass value from attribute if present
        auto matching = findMatchingAttrOrUniform(v);
        if (matching != nullptr)
            vsPassthroughCode += "    vOut." + v.name + " = " + matching->name + ";\n";

        // Prefill the output with the value of the given vertex, not emitted yet
        if (mGeometryShaderUsed) gsPassthroughFunc += "    vOut." + v.name + " = vIn[vInIndex]." + v.name + ";\n";
        if (mGeometryShaderUsed) gsPassthroughMixFunc += "    vOut." + v.name + " = mix(vIn[index1]." + v.name + ", vIn[index2]." + v.name + ", alpha);\n";

        // Copy interface block to local variables so that they can be overridden by modular components
        fsPassthroughCode += "    " + v.type + " v" + v.name + " = vIn." + v.name + ";\n";
    }
    mVertexShaderDecl = "out " + interfaceBlockContent + "} vOut;\n\n" + mVertexShaderDecl;
    mFragmentShaderDecl = "in " + interfaceBlockContent + "} vIn;\n\n" + mFragmentShaderDecl;
    if (mGeometryShaderUsed)
    {
        mGeometryShaderDecl = "in " + interfaceBlockContent + "} vIn[];\n"
                            + "out " + interfaceBlockContent + "} vOut;\n\n"
                            + gsPassthroughFunc + "}\n"
                            + gsPassthroughMixFunc + "}\n\n"
                            + mGeometryShaderDecl;
    }

    // Uniforms
    std::string uniformCode;
    for (auto const& u : mUniforms)
    {
        uniformCode += "uniform " + u.type + " " + u.name + ";\n";
    }

    // Attributes
    std::string attrCode;
    for (auto const& a : mAttributes)
    {
        attrCode += "in " + a.type + " " + a.name + ";\n";
    }

    // vertex shader
    std::string vsCode;
    {
        vsCode += uniformCode + "\n";
        vsCode += attrCode + "\n";
        vsCode += mVertexShaderDecl + "\n";
        vsCode += "void main() {\n";
        vsCode += vsPassthroughCode;
        vsCode += mVertexShaderBody;
        vsCode += "}\n";
    }

    // geometry shader
    std::string gsCode;
    if (mGeometryShaderUsed)
    {
        gsCode += uniformCode + "\n";
        gsCode += mGeometryShaderDecl + "\n";
        gsCode += "void main() {\n";
        gsCode += mGeometryShaderBody;
        gsCode += "}\n";
    }

    // fragment shader
    std::string fsCode;
    {
        fsCode += uniformCode + "\n";
        fsCode += mFragmentLocations + "\n";
        fsCode += mFragmentShaderDecl + "\n";
        fsCode += R"(
                  uint hash_combine(uint seed, uint h)
                  {
                      return seed ^ h + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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
                  )";
        fsCode += "void main() {\n";
        fsCode += fsPassthroughCode;
        fsCode += mFragmentShaderBody;
        fsCode += "}\n";
    }

    // glow::info() << "Vertex Shader:\n" << vsCode;
    // glow::info() << "Geometry Shader:\n" << gsCode;
    // glow::info() << "Fragment Shader:\n" << fsCode;

    auto vs = Shader::createFromSource(GL_VERTEX_SHADER, vsCode);
    auto fs = Shader::createFromSource(GL_FRAGMENT_SHADER, fsCode);
    if (mGeometryShaderUsed)
    {
        auto gs = Shader::createFromSource(GL_GEOMETRY_SHADER, gsCode);
        return Program::create({vs, gs, fs});
    }
    return Program::create({vs, fs});
}
