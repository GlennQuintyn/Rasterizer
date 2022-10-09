#include "pch.h"
#include "EffectOpaque.h"
#include <sstream>

EffectOpaque::EffectOpaque(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);

#pragma region Techniques
	m_pDefaultTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pDefaultTechnique->IsValid())
	{
		std::wcout << L"DefaultTechnique Technique not valid\n";
	}

	m_pPointTechnique = m_pEffect->GetTechniqueByName("PointTechnique");
	if (!m_pPointTechnique->IsValid())
	{
		std::wcout << L"Point Technique not valid\n";
	}

	m_pLinearTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
	if (!m_pLinearTechnique->IsValid())
	{
		std::wcout << L"Linear Technique not valid\n";
	}

	m_pAnisotropicTechnique = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
	if (!m_pAnisotropicTechnique->IsValid())
	{
		std::wcout << L"Anisotropic Technique not valid\n";
	}
#pragma endregion

#pragma region Matrices
	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
	{
		std::wcout << L"Variable m_pMatWorldViewProjVariable not found\n";
	}

	m_pWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pWorldVariable->IsValid())
	{
		std::wcout << L"Variable m_pWorldVariable not found\n";
	}

	m_pInverseViewVariable = m_pEffect->GetVariableByName("gInvViewMatrix")->AsMatrix();
	if (!m_pInverseViewVariable->IsValid())
	{
		std::wcout << L"Variable m_pInverseViewVariable not found\n";
	}
#pragma endregion

#pragma region Textures
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"Variable m_pDiffuseMapVariable not found\n";
	}

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"Variable m_pNormalMapVariable not found\n";
	}

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"Variable m_pSpecularMapVariable not found\n";
	}

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
	{
		std::wcout << L"Variable m_pGlossinessMapVariable not found\n";
	}
#pragma endregion

	m_pRasterizerVariable = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();
	if (!m_pRasterizerVariable->IsValid())
	{
		std::wcout << L"Variable m_pRasterizerVariable not found\n";
		return;
	}

	m_pRasterizerDesc = new D3D11_RASTERIZER_DESC{};
	m_pRasterizerDesc->FillMode = D3D11_FILL_SOLID;
	m_pRasterizerDesc->CullMode = D3D11_CULL_BACK;
	m_pRasterizerDesc->FrontCounterClockwise = true;

	HRESULT result = pDevice->CreateRasterizerState(m_pRasterizerDesc, &m_pRasterizerState);
	if (FAILED(result))
	{
		std::cout << "Making Rasterize state FAILED\n";
		return;
	}
	m_pRasterizerVariable->SetRasterizerState(0, m_pRasterizerState);

}

EffectOpaque::~EffectOpaque()
{
#pragma region Matrices
	if (m_pMatWorldViewProjVariable)
	{
		m_pMatWorldViewProjVariable->Release();
	}
	if (m_pWorldVariable)
	{
		m_pWorldVariable->Release();
	}
	if (m_pInverseViewVariable)
	{
		m_pInverseViewVariable->Release();
	}
#pragma endregion

#pragma region Techniques
	if (m_pDefaultTechnique)
	{
		m_pDefaultTechnique->Release();
	}
	if (m_pPointTechnique)
	{
		m_pPointTechnique->Release();
	}
	if (m_pLinearTechnique)
	{
		m_pLinearTechnique->Release();
	}
	if (m_pAnisotropicTechnique)
	{
		m_pAnisotropicTechnique->Release();
	}
#pragma endregion

#pragma region Textures
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
	}
	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->Release();
	}
	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->Release();
	}
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->Release();
	}
#pragma endregion

#pragma region Blend state
	if (m_pRasterizerState)
	{
		m_pRasterizerState->Release();
	}
	if (m_pRasterizerDesc)
	{
		delete m_pRasterizerDesc;
	}
	if (m_pRasterizerVariable)
	{
		m_pRasterizerVariable->Release();
	}
#pragma endregion

	if (m_pEffect)
	{
		m_pEffect->Release();
	}
}

void EffectOpaque::SetCullMode(ID3D11Device* pDevice, CullingModes cullMode)
{
	HRESULT result{};

	switch (cullMode)
	{
	case CullingModes::BackFace:
	{
		m_pRasterizerDesc->CullMode = D3D11_CULL_BACK;
		m_pRasterizerState->Release();

		result = pDevice->CreateRasterizerState(m_pRasterizerDesc, &m_pRasterizerState);
		if (FAILED(result))
		{
			std::cout << "Making Rasterize state FAILED\n";
			return;
		}
		m_pRasterizerVariable->SetRasterizerState(0, m_pRasterizerState);
	}
	break;
	case CullingModes::FrontFace:
	{
		m_pRasterizerDesc->CullMode = D3D11_CULL_FRONT;
		m_pRasterizerState->Release();

		result = pDevice->CreateRasterizerState(m_pRasterizerDesc, &m_pRasterizerState);
		if (FAILED(result))
		{
			std::cout << "Making Rasterize state FAILED\n";
			return;
		}
		m_pRasterizerVariable->SetRasterizerState(0, m_pRasterizerState);
	}
	break;
	case CullingModes::NoCulling:
	{
		m_pRasterizerDesc->CullMode = D3D11_CULL_NONE;
		m_pRasterizerState->Release();

		result = pDevice->CreateRasterizerState(m_pRasterizerDesc, &m_pRasterizerState);
		if (FAILED(result))
		{
			std::cout << "Making Rasterize state FAILED\n";
			return;
		}
		m_pRasterizerVariable->SetRasterizerState(0, m_pRasterizerState);
	}
	break;
	}

}

#pragma region SetMaps
void EffectOpaque::SetDiffuseMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pDiffuseMapVariable->IsValid())
	{
		m_pDiffuseMapVariable->SetResource(pResourceView);
	}
}

void EffectOpaque::SetNormalMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pNormalMapVariable->IsValid())
	{
		m_pNormalMapVariable->SetResource(pResourceView);
	}
}

void EffectOpaque::SetSpecularMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pSpecularMapVariable->IsValid())
	{
		m_pSpecularMapVariable->SetResource(pResourceView);
	}
}

void EffectOpaque::SetGlossinessMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pGlossinessMapVariable->IsValid())
	{
		m_pGlossinessMapVariable->SetResource(pResourceView);
	}
}
#pragma endregion

ID3DX11EffectTechnique* EffectOpaque::GetTechnique(int index)
{
	switch (index)
	{
	case 0:
		return m_pPointTechnique;
		break;
	case 1:
		return m_pLinearTechnique;
		break;
	case 2:
		return m_pAnisotropicTechnique;
		break;
	default:
		return m_pDefaultTechnique;
		break;
	}
}


