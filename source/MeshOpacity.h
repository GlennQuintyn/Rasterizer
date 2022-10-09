#pragma once
#include <vector>
#include "structs.h"
#include "EffectOpacity.h"
#include "Camera.h"
#include "Texture.h"

class MeshOpacity
{
public:

	MeshOpacity(ID3D11Device* pDevice, const std::vector<Vertex_Input>& vertexBuffer, const std::vector<uint32_t>& indexBuffer);
	~MeshOpacity();

	void Render(ID3D11DeviceContext* pDeviceContext, const Camera* pCamera, const Texture* pDiffuseTexture) const;

	void ChangeSampleTech();

	void ToggleTransparency(ID3D11Device* pDevice);

	const Elite::FMatrix4& GetWorldMatrix() const { return m_MeshWorldMatrix; }

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

	EffectOpacity* m_pEffect{};

	int m_SampleTech;
	bool m_TransparencyEnabled;

};


