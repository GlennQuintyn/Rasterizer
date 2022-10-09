//global variables
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WORLD;
float4x4 gInvViewMatrix : VIEWINVERSE;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float3 gLightDirection : LightDirection = { 0.577f, -0.577f, 0.577f };
float3 gLightColor : LightColor = { 1.f, 1.f, 1.f };
float gPI : PI = 3.1415927f;
float gLightIntesity : LightIntensity = 7.0f;
float gShininess : Shininess = 25.0f;

//-------------------------------------
//	Input/Output Structs
//-------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float4 WorldPosition : WORLDPOS;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
	float4 WorldPosition : WORLDPOS;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoord : TEXCOORD;
};


//-------------------------------------
//	Different States
//-------------------------------------
//RasterizerState gRasterizerState
//{
//	CullMode = back;
//	FrontCounterClockWise = true;
//};
RasterizerState gRasterizerState;

BlendState gBlendState
{
	BlendEnable[0] = false;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	StencilEnable = true;
};

//-------------------------------------
//	Pixel Sampler Settings
//-------------------------------------
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Border; // or "Mirror", "Clamp", "Border"
	AddressV = Clamp; // or "Mirror", "Clamp", "Border"
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border; // or "Mirror", "Clamp", "Border"
	AddressV = Clamp; // or "Mirror", "Clamp", "Border"
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Border; // or "Mirror", "Clamp", "Border"
	AddressV = Clamp; // or "Mirror", "Clamp", "Border"
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

//-------------------------------------
//	Vertex Shader
//-------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.Color = input.Color;
	output.WorldPosition = mul(input.WorldPosition, gWorldMatrix);
	output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);
	output.TexCoord = input.TexCoord;
	return output;
}

//-------------------------------------
//	funtions
//-------------------------------------
float3 CalcNormal(VS_OUTPUT input, SamplerState sampleState)
{
	//Calculate local axis for normal mapping (tangent space)
	float3 binormal = cross(input.Normal, input.Tangent);
	float3x3 tangentSpaceAxis = { input.Tangent, binormal, input.Normal };
	
	//making the ONB matrix for to put the sampled normal in the correct space
	float4 pixelSample = (gNormalMap.Sample(sampleState, input.TexCoord));
	float3 pixelNormal = { (pixelSample.r * 2.f) - 1.f, (pixelSample.g * 2.f) - 1.f, (pixelSample.b * 2.f) - 1.f };
	
	return mul(pixelNormal, tangentSpaceAxis);
}

float3 CalcLambertDiffuse(VS_OUTPUT input, float lambertCosine, SamplerState sampleState)
{
	float4 diffuseSample = (gDiffuseMap.Sample(sampleState, input.TexCoord));
	diffuseSample /= gPI;
	return gLightColor * diffuseSample.rgb * gLightIntesity * lambertCosine;
}

float3 CalcPhong(VS_OUTPUT input, SamplerState sampleState)
{
	float3 reflectVec = reflect(gLightDirection, -input.Normal);
	
	//getting the view direction
	float3 viewDirection = normalize(input.WorldPosition.xyz - gInvViewMatrix[3].xyz);
		
	reflectVec = mul(reflectVec, -viewDirection);
	reflectVec = saturate(reflectVec); //claming the result
	
	//sampling the maps
	float4 specularSample = (gSpecularMap.Sample(sampleState, input.TexCoord));
	float4 glossinessSample = (gGlossinessMap.Sample(sampleState, input.TexCoord));
		
	float3 phongColor = pow(reflectVec, glossinessSample.r * gShininess);
	
	return specularSample.rgb * phongColor;
}

float4 CalcLambertPhongShading(VS_OUTPUT input, SamplerState sampleState)
{
	float4 finalColor;
	
	float3 pixelNormal = CalcNormal(input, sampleState);
	
	//getting the observed area
	float lambertCosine = mul(-pixelNormal, gLightDirection);
	lambertCosine = saturate(lambertCosine);
	
	//calculating the diffuse
	float3 diffuseColor = CalcLambertDiffuse(input, lambertCosine, sampleState);
	
	//calculating Phong
	float3 phongColor = CalcPhong(input, sampleState);

	//Final color calculation
	finalColor.rgb = diffuseColor + phongColor;
	finalColor.a = 1;
	finalColor = saturate(finalColor);
		
	return finalColor;
}

//-------------------------------------
//	Pixel Shader(s)
//-------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET // vertex color sampling
{
	return float4(input.Color, 1.f);
}

float4 PS_P(VS_OUTPUT input) : SV_TARGET // for point sampling
{
	//calculates the final color with lambert diffuse and phong specular and returns this value
	return CalcLambertPhongShading(input, samPoint);
}

float4 PS_L(VS_OUTPUT input) : SV_TARGET //for linear sampling
{
	//calculates the final color with lambert diffuse and phong specular and returns this value
	return CalcLambertPhongShading(input, samLinear);
}

float4 PS_A(VS_OUTPUT input) : SV_TARGET //for Anisotropic sampling
{
	//calculates the final color with lambert diffuse and phong specular and returns this value
	return CalcLambertPhongShading(input, samAnisotropic);
}

//-------------------------------------
//	Technique(s)
//-------------------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 PointTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_P()));
	}
}

technique11 LinearTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_L()));
	}
}

technique11 AnisotropicTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_A()));
	}
}
