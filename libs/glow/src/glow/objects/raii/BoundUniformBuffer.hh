#pragma once

#include <vector>

#include <glow/common/non_copyable.hh>
#include <glow/fwd.hh>
#include <glow/gl.hh>

namespace glow
{
/// RAII-object that defines a "bind"-scope for an UniformBuffer
/// All functions that operate on the currently bound buffer are accessed here
struct BoundUniformBuffer
{
    GLOW_RAII_CLASS(BoundUniformBuffer);

    /// Backreference to the buffer
    UniformBuffer* const buffer;

    /// Sets the data of this uniform buffer (generic version)
    void setData(size_t size, const void* data, GLenum usage = GL_STATIC_DRAW);
    /// Sets the data of this uniform buffer (vector-of-data version)
    template <typename DataT>
    void setData(std::vector<DataT> const& data, GLenum usage = GL_STATIC_DRAW)
    {
        setData(data.size() * sizeof(DataT), data.data(), usage);
    }
    /// Sets the data of this uniform buffer (POD version)
    template <typename DataT>
    void setData(DataT const& data, GLenum usage = GL_STATIC_DRAW)
    {
        setData(sizeof(data), &data, usage);
    }

private:
    GLint previousBuffer;                  ///< previously bound buffer
    BoundUniformBuffer* previousBufferPtr; ///< previously bound buffer
    BoundUniformBuffer(UniformBuffer* buffer);
    friend class UniformBuffer;

    /// returns true iff it's safe to use this bound class
    /// otherwise, runtime error
    bool isCurrent() const;

public:
    BoundUniformBuffer(BoundUniformBuffer&&); // allow move
    ~BoundUniformBuffer();
};
}
