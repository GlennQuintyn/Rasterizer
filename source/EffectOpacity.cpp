#include "pch.h"
#include "EffectOpacity.h"
#include <sstream>

EffectOpacity::EffectOpacity(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);

#pragma region Techniques
	m_pDefaultTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pDefaultTechnique->IsValid())
	{
		std::wcout << L"DefaultTechnique Technique not valid\n";
		return;
	}

	m_pPointTechnique = m_pEffect->GetTechniqueByName("PointTechnique");
	if (!m_pPointTechnique->IsValid())
	{
		std::wcout << L"Point Technique not valid\n";
		return;
	}

	m_pLinearTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
	if (!m_pLinearTechnique->IsValid())
	{
		std::wcout << L"Linear Technique not valid\n";
		return;
	}

	m_pAnisotropicTechnique = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
	if (!m_pAnisotropicTechnique->IsValid())
	{
		std::wcout << L"Anisotropic Technique not valid\n";
		return;
	}
#pragma endregion

	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
	{
		std::wcout << L"Variable m_pMatWorldViewProjVariable not found\n";
		return;
	}

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"Variable m_pDiffuseMapVariable not found\n";
		return;
	}

	m_pBlendVariable = m_pEffect->GetVariableByName("gBlendState")->AsBlend();
	if (!m_pBlendVariable->IsValid())
	{
		std::wcout << L"Variable m_pBlendVar not found\n";
		return;
	}

	m_pBlendDesc = new D3D11_BLEND_DESC{};
	m_pBlendDesc->RenderTarget[0].BlendEnable = true;
	m_pBlendDesc->RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	m_pBlendDesc->RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	m_pBlendDesc->RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_pBlendDesc->RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	m_pBlendDesc->RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	m_pBlendDesc->RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m_pBlendDesc->RenderTarget[0].RenderTargetWriteMask = 0x0F;

	HRESULT result = pDevice->CreateBlendState(m_pBlendDesc, &m_pBlendState);
	if (FAILED(result))
	{
		std::cout << "Making Blendstate FAILED\n";
		return;
	}
	m_pBlendVariable->SetBlendState(0, m_pBlendState);
}

EffectOpacity::~EffectOpacity()
{
	if (m_pMatWorldViewProjVariable)
	{
		m_pMatWorldViewProjVariable->Release();
	}

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

#pragma region Blend state
	if (m_pBlendState)
	{
		m_pBlendState->Release();
	}
	if (m_pBlendDesc)
	{
		delete m_pBlendDesc;
	}
	if (m_pBlendVariable)
	{
		m_pBlendVariable->Release();
	}
#pragma endregion

	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
	}

	if (m_pEffect)
	{
		m_pEffect->Release();
	}
}

#pragma region SetMaps
void EffectOpacity::SetDiffuseMap(ID3D11ShaderResourceView* pResourceView)
{
	if (m_pDiffuseMapVariable->IsValid())
	{
		m_pDiffuseMapVariable->SetResource(pResourceView);
	}
}
#pragma endregion

ID3DX11EffectTechnique* EffectOpacity::GetTechnique(int index)
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

void EffectOpacity::ToggleTransparency(ID3D11Device* pDevice, bool boolean)
{
	if (boolean)
	{
		m_pBlendDesc->RenderTarget[0].BlendEnable = true;

		m_pBlendState->Release();
		HRESULT result= pDevice->CreateBlendState(m_pBlendDesc, &m_pBlendState);
		if (FAILED(result))
		{
			std::cout << "Switching Blendstate FAILED\n";
			return;
		}
		m_pBlendVariable->SetBlendState(0, m_pBlendState);
	}
	else
	{
		m_pBlendDesc->RenderTarget[0].BlendEnable = false;

		m_pBlendState->Release();
		HRESULT result = pDevice->CreateBlendState(m_pBlendDesc, &m_pBlendState);
		if (FAILED(result))
		{
			std::cout << "Switching Blendstate FAILED\n";
			return;
		}
		m_pBlendVariable->SetBlendState(0, m_pBlendState);
	}
}

