#include "Cloth.hh"

#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>

using namespace glow;

Cloth::Cloth(int res, float size, float weight) : res(res)
{
    TG_ASSERT(res >= 2);

    auto numberOfParticles = res * res;
    particles.resize(numberOfParticles);

    // create particles
    tg::vec3 offset = size * 0.5f * tg::vec3(1, 0, 1);
    for (int y = 0; y < res; ++y)
        for (int x = 0; x < res; ++x)
        {
            tg::vec3 pos = {size * float(x) / res, 0, size * float(y) / res};
            pos -= offset;
            particles[y * res + x] = Particle(pos, weight / numberOfParticles);
        }

    // create springs
    for (int y = 0; y < res; ++y)
        for (int x = 0; x < res; ++x)
        {
            // Structural
            if (x > 0)
                addSpring(particle(x, y), particle(x - 1, y));
            if (y > 0)
                addSpring(particle(x, y), particle(x, y - 1));

            // Shearing
            if (x > 0 && y > 0)
            {
                addSpring(particle(x, y), particle(x - 1, y - 1));
                addSpring(particle(x - 1, y), particle(x, y - 1));
            }

            // Bending
            if (x > 1)
                addSpring(particle(x, y), particle(x - 2, y));
            if (y > 1)
                addSpring(particle(x, y), particle(x, y - 2));
            if (x > 1 && y > 1)
            {
                addSpring(particle(x, y), particle(x - 2, y - 2));
                addSpring(particle(x - 2, y), particle(x, y - 2));
            }
        }

    // set to proper initial motion state
    resetMotionState();
}

void Cloth::resetMotionState()
{
    for (auto& p : particles)
    {
        p.fixed = false;
        p.position = p.initialPosition;
        p.velocity = {0, 0, 0};
    }

    // fix proper particles
    for (int y = 0; y < res; ++y)
        for (int x = 0; x < res; ++x)
            switch (fixedParticles)
            {
            case FixedParticles::FourSides:
                if (x == 0 || y == 0 || x == res - 1 || y == res - 1)
                    particle(x, y)->fixed = true;
                break;
            case FixedParticles::TwoSides:
                if (x == 0 || x == res - 1)
                    particle(x, y)->fixed = true;
                break;
            case FixedParticles::OneSide:
                if (x == 0)
                    particle(x, y)->fixed = true;
                break;
            case FixedParticles::FourCorners:
                if ((x == 0 || x == res - 1) && ((x + y) % (res - 1) == 0))
                    particle(x, y)->fixed = true;
                break;
            }
}

tg::vec3 Cloth::color(int x, int y, int dx, int dy)
{
    // Checker board colors
    tg::vec3 col1 = {0.00f, 0.33f, 0.66f};
    tg::vec3 col2 = {0.66f, 0.00f, 0.33f};

    switch (coloring)
    {
    case Coloring::Checker:
        return (x - dx + y - dy) % 2 ? col1 : col2;

    case Coloring::Stress:
        // Color dependent on strain
        return tg::mix(tg::vec3(0, 1, 0), tg::vec3(1, 0, 0), tg::min(particle(x, y)->stress * 0.03f, 1.0f));

    case Coloring::Unicolored:
    default:
        return {0.00f, 0.33f, 0.66f};
    }
}

void Cloth::resetForces()
{
    // Reset all forces
    for (auto& p : particles)
    {
        p.stress = 0;
        p.accumulatedForces = {0, 0, 0};
    }
}

void Cloth::updateForces()
{
    /// Task 2.d
    /// Gravity force is applied to all particles
    ///
    /// Your job is to:
    ///     - add gravity force to the each particle's accumulated force
    ///
    /// Notes:
    ///     - the currently set gravity is stored in the variable gravity.
    ///     - gravity operates "downwards" (negative y-direction)
    ///     - mass is stored in the particle
    ///
    /// ============= STUDENT CODE BEGIN =============

    /// ============= STUDENT CODE END =============

    // Apply Hooke's Law
    for (auto& s : springs)
    {
        TG_ASSERT(s.p0 != s.p1);

        /// Task 2.b
        /// Using Hooke's law, forces act on connected particles.
        ///
        /// Your job is to:
        ///     - compare current distance and rest distance of the two particles.
        ///     - apply appropriate force to both particles
        ///
        /// Notes:
        ///     - the (stiffness) spring constant is stored in springK
        ///     - forces on the two particles counteract (i.e. contrary signs)
        ///     - if you want, you can set the particle stress variables to allow
        ///       for stress rendering (optional)
        ///
        /// ============= STUDENT CODE BEGIN =============

        /// ============= STUDENT CODE END =============
    }

    // Dragging
    if (draggedParticle && !draggedParticle->fixed)
    {
        draggedParticle->position = draggedPosition;
        draggedParticle->velocity = {0, 0, 0};
        draggedParticle->accumulatedForces = {0, 0, 0};
    }

    // Damping
    for (auto& p : particles)
    {
        /// Task 2.c
        /// For the system to come to rest, damping is applied.
        ///
        /// your job is to:
        ///     - damp the particle forces
        ///
        /// notes:
        ///     - the damping factor is stored in dampingD
        ///     - the amount of damping also depends on particle velocity
        ///
        /// ============= STUDENT CODE BEGIN =============

        /// ============= STUDENT CODE END =============
    }
}

void Cloth::updateMotion(float elapsedSeconds)
{
    // Motion equations
    for (auto& p : particles)
    {
        if (p.fixed)
            continue;

        /// Task 2.a
        /// Particles move according to forces that act upon them.
        ///
        /// Your job is to:
        ///     - update the particle velocity (p.velocity)
        ///     - update the particle position (p.position)
        ///
        /// Notes:
        ///     - Using the updated velocity (v_i+1) to compute the new position is the "semi-implicit Euler"
        ///       which is more stable than the standard explicit euler that uses v_i
        ///     - particle force is stored in p.accumulatedForces
        ///     - particle mass is stored in p.mass
        ///
        /// ============= STUDENT CODE BEGIN =============

        /// ============= STUDENT CODE END =============
    }
}

void Cloth::addSphereCollision(tg::pos3 center, float radius)
{
    for (auto& p : particles)
    {
        /// Task 3
        /// Check for particle-sphere collision
        ///
        /// Your job is to:
        ///     - detect whether a particle collides with the sphere
        ///     - project the particle position to the sphere surface
        ///     - project the particle velocity to the tangent plane
        ///
        /// Notes:
        ///     - The tangent plane is unambiguously defined by the normal
        ///       on the sphere surface
        ///     - After projection, the velocity vector lies in that plane
        ///
        /// ============= STUDENT CODE BEGIN =============

        /// ============= STUDENT CODE END =============
    }
}

bool Cloth::drag(tg::pos3 const& pos, tg::vec3 const& dir, const tg::vec3& camDir)
{
    // if new: find particle closest to ray
    if (draggedParticle == nullptr)
    {
        // Do not drag any particle if too far away
        const float max_part_dist = 0.3f;

        auto bestDist = tg::max<float>();
        auto bestDistToCam = tg::max<float>();
        Particle* bestParticle = nullptr;

        for (auto& p : particles)
        {
            auto toPart = p.position - pos;
            auto closestPointOnRay = pos + dot(toPart, dir) * dir;
            auto dist = distance(p.position, closestPointOnRay);

            if (dist > max_part_dist || p.fixed)
                continue;

            if (dist < bestDist || length(toPart) < bestDistToCam - max_part_dist)
            {
                bestDist = dist;
                bestParticle = &p;
                bestDistToCam = length(toPart);
            }
        }

        // Could not find any particle close to ray
        if (!bestParticle)
            return false;

        draggedParticle = bestParticle;
    }

    // set drag position
    {
        auto pPos = draggedParticle->position;
        auto toCam = pos - pPos;
        auto p = pos - dir * dot(toCam, camDir) / dot(dir, camDir);
        draggedPosition = p;
    }
    return true;
}

SharedVertexArray Cloth::createVAO()
{
    std::vector<Vertex> vertices;
    vertices.reserve((3 * 2) * (res - 1) * (res - 1));

    /// Task 1.b
    ///
    ///          0,-1
    ///         / | \
    ///       /   |   \
    ///     /     |     \
    /// -1,0 --- p_c --- 1,0
    ///     \     |     /
    ///       \   |   /
    ///         \ | /
    ///          0,1
    ///
    /// Your job is to:
    ///     - calculate a smoothed normal for every particle
    ///     - the normal of p_c is the average of all adjacent triangles' normals (see ASCII pic)
    ///     - skip triangles that don't exist (border)
    ///
    /// Notes:
    ///     - particles are stored in a 2D grid with x and y from 0 to res-1 (inclusive)
    ///     - you can query the position of a particle via `pos(x, y)`
    ///     - you probably want to store them similar to the default code
    ///     - you are allowed to weight the normals by triangle area if that simplifies your code
    ///     - these triangles are only conceptual (the actual geometry is a bit different)
    ///     - notes of Task 1.a also apply
    ///
    /// ============= STUDENT CODE BEGIN =============

    std::vector<tg::vec3> normals(res * res);
    for (int y = 0; y < res; ++y)
        for (int x = 0; x < res; ++x)
        {
            tg::vec3 n = {0, 1, 0}; // FIXME

            normals[y * res + x] = n;
        }

    /// ============= STUDENT CODE END =============
    // No shared vertices to allow for distinct colors.
    for (int y = 0; y < res - 1; ++y)
        for (int x = 0; x < res - 1; ++x)
        {
            auto c00 = color(x + 0, y + 0, 0, 0);
            auto c01 = color(x + 0, y + 1, 0, 1);
            auto c10 = color(x + 1, y + 0, 1, 0);
            auto c11 = color(x + 1, y + 1, 1, 1);

            auto p00 = pos(x + 0, y + 0);
            auto p01 = pos(x + 0, y + 1);
            auto p10 = pos(x + 1, y + 0);
            auto p11 = pos(x + 1, y + 1);

            if (smoothShading)
            {
                /// Task 1.c
                ///
                /// p00 -- p10
                ///  | \  / |
                ///  |  pc  |
                ///  | /  \ |
                /// p01 -- p11
                ///
                /// Your job is to:
                ///     - generate smooth-shaded geometry for the particles
                ///     - each quad should generate four triangles
                ///     - the center position is the average of the corners
                ///     - the center color is the average of the corner colors
                ///     - normals should be taken from Task 1.b
                ///     - normal for the center vertex is the average corner normal
                ///
                /// Notes:
                ///     - see notes for Task 1.a
                ///
                /// ============= STUDENT CODE BEGIN =============

                /// ============= STUDENT CODE END =============
            }
            else
            {
                /// Task 1.a
                ///
                /// p00 --- p10
                ///  |    /  |
                ///  |   /   |
                ///  |  /    |
                /// p01 --- p11
                ///
                /// Your job is to:
                ///     - generate flat-shaded geometry for the particles
                ///     - each quad should generate two triangles
                ///
                /// Notes:
                ///     - OpenGL default order is counter-clockwise
                ///     - you will notice inconsistent normals in the lighting
                ///     - normals must be normalized
                ///     - positions and colors are already stored in p00..p11 and c00..c11
                ///     - add vertices via `vertices.push_back({<position>, <normal>, <color>});`
                ///
                /// ============= STUDENT CODE BEGIN =============

                /// ============= STUDENT CODE END =============
            }
        }

    if (clothAB == nullptr)
        clothAB = ArrayBuffer::create({{&Vertex::position, "aPosition"}, //
                                       {&Vertex::normal, "aNormal"},     //
                                       {&Vertex::color, "aColor"}});

    clothAB->bind().setData(vertices);

    if (clothVAO == nullptr)
        clothVAO = VertexArray::create(clothAB);

    return clothVAO;
}
