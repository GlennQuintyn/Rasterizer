#include "pch.h"
#include "Texture.h"
#include <fstream>
#include <iostream>


Texture::Texture(ID3D11Device* pDevice, const std::string& filePathAndName)
	: m_pSurface{}
	, m_pTexture{}
	, m_pTextureResourceView{}
{
	std::ifstream file(filePathAndName.c_str());
	//check if files exists
	if (file)
	{
		m_pSurface = IMG_Load(filePathAndName.c_str());

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = m_pSurface->w;
		desc.Height = m_pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = m_pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture);
		if (FAILED(hr))
		{
			std::cout << "Creating Texture FAILED\n";
			return;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_pTextureResourceView);
		if (FAILED(hr))
		{
			std::cout << "Creating Texture FAILED\n";
			return;
		}
	}
	else
	{
		std::cerr << "Failed to load file " << filePathAndName << std::endl;
	}
}

Texture::~Texture()
{
	if (m_pTexture)
	{
		m_pTexture->Release();
	}
	if (m_pTextureResourceView)
	{
		m_pTextureResourceView->Release();
	}
	if (m_pSurface)
	{
		SDL_FreeSurface(m_pSurface);
		m_pSurface = nullptr;
	}
}

Elite::RGBColor Texture::Sample(const Elite::FVector2& uv) const
{
	Elite::RGBColor color{};
	//check if in range of UV space
	if ((uv.x < 0 || uv.x > 1) || (uv.y < 0 || uv.y > 1)) { return color; }

	uint32_t pixelIndex{ uint32_t(m_pSurface->w * uv.x) + (uint32_t(m_pSurface->h * uv.y) * m_pSurface->w) };
	uint8_t r{};
	uint8_t g{};
	uint8_t b{};

	SDL_PixelFormat* pixelFormat{ m_pSurface->format };
	uint32_t colorPixel{ reinterpret_cast<uint32_t*>(m_pSurface->pixels)[pixelIndex] };
	SDL_GetRGB(colorPixel, pixelFormat, &r, &g, &b);

	color.r = r / 255.f;
	color.g = g / 255.f;
	color.b = b / 255.f;

	return color;
}
