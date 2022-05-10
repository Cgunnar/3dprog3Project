struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 posWorld : WORLD_POS;
	float4 normal : NORMAL;
	float2 uv : UV;
	int materialID : MATERIAL_ID;
};

cbuffer CameraCB : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 viewMatrix;
	float4x4 viewProjectionMatrix;
	float3 cameraPosition;
}

struct Material
{
	float4 albedoFactor;
	float4 emissionFactor;
	int albedoTextureIndex;
};

ConstantBuffer<Material> materials[]: register(b0, space2);

struct PointLight
{
	float3 position;
	float strength;
	float3 color;
	float constAtt;
	float linAtt;
	float expAtt;
};
SamplerState anisotropicSampler : register(s0);
StructuredBuffer<PointLight> dynamicPointLights : register(t0);
Texture2D albedoMap[] : register(t0, space1);

float Attenuate(float length, float constAtt, float linAtt , float expAtt)
{
	return 1.0f / (constAtt + linAtt * length + expAtt * length * length);
}

float4 CalcLightForTexturedMaterial(float3 pos, float3 normal, float2 uv, int matID)
{
	float3 outputColor = float3(0, 0, 0);
	float4 albedo;
	Material mat = materials[NonUniformResourceIndex(matID)];
	
	if(mat.albedoTextureIndex != -1)
		albedo = mat.albedoFactor * albedoMap[NonUniformResourceIndex(mat.albedoTextureIndex)].Sample(anisotropicSampler, uv).rgba;
	else
		albedo = mat.albedoFactor;

	unsigned int lightSize = 0;
	unsigned int numLights = 0;
	dynamicPointLights.GetDimensions(numLights, lightSize);
	for (int i = 0; i < numLights; i++)
	{
		PointLight pl = dynamicPointLights[i];
		float3 vecToLight = pl.position - pos;
		float3 dirToLight = normalize(vecToLight);

		float diffFactor = saturate(dot(normal, dirToLight));
		float3 r = normalize(reflect(-dirToLight, normal));
		float3 v = normalize(cameraPosition - pos);
		float specFactor = pow(saturate(dot(r, v)), 1000);

		float3 inLight = pl.color * pl.strength;
		float3 spec = specFactor * inLight;
		float3 diff = diffFactor * albedo.rgb * inLight;

		outputColor += (diff + spec) * Attenuate(length(vecToLight), pl.constAtt, pl.linAtt, pl.expAtt);
	}
	outputColor += mat.emissionFactor.rgb;
	outputColor += 0.2f * albedo.rgb;
	return float4(saturate(outputColor), albedo.a);
}


float4 main(VS_OUT input) : SV_TARGET
{
	float4 outputColor = float4(0,0,0,0);
	outputColor = CalcLightForTexturedMaterial(input.posWorld.xyz, input.normal.xyz, input.uv, input.materialID);
	return outputColor;
}