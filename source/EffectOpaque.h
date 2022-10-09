#pragma once
#include <sstream>
#include "structs.h"

class EffectOpaque final
{
public:
	EffectOpaque(ID3D11Device* pDevice, const std::wstring& assetFile);
	~EffectOpaque();

	void SetDiffuseMap(ID3D11ShaderResourceView* pResourceView);
	void SetNormalMap(ID3D11ShaderResourceView* pResourceView);
	void SetSpecularMap(ID3D11ShaderResourceView* pResourceView);
	void SetGlossinessMap(ID3D11ShaderResourceView* pResourceView);


	ID3DX11EffectTechnique* GetTechnique(int index);

	ID3DX11Effect* GetEffect() { return m_pEffect; }
	ID3DX11EffectTechnique* GetTechnique() { return m_pDefaultTechnique; }

	ID3DX11EffectMatrixVariable* GetWorldViewProjMat() { return m_pMatWorldViewProjVariable; }
	ID3DX11EffectMatrixVariable* GetWorldMat() { return m_pWorldVariable; }
	ID3DX11EffectMatrixVariable* GetInverseView() { return m_pInverseViewVariable; }

	ID3DX11EffectShaderResourceVariable* GetDiffuseShaderResource() { return m_pDiffuseMapVariable; }
	ID3DX11EffectShaderResourceVariable* GetNormalShaderResource() { return m_pNormalMapVariable; }
	ID3DX11EffectShaderResourceVariable* GetSpecularShaderResource() { return m_pSpecularMapVariable; }
	ID3DX11EffectShaderResourceVariable* GetGlossinessShaderResource() { return m_pGlossinessMapVariable; }

	void SetCullMode(ID3D11Device* pDevice, CullingModes cullMode);

private:

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result = S_OK;
		ID3D10Blob* pErrorBlob = nullptr;
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob
		);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				char* pErrors = (char*)pErrorBlob->GetBufferPointer();

				std::wstringstream ss;
				for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << std::endl;
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
				std::wcout << ss.str() << std::endl;
				return nullptr;
			}
		}

		return pEffect;
	}

	ID3DX11Effect* m_pEffect;

	//different blend states
	ID3D11RasterizerState* m_pRasterizerState;
	D3D11_RASTERIZER_DESC* m_pRasterizerDesc;
	ID3DX11EffectRasterizerVariable* m_pRasterizerVariable;

	//matrices
	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
	ID3DX11EffectMatrixVariable* m_pWorldVariable;
	ID3DX11EffectMatrixVariable* m_pInverseViewVariable;

	//textures
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;

	//techniques
	ID3DX11EffectTechnique* m_pDefaultTechnique;
	ID3DX11EffectTechnique* m_pPointTechnique;
	ID3DX11EffectTechnique* m_pLinearTechnique;
	ID3DX11EffectTechnique* m_pAnisotropicTechnique;
};

