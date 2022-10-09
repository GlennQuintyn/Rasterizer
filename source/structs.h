#pragma once
#include "EMath.h"
#include "ERGBColor.h"
#include "Texture.h"

//-------------------------------------
//	Input/Output Structs
//-------------------------------------
struct Vertex_Input
{
	Elite::FPoint3 Position;
	Elite::RGBColor Color;
	Elite::FPoint4 WorldPosition;
	Elite::FVector3 Normal;
	Elite::FVector3 Tangent;
	Elite::FVector2 UV;

	//overloading the == operator to be able to compare to other vertices to find duplicates
	bool operator==(const Vertex_Input& other) const noexcept
	{
		return
		{
			this->Position == other.Position &&
			this->Normal == other.Normal &&
			//the color does not need to be checked if its the same
			//nor does the tangent because that gets constructed after checking for duplicates
			//and the worldposition is the same as position
			this->UV == other.UV
		};
	}

};

struct Vertex_Ouput
{
	Elite::FPoint4 Position;
	Elite::RGBColor Color;
	Elite::FPoint4 WorldPosition;
	Elite::FVector3 Normal;
	Elite::FVector3 Tangent;
	Elite::FVector2 UV;
};

//-------------------------------------
//	Extra Structs
//-------------------------------------
enum class PrimitiveTopology
{
	TriangleList,
	TirangleStrip
};

enum class CullingModes
{
	BackFace,
	FrontFace,
	NoCulling
};

struct TextureHolder
{
	//constructor with default nullptr
	TextureHolder(const Texture* pDiffuse = nullptr, const Texture* pNormalMap = nullptr, const Texture* pGlossiness = nullptr, const Texture* pSpecular = nullptr)
		: pDiffuse{ pDiffuse }
		, pNormalMap{ pNormalMap }
		, pGlossiness{ pGlossiness }
		, pSpecular{ pSpecular }
	{
	}

	~TextureHolder()
	{
		//safe delete
		if (pDiffuse)
		{
			delete pDiffuse;
			pDiffuse = nullptr;
		}
		if (pNormalMap)
		{
			delete pNormalMap;
			pNormalMap = nullptr;
		}
		if (pGlossiness)
		{
			delete pGlossiness;
			pGlossiness = nullptr;
		}
		if (pSpecular)
		{
			delete pSpecular;
			pSpecular = nullptr;
		}
	}

	//rule of five with only move constructor enabled
	TextureHolder(const TextureHolder&) = delete;
	TextureHolder& operator=(const TextureHolder&) = delete;
	TextureHolder& operator=(TextureHolder&&) noexcept = delete;
	TextureHolder(TextureHolder&& other) noexcept
		: pDiffuse{ other.pDiffuse }
		, pNormalMap{ other.pNormalMap }
		, pGlossiness{ other.pGlossiness }
		, pSpecular{ other.pSpecular }
	{
		other.pDiffuse = nullptr;
		other.pNormalMap = nullptr;
		other.pGlossiness = nullptr;
		other.pSpecular = nullptr;
	}

	//members
	const Texture* pDiffuse;
	const Texture* pNormalMap;
	const Texture* pGlossiness;
	const Texture* pSpecular;
};