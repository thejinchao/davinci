#include "dv_precompiled.h"
#include "dv_pipe_PS.h"

#include "device/dv_render_target.h"
#include "dv_pipe_VS.h"
#include "dv_rasterizer.h"
#include "device/dv_device_buffer.h"

namespace davinci
{
//-------------------------------------------------------------------------------------
void PixelShader::process(const PrimitiveAfterVS& input, RenderTarget& output)
{

	int32_t targetWidth = output.getWidth();
	int32_t targetHeight = output.getHeight();

	Matrix4 view_trans = 
		Matrix4::makeScale(targetWidth / 2.f, targetHeight / 2.f, 1.f) * 
		Matrix4::makeTrans(targetWidth / 2.f, targetHeight / 2.f, 0.f);

	input.visitor([view_trans, &output](const PrimitiveAfterVS::Node& node) {

		switch (node.primitiveType) {
		case PT_POINT_LIST:
		{
			for (size_t i = 0; i < node.vertexCounts; i++) {
				const float* vertex_data = (const float*)(node.vertexData->ptr(i * node.vertexSize*sizeof(float)));

				Vector4 color;
				float depth;
				node.ps->psFunction(node.psConstantBuffer, vertex_data, color, depth);

				const Vector3* input_pos = (const Vector3*)(vertex_data);
				Vector3 view_pos = (*input_pos) * view_trans;

				int16_t x = (int16_t)(view_pos.x + 0.5f);
				int16_t y = (int16_t)(view_pos.y + 0.5f);
				output.setPixel(x, y, color, depth);
			}
		}
		break;

		case PT_LINE_LIST:
		{
			for (size_t i = 0; i < node.vertexCounts; i+=2) {
				const float* vertex_start = (const float*)node.vertexData->ptr(i* node.vertexSize*sizeof(float));
				const float* vertex_end = (const float*)node.vertexData->ptr((i + 1) * node.vertexSize*sizeof(float));

				Vector3 start = (*(const Vector3*)(vertex_start)) * view_trans;
				Vector3 end = (*(const Vector3*)(vertex_end)) * view_trans;

				Rasterizer::drawLine(output.getWidth(), output.getHeight(), start.xy(), end.xy(), [vertex_start, vertex_end, &node, &output](const std::pair<int32_t, int32_t> dot, float percent) {

					std::vector<float> vertex(node.vertexSize);

					for (size_t j = 0; j < node.vertexSize; j++) {
						vertex[j] = MathUtil::lerp(vertex_start[j], vertex_end[j], percent);
					}

					Vector4 color;
					float depth;
					node.ps->psFunction(node.psConstantBuffer, &(vertex[0]), color, depth);

					output.setPixel(dot.first, dot.second, color, depth);
				});
			}

		}
		break;

		case PT_TRIANGLE_LIST:
		{
			for (size_t i = 0; i < node.vertexCounts; i += 3) {
				const float* vertex0 = (const float*)(node.vertexData->ptr(i* node.vertexSize * sizeof(float)));
				const float* vertex1 = (const float*)(node.vertexData->ptr((i + 1) * node.vertexSize * sizeof(float)));
				const float* vertex2 = (const float*)(node.vertexData->ptr((i + 2) * node.vertexSize * sizeof(float)));

				Vector3 pos0 = (*(const Vector3*)(vertex0)) * view_trans;
				Vector3 pos1 = (*(const Vector3*)(vertex1)) * view_trans;
				Vector3 pos2 = (*(const Vector3*)(vertex2)) * view_trans;

				pos0.z = node.invZ[i];
				pos1.z = node.invZ[i+1];
				pos2.z = node.invZ[i+2];

				Rasterizer::drawTriangleLarrabee(output.getWidth(), output.getHeight(), pos0, pos1, pos2,
					[vertex0, vertex1, vertex2, &node, &output](const std::pair<int32_t, int32_t>& dot, const Vector3& percent) {

						static std::vector<float> vertex;
						vertex.resize(node.vertexSize);

						for (size_t j = 0; j < node.vertexSize; j++) {
							vertex[j] = MathUtil::lerp3(vertex0[j], vertex1[j], vertex2[j], percent);
						}

						Vector4 color;
						float depth;
						node.ps->psFunction(node.psConstantBuffer, &(vertex[0]), color, depth);

						output.setPixel(dot.first, dot.second, color, depth);
				});
			}
		}
		break;

	default:
		break;
		}

	});
}

}

