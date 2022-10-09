#include "pch.h"
#include "MeshOpaque.h"


MeshOpaque::MeshOpaque(ID3D11Device* pDevice, const std::vector<Vertex_Input>& vertexBuffer, const std::vector<uint32_t>& indexBuffer)
	: m_pEffect{ new EffectOpaque{ pDevice, L"Resources/PosCol3D.fx"} }
	, m_MeshWorldMatrix{ m_MeshWorldMatrix.Identity() }
	, m_CullMode{ CullingModes::BackFace }
	, m_SampleTech{}
{
	//Create Vertex Layout
	HRESULT result = S_OK;
	static const uint32_t numElements{ 6 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;//size 12
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;//size 12
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "WORLDPOS";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//size 16
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;//size 12
	vertexDesc[3].AlignedByteOffset = 40;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;//size 12
	vertexDesc[4].AlignedByteOffset = 52;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[5].SemanticName = "TEXCOORD";
	vertexDesc[5].Format = DXGI_FORMAT_R32G32_FLOAT;//size 8
	vertexDesc[5].AlignedByteOffset = 64;
	vertexDesc[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create Vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_Input) * (uint32_t)vertexBuffer.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = vertexBuffer.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
	{
		std::cout << "Creating Mesh FAILED\n";
		return;
	}

	//Create the input layout
	D3DX11_PASS_DESC passDesc;

	//m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->GetDesc(&passDesc);


	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pVertexLayout
	);
	if (FAILED(result))
	{
		std::cout << "Creating Mesh FAILED\n";
		return;
	}

	//Create index buffer
	m_AmountIndices = (uint32_t)indexBuffer.size();
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indexBuffer.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
	{
		std::cout << "Creating Mesh FAILED\n";
		return;
	}
}

MeshOpaque::~MeshOpaque()
{
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
	}
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
	}
	if (m_pVertexLayout)
	{
		m_pVertexLayout->Release();
	}
	if (m_pEffect)
	{
		delete m_pEffect;
	}
}


void MeshOpaque::Render(ID3D11DeviceContext* pDeviceContext, const Camera* pCamera, const Texture* pDiffuseTexture,
	const Texture* pNormalTexture, const Texture* pSpecularTexture, const Texture* pGlossInessTexture) const
{
	//Set Vertex Buffer
	UINT stride = sizeof(Vertex_Input);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Set Index Buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set Input Layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	//Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//set worldviewProjectionMatrix
	float* pWorldViewProjMatrix{ (pCamera->GetLeftHandedProjectionMatrix() * pCamera->GetInverseLookAtMatrix() * m_MeshWorldMatrix).data[0] };
	m_pEffect->GetWorldViewProjMat()->SetMatrix(pWorldViewProjMatrix);

	//set worldMatrix
	float* pWorldMatrix{ (float*)(m_MeshWorldMatrix.data[0]) };
	m_pEffect->GetWorldMat()->SetMatrix(pWorldMatrix);

	//set inververse ViewMatrix
	float* pInverseView{ (pCamera->GetLookAtMatrix()).data[0] };
	m_pEffect->GetInverseView()->SetMatrix(pInverseView);


	//set different maps
	m_pEffect->SetDiffuseMap(pDiffuseTexture->GetResourceView());
	m_pEffect->SetNormalMap(pNormalTexture->GetResourceView());
	m_pEffect->SetSpecularMap(pSpecularTexture->GetResourceView());
	m_pEffect->SetGlossinessMap(pGlossInessTexture->GetResourceView());


	//Render a Triangle
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pEffect->GetTechnique(m_SampleTech)->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique(m_SampleTech)->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
	}
}

void MeshOpaque::ChangeSampleTech()
{
	if (m_SampleTech > 2) { m_SampleTech = 0; }
	else { ++m_SampleTech; }
}

void MeshOpaque::ChangeCullMode(ID3D11Device* pDevice)
{
	switch (m_CullMode)
	{
	case CullingModes::BackFace:
		m_CullMode = CullingModes::FrontFace;
		break;
	case CullingModes::FrontFace:
		m_CullMode = CullingModes::NoCulling;
		break;
	case CullingModes::NoCulling:
		m_CullMode = CullingModes::BackFace;
		break;
	}
	m_pEffect->SetCullMode(pDevice, m_CullMode);
}

void MeshOpaque::TranslateMesh(const Elite::FVector3& translation)
{
	Elite::FMatrix4 translationMatrix{ Elite::MakeTranslation(translation) };
	m_MeshWorldMatrix *= translationMatrix;
}

void MeshOpaque::RotateMeshXYZ(const float angleX, const float angleY, const float angleZ)
{
	Elite::FMatrix4 rotionMatrix{ Elite::MakeRotationZYX(Elite::ToRadians(angleX),Elite::ToRadians(angleY),Elite::ToRadians(angleZ)) };
	m_MeshWorldMatrix *= rotionMatrix;
}

void MeshOpaque::ScaleMesh(const float scalingX, const float scalingY, const float scalingZ)
{
	Elite::FMatrix4 scalingMatrix{ Elite::MakeScale(scalingX, scalingY, scalingZ) };
	m_MeshWorldMatrix *= scalingMatrix;
}