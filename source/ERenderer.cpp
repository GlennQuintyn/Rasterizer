#include "pch.h"

//Project includes
#include "ERenderer.h"

Elite::Renderer::Renderer(SDL_Window* pWindow, PrimitiveTopology primitiveTopology)
	: m_pWindow{ pWindow }
	, m_PrimitiveTopology{ primitiveTopology }
	, m_Width{}
	, m_Height{}
	, m_RenderMode{}
	, m_MaxRenderModes{ 4 }
	, m_IsInitialized{ false }
	, m_CullMode{ CullingModes::BackFace }
	, m_RenderInSoftware{ false }//false
{
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//Initialize DirectX pipeline
	if (SUCCEEDED(InitializeDirectX()))
	{
		m_IsInitialized = true;
		std::cout << "DirectX is ready\n";
	}
	else
	{
		std::cout << "DirectX FAILED and is NOT ready\n";
	}
}

Elite::Renderer::~Renderer()
{
#pragma region DirectX
	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
	}

	if (m_pRenderTargetBuffer)
	{
		m_pRenderTargetBuffer->Release();
	}

	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
	}

	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
	}

	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
	}

	if (m_pDXGIFactory)
	{
		m_pDXGIFactory->Release();
	}

	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}

	if (m_pDevice)
	{
		m_pDevice->Release();
	}
#pragma endregion

#pragma region Software Rasterizer
	if (m_pFrontBuffer)
	{
		SDL_FreeSurface(m_pFrontBuffer);
	}
	if (m_pBackBuffer)
	{
		SDL_FreeSurface(m_pBackBuffer);
	}
#pragma endregion
}

void Elite::Renderer::Render(const MeshOpaque* pMeshVechicle, const MeshOpacity* pMeshFire, const Camera* pCamera, const Texture* pDiffuseTextureVechicle,
	const Texture* pNormalTextureVechicle, const Texture* pSpecularTextureVechicle, const Texture* pGlossInessTextureVechicle, const Texture* pDiffuseFireTexture,
	const TriangleMesh* pTriangleMesh)
{
	if (m_RenderInSoftware)
	{
		RenderSoftware(pCamera, pTriangleMesh);
	}
	else
	{
		RenderDirectX(pMeshVechicle, pMeshFire, pCamera, pDiffuseTextureVechicle, pNormalTextureVechicle,
			pSpecularTextureVechicle, pGlossInessTextureVechicle, pDiffuseFireTexture);
	}
}

#pragma region DirectXCode
void Elite::Renderer::RenderDirectX(const MeshOpaque* pMeshVechicle, const MeshOpacity* pMeshFire, const Camera* pCamera,
	const Texture* pDiffuseTextureVechicle, const Texture* pNormalTextureVechicle, const Texture* pSpecularTextureVechicle,
	const Texture* pGlossInessTextureVechicle, const Texture* pDiffuseFireTexture)
{
	if (!m_IsInitialized)
		return;

	//Clear Buffers
	RGBColor clearColor{ 0.0f, 0.8f, 0.8f };//cyan clearcolor
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	//Render
	pMeshVechicle->Render(m_pDeviceContext, pCamera, pDiffuseTextureVechicle, pNormalTextureVechicle, pSpecularTextureVechicle, pGlossInessTextureVechicle);
	pMeshFire->Render(m_pDeviceContext, pCamera, pDiffuseFireTexture);

	//Presentd
	m_pSwapChain->Present(0, 0);
}

HRESULT Elite::Renderer::InitializeDirectX()
{
	//Create Device and Device context, using hardware acceleration
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	uint32_t creatDeviceFlags = 0;
#if defined(DEBUG)|| defined(_DEBUG)
	creatDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, creatDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);

	if (FAILED(result))
		return result;

	//creat DXGI Factory to create the SwapChain base on hardware
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));
	if (FAILED(result))
		return result;

	//creat SwapChain Descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//Get the handle HWND from SDL backbuffer
	SDL_SysWMinfo sysWMinfo{};
	SDL_VERSION(&sysWMinfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMinfo);
	swapChainDesc.OutputWindow = sysWMinfo.info.win.window;

	//Create SwapChain and hook it into the handle of the SDL window
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
		return result;

	//Create the Depth/Stencil Buffer and View
	D3D11_TEXTURE2D_DESC deptStencilDesc{};
	deptStencilDesc.Width = m_Width;
	deptStencilDesc.Height = m_Height;
	deptStencilDesc.MipLevels = 1;
	deptStencilDesc.ArraySize = 1;
	deptStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	deptStencilDesc.SampleDesc.Count = 1;
	deptStencilDesc.SampleDesc.Quality = 0;
	deptStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	deptStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	deptStencilDesc.CPUAccessFlags = 0;
	deptStencilDesc.MiscFlags = 0;


	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = deptStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&deptStencilDesc, 0, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;

	//Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, 0, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	//Bind the Views to the Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the Viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);

	return result;
}
#pragma endregion

#pragma region Software Rasterizer code
void Elite::Renderer::RenderSoftware(const Camera* pCamera, const TriangleMesh* pTriangleMesh)
{
	SDL_LockSurface(m_pBackBuffer);
	std::vector<float> deptBuffer{ };
	deptBuffer.reserve(size_t(m_Width) * m_Height);
	deptBuffer.resize(size_t(m_Width) * m_Height);
	for (float& deptValue : deptBuffer)
	{
		deptValue = FLT_MAX;
	}
	Elite::FVector3 camerPos{ pCamera->GetCameraPos() };

	const TextureHolder& textures{ pTriangleMesh->GetTextures() };
	std::vector<Vertex_Ouput> result(pTriangleMesh->GetVertexBuffer().size());//->GetVertexBuffer().size()
	std::vector<uint32_t> indexBuffer{ pTriangleMesh->GetIndexBuffer() };

	//take the orriginal vertices and put them in NDC space 
	VertexTransformationtoNDC(pTriangleMesh, result, pCamera);

	bool done{};
	size_t index{};
	//loop over all the triangles
	while (!done)
	{
		Vertex_Ouput v0{}, v1{}, v2{};
		//check PrimitiveTopology if its a triangle strip and check if its an odd triangle
		if (m_PrimitiveTopology == PrimitiveTopology::TirangleStrip && (index & 1))
		{
			//odd
			v0 = { result[indexBuffer[index]] };
			v1 = { result[indexBuffer[index + 2]] };
			v2 = { result[indexBuffer[index + 1]] };
		}
		else
		{
			v0 = { result[indexBuffer[index]] };
			v1 = { result[indexBuffer[index + 1]] };
			v2 = { result[indexBuffer[index + 2]] };
		}

		//if all vertices are outside don't render this triangle
		if (VerticesInsideWindow(v0, v1, v2))
		{
			//if inside frustrum then put them into rasterspace
			VertexTransformationtoRasterSpace(v0, v1, v2);

			//if its not a degenerate triangle, do the calc for that triangle
			if (indexBuffer[index] != indexBuffer[index + 1] && indexBuffer[index + 1] != indexBuffer[index + 2])
			{
				//calc the bounding box
#pragma region BoundingBox
				uint32_t minR{}, maxR{};
				uint32_t minC{}, maxC{};

				float minCf = std::min(std::min(v0.Position.x, v1.Position.x), v2.Position.x);
				float maxCf = std::max(std::max(v0.Position.x, v1.Position.x), v2.Position.x);
				(minCf < 0) ? minC = 0 : minC = uint32_t(minCf);
				(maxCf >= m_Width) ? maxC = m_Width - 1 : maxC = uint32_t(maxCf);

				float minRf = std::min(std::min(v0.Position.y, v1.Position.y), v2.Position.y);
				float maxRf = std::max(std::max(v0.Position.y, v1.Position.y), v2.Position.y);
				(minRf < 0) ? minR = 0 : minR = uint32_t(minRf);
				(maxRf >= m_Height) ? maxR = m_Height - 1 : maxR = uint32_t(maxRf);
#pragma endregion

				//Loop over all the pixels
				for (uint32_t r = minR; r <= maxR; ++r)
				{
					for (uint32_t c = minC; c <= maxC; ++c)
					{
						//doing the inside outside test wiht pixel as point
						if (InsideOutside(v0, v1, v2, { float(c),float(r) }))
						{
							Elite::RGBColor pixelColor{};
							float w0{}, w1{}, w2{};
							//calc the weight via barycentricCoordinates
							BarycentricCoordinates(v0, v1, v2, { float(c),float(r) }, w0, w1, w2);

							float newDeptValue{ 1 / ((w0 / v0.Position.z) + (w1 / v1.Position.z) + (w2 / v2.Position.z)) };
							float wValue{ 1 / ((w0 / v0.Position.w) + (w1 / v1.Position.w) + (w2 / v2.Position.w)) };

							//dept test
							if ((newDeptValue > 0 && newDeptValue < 1) && deptBuffer[c + (r * size_t(m_Width))] > newDeptValue)
							{
								deptBuffer[c + (r * size_t(m_Width))] = newDeptValue;

								switch (m_RenderMode)
								{
								case 0://complete pixel shading
								{
									//interpolation calc
									Elite::FVector3 normalInterpolated{ Elite::GetNormalized(((v0.Normal / v0.Position.w) * w0) + ((v1.Normal / v1.Position.w) * w1) + ((v2.Normal / v2.Position.w) * w2)) };
									Elite::FVector3 tangentInterpolated{ Elite::GetNormalized(((v0.Tangent / v0.Position.w) * w0) + ((v1.Tangent / v1.Position.w) * w1) + ((v2.Tangent / v2.Position.w) * w2)) };
									Elite::FVector2 textureSamplePos{ (((v0.UV / v0.Position.w) * w0) + ((v1.UV / v1.Position.w) * w1) + ((v2.UV / v2.Position.w) * w2)) * wValue };
									Elite::FPoint4 worldPosInterpolated{ (((Elite::FVector4{v0.WorldPosition} / v0.Position.w) * w0) + ((Elite::FVector4{v1.WorldPosition} / v1.Position.w) * w1) + ((Elite::FVector4{v2.WorldPosition } / v2.Position.w) * w2)) * wValue };

									PixelShading(Vertex_Ouput{ {},{}, worldPosInterpolated, normalInterpolated, tangentInterpolated, textureSamplePos },
										pixelColor, camerPos, textures);

								}
								break;
								case 1://sample only diffuse texture to get pixel color without pixel shading
								{
									if (textures.pDiffuse)
									{
										FVector2 textureSamplePos{ (((v0.UV / v0.Position.w) * w0) + ((v1.UV / v1.Position.w) * w1) + ((v2.UV / v2.Position.w) * w2)) * wValue };
										pixelColor = { textures.pDiffuse->Sample(textureSamplePos) };
									}
									//error color, meaning there is no diffuse map
									else { pixelColor = { 0.f,1.f,1.f }; }
								}
								break;
								case 2://take deptbuffer and remap to get pixel color
								{
									float temp{ std::max(0.f, Elite::Remap(newDeptValue, 0.985f, 1.f)) };
									pixelColor = { temp,temp,temp };
								}
								break;
								case 3://interpolate vertices color to get pixel color
								{
									pixelColor = { (((v0.Color * w0) / v0.Position.w) + ((v1.Color * w1) / v1.Position.w) + ((v2.Color * w2) / v2.Position.w)) * wValue };
								}
								break;
								}

								//write pixel color to backbuffer
								m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
									static_cast<uint8_t>(pixelColor.r * 255),
									static_cast<uint8_t>(pixelColor.g * 255),
									static_cast<uint8_t>(pixelColor.b * 255));
							}
						}
					}
				}
			}
		}
		//increment the index according to which PrimitiveTopology is being used
		if (m_PrimitiveTopology == PrimitiveTopology::TriangleList)
		{
			index += 3;
		}
		else if (m_PrimitiveTopology == PrimitiveTopology::TirangleStrip)
		{
			index++;
		}

		//if the index of the next 3 vertices is outside the indexbuffer size than end the while loop
		if (index + 2 >= indexBuffer.size())
		{
			done = true;
		}
	}

	//change all pixels that still have max dept value to backcolor
	Elite::RGBColor backColor{ .2f,.2f,.2f };
	for (uint32_t r = 0; r < m_Height; ++r)
	{
		for (uint32_t c = 0; c < m_Width; ++c)
		{
			if (deptBuffer[c + (r * size_t(m_Width))] >= (FLT_MAX - FLT_EPSILON))//make sure that the value is basically maximum value
			{
				m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(backColor.r * 255),
					static_cast<uint8_t>(backColor.g * 255),
					static_cast<uint8_t>(backColor.b * 255));
			}
		}
	}

	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

//checks if all 3 vertices are inside the screen
bool Elite::Renderer::VerticesInsideWindow(const Vertex_Ouput& v0, const Vertex_Ouput& v1, const Vertex_Ouput& v2)
{
	bool inside{
	((v0.Position.x > -1.f && v0.Position.x < 1.f) && (v0.Position.y > -1.f && v0.Position.y < 1.f)) &&
	((v1.Position.x > -1.f && v1.Position.x < 1.f) && (v1.Position.y > -1.f && v1.Position.y < 1.f)) &&
	((v2.Position.x > -1.f && v2.Position.x < 1.f) && (v2.Position.y > -1.f && v2.Position.y < 1.f))
	};
	return inside;
}

void Elite::Renderer::BarycentricCoordinates(const Vertex_Ouput& v0, const Vertex_Ouput& v1, const Vertex_Ouput& v2, const Elite::FPoint2& p, float& w0, float& w1, float& w2)
{
	Elite::FVector2 edgeA{ v1.Position.x - v0.Position.x, v1.Position.y - v0.Position.y };
	Elite::FVector2	pointToSide{ p.x - v0.Position.x, p.y - v0.Position.y };
	w2 = { Elite::Cross(pointToSide,edgeA) };

	Elite::FVector2 edgeB{ v2.Position.x - v1.Position.x, v2.Position.y - v1.Position.y };
	pointToSide = { p.x - v1.Position.x, p.y - v1.Position.y };
	w0 = { Elite::Cross(pointToSide,edgeB) };

	Elite::FVector2 edgeC{ v0.Position.x - v2.Position.x, v0.Position.y - v2.Position.y };
	pointToSide = { p.x - v2.Position.x, p.y - v2.Position.y };
	w1 = { Elite::Cross(pointToSide,edgeC) };

	Elite::FVector2 totalTriangl1{ v0.Position.x - v1.Position.x,v0.Position.y - v1.Position.y };

	float totalTriangleArea{ Elite::Cross(edgeC,totalTriangl1) };

	w0 /= totalTriangleArea;
	w1 /= totalTriangleArea;
	w2 /= totalTriangleArea;
}

//calcs the pixel shading
void Elite::Renderer::PixelShading(const Vertex_Ouput& vertex, Elite::RGBColor& pixelColor, const Elite::FVector3& cameraPos, const TextureHolder& textures)
{
	//hardcoded light direction, color and intensity
	Elite::FVector3 lightDirection{ .577f, -.577f, -.577f };
	Elite::RGBColor lightColor{ 1.f, 1.f, 1.f };
	float lightIntensity{ 7.f };
	float shininess{ 25.f };

	//making the ONB matrix for to put the sampled normal in the correct space
	Elite::FVector3 binormal{ Elite::Cross(vertex.Tangent,vertex.Normal) };
	Elite::FMatrix3 tangentSpaceAxis{ vertex.Tangent,binormal,vertex.Normal };
	Elite::FVector3 pixelNormal{};
	if (textures.pNormalMap)//checking if there is a normal map
	{
		//sampling normal from normal map
		Elite::RGBColor sampleColor = textures.pNormalMap->Sample(vertex.UV);
		pixelNormal = { (sampleColor.r * 2.f) - 1.f,(sampleColor.g * 2.f) - 1.f,(sampleColor.b * 2.f) - 1.f };
	}
	else
	{
		//if there is not normal map just take the interpolated vertex normal
		pixelNormal = vertex.Normal;
	}
	pixelNormal = tangentSpaceAxis * pixelNormal;

	//getting the observed area
	float lambertCosine = Elite::Dot(-pixelNormal, lightDirection);
	lambertCosine = Elite::Clamp(lambertCosine, 0.f, 1.f);

	//calculating the lambert diffuse
	Elite::RGBColor lambert{ };
	if (textures.pDiffuse)
	{
		lambert = textures.pDiffuse->Sample(vertex.UV);
		lambert /= float(E_PI);
		lambert = lightColor * lambert * lightIntensity * lambertCosine;
	}
	else { lambert = { 0.f,1.f,1.f }; }//error color meaning there is no diffuse map

	//calculating view direction
	Elite::FVector3 viewDirection{ vertex.WorldPosition.xyz - cameraPos };
	viewDirection = Elite::GetNormalized(viewDirection);

	//gloss and specular sampling
	Elite::RGBColor glossiness{};
	Elite::RGBColor specularStrenght{};
	if (textures.pGlossiness)//checking if there is a gloss map
	{
		glossiness = textures.pGlossiness->Sample(vertex.UV);
	}
	if (textures.pSpecular)//checking if there is a specular map
	{
		specularStrenght = textures.pSpecular->Sample(vertex.UV);
	}

	//calculating the specular phong
	Elite::RGBColor phong{};
	Elite::FVector3 reflectVec{ Elite::Reflect(lightDirection, -vertex.Normal) };

	float tempResult{ Elite::Dot(reflectVec, -viewDirection) };
	tempResult = Elite::Clamp(tempResult, 0.f, 1.f); //claming the result

	float phongColor = std::powf(tempResult, glossiness.r * shininess);
	phong = specularStrenght * phongColor;

	//final light calulation
	pixelColor = lambert + phong;
	pixelColor.MaxToOne();
}

//transforms the vertices to NDC space with a WorldViewProjectionMatrix
void Elite::Renderer::VertexTransformationtoNDC(const TriangleMesh* ptriangleMesh, std::vector<Vertex_Ouput>& transformedVerticesVec, const Camera* pCamera)
{
	Elite::FMatrix4 worldMatrix{ ptriangleMesh->GetWorldMatrix() };

	Elite::FMatrix4 WorldViewProjectionMatrix{ pCamera->GetRightHandedProjectionMatrix() * pCamera->GetInverseLookAtMatrix() * worldMatrix };

	std::vector<Vertex_Input> vertexBuffer{ ptriangleMesh->GetVertexBuffer() };
	Elite::FMatrix4 inverseTransposeWorldMatrix{ Elite::Transpose(Elite::Inverse(worldMatrix)) };

	int indx{};
	for (const Vertex_Input& vertex : vertexBuffer)
	{
		Elite::FPoint4 projectedVertex = WorldViewProjectionMatrix * Elite::FPoint4{ vertex.Position,1.f };
		Elite::FVector4 projectedNormal = inverseTransposeWorldMatrix * Elite::FVector4{ vertex.Normal };
		Elite::FVector4 projectedTangent = inverseTransposeWorldMatrix * Elite::FVector4{ vertex.Tangent };
		Elite::FPoint4 NewWorlPosition = inverseTransposeWorldMatrix * vertex.WorldPosition;
		projectedNormal = Elite::GetNormalized(projectedNormal);
		projectedTangent = Elite::GetNormalized(projectedTangent);

		projectedVertex.x /= projectedVertex.w;
		projectedVertex.y /= projectedVertex.w;
		projectedVertex.z /= projectedVertex.w;
		projectedVertex.w = projectedVertex.w;

		transformedVerticesVec[indx] = (Vertex_Ouput{ projectedVertex, vertex.Color, NewWorlPosition,
			projectedNormal.xyz, projectedTangent.xyz , vertex.UV });
		++indx;
	}
}

//takes the vertices and puts them from NDC to rasterspace
void Elite::Renderer::VertexTransformationtoRasterSpace(Vertex_Ouput& v0, Vertex_Ouput& v1, Vertex_Ouput& v2)
{
	v0.Position.x = ((v0.Position.x + 1) / 2) * m_Width;
	v0.Position.y = ((1 - v0.Position.y) / 2) * m_Height;

	v1.Position.x = ((v1.Position.x + 1) / 2) * m_Width;
	v1.Position.y = ((1 - v1.Position.y) / 2) * m_Height;

	v2.Position.x = ((v2.Position.x + 1) / 2) * m_Width;
	v2.Position.y = ((1 - v2.Position.y) / 2) * m_Height;
}

//checks if the pixel is inside or outside the triangle with 3 cross products
bool Elite::Renderer::InsideOutside(const Vertex_Ouput& v0, const Vertex_Ouput& v1, const Vertex_Ouput& v2, const Elite::FPoint2& p) const
{
	switch (m_CullMode)
	{
	case CullingModes::BackFace:
	{
		Elite::FVector2 edgeA{ v1.Position.x - v0.Position.x,v1.Position.y - v0.Position.y };
		Elite::FVector2	pointToSide{ p.x - v0.Position.x, p.y - v0.Position.y };
		if (Elite::Cross(edgeA, pointToSide) > 0) { return false; }

		Elite::FVector2 edgeB{ v2.Position.x - v1.Position.x, v2.Position.y - v1.Position.y };
		pointToSide = { p.x - v1.Position.x, p.y - v1.Position.y };
		if (Elite::Cross(edgeB, pointToSide) > 0) { return false; }

		Elite::FVector2 edgeC{ v0.Position.x - v2.Position.x, v0.Position.y - v2.Position.y };
		pointToSide = { p.x - v2.Position.x, p.y - v2.Position.y };
		if (Elite::Cross(edgeC, pointToSide) > 0) { return false; }

		return true;
	}
	break;
	case CullingModes::FrontFace:
	{
		Elite::FVector2 edgeA{ v1.Position.x - v0.Position.x,v1.Position.y - v0.Position.y };
		Elite::FVector2	pointToSide{ p.x - v0.Position.x, p.y - v0.Position.y };
		if (Elite::Cross(edgeA, pointToSide) < 0) { return false; }

		Elite::FVector2 edgeB{ v2.Position.x - v1.Position.x, v2.Position.y - v1.Position.y };
		pointToSide = { p.x - v1.Position.x, p.y - v1.Position.y };
		if (Elite::Cross(edgeB, pointToSide) < 0) { return false; }

		Elite::FVector2 edgeC{ v0.Position.x - v2.Position.x, v0.Position.y - v2.Position.y };
		pointToSide = { p.x - v2.Position.x, p.y - v2.Position.y };
		if (Elite::Cross(edgeC, pointToSide) < 0) { return false; }

		return true;
	}
	break;
	case CullingModes::NoCulling:
	{
		bool test1{}, test2{}, test3{};

		Elite::FVector2 edgeA{ v1.Position.x - v0.Position.x,v1.Position.y - v0.Position.y };
		Elite::FVector2	pointToSide{ p.x - v0.Position.x, p.y - v0.Position.y };
		test1 = Elite::Cross(edgeA, pointToSide) < 0;

		Elite::FVector2 edgeB{ v2.Position.x - v1.Position.x, v2.Position.y - v1.Position.y };
		pointToSide = { p.x - v1.Position.x, p.y - v1.Position.y };
		test2 = Elite::Cross(edgeB, pointToSide) < 0;

		Elite::FVector2 edgeC{ v0.Position.x - v2.Position.x, v0.Position.y - v2.Position.y };
		pointToSide = { p.x - v2.Position.x, p.y - v2.Position.y };
		test3 = Elite::Cross(edgeC, pointToSide) < 0;

		return (test1 == test2 && test2 == test3);
	}
	break;
	}

	return true;
}

//switches between different render modes atm they are just texture sampling and shading, just diffuse texture sampling without shading, deptbuffer, and the vertex colors
void Elite::Renderer::SwitchSoftwareRenderMode()
{
	++m_RenderMode;
	if (m_RenderMode >= m_MaxRenderModes)
	{
		m_RenderMode = 0;
	}
}

//to switch between the different cull mode for software rasterizer
void Elite::Renderer::ChangeCullMode()
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
}
#pragma endregion