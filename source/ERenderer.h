/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>

#include <vector>
#include "MeshOpaque.h"
#include "MeshOpacity.h"
#include "Camera.h"
#include "TriangleMesh.h"
#include "Texture.h"

struct SDL_Window;
struct SDL_Surface;

namespace Elite
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow, PrimitiveTopology primitiveTopology);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(const MeshOpaque* pMeshVechicle, const MeshOpacity* pMeshFire, const Camera* pCamera, const Texture* pDiffuseTextureVechicle,
			const Texture* pNormalTextureVechicle, const Texture* pSpecularTextureVechicle, const Texture* pGlossInessTextureVechicle, const Texture* pDiffuseFireTexture,
			const TriangleMesh* triangleMesh);

		bool IsInitizalized() { return m_IsInitialized; };
		ID3D11Device* GetDevice() { return m_pDevice; };

		void ChangeCullMode();
		void ToggleRenderMode() { m_RenderInSoftware = !m_RenderInSoftware; };
		void SwitchSoftwareRenderMode();
		
	private:
		//Funtions
		HRESULT InitializeDirectX();
		void RenderDirectX(const MeshOpaque* pMeshVechicle, const MeshOpacity* pMeshFire, const Camera* pCamera, const Texture* pDiffuseTextureVechicle,
			const Texture* pNormalTextureVechicle, const Texture* pSpecularTextureVechicle, const Texture* pGlossInessTextureVechicle, const Texture* pDiffuseFireTexture);

		void RenderSoftware(const Camera* pCamera, const TriangleMesh* pTriangleMesh);

		//helper function for the software rasterizer
		void VertexTransformationtoNDC(const TriangleMesh* ptriangleMesh, std::vector<Vertex_Ouput>& transformedVerticesVec, const Camera* pCamera);
		void VertexTransformationtoRasterSpace(Vertex_Ouput& v0, Vertex_Ouput& v1, Vertex_Ouput& v2);

		bool VerticesInsideWindow(const Vertex_Ouput& v0, const Vertex_Ouput& v1, const Vertex_Ouput& v2);
		bool InsideOutside(const Vertex_Ouput& v0, const Vertex_Ouput& v1, const Vertex_Ouput& v2, const Elite::FPoint2& p) const;
		void BarycentricCoordinates(const Vertex_Ouput& v0, const Vertex_Ouput& v1, const Vertex_Ouput& v2, const Elite::FPoint2& p, float& w0, float& w1, float& w2);
		void PixelShading(const Vertex_Ouput& v, Elite::RGBColor& pixelColor, const Elite::FVector3& cameraPos, const TextureHolder& textures);


		//Data Memebers
		SDL_Window* m_pWindow;
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;
		uint32_t m_Width;
		uint32_t m_Height;

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGIFactory* m_pDXGIFactory;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		PrimitiveTopology m_PrimitiveTopology;
		CullingModes m_CullMode;

		int m_RenderMode;
		const int m_MaxRenderModes;

		bool m_IsInitialized;
		bool m_RenderInSoftware;
	};
}

#endif