struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 uv : UV;
};

cbuffer ColorBuffer : register(b0)
{
	float4 color;
}

float4 main(VS_OUT input) : SV_TARGET
{
	return color;
}