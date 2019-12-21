#pragma once

#include <vector>

#include <typed-geometry/tg-lean.hh>

#include <glow/objects/ArrayBufferAttribute.hh>

///
/// The vertex type used for all terrain meshes
///
/// Your job is to:
///     - create/modify the vertex type to fit your needs
///
///     - Advanced: Pack everything you need in 16 byte or less (e.g. a single (tg::)pos4)
///
/// Notes:
///     - do not upload tangents or texture coords!
///       (those can -and have to- be computed in the shader)
///
/// Grading:
///     3p if <= 16 byte per vertex
///     2p if <= 28 byte per vertex
///     1p otherwise
///
///     (~4.9 byte is theoretical minimum but <= 16 is fine for full points)
///
/// ============= STUDENT CODE BEGIN =============

struct TerrainVertex
{
	tg::ipos3 root;

	int axis;

	float ao;

	int osA;
	int osB;
	int osC;
	int osD;

	int which;

	TerrainVertex(tg::ipos3 &root, int axis, float ao, int osA, int osB, int osC, int osD, int which)
	{
		this->root = root;

		this->axis = axis;

		this->ao = ao;

		this->osA = osA;
		this->osB = osB;
		this->osC = osC;
		this->osD = osD;

		this->which = which;
	}

	static std::vector<glow::ArrayBufferAttribute> attributes()
	{
		return {
			{
				&TerrainVertex::root,
				"root"
			},
			{
				&TerrainVertex::axis,
				"axis"
			},
			{
				&TerrainVertex::ao,
				"ao"
			},
			{
				&TerrainVertex::osA,
				"osA"
			},
			{
				&TerrainVertex::osB,
				"osB"
			},
			{
				&TerrainVertex::osC,
				"osC"
			},
			{
				&TerrainVertex::osD,
				"osD"
			},
			{
				&TerrainVertex::which,
				"which"
			}
		};
	}
};

/// ============= STUDENT CODE END =============
