#pragma once

#include <vector>

#include <glow/common/log.hh>
#include <glow/common/non_copyable.hh>
#include <glow/gl.hh>

namespace glow
{
class ArrayBuffer;

/// RAII-object that defines a "bind"-scope for an ArrayBuffer
/// All functions that operate on the currently bound buffer are accessed here
struct BoundArrayBuffer
{
    GLOW_RAII_CLASS(BoundArrayBuffer);

    /// Backreference to the buffer
    ArrayBuffer* const buffer;

    /// Sets the raw data contained in the array buffer
    void setData(size_t sizeInBytes, const void* data = nullptr, GLenum usage = GL_STATIC_DRAW);

    /// Sets the array buffer data using a vector of POD (plain-old-data)
    /// Works excellent for glm types or user-defined vertex structs
    /// (e.g. struct Vertex { glm::vec3 pos; float f; })
    /// Warns if stride is not equal data size
    template <typename DataT>
    void setData(std::vector<DataT> const& data, GLenum usage = GL_STATIC_DRAW)
    {
        implSetData(sizeof(DataT) * data.size(), data.data(), sizeof(DataT), usage);
    }

    /// Same as above
    /// Usage:
    ///   Vertex vertices[] = { ... }
    ///   setData(vertices);
    template <typename DataT, std::size_t N>
    void setData(const DataT (&data)[N], GLenum usage = GL_STATIC_DRAW)
    {
        implSetData(sizeof(DataT) * N, data, sizeof(DataT), usage);
    }

private:
    GLint previousBuffer;                ///< previously bound buffer
    BoundArrayBuffer* previousBufferPtr; ///< previously bound buffer
    BoundArrayBuffer(ArrayBuffer* buffer);
    friend class glow::ArrayBuffer;

    /// returns true iff it's safe to use this bound class
    /// otherwise, runtime error
    bool isCurrent() const;

    void implSetData(size_t sizeInBytes, const void* data, size_t stride, GLenum usage);

public:
    BoundArrayBuffer(BoundArrayBuffer&&); // allow move
    ~BoundArrayBuffer();
};
}
