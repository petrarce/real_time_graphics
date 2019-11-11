#pragma once

#include <vector>

#include <glow/fwd.hh>

#include <glow-extras/glfw/GlfwApp.hh>

#include <typed-geometry/tg-lean.hh>

class Assignment01 : public glow::glfw::GlfwApp
{
    struct Student
    {
        std::string name;
        int nr;
    };

private:
    glow::SharedTexture2D mTextureTask;
    glow::SharedVertexArray mQuad;
    glow::SharedVertexArray mMeshTask;
    glow::SharedProgram mShaderTask;
    glow::SharedProgram mShaderObj;

    tg::vec2 mVelocity = {0.234f, -0.345f};
    tg::pos2 mPosition = {0.45f, 0.45f};
    tg::size2 mInitialQuadSize = {0.1f, 0.1f};
    tg::size2 mSize;

private:
    std::vector<Student> getGroup() const;
    void generateTask();

public:
    void mainLoop() override;
    void init() override;
    void update(float elapsedSeconds) override;
    void render(float = 0.0f) override;
};
