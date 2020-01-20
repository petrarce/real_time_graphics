#pragma once

#include <cstddef>
#include <cstdint>

struct Block
{
    /// Material index
    /// 0  means air
    /// >0 means solid
    /// <0 means translucent
    /// 127 means "invalid"
    int8_t mat = 0;

public: // properties
    bool isAir() const { return mat == 0; }
    bool isSolid() const { return mat > 0; }
    bool isTranslucent() const { return mat < 0; }
    bool isInvalid() const { return mat == 127; }

public: // ctor
    explicit Block(int8_t mat = 0) : mat(mat) {}

public: // statics
    static Block air() { return Block(); }
    static Block invalid() { return Block(127); }
};
