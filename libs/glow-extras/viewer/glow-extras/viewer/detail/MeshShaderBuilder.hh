#pragma once

#include <string>
#include <vector>

#include <glow/fwd.hh>

namespace glow
{
namespace viewer
{
namespace detail
{
// TODO: cache shaders
class MeshShaderBuilder
{
private:
    struct Variable
    {
        std::string type;
        std::string name;
    };

private:
    std::vector<Variable> mAttributes;
    std::vector<Variable> mUniforms;

    std::string mVertexShaderBody;
    std::string mVertexShaderDecl;

    std::string mGeometryShaderBody;
    std::string mGeometryShaderDecl;

    std::string mFragmentLocations;
    std::string mFragmentShaderBody;
    std::string mFragmentShaderDecl;

    std::vector<Variable> mInterfaceBlockVars;
    bool mGeometryShaderUsed = false;

public:
    void addUniform(std::string const& type, std::string const& name);
    void addAttribute(std::string const& type, std::string const& name);
    void addFragmentLocation(std::string const& type, std::string const& name);
    void addVertexShaderCode(std::string const& code);
    void addGeometryShaderCode(std::string const& code);
    void addFragmentShaderCode(std::string const& code);
    void addVertexShaderDecl(std::string const& code);
    void addGeometryShaderDecl(std::string const& code);
    void addFragmentShaderDecl(std::string const& code);

    /// addPassthrough("Color") adds Color to interface blocks
    void addPassthrough(std::string const& type, std::string const& nameWithoutPrefix);

private:
    Variable const* findMatchingAttrOrUniform(Variable const& toSearch);

public:
    SharedProgram createProgram();
};
}
}
}
