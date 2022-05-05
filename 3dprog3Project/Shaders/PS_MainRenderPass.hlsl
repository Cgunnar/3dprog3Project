struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 posWorld : WORLD_POS;
	float4 normal : NORMAL;
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

struct PointLight
{
	float3 position;
	float strength;
	float3 color;
	float constAtt;
	float linAtt;
	float expAtt;
};
StructuredBuffer<PointLight> dynamicPointLights : register(t0);


float4 main(VS_OUT input) : SV_TARGET
{
	float3 outputColor = float3(0,0,0);
	unsigned int lightSize = 0;
	unsigned int numLights = 0;
	dynamicPointLights.GetDimensions(numLights, lightSize);
	for (int i = 0; i < numLights; i++)
	{
		float3 dirToLight = normalize(dynamicPointLights[i].position - input.posWorld.xyz);
		float diffFactor = dot(input.normal.xyz, dirToLight);

		outputColor += diffFactor * color * dynamicPointLights[i].color;
	}
	return float4(outputColor, 1.0f);
}