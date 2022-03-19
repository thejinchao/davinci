#pragma once

#include "dv_prerequisites.h"
#include "dv_asset_utility.h"

namespace davinci
{

class Model : noncopyable
{
public:
	struct MeshPart
	{
		std::string		m_name;
		VertexBufferPtr	m_vertexBuffer;
		IndexBufferPtr	m_indexBuffer;
		PrimitiveType	m_primitiveType;
	};

	struct Node
	{
		std::string			m_name;
		Matrix4				m_transform;
		std::vector<Node>	m_childen;
		std::vector<size_t>	m_parts;	//index of m_meshes
	};

public:
	MeshPart* _hackGetMeshPart(void) {
		return &(m_meshes[0]);
	}
	typedef std::function<void(const Matrix4& transform, const MeshPart* meshPart)> VisotorFunction;
	void visit(const Matrix4& transform, VisotorFunction func) const;

private:
	void _visitNode(const Matrix4& transform, const Node& node, VisotorFunction vistorFunc) const;

private:
	std::string				m_name;
	std::vector<MeshPart>	m_meshes;
	Node					m_root;

public:
	Model();
	~Model() {}

	//create by AssetManager
	friend ModelPtr AssetUtility::createStandardModel_Box(RenderDevice* device, float x_size, float y_size, float z_size, PrimitiveType primitiveType, bool color, bool uv, bool normal);
	friend ModelPtr AssetUtility::createStandardModel_Sphere(RenderDevice* device, float radius, PrimitiveType primitiveType, int32_t widthSegments, int32_t heightSegments);
	friend ModelPtr AssetUtility::loadModel(RenderDevice* device, const char* jsonFileName);
};

}
