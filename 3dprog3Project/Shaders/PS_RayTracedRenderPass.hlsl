struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 posWorld : WORLD_POS;
	float4 normal : NORMAL;
	float2 uv : UV;
	int materialIndex : MATERIAL_INDEX;
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

struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
};
StructuredBuffer<Vertex> vertices[] : register(t0, space4);
StructuredBuffer<unsigned int> indices[] : register(t0, space2);

float Attenuate(float length, float constAtt, float linAtt , float expAtt)
{
	return 1.0f / (constAtt + linAtt * length + expAtt * length * length);
}

float4 CalcLightForTexturedMaterial(float3 pos, float3 normal, float2 uv, int matIndex)
{
	float3 outputColor = float3(0, 0, 0);
	float4 albedo;
    Material mat = materials[NonUniformResourceIndex(matIndex)];
	
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
		
        //shadows does not look good in the current scene
        //RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
        //     RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
        //     RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
        //RayDesc ray;
        //ray.Origin = pos;
        //ray.Direction = dirToLight;
        //ray.TMin = 1.0f;
        //ray.TMax = length(vecToLight) - 0.21; //pointlights has a sphere mesh with radius 0.2, and i have no mask for the mesh to filter
        //q.TraceRayInline(accelerationStructure, 0, 0xff, ray);
        //q.Proceed();
        //if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
        //    continue;

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

struct RayTracedObject
{
    float3 position;
    uint matIndex;
    float3 normal;
    uint meshIndex;
    float2 uv;
    bool hit;
};

RayTracedObject RayTrace(float3 origin, float3 dir)
{
    RayTracedObject obj;
    RayQuery<RAY_FLAG_CULL_NON_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> q;
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = dir;
    ray.TMin = 0.5f;
    ray.TMax = 100.0f;
    q.TraceRayInline(accelerationStructure, 0, 0xff, ray);
    q.Proceed();
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        obj.hit = true;
        uint combinedMatIndexAndMeshIndex = q.CommittedInstanceContributionToHitGroupIndex();
        obj.meshIndex = combinedMatIndexAndMeshIndex >> 16;
        obj.matIndex = combinedMatIndexAndMeshIndex & 0xffff;
        
        uint primitiveIndex = 3 * q.CommittedPrimitiveIndex();
        float2 bar = q.CommittedTriangleBarycentrics();
        uint index0 = indices[NonUniformResourceIndex(obj.meshIndex)][primitiveIndex + 0];
        uint index1 = indices[NonUniformResourceIndex(obj.meshIndex)][primitiveIndex + 1];
        uint index2 = indices[NonUniformResourceIndex(obj.meshIndex)][primitiveIndex + 2];
        
        Vertex vertex0 = vertices[NonUniformResourceIndex(obj.meshIndex)][index0];
        Vertex vertex1 = vertices[NonUniformResourceIndex(obj.meshIndex)][index1];
        Vertex vertex2 = vertices[NonUniformResourceIndex(obj.meshIndex)][index2];

        float3x4 worldMatrix = q.CommittedObjectToWorld3x4();
        vertex0.position = mul(worldMatrix, float4(vertex0.position, 1));
        vertex1.position = mul(worldMatrix, float4(vertex1.position, 1));
        vertex2.position = mul(worldMatrix, float4(vertex2.position, 1));
        vertex0.normal = normalize(mul(worldMatrix, float4(vertex0.normal, 0)));
        vertex1.normal = normalize(mul(worldMatrix, float4(vertex1.normal, 0)));
        vertex2.normal = normalize(mul(worldMatrix, float4(vertex2.normal, 0)));
        
        obj.uv = vertex0.uv + bar.x * (vertex1.uv - vertex0.uv) + bar.y * (vertex2.uv - vertex0.uv);
        obj.normal = vertex0.normal + bar.x * (vertex1.normal - vertex0.normal) + bar.y * (vertex2.normal - vertex0.normal);
        obj.position = vertex0.position + bar.x * (vertex1.position - vertex0.position) + bar.y * (vertex2.position - vertex0.position);
    }
    else
    {
        obj.hit = false;
    }
    return obj;
}


float4 main(VS_OUT input) : SV_TARGET
{
	float4 outputColor = float4(0,0,0,0);
	outputColor = CalcLightForTexturedMaterial(input.posWorld.xyz, input.normal.xyz, input.uv, input.materialIndex);
    
    int bounceCount = 0;
    float3 origin = input.posWorld.xyz;
    float3 dir = reflect(normalize(input.posWorld.xyz - cameraPosition), normalize(input.normal.xyz));
    while (bounceCount < 3)
    {
        RayTracedObject obj1 = RayTrace(origin, dir);
        
        if (obj1.hit)
        {
            outputColor =  CalcLightForTexturedMaterial(obj1.position, obj1.normal, obj1.uv, obj1.matIndex);
            origin = obj1.position;
            dir = reflect(dir, obj1.normal);
        }
        else
        {
            break;
        }
        bounceCount++;
    }
    return outputColor;
}