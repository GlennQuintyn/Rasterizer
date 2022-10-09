#pragma once
#include "EMath.h"
#include "ERGBColor.h"
#include "Structs.h"
#include <vector>

class TriangleMesh
{
public:
	TriangleMesh();//default constructor to make empty mesh to later fill in
	TriangleMesh(const std::vector<Vertex_Input>& vertexBuffer, const std::vector<uint32_t>& indexBuffer, TextureHolder&& textureHolder);
	~TriangleMesh() = default;

	const std::vector<Vertex_Input>& GetVertexBuffer() const { return m_VertexBuffer; }
	const std::vector<uint32_t>& GetIndexBuffer() const { return m_IndexBuffer; }
	const Elite::FMatrix4& GetWorldMatrix() const { return m_MeshWorldMatrix; }
	const TextureHolder& GetTextures() const { return m_Textures; }

	//change the worldMatrix of the mesh
	void TranslateMesh(const Elite::FVector3& translation);
	void RotateMeshXYZ(const float angleX,const float angleY, const float angleZ);
	void ScaleMesh(const float scalingX, const float scalingY, const float scalingZ);

private:

	std::vector<Vertex_Input> m_VertexBuffer;
	std::vector<uint32_t> m_IndexBuffer;
	Elite::FMatrix4 m_MeshWorldMatrix;
	TextureHolder m_Textures;
};

