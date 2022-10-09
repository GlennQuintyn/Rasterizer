#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

class ObjReader
{
public:
	ObjReader() = default;
	~ObjReader() = default;

	void SetInverseZMode(bool boolean) { m_InverseZ = boolean; };
	void ParseObjFile(std::string fileNamePath, std::vector<Vertex_Input>& vertexBuffer, std::vector<uint32_t>& indexBuffer);

private:
	void ProcessLine(const std::string& line, std::vector<Vertex_Input>& vertexBuffer, std::vector<uint32_t>& indexBuffer);

	std::vector<Elite::FPoint3> m_VertexVec;
	std::vector<Elite::FVector3> m_NormalVec;
	std::vector<Elite::FPoint2> m_UvVec;

	bool m_InverseZ{};
};

void ObjReader::ParseObjFile(std::string fileNamePath, std::vector<Vertex_Input>& vertexBuffer, std::vector<uint32_t>& indexBuffer)
{
	// Open the file
	std::ifstream objFile(fileNamePath.c_str());

	if (!objFile)
	{
		std::cerr << "Failed to load file " << fileNamePath << std::endl;
		return;// false;
	}

	//clear vectors
	m_VertexVec.clear();
	m_NormalVec.clear();
	m_UvVec.clear();

	// Read the file
	std::string objLine;
	while (!objFile.eof())
	{
		getline(objFile, objLine);
		ProcessLine(objLine, vertexBuffer, indexBuffer);
	}


	/// making the tangents from source(s)
	///https://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal
	/// https://www.amazon.com/Foundations-Game-Engine-Development-Rendering/dp/0985811757/ref=pd_lpo_14_t_0/143-1165761-6756566?_encoding=UTF8&pd_rd_i=0985811757&pd_rd_r=19e74d20-d153-4908-bd12-0cca375cab65&pd_rd_w=JFZty&pd_rd_wg=qpwzW&pf_rd_p=7b36d496-f366-4631-94d3-61b87b52511b&pf_rd_r=E5CPZJ1G3EYT2D1NTF0H&psc=1&refRID=E5CPZJ1G3EYT2D1NTF0H
	for (uint32_t index = 0; index < indexBuffer.size(); index += 3)
	{
		//getting indexes
		uint32_t index0{ indexBuffer[index] };
		uint32_t index1{ indexBuffer[size_t(index) + 1] };
		uint32_t index2{ indexBuffer[size_t(index) + 2] };

		const Elite::FPoint3& p0 = vertexBuffer[index0].Position;
		const Elite::FPoint3& p1 = vertexBuffer[index1].Position;
		const Elite::FPoint3& p2 = vertexBuffer[index2].Position;
		const Elite::FVector2& uv0 = vertexBuffer[index0].UV;
		const Elite::FVector2& uv1 = vertexBuffer[index1].UV;
		const Elite::FVector2& uv2 = vertexBuffer[index2].UV;

		const Elite::FVector3 edge0 = p1 - p0;
		const Elite::FVector3 edge1 = p2 - p0;
		const Elite::FVector2 diffX = Elite::FVector2(uv1.x - uv0.x, uv2.x - uv0.x);
		const Elite::FVector2 diffY = Elite::FVector2(uv1.y - uv0.y, uv2.y - uv0.y);
		float r = 1.f / Elite::Cross(diffX, diffY);

		const Elite::FVector3 tangent = ((edge0 * diffY.y) - (edge1 * diffY.x)) * r;
		vertexBuffer[index0].Tangent += tangent;
		vertexBuffer[index1].Tangent += tangent;
		vertexBuffer[index2].Tangent += tangent;
	}
	//Create the tangents (reject vector) + fix the tangents per vertex
	for (auto& vertex : vertexBuffer)
	{
		vertex.Tangent = Elite::GetNormalized(Elite::Reject(vertex.Tangent, vertex.Normal));
		if (m_InverseZ)
		{
			vertex.Tangent = { vertex.Tangent.xy, -vertex.Tangent.z };
		}
		else
		{
			vertex.Tangent = vertex.Tangent;
		}
	}

	// close the file
	objFile.close();
}

void ObjReader::ProcessLine(const std::string& line, std::vector<Vertex_Input>& vertexBuffer, std::vector<uint32_t>& indexBuffer)
{
	if (!line.empty())
	{
		std::string objFeature{ line.substr(0, line.find(' ')) };

		if (objFeature == "v")//Vertices
		{
			uint64_t posBeginFloat{ line.find(' ') + 2 };//+2 because there are 2 spaces after the v
			uint64_t posEndFloat{ line.find(' ',posBeginFloat) };
			std::string number{ line.substr(posBeginFloat,posEndFloat - posBeginFloat) };
			float vertex1{ std::stof(number) };

			posBeginFloat = posEndFloat + 1;
			posEndFloat = line.find(' ', posBeginFloat);
			number = { line.substr(posBeginFloat,posEndFloat - posBeginFloat) };
			float vertex2{ std::stof(number) };

			posBeginFloat = posEndFloat + 1;
			posEndFloat = line.find(' ', posBeginFloat);
			number = { line.substr(posBeginFloat,posEndFloat - posBeginFloat) };
			float vertex3{ std::stof(number) };

			if (m_InverseZ)
			{
				m_VertexVec.push_back(Elite::FPoint3{ vertex1, vertex2, -vertex3 });
			}
			else
			{
				m_VertexVec.push_back(Elite::FPoint3{ vertex1, vertex2, vertex3 });
			}			
		}
		else if (objFeature == "f")//faces
		{
			//get Vertext1 indices
#pragma region Vertext1
			uint64_t posBegin{ line.find(' ') + 1 };//+1 because there is one space after f
			uint64_t posEnd{ line.find('/', posBegin) };
			std::string number{ line.substr(posBegin,posEnd - posBegin) };
			int FaceVertex1{ std::stoi(number) };

			posBegin = posEnd + 1;
			posEnd = line.find('/', posBegin);
			number = { line.substr(posBegin,posEnd - posBegin) };
			int FaceVertexUv1{ std::stoi(number) };

			posBegin = posEnd + 1;
			posEnd = line.find(' ', posBegin);
			number = { line.substr(posBegin,posEnd - posBegin) };
			int FaceVertexNormal1{ std::stoi(number) };
#pragma endregion

			//get Vertext2 indices
#pragma region Vertext2
			posBegin = line.find(' ', posEnd) + 1;
			posEnd = line.find('/', posBegin - 1);
			number = { line.substr(posBegin,posEnd - posBegin) };
			int FaceVertex2{ std::stoi(number) };

			posBegin = posEnd + 1;
			posEnd = line.find('/', posBegin);
			number = { line.substr(posBegin,posEnd - posBegin) };
			int FaceVertexUv2{ std::stoi(number) };

			posBegin = posEnd + 1;
			posEnd = line.find(' ', posBegin);
			number = { line.substr(posBegin,posEnd - posBegin) };
			int FaceVertexNormal2{ std::stoi(number) };
#pragma endregion

			//get Vertext3 indices
#pragma region Vertext3
			posBegin = line.find(' ', posEnd) + 1;
			posEnd = line.find('/', posBegin - 1);
			number = { line.substr(posBegin,posEnd - posBegin) };
			int FaceVertex3{ std::stoi(number) };

			posBegin = posEnd + 1;
			posEnd = line.find('/', posBegin);
			number = { line.substr(posBegin,posEnd - posBegin) };
			int FaceVertexUv3{ std::stoi(number) };

			posBegin = posEnd + 1;
			posEnd = line.find(' ', posBegin);
			number = { line.substr(posBegin,posEnd - posBegin) };
			int FaceVertexNormal3{ std::stoi(number) };
#pragma endregion

			//lambda that checks if a duplicate exists
			const auto checkDuplicate = [&vertexBuffer, &indexBuffer](Vertex_Input& tempVertex)
			{
				//search for the duplicate and return iterator
				auto duplicateIT = std::find(vertexBuffer.begin(), vertexBuffer.end(), tempVertex);
				//if iterator points to the end of the vector that means no dubplicate has been found
				if (duplicateIT == vertexBuffer.end())
				{
					vertexBuffer.push_back(tempVertex);
					indexBuffer.push_back(int(vertexBuffer.size() - 1));
				}
				else
				{
					//get index of the vertex in the vertexbuffer to put in the indexbuffer
					int indexOfDuplicate{ int(std::distance(vertexBuffer.begin(), duplicateIT)) };
					indexBuffer.push_back(indexOfDuplicate);
				}
			};
			//NUMBER -1 BECAUSE Obj INDICES START AT 1

			//giving each vertex a random color to give nothing more than an interesting effect when switching render mode to vertex color interpolation
			Elite::RGBColor color1{ Elite::RandomFloat(), Elite::RandomFloat() ,Elite::RandomFloat() };
			Elite::RGBColor color2{ Elite::RandomFloat(), Elite::RandomFloat() ,Elite::RandomFloat() };
			Elite::RGBColor color3{ Elite::RandomFloat(), Elite::RandomFloat() ,Elite::RandomFloat() };

			//pos						  , color , worldpos					 , normal							 , tangent, UV
			Vertex_Input tempVertex1{ m_VertexVec[FaceVertex1 - 1], color1, m_VertexVec[FaceVertex1 - 1], m_NormalVec[FaceVertexNormal1 - 1], {0,0,0}, Elite::FVector2{m_UvVec[FaceVertexUv1 - 1]} };
			Vertex_Input tempVertex2{ m_VertexVec[FaceVertex2 - 1], color2, m_VertexVec[FaceVertex2 - 1], m_NormalVec[FaceVertexNormal2 - 1], {0,0,0}, Elite::FVector2{m_UvVec[FaceVertexUv2 - 1]} };
			Vertex_Input tempVertex3{ m_VertexVec[FaceVertex3 - 1], color3, m_VertexVec[FaceVertex3 - 1], m_NormalVec[FaceVertexNormal3 - 1], {0,0,0}, Elite::FVector2{m_UvVec[FaceVertexUv3 - 1]} };

			//vertexBuffer.push_back(tempVertex1);
			//indexBuffer.push_back(int(vertexBuffer.size() - 1));
			//vertexBuffer.push_back(tempVertex2);
			//indexBuffer.push_back(int(vertexBuffer.size() - 1));
			//vertexBuffer.push_back(tempVertex3);
			//indexBuffer.push_back(int(vertexBuffer.size() - 1));


			checkDuplicate(tempVertex1);
			checkDuplicate(tempVertex2);
			checkDuplicate(tempVertex3);
		}
		else if (objFeature == "vn")//vertex normal
		{
			uint64_t posBeginFloat{ line.find(' ') + 1 };//+1 because there is 1 spaces after the vn
			uint64_t posEndFloat{ line.find(' ',posBeginFloat) };
			std::string number{ line.substr(posBeginFloat,posEndFloat - posBeginFloat) };
			float vertexNormal1{ std::stof(number) };

			posBeginFloat = posEndFloat + 1;
			posEndFloat = line.find(' ', posBeginFloat);
			number = { line.substr(posBeginFloat,posEndFloat - posBeginFloat) };
			float vertexNormal2{ std::stof(number) };

			posBeginFloat = posEndFloat + 1;
			posEndFloat = line.find(' ', posBeginFloat);
			number = { line.substr(posBeginFloat,posEndFloat - posBeginFloat) };
			float vertexNormal3{ std::stof(number) };

			if (m_InverseZ)
			{
				m_NormalVec.push_back(Elite::FVector3{ vertexNormal1, vertexNormal2, -vertexNormal3 });
			}
			else
			{
				m_NormalVec.push_back(Elite::FVector3{ vertexNormal1, vertexNormal2, vertexNormal3 });
			}			
		}
		else if (objFeature == "vt")//Vertex texture coordinate
		{
			uint64_t posBeginFloat{ line.find(' ') + 1 };//+1 because there is 1 spaces after the vt
			uint64_t posEndFloat{ line.find(' ',posBeginFloat) };
			std::string number{ line.substr(posBeginFloat,posEndFloat - posBeginFloat) };
			float uvX{ std::stof(number) };

			posBeginFloat = posEndFloat + 1;
			posEndFloat = line.find(' ', posBeginFloat);
			number = { line.substr(posBeginFloat,posEndFloat - posBeginFloat) };
			float uvY{ 1 - std::stof(number) };

			m_UvVec.push_back(Elite::FPoint2{ uvX, uvY });
		}
	}
}