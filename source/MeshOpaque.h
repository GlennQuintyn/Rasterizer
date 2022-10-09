#pragma once
#include <vector>
#include "structs.h"
#include "EffectOpaque.h"
#include "Camera.h"
#include "Texture.h"

class MeshOpaque
{
public:

	MeshOpaque(ID3D11Device* pDevice, const std::vector<Vertex_Input>& vertexBuffer, const std::vector<uint32_t>& indexBuffer);
	~MeshOpaque();

	void Render(ID3D11DeviceContext* pDeviceContext, const Camera* pCamera, const Texture* pDiffuseTexture,
		const Texture* pNormalTexture, const Texture* pSpecularTexture, const Texture* pGlossInessTexture) const;

	void ChangeSampleTech();

	const Elite::FMatrix4& GetWorldMatrix() const { return m_MeshWorldMatrix; }

	void ChangeCullMode(ID3D11Device* pDevice);

	//change the worldMatrix of the mesh
	void TranslateMesh(const Elite::FVector3& translation);
	void RotateMeshXYZ(const float angleX, const float angleY, const float angleZ);
	void ScaleMesh(const float scalingX, const float scalingY, const float scalingZ);


private:

	Elite::FMatrix4 m_MeshWorldMatrix;

	ID3D11Buffer* m_pVertexBuffer{};
	ID3D11Buffer* m_pIndexBuffer{};

	ID3D11InputLayout* m_pVertexLayout{};

	uint32_t m_AmountIndices{};

	EffectOpaque* m_pEffect{};
	CullingModes m_CullMode;
	int m_SampleTech;
};

