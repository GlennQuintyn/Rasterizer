#pragma once
#include <string>
#include "EMath.h"
#include "ERGBColor.h"
#include <SDL_image.h>

class Texture
{
public:
	Texture(ID3D11Device* pDevice, const std::string& filePathAndName);
	~Texture();

	//sample only gets used for the software rasterizer
	Elite::RGBColor Sample(const Elite::FVector2& uv) const;
	//getResourceView is only for DirectX
	ID3D11ShaderResourceView* GetResourceView() const { return m_pTextureResourceView; };

private:
	
	SDL_Surface* m_pSurface;

	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pTextureResourceView;

};

