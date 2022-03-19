#include "dv_precompiled.h"
#include "dv_rasterizer.h"

/*
https://github.com/nlguillemot/vigilant-system
*/

namespace davinci
{

typedef int64_t ifloat_t;
typedef int64_t ifloat2_t[2];
typedef int64_t ifloat3_t[3];

#define IFLOAT_SHIFT (16)
#define IFLOAT_SCALE ((double)(1<<IFLOAT_SHIFT))

//-------------------------------------------------------------------------------------
inline ifloat_t ifloat_init(float a)
{
	return (ifloat_t)((double)a * IFLOAT_SCALE + 0.5);
}

//-------------------------------------------------------------------------------------
inline ifloat_t ifloat_init(int32_t a)
{
	return (ifloat_t)(a<< IFLOAT_SHIFT);
}

//-------------------------------------------------------------------------------------
inline float ifloat_get_float(ifloat_t a)
{
	return (float)(a / IFLOAT_SCALE);
}

//-------------------------------------------------------------------------------------
inline ifloat_t ifloat_get_int(ifloat_t a)
{
	return (ifloat_t)(a >> IFLOAT_SHIFT);
}

//-------------------------------------------------------------------------------------
inline void ifloat3_add(ifloat3_t& a, const ifloat3_t& b)
{
	a[0] += b[0]; a[1] += b[1]; a[2] += b[2];
}

//-------------------------------------------------------------------------------------
inline void ifloat3_sub(ifloat3_t& a, const ifloat3_t& b)
{
	a[0] -= b[0]; a[1] -= b[1]; a[2] -= b[2];
}

//-------------------------------------------------------------------------------------
inline void ifloat3_cpy(ifloat3_t& a, const ifloat3_t& b)
{
	a[0] = b[0]; a[1] = b[1]; a[2] = b[2];
}

//-------------------------------------------------------------------------------------
#define ifloat3_element(a) { a[0], a[1], a[2] }

//-------------------------------------------------------------------------------------
inline float _edge(const Vector3& a, const Vector3& b, const Vector3& p)
{
	return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

//-------------------------------------------------------------------------------------
struct DrawTriangleParam
{
	int32_t		widthInPixel;
	int32_t		heightInPixel;
	int32_t		widthInTiles;
	Vector3		verts[3];
	float		bbox_min_x;
	float		bbox_max_x;
	float		bbox_min_y;
	float		bbox_max_y;
	bool		tlBorder[3];
	float		area;
	ifloat3_t	edgesDX, edgesDY;

	Rasterizer::DebugParam	debug;
};

//-------------------------------------------------------------------------------------
enum { TILE_WIDTH_IN_PIXELS = 64, COARSE_BLOCK_WIDTH_IN_PIXELS = 16, FINE_BLOCK_WIDTH_IN_PIXELS = 4 };

//-------------------------------------------------------------------------------------
void _drawTriangle_Fine(int32_t tile_id, int32_t coarse_id, int32_t fine_id,
	const DrawTriangleParam& param, const ifloat3_t& edges0, uint32_t testEdgeMask, 
	Rasterizer::DrawTriangleCallback callback)
{
	ifloat3_t pixelEdges = ifloat3_element(edges0);

	const Vector3 v0(param.verts[0]), v1(param.verts[1]), v2(param.verts[2]);

	int32_t fine_start_x = (tile_id%param.widthInTiles) * TILE_WIDTH_IN_PIXELS + (coarse_id % 4)*COARSE_BLOCK_WIDTH_IN_PIXELS + (fine_id % 4)*FINE_BLOCK_WIDTH_IN_PIXELS;
	int32_t fine_start_y = (tile_id / param.widthInTiles) * TILE_WIDTH_IN_PIXELS + (coarse_id / 4) * COARSE_BLOCK_WIDTH_IN_PIXELS + (fine_id / 4)*FINE_BLOCK_WIDTH_IN_PIXELS;

	for (int32_t y_index=0, y= fine_start_y; y_index < 4; y_index++, y++) {
		ifloat3_t edgesRow = ifloat3_element(pixelEdges);

		for (int32_t x_index = 0, x=fine_start_x; x_index < 4; x_index++, x++) {

			bool rejected =
				((testEdgeMask & 1) && (edgesRow[0] < 0 || (!param.tlBorder[0] && edgesRow[0] == 0))) ||
				((testEdgeMask & 2) && (edgesRow[1] < 0 || (!param.tlBorder[1] && edgesRow[1] == 0))) ||
				((testEdgeMask & 4) && (edgesRow[2] < 0 || (!param.tlBorder[2] && edgesRow[2] == 0)));
			
			if (param.debug.debug && param.debug.tile_id == tile_id && param.debug.coarse_id == coarse_id && param.debug.fine_id == fine_id && param.debug.x == x && param.debug.y == y) {
				printf("\t\t\t--- [%d,%d : %d,%d] ---\n", x_index, y_index, x, y);

				printf("\t\t\tEdge%d: %lld - %d*%lld + %d*%lld=%lld\n", 
					param.debug.edge_id, 
					edges0[param.debug.edge_id], y_index, param.edgesDX[param.debug.edge_id], x_index, param.edgesDY[param.debug.edge_id], edgesRow[param.debug.edge_id]);
			}
			
			if (!rejected) {

				Vector3 pixel(x + 0.5f, y + 0.5f, 0.f);
				float w0 = _edge(v1, v2, pixel);
				float w1 = _edge(v2, v0, pixel);
				float w2 = _edge(v0, v1, pixel);

				Vector3 t = Vector3(w0 / param.area, w1 / param.area, w2 / param.area);

				float invZ = MathUtil::lerp3(v0.z, v1.z, v2.z, t);
				t *= Vector3(v0.z / invZ, v1.z / invZ, v2.z / invZ);
				callback(std::make_pair(x, y), t);
			}
			ifloat3_add(edgesRow, param.edgesDY);
		}
		ifloat3_sub(pixelEdges, param.edgesDX);
	}
}

//-------------------------------------------------------------------------------------
void _drawTriangle_Coarse(int32_t tile_id, int32_t coarse_id,
	DrawTriangleParam& param, const ifloat3_t& edges0, uint32_t testEdgeMask, 
	Rasterizer::DrawTriangleCallback callback)
{
	const ifloat3_t blockEdgesDX = { param.edgesDX[0] * FINE_BLOCK_WIDTH_IN_PIXELS, param.edgesDX[1] * FINE_BLOCK_WIDTH_IN_PIXELS, param.edgesDX[2] * FINE_BLOCK_WIDTH_IN_PIXELS };
	const ifloat3_t blockEdgesDY = { param.edgesDY[0] * FINE_BLOCK_WIDTH_IN_PIXELS, param.edgesDY[1] * FINE_BLOCK_WIDTH_IN_PIXELS, param.edgesDY[2] * FINE_BLOCK_WIDTH_IN_PIXELS };

	ifloat3_t blockReject = ifloat3_element(edges0);
	ifloat3_t blockAccept = ifloat3_element(edges0);
	ifloat3_t blockEdges = ifloat3_element(edges0);

	for (size_t v = 0; v < 3; v++) {
		if (testEdgeMask & (1 << v)) {	
			if (blockEdgesDX[v] > 0) blockAccept[v] -= blockEdgesDX[v];
			if (blockEdgesDX[v] < 0) blockReject[v] -= blockEdgesDX[v];
			if (blockEdgesDY[v] < 0) blockAccept[v] += blockEdgesDY[v];
			if (blockEdgesDY[v] > 0) blockReject[v] += blockEdgesDY[v];
		}
	}

	int32_t block_start_x = (tile_id%param.widthInTiles) * TILE_WIDTH_IN_PIXELS + (coarse_id % 4)*COARSE_BLOCK_WIDTH_IN_PIXELS;
	int32_t block_start_y = (tile_id / param.widthInTiles) * TILE_WIDTH_IN_PIXELS + (coarse_id / 4) * COARSE_BLOCK_WIDTH_IN_PIXELS;

	for (int32_t y_index = 0; y_index < 4; y_index++) {
		if (block_start_y + y_index*FINE_BLOCK_WIDTH_IN_PIXELS > param.bbox_max_y) break;
		if (block_start_y + (y_index + 1)*FINE_BLOCK_WIDTH_IN_PIXELS >= param.bbox_min_y) {

			ifloat3_t edgesRow = ifloat3_element(blockEdges);
			ifloat3_t edgesRowReject = { 0 }, edgesRowAccept = { 0 };

			for (size_t v = 0; v < 3; v++) {
				if (testEdgeMask & (1 << v)) {
					edgesRowReject[v] = blockReject[v];
					edgesRowAccept[v] = blockAccept[v];
				}
			}
			for (int32_t x_index = 0; x_index < 4; x_index++) {
				if (block_start_x + x_index*FINE_BLOCK_WIDTH_IN_PIXELS > param.bbox_max_x) break;
				if (block_start_x + (x_index + 1)*FINE_BLOCK_WIDTH_IN_PIXELS >= param.bbox_min_x) {

					bool rejected = 
						((testEdgeMask & 1) && (edgesRowReject[0] < 0 || (!param.tlBorder[0] && edgesRowReject[0] == 0))) ||
						((testEdgeMask & 2) && (edgesRowReject[1] < 0 || (!param.tlBorder[1] && edgesRowReject[1] == 0))) ||
						((testEdgeMask & 4) && (edgesRowReject[2] < 0 || (!param.tlBorder[2] && edgesRowReject[2] == 0)));

					if (!rejected) {
						uint32_t newTestEdgeMask = testEdgeMask;
						for (size_t v = 0; v < 3; v++) {
							if (testEdgeMask & (1 << v)) {
								if (edgesRowAccept[v] > 0 || (edgesRowAccept[v] == 0 && param.tlBorder[v])) {
									newTestEdgeMask &= ~(1 << v);
								}
							}
						}

						if (param.debug.debug && param.debug.tile_id==tile_id&& param.debug.coarse_id==coarse_id && param.debug.fine_id == x_index+y_index*4) {
							printf("\t\t--- fine {%d, %d, %d} ---\n", tile_id, coarse_id, y_index * 4 + x_index);
							printf("\t\t\tEdgeMask\t=%d\n"
								"\t\t\tEdge\t\t=<%f,%f,%f>\n"
								"\t\t\tRej0\t\t=<%.1f,%.1f,%.1f>\n"
								"\t\t\tAcc0\t\t=<%.1f,%.1f,%.1f>\n",
								newTestEdgeMask,
								ifloat_get_float(edgesRow[0]), ifloat_get_float(edgesRow[1]), ifloat_get_float(edgesRow[2]),
								ifloat_get_float(edgesRowReject[0]), ifloat_get_float(edgesRowReject[1]), ifloat_get_float(edgesRowReject[2]),
								ifloat_get_float(edgesRowAccept[0]), ifloat_get_float(edgesRowAccept[1]), ifloat_get_float(edgesRowAccept[2])
							);	

							printf("\t\t\tEdge%d: %lld-%d*%lld+%d*%lld=%lld\n", 
								param.debug.edge_id,
								edges0[param.debug.edge_id], y_index, blockEdgesDX[param.debug.edge_id], x_index, blockEdgesDY[param.debug.edge_id], edgesRow[param.debug.edge_id]);
						}
						_drawTriangle_Fine(tile_id, coarse_id, y_index * 4 + x_index, param, edgesRow, newTestEdgeMask, callback);
					}
				}
				ifloat3_add(edgesRow, blockEdgesDY);

				for (size_t v = 0; v < 3; v++) {
					if (testEdgeMask & (1 << v)) {

						edgesRowReject[v] += blockEdgesDY[v];
						edgesRowAccept[v] += blockEdgesDY[v];
					}
				}
			}
		}
		ifloat3_sub(blockEdges, blockEdgesDX);

		for (size_t v = 0; v < 3; v++) {
			if (testEdgeMask & (1 << v)) {
				blockReject[v] -= blockEdgesDX[v];
				blockAccept[v] -= blockEdgesDX[v];
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void _drawTriangle_Title(int32_t tile_id,
	DrawTriangleParam& param, const ifloat3_t& edges0, uint32_t testEdgeMask, 
	Rasterizer::DrawTriangleCallback callback)
{
	const ifloat3_t blockEdgesDX = { param.edgesDX[0] * COARSE_BLOCK_WIDTH_IN_PIXELS, param.edgesDX[1] * COARSE_BLOCK_WIDTH_IN_PIXELS, param.edgesDX[2] * COARSE_BLOCK_WIDTH_IN_PIXELS };
	const ifloat3_t blockEdgesDY = { param.edgesDY[0] * COARSE_BLOCK_WIDTH_IN_PIXELS, param.edgesDY[1] * COARSE_BLOCK_WIDTH_IN_PIXELS, param.edgesDY[2] * COARSE_BLOCK_WIDTH_IN_PIXELS };

	ifloat3_t blockReject = ifloat3_element(edges0);
	ifloat3_t blockAccept = ifloat3_element(edges0);
	ifloat3_t blockEdges = ifloat3_element(edges0);

	for (size_t v = 0; v < 3; v++) {
		if (testEdgeMask & (1 << v)) {
			if (blockEdgesDX[v] > 0) blockAccept[v] -= blockEdgesDX[v];
			if (blockEdgesDX[v] < 0) blockReject[v] -= blockEdgesDX[v];
			if (blockEdgesDY[v] < 0) blockAccept[v] += blockEdgesDY[v];
			if (blockEdgesDY[v] > 0) blockReject[v] += blockEdgesDY[v];
		}
	}

	int32_t block_start_x = (tile_id%param.widthInTiles) * TILE_WIDTH_IN_PIXELS;
	int32_t block_start_y = (tile_id / param.widthInTiles) * TILE_WIDTH_IN_PIXELS;

	for (int32_t y_index = 0; y_index < 4; y_index++) {
		if (block_start_y + y_index*COARSE_BLOCK_WIDTH_IN_PIXELS > param.bbox_max_y) break;
		if (block_start_y + (y_index + 1)*COARSE_BLOCK_WIDTH_IN_PIXELS >= param.bbox_min_y) {

			ifloat3_t edges = ifloat3_element(blockEdges);
			ifloat3_t edgesRowReject = { 0 }, edgesRowAccept = { 0 };

			for (size_t v = 0; v < 3; v++) {
				if (testEdgeMask & (1 << v)) {
					edgesRowReject[v] = blockReject[v];
					edgesRowAccept[v] = blockAccept[v];
				}
			}

			for (int32_t x_index = 0; x_index < 4; x_index++) {
				if (block_start_x + x_index*COARSE_BLOCK_WIDTH_IN_PIXELS > param.bbox_max_x) break;
				if (block_start_x + (x_index + 1)*COARSE_BLOCK_WIDTH_IN_PIXELS >= param.bbox_min_x) {

					bool rejected = 
						((testEdgeMask & 1) && (edgesRowReject[0] < 0 || (!param.tlBorder[0] && edgesRowReject[0] == 0))) ||
						((testEdgeMask & 2) && (edgesRowReject[1] < 0 || (!param.tlBorder[1] && edgesRowReject[1] == 0))) ||
						((testEdgeMask & 4) && (edgesRowReject[2] < 0 || (!param.tlBorder[2] && edgesRowReject[2] == 0)));

					if (!rejected) {

						uint32_t newTestEdgeMask = testEdgeMask;
						for (size_t v = 0; v < 3; v++) {
							if (testEdgeMask & (1 << v)) {
								if (edgesRowAccept[v] > 0 || (edgesRowAccept[v] == 0 && param.tlBorder[v])) {
									newTestEdgeMask &= ~(1 << v);
								}
							}
						}
						if (param.debug.debug && param.debug.tile_id==tile_id && param.debug.coarse_id==x_index+y_index*4) {
							printf("\t---- coarse{%d, %d} ----\n", tile_id, y_index * 4 + x_index);
							printf("\t\tEdgeMask\t=%d\n"
								"\t\tEdge0\t\t=<%f,%f,%f>\n"
								"\t\tRej0\t\t=<%.1f,%.1f,%.1f>\n"
								"\t\tAcc0\t\t=<%.1f,%.1f,%.1f>\n",
								newTestEdgeMask,
								ifloat_get_float(edges[0]), ifloat_get_float(edges[1]), ifloat_get_float(edges[2]),
								ifloat_get_float(edgesRowReject[0]), ifloat_get_float(edgesRowReject[1]), ifloat_get_float(edgesRowReject[2]),
								ifloat_get_float(edgesRowAccept[0]), ifloat_get_float(edgesRowAccept[1]), ifloat_get_float(edgesRowAccept[2]));

							printf("\t\t*Edge%d: %lld-%d*%lld+%d*%lld=%lld\n",
								param.debug.edge_id,
								edges0[param.debug.edge_id], y_index, blockEdgesDX[param.debug.edge_id], x_index, blockEdgesDY[param.debug.edge_id], edges[param.debug.edge_id]);
						}
						_drawTriangle_Coarse(tile_id, y_index * 4 + x_index, param, edges, newTestEdgeMask, callback);
					}
				}

				ifloat3_add(edges, blockEdgesDY);

				for (size_t v = 0; v < 3; v++) {
					if (testEdgeMask & (1 << v)) {
						edgesRowReject[v] += blockEdgesDY[v];
						edgesRowAccept[v] += blockEdgesDY[v];
					}
				}
			}
		}

		ifloat3_sub(blockEdges, blockEdgesDX);
		for (size_t v = 0; v < 3; v++) {
			if (testEdgeMask & (1 << v)) {
				blockReject[v] -= blockEdgesDX[v];
				blockAccept[v] -= blockEdgesDX[v];
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Rasterizer::drawTriangleLarrabee(int32_t canvasWidth, int32_t canvasHeight, const Vector3& v0, const Vector3& v1, const Vector3& v2, DrawTriangleCallback callback, const DebugParam* debug)
{
	assert(canvasWidth >= TILE_WIDTH_IN_PIXELS && canvasHeight >= TILE_WIDTH_IN_PIXELS);
	assert(canvasWidth%TILE_WIDTH_IN_PIXELS == 0 && canvasHeight%canvasHeight == 0);

	DrawTriangleParam param;
	if (debug != nullptr) {
		param.debug = *debug;
	}
	else {
		param.debug.debug = false;
	}
	param.widthInPixel = canvasWidth;
	param.heightInPixel = canvasHeight;
	param.widthInTiles = canvasWidth / TILE_WIDTH_IN_PIXELS;

	//back cull
	Vector3 vnormal = (v2 - v1).crossProduct(v0 - v2);
	if (vnormal.z > 0) return;

	//to screen space(integer)
	const Vector3 verts[3] = { v0, v1, v2 };
	param.verts[0] = verts[0];
	param.verts[1] = verts[1];
	param.verts[2] = verts[2];
	param.area = _edge(v0, v1, v2);

	ifloat2_t vertices[3];
	for (size_t i = 0; i<3; i++) for (size_t j = 0; j<2; j++)
		vertices[i][j] = ifloat_init(verts[i][j]);

	//get window coordinates bounding box
	param.bbox_min_x = MathUtil::min3(v0.x, v1.x, v2.x);
	param.bbox_max_x = MathUtil::max3(v0.x, v1.x, v2.x);
	param.bbox_min_y = MathUtil::min3(v0.y, v1.y, v2.y);
	param.bbox_max_y = MathUtil::max3(v0.y, v1.y, v2.y);

	//canvas size
	ifloat_t iCanvasWidth = ifloat_init(canvasWidth);
	ifloat_t iCanvasHeight = ifloat_init(canvasHeight);

	// clip triangles that are fully outside the scissor rect (scissor rect = whole window)
	if (param.bbox_max_x < 0 || param.bbox_max_y < 0 || param.bbox_min_x >= canvasWidth || param.bbox_min_y >= canvasHeight) {
		return;
	}
	if (param.bbox_min_x < 0) param.bbox_min_x = 0;
	if (param.bbox_min_y < 0) param.bbox_min_y = 0;
	if (param.bbox_max_x > canvasWidth - 1.f) param.bbox_max_x = canvasWidth - 1.f;
	if (param.bbox_max_y > canvasHeight - 1.f) param.bbox_max_y = canvasHeight - 1.f;

	// tile range
	int32_t first_tile_x = (int32_t)(param.bbox_min_x / TILE_WIDTH_IN_PIXELS);
	int32_t first_tile_y = (int32_t)(param.bbox_min_y / TILE_WIDTH_IN_PIXELS);
	int32_t last_tile_x = (int32_t)(param.bbox_max_x / TILE_WIDTH_IN_PIXELS);
	int32_t last_tile_y = (int32_t)(param.bbox_max_y / TILE_WIDTH_IN_PIXELS);

	// evaluate edge equation at the top left tile
	ifloat_t firstTileX = first_tile_x * ifloat_init(TILE_WIDTH_IN_PIXELS);
	ifloat_t firstTileY = first_tile_y * ifloat_init(TILE_WIDTH_IN_PIXELS);

	ifloat3_t edges0;
	ifloat3_t tileEdgesDX, tileEdgesDY, edgesReject, edgesAccept;
	const ifloat_t kZeroPointFive = ifloat_init(0.5f);

	for (size_t v = 0; v < 3; v++)
	{
		size_t v_end = (v + 1) % 3;
		param.edgesDX[v] = vertices[v_end][0] - vertices[v][0];
		param.edgesDY[v] = vertices[v_end][1] - vertices[v][1];

		tileEdgesDX[v] = param.edgesDX[v] * TILE_WIDTH_IN_PIXELS;
		tileEdgesDY[v] = param.edgesDY[v] * TILE_WIDTH_IN_PIXELS;

		edges0[v] = (firstTileX + kZeroPointFive - vertices[v][0]) * param.edgesDY[v] - (firstTileY + kZeroPointFive - vertices[v][1])*param.edgesDX[v];
		edges0[v] = ifloat_get_int(edges0[v]); //after multi

		// Top-left rule: shift top-left edges ever so slightly outward to make the top-left edges be the tie-breakers when rasterizing adjacent triangles
		if ((vertices[v_end][1] == vertices[v][1] && vertices[v_end][0] > vertices[v][0]) || vertices[v_end][1] > vertices[v][1]) param.tlBorder[v] = true;
		else param.tlBorder[v] = false;

		edgesReject[v] = edgesAccept[v] = edges0[v];
		if (tileEdgesDX[v] > 0) edgesAccept[v] -= tileEdgesDX[v];
		if (tileEdgesDX[v] < 0) edgesReject[v] -= tileEdgesDX[v];
		if (tileEdgesDY[v] < 0) edgesAccept[v] += tileEdgesDY[v];
		if (tileEdgesDY[v] > 0) edgesReject[v] += tileEdgesDY[v];
	}

	if (param.debug.debug) {
		printf("================================================================\n");
		printf("v0\t\t=%f,%f,%f\n", verts[0].x, verts[0].y, verts[0].z);
		printf("v1\t\t=%f,%f,%f\n", verts[1].x, verts[1].y, verts[1].z);
		printf("v2\t\t=%f,%f,%f\n", verts[2].x, verts[2].y, verts[2].z);

		printf("*v0\t\t=%lld,%lld\n", vertices[0][0], vertices[0][1]);
		printf("*v1\t\t=%lld,%lld\n", vertices[1][0], vertices[1][1]);
		printf("*v2\t\t=%lld,%lld\n", vertices[2][0], vertices[2][1]);

		printf("tile_x\t\t={%d, %d}\ntile_y\t\t={%d, %d}\n", first_tile_x, last_tile_x, first_tile_y, last_tile_y);

		printf("*dx\t\t=<%lld,%lld,%lld>\n*dy\t\t=<%lld,%lld,%lld>\n",
			param.edgesDX[0], param.edgesDX[1], param.edgesDX[2],
			param.edgesDY[0], param.edgesDY[1], param.edgesDY[2]);

		printf("*Edge%d: (%lld+0x%llx-%lld)*%lld - (%lld + 0x%llx-%lld)*%lld=%lld\n", param.debug.edge_id,
			firstTileX, kZeroPointFive, vertices[param.debug.edge_id][0], param.edgesDY[param.debug.edge_id],
			firstTileY, kZeroPointFive, vertices[param.debug.edge_id][1], param.edgesDX[param.debug.edge_id],
			edges0[param.debug.edge_id]);
	}

	ifloat3_t rowEdges = ifloat3_element(edges0);

	int32_t tile_row_start = first_tile_y * param.widthInTiles + first_tile_x;
	for (int32_t tile_y = first_tile_y; tile_y <= last_tile_y; tile_y++)
	{
		ifloat3_t edges = ifloat3_element(rowEdges);
		ifloat3_t tileEdgesReject = ifloat3_element(edgesReject);
		ifloat3_t tileEdgesAccept = ifloat3_element(edgesAccept);

		int32_t tile_i = tile_row_start;
		for (int32_t tile_x = first_tile_x; tile_x <= last_tile_x; tile_x++) {

			bool rejected =
				((tileEdgesReject[0] < 0 || (!param.tlBorder[0] && tileEdgesReject[0] == 0))) ||
				((tileEdgesReject[1] < 0 || (!param.tlBorder[1] && tileEdgesReject[1] == 0))) ||
				((tileEdgesReject[2] < 0 || (!param.tlBorder[2] && tileEdgesReject[2] == 0)));

			if (!rejected) {
				//draw title
				uint32_t testEdgeMask = 0;
				for (size_t v = 0; v < 3; v++) {
					if (tileEdgesAccept[v] < 0 || (tileEdgesAccept[v] == 0 && !param.tlBorder[v])) {
						testEdgeMask += (1 << v);
					}
				}

				if (param.debug.debug && param.debug.tile_id == tile_x + tile_y*param.widthInTiles) {
					printf("---- tile{%d} ---\n", tile_i);
					printf("\tEdgeMask\t=%d\n\t*Edge0\t\t=<%.1f,%.1f,%.1f>\n", testEdgeMask, 
						ifloat_get_float(edges[0]), ifloat_get_float(edges[1]), ifloat_get_float(edges[2]));

					printf("\t*Edge%d: %lld-%d*%lld+%d*%lld=%lld\n",
						param.debug.edge_id,
						edges0[param.debug.edge_id], (tile_y- first_tile_y), tileEdgesDX[param.debug.edge_id], (tile_x- first_tile_x), tileEdgesDY[param.debug.edge_id], edges[param.debug.edge_id]);
				}

				_drawTriangle_Title(tile_i, param, edges, testEdgeMask, callback);
			}

			//x setp
			ifloat3_add(edges, tileEdgesDY);
			ifloat3_add(tileEdgesReject, tileEdgesDY);
			ifloat3_add(tileEdgesAccept, tileEdgesDY);
			tile_i++;
		}

		//y step
		ifloat3_sub(rowEdges, tileEdgesDX);
		ifloat3_sub(edgesReject, tileEdgesDX);
		ifloat3_sub(edgesAccept, tileEdgesDX);
		tile_row_start += param.widthInTiles;
	}
}

//-------------------------------------------------------------------------------------
void Rasterizer::drawTriangleScanline(int32_t canvasWidth, int32_t canvasHeight, const Vector3& v0, const Vector3& v1, const Vector3& v2, DrawTriangleCallback callback)
{
	//back cull
	Vector3 edge0 = v2 - v1;
	Vector3 edge1 = v0 - v2;
	Vector3 vnormal = edge0.crossProduct(edge1);
	if (vnormal.z > 0) return;

	Vector3 edge2 = v1 - v0;

	float xmin = MathUtil::min3(v0.x, v1.x, v2.x);
	float ymin = MathUtil::min3(v0.y, v1.y, v2.y);
	float xmax = MathUtil::max3(v0.x, v1.x, v2.x);
	float ymax = MathUtil::max3(v0.y, v1.y, v2.y);

	float area = _edge(v0, v1, v2);

	int32_t x0 = MathUtil::max2(0, (int32_t)std::floor(xmin));
	int32_t x1 = MathUtil::min2(canvasWidth - 1, (int32_t)std::floor(xmax));
	int32_t y0 = MathUtil::max2(0, (int32_t)std::floor(ymin));
	int32_t y1 = MathUtil::min2(canvasHeight - 1, (int32_t)std::floor(ymax));

	for (int32_t y = y0; y <= y1; y++) {
		for (int32_t x = x0; x <= x1; x++) {

			Vector3 pixel(x + 0.5f, y + 0.5f, 0.f);

			bool overlaps = true;

			float w0 = _edge(v1, v2, pixel);
			overlaps &= (w0 == 0 ? ((edge0.y == 0 && edge0.x > 0) || edge0.y > 0) : (w0 > 0));
			if (!overlaps) continue;

			float w1 = _edge(v2, v0, pixel);
			overlaps &= (w1 == 0 ? ((edge1.y == 0 && edge1.x > 0) || edge1.y > 0) : (w1 > 0));
			if (!overlaps) continue;

			float w2 = _edge(v0, v1, pixel);
			overlaps &= (w2 == 0 ? ((edge2.y == 0 && edge2.x > 0) || edge2.y > 0) : (w2 > 0));
			if (!overlaps) continue;


			Vector3 t = Vector3(w0 / area, w1 / area, w2 / area);

			float invZ = MathUtil::lerp3(v0.z, v1.z, v2.z, t);
			t *= Vector3(v0.z / invZ, v1.z / invZ, v2.z / invZ);

			callback(std::make_pair(x, y), t);
		}
	}
}

}