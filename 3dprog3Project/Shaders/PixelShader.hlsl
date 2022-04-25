struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 posWorld : WORLD_POS;
	float2 uv : UV;
};

cbuffer CameraCB : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 viewMatrix;
	float4x4 viewProjectionMatrix;
	float3 cameraPosition;
}

cbuffer ColorBuffer : register(b1)
{
	float4 color;
}

float4 main(VS_OUT input) : SV_TARGET
{
	return color;
}