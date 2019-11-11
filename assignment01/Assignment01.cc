#include "Assignment01.hh"

// OpenGL header
#include <glow/gl.hh>

// Glow helper
#include <glow/common/log.hh>
#include <glow/common/str_utils.hh>

// used OpenGL object wrappers
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/VertexArray.hh>

// extra helper
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/timing/CpuTimer.hh>

// in the implementation, we want to omit the glow:: prefix
using namespace glow;

/// Returns the number of seconds since program start (as double)
static double timeInSeconds();

std::vector<Assignment01::Student> Assignment01::getGroup() const
{
    std::vector<Student> group;
    /// Add all group members here
    /// e.g. group.push_back({"John Doe", 333333});
    /// ============= STUDENT CODE BEGIN =============

    group.push_back({"Folbort, Iohannes", 392157});
    group.push_back({"Liu, Yinglun", 391562});
    group.push_back({"Niu, Xueru", 392344});

    /// ============= STUDENT CODE END =============
    return group;
}

void Assignment01::mainLoop()
{
    // Show initial image so that task is known
    render();

    auto totalFrames = 0;
    auto totalStartTime = timeInSeconds();

    /// Implement the mainLoop variant that is shown on screen
    /// You can get the current wall clock via:
    ///     double secs = timeInSeconds();
    /// ============ STUDENT CODE BEGIN =============

    //min milliseconds to run for each loop
    constexpr double timeStep = 33./1000;
    double gameTime = timeInSeconds();
    int totalUpdates = 0;
    // this is an example of the simplest main loop:
    // variable timestep with no time measuring
    while (!shouldClose())
    {
        // update the simulation
        // (parameter is timestep in seconds)
        while(gameTime < timeInSeconds()){
            update(timeStep);
            gameTime += timeStep;
            totalUpdates++;

        }

        // render the simulation
        render();

        // count the number of frames so you can verify your FPS
        totalFrames += 1;

        // give the CPU time to breathe
        // (negative arguments are ignored)
        sleepSeconds(16 / 1000.0);
    }

    /// ============= STUDENT CODE END =============

    // output FPS
    // only valid if totalFrames is properly incremented
    auto totalTime = timeInSeconds() - totalStartTime;
    glow::info() << totalFrames << " frames in " << totalTime << " secs: " << totalFrames / totalTime << " FPS";
    glow::info() << totalUpdates << " updates in " << totalTime << " secs: " << totalUpdates / totalTime << " UPS";
}

void Assignment01::update(float elapsedSeconds)
{
    // update external input
    updateInput();

    // a simple moving quad
    {
        // make quad size independent from window aspect ratio
        auto ws = getWindowSize();
        mSize = mInitialQuadSize * tg::size2(ws.height / (float)ws.width, 1.0f);

        // equation of motion
        mPosition += mVelocity * elapsedSeconds;

        // reflection on boundary
        if (mPosition.x < 0 && mVelocity.x < 0)
            mVelocity.x *= -1.0f;
        if (mPosition.x + mSize.width > 1 && mVelocity.x > 0)
            mVelocity.x *= -1.0f;

        if (mPosition.y < 0 && mVelocity.y < 0)
            mVelocity.y *= -1.0f;
        if (mPosition.y + mSize.height > 1 && mVelocity.y > 0)
            mVelocity.y *= -1.0f;
    }

    // simulate some "heavy work"
    sleepSeconds(1.5 / 1000.0);
}


void Assignment01::render(float)
{
    // start rendering
    beginRender();

    // clear the screen in RWTH blue
    glClearColor(0.00f, 0.33f, 0.62f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw the simulation
    {
        auto shader = mShaderObj->use();
        shader.setUniform("uPosition", mPosition);
        shader.setUniform("uSize", mSize);
        mQuad->bind().draw();
    }

    // draw the task
    {
        auto shader = mShaderTask->use();
        shader.setTexture("uTexture", mTextureTask);
        shader.setUniform("uWidth", (float)getWindowWidth());
        shader.setUniform("uHeight", (float)getWindowHeight());
        mMeshTask->bind().draw();
    }

    // simulate some "heavy rendering"
    sleepSeconds(3.5 / 1000.0);

    // finalize rendering, swap buffers
    endRender();
}


void Assignment01::init()
{
    GlfwApp::init();

    // don't show AION timings to keep output clean for this task
    setDumpTimingsOnShutdown(false);

    // no vsync here due to custom mainLoop
    setVSync(false);

    // load simulation resources
    mQuad = geometry::Quad<>().generate();
    mShaderObj = Program::createFromFile(util::pathOf(__FILE__) + "/shaderObj");

    // load task resources
    mTextureTask = Texture2D::createFromFile(util::pathOf(__FILE__) + "/task.png", glow::ColorSpace::sRGB);
    mShaderTask = Program::createFromFile(util::pathOf(__FILE__) + "/shaderTask");
    generateTask();
}

void Assignment01::generateTask()
{
    auto group = getGroup();
    if (group.empty())
    {
        glow::error() << "Group is empty! Please enter students at Assignment01::getGroup().";
        return;
    }

    std::sort(begin(group), end(group), [](Student const& s1, Student const& s2) { return s1.nr < s2.nr; });

    auto valCombine = [](uint64_t seed, uint64_t h) { return seed ^ (h + 0x9e3779b9 + (seed << 6) + (seed >> 2)); };

    uint64_t hash = 42;
    for (auto const& s : group)
        hash = valCombine(hash, s.nr);

    std::vector<uint64_t> chunks;
    switch (hash % 3)
    {
    case 0:
        chunks = {80, 60, 21, 24, 84, 7 + valCombine(hash, 91) % 3, 26};
        break;
    case 1:
        chunks = {67, 2, 4 + valCombine(hash, 91) % 3, 66, 21, 19, 35, 35, 25 + valCombine(hash, 17) % 2, 27};
        break;
    case 2:
        chunks = {11, 0, 62 + valCombine(hash, 91) % 3, 8, 35, 20, 16, 26, 18};
        break;
    }

    std::map<uint64_t, tg::ivec4> nuggets
        = {{0, {2, 17, 73, 41}},      {1, {186, 202, 61, 41}},   {2, {341, 13, 123, 41}},   {3, {360, 392, 40, 41}},
           {4, {200, 249, 40, 41}},   {5, {140, 208, 40, 41}},   {6, {25, 256, 43, 41}},    {7, {427, 135, 70, 41}},
           {8, {127, 467, 135, 41}},  {9, {129, 361, 59, 41}},   {10, {192, 369, 165, 41}}, {11, {34, 141, 149, 41}},
           {12, {339, 233, 125, 41}}, {13, {277, 173, 120, 41}}, {14, {245, 75, 135, 41}},  {15, {284, 456, 157, 41}},
           {16, {392, 337, 20, 41}},  {17, {24, 199, 20, 41}},   {18, {3, 310, 136, 41}},   {19, {358, 287, 111, 41}},
           {20, {91, 81, 72, 41}},    {21, {72, 417, 201, 41}},  {22, {150, 297, 118, 41}}, {23, {459, 405, 30, 41}}};

    struct Nugget
    {
        tg::vec2 a;
        tg::vec2 b;
        tg::vec2 c;
        tg::vec2 d;
    };

    auto ts = 0.0f;
    auto s = 512.0f;
    std::vector<Nugget> vd;
    for (auto i = 0u; i < chunks.size(); ++i)
    {
        auto j = i + hash % 3 + 7;
        auto ni = chunks[i] - (j * j * j + i) % 91;
        auto const& n = nuggets[ni];

        ts += 15 * (ts > 0);
        vd.push_back({{ts, 0}, {n.x / s, n.y / s}, {n.z / s, n.w / s}, {(float)n.z, (float)n.w}});
        ts += n.z;
    }
    for (auto& v : vd)
    {
        v.a /= ts;
        v.d /= ts;
    }
    auto abt = ArrayBuffer::create();
    abt->defineAttribute(&Nugget::a, "A", AttributeMode::Float, 1);
    abt->defineAttribute(&Nugget::b, "B", AttributeMode::Float, 1);
    abt->defineAttribute(&Nugget::c, "C", AttributeMode::Float, 1);
    abt->defineAttribute(&Nugget::d, "D", AttributeMode::Float, 1);
    abt->bind().setData(vd);
    mMeshTask = geometry::Quad<>().generate();
    mMeshTask->bind().attach(abt);
}

// timing
static timing::CpuTimer sTimer;
static double timeInSeconds()
{
    return sTimer.elapsedSecondsD();
}
