out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vTangent;
out vec2 vTexCoord;
out vec3 vViewPos;
out vec4 vScreenPos;

out float vAO;

uniform mat4 uProj;
uniform mat4 uView;
uniform float uTextureScale;

///
/// Vertex shader code for all terrain materials
///
/// Your job is to:
///     - define vertex attribute(s) for the data you need
///     - compute tangent and texture coordinate
///     - write to all defined out-variables (vWorldPos, ...)
///
///     - Advanced: unpack position, ao value and normal from your optimized vertex format
///
/// Notes:
///     - don't forget to divide texture coords by uTextureScale
///
/// ============= STUDENT CODE BEGIN =============

in ivec3 root;

in int direction;
in int axis;

in float ao;

in int osA;
in int osB;
in int osC;
in int osD;

in int which;

ivec3 index_to_offset(int index)
{
	int x = index / 4;
	int y = (index % 4) / 2;
	int z = (index % 2);

	return ivec3(x, y, z);
}

ivec2 getTexCoord(ivec3 position)
{
	if (axis == 0) {
		return position.yz;
	} else if (axis == 1) {
		return position.zx;
	} else {
		return position.xy;
	}
}

void main()
{
	ivec3 offsetA = index_to_offset(osA);
	ivec3 offsetB = index_to_offset(osB);
	ivec3 offsetC = index_to_offset(osC);
	ivec3 offsetD = index_to_offset(osD);

	ivec3 positionA = offsetA + root;
	ivec3 positionB = offsetB + root;
	ivec3 positionC = offsetC + root;
	ivec3 positionD = offsetD + root;

	ivec2 tcA = getTexCoord(positionA);
	ivec2 tcB = getTexCoord(positionB);
	ivec2 tcC = getTexCoord(positionC);
	ivec2 tcD = getTexCoord(positionD);

	ivec3 qB = offsetB - offsetA;
	ivec3 qC = offsetC - offsetA;

	vAO = ao;

	vNormal = ivec3(axis == 0, axis == 1, axis == 2);

	vTangent = (qC * (tcB.y - tcA.y) - qB * (tcC.y - tcA.y)) / (tcC.x - tcA.x) * (tcB.y - tcA.y) - (tcB.x - tcA.x) * (tcC.y - tcA.y);

	vTangent = normalize(vTangent);

	if (which == 0) {

		vTexCoord = tcA / uTextureScale;
		vWorldPos = positionA;

	} else if (which == 1) {

		vTexCoord = tcB / uTextureScale;
		vWorldPos = positionB;

	} else if (which == 2) {

		vTexCoord = tcC / uTextureScale;
		vWorldPos = positionC;

	} else {

		vTexCoord = tcD / uTextureScale;
		vWorldPos = positionD;
	}
	vViewPos = vec3(uView * vec4(vWorldPos, 1.0));

	vScreenPos = uProj * vec4(vViewPos, 1.0);

	gl_Position = vScreenPos;
}


/// ============= STUDENT CODE END =============
