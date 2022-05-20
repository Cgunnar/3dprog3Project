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
RaytracingAccelerationStructure accelerationStructure : register(t1);
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
	
	//https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html
	//if no texture exists, texture is assumed to gave value 1 for each pixel => factor is the color
	//this is true for the roughness and the other pbr materials as well.
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
		
        RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
             RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
             RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
        RayDesc ray;
        ray.Origin = pos;
        ray.Direction = dirToLight;
        ray.TMin = 1.0f;
        ray.TMax = length(vecToLight) - 0.21; //pointlights has a sphere mesh with radius 0.2, and i have no mask for the mesh to filter
        q.TraceRayInline(accelerationStructure, 0, 0xff, ray);
        q.Proceed();
        if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
            continue;

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
	return float4(outputColor, albedo.a);
}


float4 main(VS_OUT input) : SV_TARGET
{
	float4 outputColor = float4(0,0,0,0);
	outputColor = CalcLightForTexturedMaterial(input.posWorld.xyz, input.normal.xyz, input.uv, input.materialID);
	
    RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
             RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
             RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH>q;
    float3 V = normalize(input.posWorld.xyz - cameraPosition);
    RayDesc ray;
    ray.Origin = input.posWorld.xyz;
    ray.Direction = reflect(V, input.normal.xyz);
    ray.TMin = 1.0f;
    ray.TMax = 100.0f;
    q.TraceRayInline(accelerationStructure, 0, 0xff, ray);
    q.Proceed();
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        //q.CommittedInstanceIndex(),
        //q.CommittedPrimitiveIndex(),
        uint matID = q.CommittedInstanceContributionToHitGroupIndex();
        Material mat = materials[NonUniformResourceIndex(matID)];
        //return float4(mat.albedoFactor.xyz, 1);
        outputColor.xyz = lerp(outputColor.xyz, mat.albedoFactor.xyz, 0.5f);
    }
	return outputColor;
}