#include "dv_precompiled.h"
#include "dv_asset_utility.h"

#include "dv_vertex_desc.h"
#include "dv_vertex_buffer.h"
#include "dv_index_buffer.h"
#include "dv_model.h"
#include "dv_texture.h"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <png.h>

namespace davinci
{

//-------------------------------------------------------------------------------------
/*
                                             |y
											 |
                                           /   \  (0,10,19,R)
                                        /          \
                          (1,14,18,G)/                \ (3,11,23, W)
                                     \                /
                                    |   \          /    |
                                    |      \    /       |
                                    |         .  (2,15,22,B)
									|		  |         |
                                    |         |         |
                                    |      (4,9,16,B)   |
                        (5,13,17,W) .         |         .  (7, 8, 20,G)
                                 /    \       |       /    \
							   / X       \    |    /         \ Z
									       \  | /       
									          .  (6,12,21,R)

*/
ModelPtr AssetUtility::createStandardModel_Box(RenderDevice* device, float x_size, float y_size, float z_size, PrimitiveType primitiveType, bool color, bool uv, bool normal)
{
	//define vertex 
	VertexDesc vertexDesc;
	vertexDesc.addElement(VertexElementType::VET_POSITION, VET_FLOAT_X3);
	if(color)
		vertexDesc.addElement(VertexElementType::VET_COLOR, VET_FLOAT_X3);
	if(uv)
		vertexDesc.addElement(VertexElementType::VET_TEXCOORD0, VET_FLOAT_X2);
	if (normal)
		vertexDesc.addElement(VertexElementType::VET_NORMAL, VET_FLOAT_X3);

	//vertices
	float box_pos[] = {
		-1.0f, 1.0f, -1.0f,	//0
		1.0f, 1.0f, -1.0f,	//1
		1.0f, 1.0f, 1.0f,	//2
		-1.0f, 1.0f, 1.0f,	//3

		-1.0f, -1.0f, -1.0f,	//4
		1.0f, -1.0f, -1.0f,		//5
		1.0f, -1.0f, 1.0f,		//6
		-1.0f, -1.0f, 1.0f,		//7

		-1.0f, -1.0f, 1.0f,		//8
		-1.0f, -1.0f, -1.0f,	//9
		-1.0f, 1.0f, -1.0f,		//10
		-1.0f, 1.0f, 1.0f,		//11

		1.0f, -1.0f, 1.0f,		//12
		1.0f, -1.0f, -1.0f,		//13
		1.0f, 1.0f, -1.0f,		//14
		1.0f, 1.0f, 1.0f,		//15

		-1.0f, -1.0f, -1.0f,	//16
		1.0f, -1.0f, -1.0f,		//17
		1.0f, 1.0f, -1.0f,		//18
		-1.0f, 1.0f, -1.0f,		//19

		-1.0f, -1.0f, 1.0f,		//20
		1.0f, -1.0f, 1.0f,		//21
		1.0f, 1.0f, 1.0f,		//22
		-1.0f, 1.0f, 1.0f,		//23
	};
	#define RED_VERTEX		Vector3::RED.x, Vector3::RED.y, Vector3::RED.z
	#define GREEN_VERTEX	Vector3::GREEN.x, Vector3::GREEN.y, Vector3::GREEN.z
	#define BLUE_VERTEX		Vector3::BLUE.x, Vector3::BLUE.y, Vector3::BLUE.z
	#define WHITE_VERTEX	Vector3::WHITE.x, Vector3::WHITE.y, Vector3::WHITE.z

	float box_color[] = {
		RED_VERTEX, GREEN_VERTEX, BLUE_VERTEX, WHITE_VERTEX,	//0, 1, 2, 3
		BLUE_VERTEX, WHITE_VERTEX, RED_VERTEX, GREEN_VERTEX,	//4, 5, 6, 7
		GREEN_VERTEX, BLUE_VERTEX, RED_VERTEX, WHITE_VERTEX,	//8, 9, 10, 11
		RED_VERTEX, WHITE_VERTEX, GREEN_VERTEX, BLUE_VERTEX, 	//12, 13, 14, 15
		BLUE_VERTEX, WHITE_VERTEX, GREEN_VERTEX, RED_VERTEX, 	//16, 17, 18, 19
		GREEN_VERTEX, RED_VERTEX, BLUE_VERTEX, WHITE_VERTEX 	//20, 21, 22, 23
	};
	float box_uv[] = {
		0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,   1.0f, 0.0f,		//0, 1, 2, 3
		0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,   0.0f, 1.0f,		//4, 5, 6, 7
		0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,   1.0f, 0.0f,		//8, 9, 10, 11
		0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,   0.0f, 1.0f,		//12, 13, 14, 15
		0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,   1.0f, 0.0f,		//16, 17, 18, 19
		0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,   0.0f, 1.0f		//20, 21, 22, 23
	};

	float box_normal[] = {
		0.f, 1.f, 0.f, 0.f, 1.f, 0.f,0.f, 1.f, 0.f,0.f, 1.f, 0.f, //0, 1, 2, 3
		0.f, -1.f, 0.f, 0.f, -1.f, 0.f,0.f, -1.f, 0.f,0.f, -1.f, 0.f, //4, 5, 6, 7
		-1.f, 0.f, 0.f, -1.f, 0.f, 0.f,-1.f, 0.f, 0.f,-1.f, 0.f, 0.f, //8, 9, 10, 11
		1.f, 0.f, 0.f, 1.f, 0.f, 0.f,1.f, 0.f, 0.f,1.f, 0.f, 0.f, //12, 13, 14, 15
		0.f, 0.f, -1.f, 0.f, 0.f, -1.f,0.f, 0.f, -1.f,0.f, 0.f, -1.f, //16, 17, 18, 19
		0.f, 0.f, 1.f, 0.f, 0.f, 1.f,0.f, 0.f, 1.f,0.f, 0.f, 1.f //20, 21, 22, 23
	};

	uint16_t point_list_indices[] = {
		0, 1, 2, 3, 4, 5, 6, 7
	};

	uint16_t line_list_indices[] =  {
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	};

	uint16_t triangle_list_indices[] = {
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	const size_t VERTEX_COUNTS = 24;

	//create vertex buffer first
	VertexBufferPtr vertexPtr = std::shared_ptr<VertexBuffer>(new VertexBuffer());

	float* data = new float[vertexDesc.vertexSize() * VERTEX_COUNTS];
	for (size_t i = 0; i < VERTEX_COUNTS; i++) {
		float* p = data + vertexDesc.vertexSize() * i;
		
		memcpy(p, box_pos + i * 3, sizeof(float) * 3); p += 3;
		if (color) {
			memcpy(p, box_color + i * 3, sizeof(float) * 3);
			p += 3;
		}
		if (uv) {
			memcpy(p, box_uv + i * 2, sizeof(float) * 2);
			p += 2;
		}
		if (normal) {
			memcpy(p, box_normal + i * 3, sizeof(float) * 3);
			p += 3;
		}
	}
	vertexPtr->build(device, vertexDesc, data, 24);
	delete[] data;

	//create index buffer
	IndexBufferPtr indexPtr = std::shared_ptr<IndexBuffer>(new IndexBuffer());
	switch (primitiveType) {
	case PT_POINT_LIST:
		indexPtr->build(device, point_list_indices, sizeof(point_list_indices) / sizeof(point_list_indices[0]));
		break;

	case PT_LINE_LIST:
		indexPtr->build(device, line_list_indices, sizeof(line_list_indices) / sizeof(line_list_indices[0]));
		break;

	case PT_TRIANGLE_LIST:
		indexPtr->build(device, triangle_list_indices, sizeof(triangle_list_indices) / sizeof(triangle_list_indices[0]));
		break;

	default:
		return nullptr;
	}

	//create model
	ModelPtr modelPtr =  std::shared_ptr<Model>(new Model());
	
	//add sub mesh
	Model::MeshPart meshPart;
	meshPart.m_vertexBuffer = vertexPtr;
	meshPart.m_indexBuffer = indexPtr;
	meshPart.m_primitiveType = primitiveType;
	modelPtr->m_meshes.push_back(meshPart);

	//add model part
	Model::Node& modelNode = modelPtr->m_root;
	modelNode.m_transform = Matrix4::makeScale(x_size, y_size, z_size);
	modelNode.m_parts.push_back(0);

	return modelPtr;
}

//-------------------------------------------------------------------------------------
ModelPtr AssetUtility::createStandardModel_Sphere(RenderDevice* device, float radius, PrimitiveType primitiveType, int32_t widthSegments, int32_t heightSegments)
{
	widthSegments = MathUtil::saturate(widthSegments, 3, 0x7FFFF);
	heightSegments = MathUtil::saturate(heightSegments, 2, 0x7FFFF);

	//define vertex 
	VertexDesc vertexDesc;
	vertexDesc.addElement(VertexElementType::VET_POSITION, VET_FLOAT_X3);
	vertexDesc.addElement(VertexElementType::VET_NORMAL, VET_FLOAT_X3);
//	vertexDesc.addElement(VertexElementType::VET_TEXCOORD0, VET_FLOAT_X2);

	size_t verices_counts = (size_t)((heightSegments - 1)*widthSegments + 2);
	std::vector<float> vertices;
	vertices.reserve(verices_counts * 3);

	for (int32_t iy = 0; iy <= heightSegments; iy++) {

		float v = (float)iy/heightSegments;

		for (int32_t ix = 0; ix < widthSegments; ix++) {

			float u = (float)ix / widthSegments;

			float x = MathUtil::cos(u * MathUtil::PI_X2) * MathUtil::sin(v * MathUtil::PI);
			float y = MathUtil::cos(v * MathUtil::PI);
			float z = MathUtil::sin(u * MathUtil::PI_X2) * MathUtil::sin(v * MathUtil::PI);
			vertices.push_back(x * radius);
			vertices.push_back(y * radius);
			vertices.push_back(z * radius);

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			if (iy == 0 || iy == heightSegments) break;
		}
	}

	//create index buffer
	IndexBufferPtr indexPtr = std::shared_ptr<IndexBuffer>(new IndexBuffer());

	switch (primitiveType) {
	case PT_POINT_LIST:
	{
		//point list indeices
		std::vector<uint16_t> point_indices(verices_counts);
		for (size_t i = 0; i < point_indices.size(); i++) point_indices[i] = (uint16_t)i;
		indexPtr->build(device, &point_indices[0], point_indices.size());

	}
		break;

	case PT_LINE_LIST:
	{
		std::vector<uint16_t> line_indices;
		for (int32_t iy = 0; iy <heightSegments; iy++) {
			for (int32_t ix = 0; ix <widthSegments; ix++) {
				line_indices.push_back((iy == 0) ? 0 : (uint16_t)((iy-1)*widthSegments+ ix + 1));
				line_indices.push_back((iy == heightSegments-1) ? ((uint16_t)verices_counts-1) : (uint16_t)(iy*widthSegments + ix + 1));
			}
		}
		for (int32_t iy = 1; iy <heightSegments; iy++) {
			for (int32_t ix = 0; ix <widthSegments; ix++) {
				line_indices.push_back((uint16_t)((iy - 1)*widthSegments + ix + 1));
				line_indices.push_back((uint16_t)((iy - 1)*widthSegments + (ix + 1)%widthSegments + 1));
			}
		}
		indexPtr->build(device, &line_indices[0], line_indices.size());
	}
		break;

	case PT_TRIANGLE_LIST:
	{
		std::vector<uint16_t> triangle_list_indices;
		
		for (int32_t ix = 0; ix < widthSegments; ix++) {
			triangle_list_indices.push_back(0);
			triangle_list_indices.push_back((uint16_t)(ix+1)%widthSegments + 1);
			triangle_list_indices.push_back((uint16_t)ix + 1);
		}

		for (int32_t iy = 1; iy <heightSegments-1; iy++) {
			for (int32_t ix = 0; ix <widthSegments; ix++) {
				uint16_t lt = (uint16_t)((iy - 1)*widthSegments + ix + 1);
				uint16_t rt = (uint16_t)((iy - 1)*widthSegments + (ix + 1) % widthSegments + 1);

				uint16_t lb = (uint16_t)(iy*widthSegments + ix + 1);
				uint16_t rb = (uint16_t)(iy*widthSegments + (ix + 1) % widthSegments + 1);

				triangle_list_indices.push_back(lt);
				triangle_list_indices.push_back(rt);
				triangle_list_indices.push_back(lb);

				triangle_list_indices.push_back(rt);
				triangle_list_indices.push_back(rb);
				triangle_list_indices.push_back(lb);
			}
		}
		
		for (int32_t ix = 0; ix < widthSegments; ix++) {
			triangle_list_indices.push_back((uint16_t)verices_counts - 1);
			triangle_list_indices.push_back((uint16_t)((heightSegments - 2)*widthSegments + ix + 1));
			triangle_list_indices.push_back((uint16_t)((heightSegments-2)*widthSegments + (ix + 1) % widthSegments + 1));
		}
		indexPtr->build(device, &triangle_list_indices[0], triangle_list_indices.size());
	}
		break;

	default:
		return nullptr;
	}

	//create vertex buffer 
	VertexBufferPtr vertexPtr = std::shared_ptr<VertexBuffer>(new VertexBuffer());
	vertexPtr->build(device, vertexDesc, &vertices[0], vertices.size()/vertexDesc.vertexSize());

	//create model
	ModelPtr modelPtr =  std::shared_ptr<Model>(new Model());
	
	//add sub mesh
	Model::MeshPart meshPart;
	meshPart.m_vertexBuffer = vertexPtr;
	meshPart.m_indexBuffer = indexPtr;
	meshPart.m_primitiveType = primitiveType;
	modelPtr->m_meshes.push_back(meshPart);

	//add model part
	Model::Node& modelNode = modelPtr->m_root;
	modelNode.m_transform = Matrix4::IDENTITY;
	modelNode.m_parts.push_back(0);

	return modelPtr;
}

//-------------------------------------------------------------------------------------
TexturePtr AssetUtility::createStandardTexture(RenderDevice* device, int32_t width, int32_t height, int32_t gridSize,
	const Vector3& ltColor, const Vector3& rtColor, const Vector3& lbColor, const Vector3& rbColor)
{
	#define  _edge(ax, ay, bx, by, px, py)  ((px - ax) * (by - ay) - (py - ay) * (bx - ax))

	//create texture
	TexturePtr texturePtr = std::shared_ptr<Texture>(new Texture(width, height, PF_FLOAT32_RGB));
	float area = -(float)width*height;

	for (int32_t i = 0; i < height; i++) {
		Vector3* p = (Vector3*)(&(texturePtr->m_pixelData[(size_t)(i*width*(texturePtr->getPixelSize()))]));
		Vector3 color;
		for (int32_t j = 0; j < width; j++) {
			if (j < width - i) {
				int32_t w0 = _edge(width, 0, 0, height, j, i);
				int32_t w1 = _edge(0, height, 0, 0, j, i);
				int32_t w2 = _edge(0, 0, width, 0, j, i);

				color = MathUtil::lerp3(ltColor, rtColor, lbColor, Vector3(w0 / area, w1 / area, w2 / area));
			}
			else {
				int32_t w0 = _edge(width, height, 0, height, j, i);
				int32_t w1 = _edge(0, height, width, 0, j, i);
				int32_t w2 = _edge(width, 0, width, height, j, i);

				color = MathUtil::lerp3(rtColor, rbColor, lbColor, Vector3(w0 / area, w1 / area, w2 / area));
			}

			bool inGrid = ((i / gridSize) & 1);
			if ((j / gridSize) & 1) inGrid = !inGrid;

			if (inGrid) {
				color = color*0.6f + 0.4f;
			}
			*p++ = color;
		}
	}
	return texturePtr;
}

//-------------------------------------------------------------------------------------
TexturePtr AssetUtility::loadTextureFromPNG(RenderDevice* device, const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (fp == 0) return nullptr;

	#define PNG_BYTES_TO_CHECK 4
	char buf[PNG_BYTES_TO_CHECK];

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	png_infop info_ptr = png_create_info_struct(png_ptr);

	setjmp(png_jmpbuf(png_ptr));

	//read first 4 byte
	if (PNG_BYTES_TO_CHECK != fread(buf, 1, 4, fp)) {
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return nullptr;
	}
	//check PNG sign
	if (png_sig_cmp((png_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0) {
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return nullptr;
	}
	//rewind fp
	fseek(fp, 0, SEEK_SET);
	png_init_io(png_ptr, fp);

	//read png info
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
	//read image info
	int32_t color_type = png_get_color_type(png_ptr, info_ptr);

	//check color type
	if (color_type != PNG_COLOR_TYPE_RGB_ALPHA && color_type != PNG_COLOR_TYPE_RGB) {
		//not support yet!
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return nullptr;
	}
	PixelFormat pixelFormat = (color_type == PNG_COLOR_TYPE_RGB_ALPHA) ? PF_FLOAT32_RGBA : PF_FLOAT32_RGB;

	//get size
	int32_t width = (int32_t)png_get_image_width(png_ptr, info_ptr);
	int32_t height = (int32_t)png_get_image_height(png_ptr, info_ptr);

	//create texture
	TexturePtr texturePtr = std::shared_ptr<Texture>(new Texture(width, height, pixelFormat));

	//get png row info
	png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);
	fclose(fp);

	//fill texture
	float* p = &(texturePtr->m_pixelData[0]);
	switch (color_type) {
	case PNG_COLOR_TYPE_RGB_ALPHA:
		for (int32_t y = 0; y < height; ++y) {
			for (int32_t x = 0; x < width * 4; ) {
				*p++ = row_pointers[y][x++] / 255.f; // red  
				*p++ = row_pointers[y][x++] / 255.f; // green  
				*p++ = row_pointers[y][x++] / 255.f; // blue  
				*p++ = row_pointers[y][x++] / 255.f; // alpha  
			}
		}
		break;

	case PNG_COLOR_TYPE_RGB:
		for (int32_t y = 0; y < height; ++y) {
			for (int32_t x = 0; x < width * 3; ) {
				*p++ = row_pointers[y][x++] / 255.f; // red  
				*p++ = row_pointers[y][x++] / 255.f; // green  
				*p++ = row_pointers[y][x++] / 255.f; // blue  
			}
		}
		break;

	default:
		break;
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	return texturePtr;
}

//-------------------------------------------------------------------------------------
bool AssetUtility::savePixelBufferToPNG(const char* filename, int32_t width, int32_t height, const void* pixelData, PixelFormat pixelFormat)
{
	int32_t bit_depth = 8;

	int32_t color_type;
	switch (pixelFormat) {
	case PF_FLOAT32_RGBA:
	case PF_UINT8_RGBA:
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;  break;
	case PF_FLOAT32_RGB:
		color_type = PNG_COLOR_TYPE_RGB; break;
	case PF_FLOAT32_R:
		color_type = PNG_COLOR_TYPE_GRAY; break;
	default:
		//not support yet!
		return false;
	}

	//create png_struct
	png_structp  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (png_ptr==0) return false;

	//create png info
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (png_ptr == 0) {
		png_destroy_write_struct(&png_ptr, 0);
		return false;
	}
	//set default error function
	setjmp(png_jmpbuf(png_ptr));

	FILE* fp = fopen(filename, "wb");
	if (fp == 0) {
		png_destroy_write_struct(&png_ptr, 0);
		return false;
	}

	//init write io
	png_init_io(png_ptr, fp);

	//write png head info
	png_set_IHDR(png_ptr, info_ptr, (png_uint_32)width, (png_uint_32)height,
		bit_depth, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	//integer pixelformat
	if (PF_UINT8_RGBA == pixelFormat) {
		for (int32_t y = 0; y < height; y++) {
			const uint32_t* p = (const uint32_t*)pixelData + (height - 1 - y)*width;
			png_write_rows(png_ptr, (png_bytepp)&p, 1);
		}
	}
	else {
		//write rows
		png_size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
		png_byte* row = (png_byte*)malloc(row_bytes);

		switch (color_type) {
		case PNG_COLOR_TYPE_RGB:
		{
			for (int32_t y = 0; y < height; y++) {
				const Vector3* p = (const Vector3*)pixelData + (height - 1 - y)*width;
				for (int32_t x = 0; x < width; x++) {
					row[x * 3 + 0] = (uint8_t)(p->x * 255);
					row[x * 3 + 1] = (uint8_t)(p->y * 255);
					row[x * 3 + 2] = (uint8_t)(p->z * 255);

					p++;
				}
				png_write_rows(png_ptr, &row, 1);
			}
		}
		break;

		case PNG_COLOR_TYPE_RGB_ALPHA:
		{
			for (int32_t y = 0; y < height; y++) {
				const Vector4* p = (const Vector4*)pixelData + (height - 1 - y)*width;
				for (int32_t x = 0; x < width; x++) {
					row[x * 4 + 0] = (uint8_t)(p->x * 255);
					row[x * 4 + 1] = (uint8_t)(p->y * 255);
					row[x * 4 + 2] = (uint8_t)(p->z * 255);
					row[x * 4 + 3] = (uint8_t)(p->w * 255);

					p++;
				}
				png_write_rows(png_ptr, &row, 1);
			}
		}
		break;

		case PNG_COLOR_TYPE_GRAY:
		{
			for (int32_t y = 0; y < height; y++) {
				const float* p = (const float*)pixelData + (height - 1 - y)*width;
				for (int32_t x = 0; x < width; x++) {
					row[x] = (uint8_t)((*p) * 255);
					p++;
				}
				png_write_rows(png_ptr, &row, 1);
			}
		}
		break;
		default:
			break;
		}
		free(row);
	}
	png_write_end(png_ptr, NULL);
	fclose(fp);
	return true;
}

//-------------------------------------------------------------------------------------
static bool _parseVertexElementType(const char* strType, VertexElementType& attribute)
{
#define IF_CHECK_VET(name)   \
	if (strcmp(strType, #name) == 0) { \
		attribute = VertexElementType::VET_##name; \
		return true; \
	}

	IF_CHECK_VET(POSITION)
	else IF_CHECK_VET(NORMAL)
	else IF_CHECK_VET(COLOR)
	else IF_CHECK_VET(TANGENT)
	else IF_CHECK_VET(BINORMAL)
	else IF_CHECK_VET(TEXCOORD0)
	else IF_CHECK_VET(TEXCOORD1)
	else IF_CHECK_VET(TEXCOORD2)
	else IF_CHECK_VET(TEXCOORD3)
	else IF_CHECK_VET(TEXCOORD4)
	else IF_CHECK_VET(TEXCOORD5)
	else IF_CHECK_VET(TEXCOORD6)
	else IF_CHECK_VET(TEXCOORD7)

	return false;
}

//-------------------------------------------------------------------------------------
static bool _parsePrimitiveType(const char* strType, PrimitiveType& type)
{
#define IF_CHECK_PT(name, t)   \
	if (strcmp(strType, #name) == 0) { \
		type = t; \
		return true; \
	}

	IF_CHECK_PT(POINTS, PT_POINT_LIST)
	else IF_CHECK_PT(LINES, PT_LINE_LIST)
	else IF_CHECK_PT(LINE_STRIP, PT_LINE_STRIP)
	else IF_CHECK_PT(TRIANGLES, PT_TRIANGLE_LIST)
	else IF_CHECK_PT(TRIANGLE_STRIP, PT_TRIANGLE_STRIP)

	return false;
}

//-------------------------------------------------------------------------------------
void _parserModelNode(Model::Node& node, const rapidjson::Value& array_json, const std::map<std::string, size_t>& meshPartID)
{
	for (const auto& node_json : array_json.GetArray()) {

		Model::Node childNode;

		//id
		if (node_json["id"].IsString()) {
			childNode.m_name = node_json["id"].GetString();
		}

		//transform
		childNode.m_transform = Matrix4::IDENTITY;

		//part(s)
		if (node_json.HasMember("parts") && node_json["parts"].IsArray()) {
			for (const auto& node_part_json : node_json["parts"].GetArray()) {

				if (!node_part_json["meshpartid"].IsString()) return;
				const char* meshPartName = node_part_json["meshpartid"].GetString();
				auto it = meshPartID.find(meshPartName);
				if (it == meshPartID.end()) return;

				//push meshpart index
				childNode.m_parts.push_back(it->second);
			}
		}

		//child(ren)
		if (node_json.HasMember("children") && node_json["children"].IsArray()) {
			for (const auto& child_json : node_json["children"].GetArray()) {

				_parserModelNode(childNode, child_json, meshPartID);
			}
		}

		//insert in model
		node.m_childen.push_back(childNode);
	}

}

//-------------------------------------------------------------------------------------
ModelPtr AssetUtility::loadModel(RenderDevice* device, const char* jsonFileName)
{
	FILE* fp = fopen(jsonFileName, "rb");
	if (fp == 0) return nullptr;

	char buffer[6 * 1024];
	rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));

	rapidjson::Document json;
	json.ParseStream(is);
	fclose(fp); fp = nullptr;

	//create model
	ModelPtr modelPtr = std::shared_ptr<Model>(new Model());

	//meshpart id map
	std::map<std::string, size_t> meshPartID;

	//build mesh(es)
	if (!json["meshes"].IsArray()) return nullptr;
	for (const auto& mesh_json : json["meshes"].GetArray()) {

		VertexDesc vertexDesc;

		//parse attributes
		if (!mesh_json["attributes"].IsArray()) return nullptr;
		for (const auto& attribute_json : mesh_json["attributes"].GetArray()) {

			//parse attribute
			VertexElementType attribute;
			if (!attribute_json.IsString() || !_parseVertexElementType(attribute_json.GetString(), attribute)) return nullptr;

			switch (attribute)
			{
			case VertexElementType::VET_POSITION:
			case VertexElementType::VET_NORMAL:
			case VertexElementType::VET_COLOR:
			case VertexElementType::VET_TANGENT:
			case VertexElementType::VET_BINORMAL:
				vertexDesc.addElement(attribute, VET_FLOAT_X3); break;
			case VertexElementType::VET_TEXCOORD0:
			case VertexElementType::VET_TEXCOORD1:
			case VertexElementType::VET_TEXCOORD2:
			case VertexElementType::VET_TEXCOORD3:
			case VertexElementType::VET_TEXCOORD4:
			case VertexElementType::VET_TEXCOORD5:
			case VertexElementType::VET_TEXCOORD6:
			case VertexElementType::VET_TEXCOORD7:
				vertexDesc.addElement(attribute, VET_FLOAT_X2); break;
			default:
				break;
			}
		}
		//check vertices size
		size_t vertexSize = vertexDesc.vertexSize();
		if (vertexSize == 0) return nullptr;

		//parse vertices
		if (!mesh_json["vertices"].IsArray()) return nullptr;
		const auto& vertices_array_json = mesh_json["vertices"].GetArray();
		if (vertices_array_json.Size() % vertexSize != 0) return nullptr;
		size_t vertexCounts = vertices_array_json.Size() / vertexSize;

		float* dataVertices = new float[vertices_array_json.Size()];
		for (rapidjson::SizeType i = 0; i < vertices_array_json.Size(); i++) {
			dataVertices[i] = vertices_array_json[i].GetFloat();
		}

		//create vertex buffer
		VertexBufferPtr vertexBufferPtr = std::shared_ptr<VertexBuffer>(new VertexBuffer());
		vertexBufferPtr->build(device, vertexDesc, dataVertices, vertexCounts);
		delete[] dataVertices;

		//create mesh part(s)
		if (!mesh_json["parts"].IsArray()) return nullptr;
		for (const auto& mesh_part_json : mesh_json["parts"].GetArray()) {
			Model::MeshPart meshPart;
			meshPart.m_vertexBuffer = vertexBufferPtr;
			meshPart.m_indexBuffer = std::shared_ptr<IndexBuffer>(new IndexBuffer());

			//id
			if (!mesh_part_json["id"].IsString()) return nullptr;
			meshPart.m_name = mesh_part_json["id"].GetString();

			//primitive type
			if (!mesh_part_json["type"].IsString()) return nullptr;
			if (!_parsePrimitiveType(mesh_part_json["type"].GetString(), meshPart.m_primitiveType)) return nullptr;

			//indices
			if (!mesh_part_json["indices"].IsArray()) return nullptr;
			const auto& mesh_part_indices_json = mesh_part_json["indices"].GetArray();
			size_t indicesCounts = mesh_part_indices_json.Size();

			uint16_t* indicesData = new uint16_t[indicesCounts];
			for(rapidjson::SizeType i = 0; i < indicesCounts; i++) {
				indicesData[i] = (uint16_t)mesh_part_indices_json[i].GetInt();
			}
			meshPart.m_indexBuffer->build(device, indicesData, indicesCounts);
			delete[] indicesData;

			//insert in model
			meshPartID.insert({ meshPart.m_name, modelPtr->m_meshes.size() });
			modelPtr->m_meshes.push_back(meshPart);
		}
	}
	if (modelPtr->m_meshes.empty()) return nullptr;

	//build node(s)
	if (!json["nodes"].IsArray()) return nullptr;
	_parserModelNode(modelPtr->m_root, json["nodes"], meshPartID);

	return modelPtr;
}



}
