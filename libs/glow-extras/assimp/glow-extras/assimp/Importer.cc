#include "Importer.hh"

#include <fstream>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <glm/glm.hpp>

#include <glow/common/profiling.hh>
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>

static glm::vec3 aiCast(aiVector3D const& v)
{
    return {v.x, v.y, v.z};
}
static glm::vec4 aiCast(aiColor4D const& v)
{
    return {v.r, v.g, v.b, v.a};
}

glow::assimp::Importer::Importer() {}

glow::SharedVertexArray glow::assimp::Importer::load(const std::string& filename) const
{
    GLOW_ACTION();

    auto data = loadData(filename);
    return data ? data->createVertexArray() : nullptr;
}

glow::assimp::SharedMeshData glow::assimp::Importer::loadData(const std::string& filename) const
{
    using namespace assimp;

    if (!std::ifstream(filename).good())
    {
        error() << "Error loading `" << filename << "' with Assimp.";
        error() << "  File not found/not readable";
        return nullptr;
    }

    uint32_t flags = aiProcess_SortByPType;

    if (mTriangulate)
        flags |= aiProcess_Triangulate;
    if (mCalculateTangents)
        flags |= aiProcess_CalcTangentSpace;
    if (mGenerateSmoothNormal)
        flags |= aiProcess_GenSmoothNormals;
    if (mGenerateUVCoords)
        flags |= aiProcess_GenUVCoords;
    if (mPreTransformVertices)
        flags |= aiProcess_PreTransformVertices;
    if (mFlipUVs)
        flags |= aiProcess_FlipUVs;

    Assimp::Importer importer;
    auto scene = importer.ReadFile(filename, flags);

    if (!scene)
    {
        error() << "Error loading `" << filename << "' with Assimp.";
        error() << "  " << importer.GetErrorString();
        return nullptr;
    }

    if (!scene->HasMeshes())
    {
        error() << "File `" << filename << "' has no meshes.";
        return nullptr;
    }

    auto data = std::make_shared<MeshData>();

    auto& positions = data->positions;
    auto& normals = data->normals;
    auto& tangents = data->tangents;
    auto& texCoords = data->texCoords;
    auto& colors = data->colors;
    auto& indices = data->indices;

    auto baseIdx = 0u;
    for (auto i = 0u; i < scene->mNumMeshes; ++i)
    {
        auto const& mesh = scene->mMeshes[i];
        auto colorsCnt = mesh->GetNumColorChannels();
        auto texCoordsCnt = mesh->GetNumUVChannels();

        if (texCoords.empty())
            texCoords.resize(texCoordsCnt);
        else if (texCoords.size() != texCoordsCnt)
        {
            error() << "File `" << filename << "':";
            error() << "  contains inconsistent texture coordinate counts";
            return nullptr;
        }

        if (colors.empty())
            colors.resize(colorsCnt);
        else if (colors.size() != colorsCnt)
        {
            error() << "File `" << filename << "':";
            error() << "  contains inconsistent vertex color counts";
            return nullptr;
        }

        // add faces
        auto fCnt = mesh->mNumFaces;
        for (auto f = 0u; f < fCnt; ++f)
        {
            auto const& face = mesh->mFaces[f];
            if (face.mNumIndices != 3)
            {
                error() << "File `" << filename << "':.";
                error() << "  non-3 faces not implemented/supported";
                return nullptr;
            }

            for (auto fi = 0u; fi < face.mNumIndices; ++fi)
            {
                indices.push_back(baseIdx + face.mIndices[fi]);
            }
        }

        data->aabbMin = glm::vec3(+std::numeric_limits<float>().infinity());
        data->aabbMax = glm::vec3(-std::numeric_limits<float>().infinity());

        // add vertices
        auto vCnt = mesh->mNumVertices;
        for (auto v = 0u; v < vCnt; ++v)
        {
            auto const vertexPosition = aiCast(mesh->mVertices[v]);

            data->aabbMin = glm::min(data->aabbMin, vertexPosition);
            data->aabbMax = glm::max(data->aabbMax, vertexPosition);

            positions.push_back(vertexPosition);

            if (mesh->HasNormals())
                normals.push_back(aiCast(mesh->mNormals[v]));
            if (mesh->HasTangentsAndBitangents())
                tangents.push_back(aiCast(mesh->mTangents[v]));

            for (auto t = 0u; t < texCoordsCnt; ++t)
                texCoords[t].push_back((glm::vec2)aiCast(mesh->mTextureCoords[t][v]));
            for (auto t = 0u; t < colorsCnt; ++t)
                colors[t].push_back(aiCast(mesh->mColors[t][v]));
        }

        baseIdx = static_cast<unsigned>(positions.size());
    }

    return data;
}
