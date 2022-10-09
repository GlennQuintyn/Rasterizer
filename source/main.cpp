#include "pch.h"
//#undef main

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"
#include "Camera.h"
#include "Texture.h"
#include "ObjReader.h"

#include "MeshOpaque.h"
#include "MeshOpacity.h"
#include "TriangleMesh.h"

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;
	SDL_Window* pWindow = SDL_CreateWindow(
		"ExamProject - Glenn Quintyn",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//objectReader
	ObjReader objReader{};

	//change the primitive topology mode
	PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleList };

	//Initialize "framework"
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow, primitiveTopology) };

#pragma region DirectX Init
	//variables
	MeshOpaque* pMeshVechicle{};
	Texture* pDiffuseTexture{};
	Texture* pNormalTexture{};
	Texture* pSpecularTexture{};
	Texture* pGlossInessTexture{};

	MeshOpacity* pMeshFlireExhaust{};
	Texture* pDiffuseFireTexture{};

	std::vector<Vertex_Input> directXVertices;
	std::vector<uint32_t> directXIndices;
	objReader.SetInverseZMode(true);
	objReader.ParseObjFile("Resources/vehicle.obj", directXVertices, directXIndices);

	std::vector<Vertex_Input> fireVertices;
	std::vector<uint32_t> fireIndices;
	objReader.ParseObjFile("Resources/fireFX.obj", fireVertices, fireIndices);

	//if DirectX initialized properly then get device and construct mesh
	if (pRenderer->IsInitizalized())
	{
		pDiffuseTexture = new Texture{ pRenderer->GetDevice() ,"Resources/vehicle_diffuse.png" };
		pNormalTexture = new Texture{ pRenderer->GetDevice() ,"Resources/vehicle_normal.png" };
		pSpecularTexture = new Texture{ pRenderer->GetDevice() ,"Resources/vehicle_specular.png" };
		pGlossInessTexture = new Texture{ pRenderer->GetDevice() ,"Resources/vehicle_gloss.png" };
		pMeshVechicle = new MeshOpaque{ pRenderer->GetDevice(), directXVertices , directXIndices };

		pMeshFlireExhaust = new MeshOpacity{ pRenderer->GetDevice(), fireVertices , fireIndices };
		pDiffuseFireTexture = new Texture{ pRenderer->GetDevice() , "Resources/fireFX_diffuse.png" };
	}
#pragma endregion

#pragma region Software Rasterizer Init
	std::vector<Vertex_Input> softwareVertexBuffer{};
	std::vector<uint32_t> softwareIndexBuffer{};

	TriangleMesh* pTriangleMesh{};
	Texture* pDiffuse{};
	Texture* pNormal{};
	Texture* PGloss{};
	Texture* pSpecular{};

	if (pRenderer->IsInitizalized())
	{
		pDiffuse = { new Texture{pRenderer->GetDevice(), "Resources/vehicle_diffuse.png"} };
		pNormal = { new Texture{pRenderer->GetDevice(), "Resources/vehicle_normal.png"} };
		PGloss = { new Texture{pRenderer->GetDevice(), "Resources/vehicle_gloss.png"} };
		pSpecular = { new Texture{pRenderer->GetDevice(), "Resources/vehicle_specular.png"} };
		//object file reading and parsing
		objReader.SetInverseZMode(false);
		objReader.ParseObjFile("Resources/vehicle.obj", softwareVertexBuffer, softwareIndexBuffer);

		pTriangleMesh = new TriangleMesh{ softwareVertexBuffer, softwareIndexBuffer, TextureHolder{pDiffuse, pNormal, PGloss, pSpecular} };
	}
	else
	{
		std::cout << "No device available!\n";
		return 0;
	}
#pragma endregion


	//camera
	Elite::FVector3 cameraPos{ 0, 0, 50.f };
	float FOV{ 60.f }, FovScaler{ 25 };
	Camera* pCamera = new Camera{ cameraPos, FOV, width, height, 0.1f, 100.f };
	int mouseRelX{}, mouseRelY{};

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	float rotationScaler{ 50 };

	//Extra controls output to console
	std::cout << "\t\t-=GENERAL CONTROLS=-\n";
	std::cout << " Hold LMB: to rota camera left/right or go forwad/backwards\n";
	std::cout << " Hold RMB: to rota camera left/right or go forwad/backwards\n";
	std::cout << " W/S: to go forwad/backwards respcetivly (this is relative to the camera foward)\n";
	std::cout << " A/D: to go left/right respcetivly (this is relative to the camera foward)\n";
	std::cout << " Q/E: to go down/up relative to the camera foward\n";

	std::cout << "\n\t\t-=EXTRA CONTROLS=-\n";
	std::cout << " R  : to toggle between switch between Software or DirectX rasterizer\n";
	std::cout << " F  : to change filtering mode (point, linear, anisotropic) DirectX ONLY!!!\n";
	std::cout << " T  : to toggle transparency DirectX ONLY!!!\n";
	std::cout << " C  : to toggle between toggle between cullmodes (front, back and no culling)\n";
	std::cout << " +/-: to increase/decrease the FOV (clamped between 0-180)\n";
	std::cout << " Z  : to toggle between visualizing modes (lambert phong, only diffuse sample, dept buffer, triangle vertex color interpolation)"
		"SOFTWARE MODE ONLY!!!\n";

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				switch (e.key.keysym.sym)
				{
				case SDLK_r://switch between Software or DirectX rasterizer
					std::cout << "Changing General Render mode\n";
					pRenderer->ToggleRenderMode();
					pCamera->ToggleHandedMode();
					break;
				case SDLK_f://change filtering mode (point, linear, anisotropic)(DX only)
					std::cout << "Changed Sample Technique\n";
					pMeshVechicle->ChangeSampleTech();
					pMeshFlireExhaust->ChangeSampleTech();
					break;
				case SDLK_t://toggle transparency (DX only)
					std::cout << "Toggling Transparency\n";
					pMeshFlireExhaust->ToggleTransparency(pRenderer->GetDevice());
					break;
				case SDLK_c://toggle between cullmodes
					std::cout << "Changing cull mode\n";
					pMeshVechicle->ChangeCullMode(pRenderer->GetDevice());
					pRenderer->ChangeCullMode();
					break;
				case SDLK_z://toggle Render modes 
					std::cout << "Changing Software Render mode\n";
					pRenderer->SwitchSoftwareRenderMode();
					break;
				}
				break;
			}
		}

#pragma region Keyboard and mouse inputs
		const Uint8* pKeyboardStates = SDL_GetKeyboardState(nullptr);
		const Uint32 mouseStates = SDL_GetRelativeMouseState(&mouseRelX, &mouseRelY);

		/// 
		///keyboard
		/// 
		if (pKeyboardStates[SDL_SCANCODE_W]) { pCamera->UpdateKeyboardMovement({ 0,0,-1 }, pTimer->GetElapsed()); }
		else if (pKeyboardStates[SDL_SCANCODE_S]) { pCamera->UpdateKeyboardMovement({ 0,0,1 }, pTimer->GetElapsed()); }
		if (pKeyboardStates[SDL_SCANCODE_A]) { pCamera->UpdateKeyboardMovement({ -1,0,0 }, pTimer->GetElapsed()); }
		else if (pKeyboardStates[SDL_SCANCODE_D]) { pCamera->UpdateKeyboardMovement({ 1,0,0 }, pTimer->GetElapsed()); }
		if (pKeyboardStates[SDL_SCANCODE_Q]) { pCamera->UpdateMouseBothDown(0, -1, pTimer->GetElapsed()); }
		else if (pKeyboardStates[SDL_SCANCODE_E]) { pCamera->UpdateMouseBothDown(0, 1, pTimer->GetElapsed()); }
		if (pKeyboardStates[SDL_SCANCODE_KP_PLUS]) { if (FOV < 179) { FOV += FovScaler * pTimer->GetElapsed(); pCamera->SetFOV(FOV); } }
		else if (pKeyboardStates[SDL_SCANCODE_KP_MINUS]) { if (FOV > 1) { FOV -= FovScaler * pTimer->GetElapsed(); pCamera->SetFOV(FOV); } }

		/// 
		///mouse
		/// 
		if ((mouseStates & SDL_BUTTON(SDL_BUTTON_LEFT)) && (mouseStates & SDL_BUTTON(SDL_BUTTON_RIGHT)))
		{
			pCamera->UpdateMouseBothDown(mouseRelX, mouseRelY, pTimer->GetElapsed());
		}
		else if (mouseStates == SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			pCamera->UpdateMouseLeftDown(mouseRelX, mouseRelY, pTimer->GetElapsed());
		}
		else if (mouseStates == SDL_BUTTON(SDL_BUTTON_RIGHT))
		{
			pCamera->UpdateMouseRightDown(mouseRelX, mouseRelY, pTimer->GetElapsed());
		}
#pragma endregion

		//--------- Render ---------
		pRenderer->Render(pMeshVechicle, pMeshFlireExhaust, pCamera, pDiffuseTexture, pNormalTexture, pSpecularTexture, pGlossInessTexture, pDiffuseFireTexture, pTriangleMesh);

		pMeshVechicle->RotateMeshXYZ(0, rotationScaler * pTimer->GetElapsed(), 0);
		pMeshFlireExhaust->RotateMeshXYZ(0, rotationScaler * pTimer->GetElapsed(), 0);
		pTriangleMesh->RotateMeshXYZ(0, -rotationScaler * pTimer->GetElapsed(), 0);

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "FPS: " << pTimer->GetFPS() <<
				"\ndelta T: " << pTimer->GetElapsed() << std::endl;
		}
	}
	pTimer->Stop();

	//Clean up 
	delete pMeshVechicle;
	delete pDiffuseTexture;
	delete pNormalTexture;
	delete pSpecularTexture;
	delete pGlossInessTexture;

	delete pMeshFlireExhaust;
	delete pDiffuseFireTexture;

	delete pTriangleMesh;

	//Shutdown "framework"
	ShutDown(pWindow);
	return 0;
}