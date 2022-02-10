#include "dv_precompiled.h"
#include "dv_model.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
Model::Model()
{
	m_root.m_transform = Matrix4::IDENTITY;
}

//-------------------------------------------------------------------------------------
void Model::visit(const Matrix4& transform, Model::VisotorFunction func) const
{
	_visitNode(transform, m_root, func);
}

//-------------------------------------------------------------------------------------
void Model::_visitNode(const Matrix4& parentTransform, const Node& node, Model::VisotorFunction vistorFunc) const
{
	Matrix4 transform = node.m_transform * parentTransform;
	for (size_t index : node.m_parts) {
		const MeshPart& parts = m_meshes[index];

		vistorFunc(transform, &parts);
	}

	for (const Node& childNode : node.m_childen) {
		_visitNode(transform, childNode, vistorFunc);
	}
}

}
