//global variables
float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
BlendState gBlendState : BlendingState;

//-------------------------------------
//	Input/Output Structs
//-------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
	float2 TexCoord : TEXCOORD;
};

//-------------------------------------
//	Different States
//-------------------------------------
RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockWise = true;
};

//BlendState gBlendStateDisabled
//{
//	BlendEnable[0] = false;
//};

//BlendState gBlendState
//{
//	BlendEnable[0] = true;
//	SrcBlend = src_alpha;
//	DestBlend = inv_src_alpha;
//	BlendOP = add;
//	SrcBlendAlpha = zero;
//	DestBlendAlpha = zero;
//	BlendOpAlpha = add;
//	RenderTargetWriteMask[0] = 0x0F;
//};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	depthFunc = less;
	StencilEnable = false;

	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
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
	output.TexCoord = input.TexCoord;
	return output;
}

//-------------------------------------
//	Pixel Shader(s)
//-------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET // vertex color sampling
{
	//float3 viewDirection = normalize(input.WorldPosition.xyz - gInvViewMatrix[3].xyz);
	return float4(input.Color, 1.f);
}

float4 PS_P(VS_OUTPUT input) : SV_TARGET // for point sampling
{
	//calculates the final color with lambert diffuse and phong specular and returns this value
	return gDiffuseMap.Sample(samPoint, input.TexCoord);
	//return CalcLambertPhongShading(input, samPoint);
}

float4 PS_L(VS_OUTPUT input) : SV_TARGET //for linear sampling
{
	//calculates the final color with lambert diffuse and phong specular and returns this value
	return gDiffuseMap.Sample(samLinear, input.TexCoord);
	//return CalcLambertPhongShading(input, samLinear);
}

float4 PS_A(VS_OUTPUT input) : SV_TARGET //for Anisotropic sampling
{
	//calculates the final color with lambert diffuse and phong specular and returns this value
	return gDiffuseMap.Sample(samAnisotropic, input.TexCoord);
	//return CalcLambertPhongShading(input, samAnisotropic);
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
