#include "pch.h"
#include "TriangleMesh.h"

TriangleMesh::TriangleMesh()
	: m_MeshWorldMatrix{ m_MeshWorldMatrix.Identity() }
	, m_Textures{}
{
}

TriangleMesh::TriangleMesh(const std::vector<Vertex_Input>& vertexBuffer, const std::vector<uint32_t>& indexBuffer, TextureHolder&& textureHolder)
	: m_MeshWorldMatrix{}
	, m_Textures{ std::move(textureHolder) }
{
	m_VertexBuffer = vertexBuffer;
	m_IndexBuffer = indexBuffer;

	//initialize the object matrix
	m_MeshWorldMatrix = m_MeshWorldMatrix.Identity();
}

void TriangleMesh::TranslateMesh(const Elite::FVector3& translation)
{
	Elite::FMatrix4 translationMatrix{ Elite::MakeTranslation(translation) };
	m_MeshWorldMatrix *= translationMatrix;
}

void TriangleMesh::RotateMeshXYZ(const float angleX, const float angleY, const float angleZ)
{
	Elite::FMatrix4 rotionMatrix{ Elite::MakeRotationZYX(Elite::ToRadians(angleX),Elite::ToRadians(angleY),Elite::ToRadians(angleZ)) };
	m_MeshWorldMatrix *= rotionMatrix;
}

void TriangleMesh::ScaleMesh(const float scalingX, const float scalingY, const float scalingZ)
{
	Elite::FMatrix4 scalingMatrix{ Elite::MakeScale(scalingX, scalingY, scalingZ) };
	m_MeshWorldMatrix *= scalingMatrix;
}