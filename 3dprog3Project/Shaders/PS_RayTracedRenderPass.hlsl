struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 posWorld : WORLD_POS;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 biTangent : BITANGENT;
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

cbuffer RootConstants : register(b3)
{
    int numberOfBounces;
    int numberOfPointLights;
    int shadows;
}

struct Material
{
	float4 albedoFactor;
    float3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
	int albedoTextureIndex;
	int normalMapIndex;
	int metallicRoughnessTextureIndex;
};

struct MaterialValues
{
    float4 albedo;
    float3 emissive;
    float metallic;
    float roughness;
    float ambientOcclusion;
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

struct TopLevelInstanceMetaData
{
    int materialDescriptorIndex;
    uint indexBufferDescriptorIndex;
    uint vertexBufferDescriptorIndex;
    uint indexStart;
    uint vertexStart;
    int vetexType;
};
SamplerState anisotropicSampler : register(s0);
StructuredBuffer<PointLight> dynamicPointLights : register(t0);
RaytracingAccelerationStructure accelerationStructure : register(t1);
StructuredBuffer<TopLevelInstanceMetaData> topLevelInstanceMetaData : register(t2);
Texture2D albedoMap[] : register(t0, space1);
Texture2D normalMap[] : register(t0, space3);
Texture2D metallicRoughnessMap[] : register(t0, space6);

struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
};
struct VertexT
{
    float3 position;
    float3 normal;
    float2 uv;
    float3 tangent;
    float3 biTangent;
};
StructuredBuffer<Vertex> vertices[] : register(t0, space4);
StructuredBuffer<VertexT> verticesT[] : register(t0, space5);
StructuredBuffer<unsigned int> indices[] : register(t0, space2);

float Attenuate(float length, float constAtt, float linAtt , float expAtt)
{
	return 1.0f / (constAtt + linAtt * length + expAtt * length * length);
}

float3 NormalMap(float3 valueFromNomalMap, float3 tangent, float3 biTangent, float3 normal)
{
    float3 normalTanSpace = 2 * valueFromNomalMap - float3(1, 1, 1);
    normalTanSpace = normalize(normalTanSpace);
    float3 T = normalize(tangent);
    float3 B = normalize(biTangent);
    float3 N = normalize(normal);
    float3x3 TBN = float3x3(T, B, N);
    TBN = transpose(TBN); //so i can use mul the right way :/
    return normalize(mul(TBN, normalTanSpace));
}

MaterialValues SampleMaterials(float2 uv, Material mat)
{
    MaterialValues matOut;
    matOut.albedo = mat.albedoFactor;
    matOut.emissive = mat.emissiveFactor;
    matOut.metallic = mat.metallicFactor;
    matOut.roughness = mat.roughnessFactor;
    matOut.ambientOcclusion = 1;
	
	//https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html
	//if no texture exists, texture is assumed to gave value 1 for each pixel => factor is the color
	//this is true for the roughness and the other pbr materials as well.
    if (mat.albedoTextureIndex != -1)
        matOut.albedo *= albedoMap[NonUniformResourceIndex(mat.albedoTextureIndex)].Sample(anisotropicSampler, uv).rgba;
    
    if (mat.metallicRoughnessTextureIndex != -1)
    {
        float3 value = metallicRoughnessMap[NonUniformResourceIndex(mat.metallicRoughnessTextureIndex)].Sample(anisotropicSampler, uv).rgb;
        matOut.ambientOcclusion = 1; // value.r; no standard for if this is included or not
        matOut.metallic *= value.b;
        matOut.roughness *= value.g;
        matOut.roughness = max(matOut.roughness, 0.05);
    }
    return matOut;
}

float4 CalcLightForTexturedMaterial(float3 pos, float3 normal, MaterialValues mat)
{
	float3 outputColor = float3(0, 0, 0);
	for (int i = 0; i < numberOfPointLights; i++)
	{
		PointLight pl = dynamicPointLights[i];
        float3 vecToLight = pl.position - pos;
        float3 dirToLight = normalize(vecToLight);
        
        if (shadows)
        {
            RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
            RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
            RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
            RayDesc ray;
            ray.Origin = pos;
            ray.Direction = dirToLight;
            ray.TMin = 0.01f;
            ray.TMax = length(vecToLight);
            q.TraceRayInline(accelerationStructure, 0, 0xff, ray);
            q.Proceed();
            if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
                continue;
        }

		float diffFactor = saturate(dot(normal, dirToLight));
		float3 r = normalize(reflect(-dirToLight, normal));
		float3 v = normalize(cameraPosition - pos);
        float specFactor = pow(saturate(dot(r, v)), (1 - mat.roughness / 2) * 1000);

		float3 inLight = pl.color * pl.strength;
        float3 spec = specFactor * (1 - mat.roughness) * inLight;
        float3 diff = diffFactor * mat.albedo.rgb * inLight;

		outputColor += (diff + spec) * Attenuate(length(vecToLight), pl.constAtt, pl.linAtt, pl.expAtt);
	}
    outputColor += mat.emissive;
    outputColor += 0.2f * mat.albedo.rgb * mat.ambientOcclusion;
    return float4(outputColor, mat.albedo.a);
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
    ray.TMin = 0.001f;
    ray.TMax = 100.0f;
    q.TraceRayInline(accelerationStructure, 0, 0xff, ray);
    q.Proceed();
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        obj.hit = true;
        uint combinedMatIndexAndMeshIndex = q.CommittedInstanceContributionToHitGroupIndex();
        TopLevelInstanceMetaData topLevelData = topLevelInstanceMetaData[combinedMatIndexAndMeshIndex];
        obj.meshIndex = topLevelData.indexBufferDescriptorIndex;
        obj.matIndex = topLevelData.materialDescriptorIndex;
        
        
        uint primitiveIndex = 3 * q.CommittedPrimitiveIndex();
        float2 bar = q.CommittedTriangleBarycentrics();
        uint index0 = indices[NonUniformResourceIndex(obj.meshIndex)][topLevelData.indexStart + primitiveIndex + 0];
        uint index1 = indices[NonUniformResourceIndex(obj.meshIndex)][topLevelData.indexStart + primitiveIndex + 1];
        uint index2 = indices[NonUniformResourceIndex(obj.meshIndex)][topLevelData.indexStart + primitiveIndex + 2];
        
        if (topLevelData.vetexType == 0)
        {
            Vertex vertex0 = vertices[NonUniformResourceIndex(obj.meshIndex)][topLevelData.vertexStart + index0];
            Vertex vertex1 = vertices[NonUniformResourceIndex(obj.meshIndex)][topLevelData.vertexStart + index1];
            Vertex vertex2 = vertices[NonUniformResourceIndex(obj.meshIndex)][topLevelData.vertexStart + index2];

            float3x4 worldMatrix = q.CommittedObjectToWorld3x4();
        
            obj.uv = vertex0.uv + bar.x * (vertex1.uv - vertex0.uv) + bar.y * (vertex2.uv - vertex0.uv);
            obj.normal = vertex0.normal + bar.x * (vertex1.normal - vertex0.normal) + bar.y * (vertex2.normal - vertex0.normal);
            obj.position = vertex0.position + bar.x * (vertex1.position - vertex0.position) + bar.y * (vertex2.position - vertex0.position);
            
            obj.position = mul(worldMatrix, float4(obj.position, 1));
            obj.normal = normalize(mul(worldMatrix, float4(obj.normal, 0)));
        }
        else if (topLevelData.vetexType == 1)
        {
            VertexT vertex0 = verticesT[NonUniformResourceIndex(obj.meshIndex)][topLevelData.vertexStart + index0];
            VertexT vertex1 = verticesT[NonUniformResourceIndex(obj.meshIndex)][topLevelData.vertexStart + index1];
            VertexT vertex2 = verticesT[NonUniformResourceIndex(obj.meshIndex)][topLevelData.vertexStart + index2];

            float3x4 worldMatrix = q.CommittedObjectToWorld3x4();
            
            obj.uv = vertex0.uv + bar.x * (vertex1.uv - vertex0.uv) + bar.y * (vertex2.uv - vertex0.uv);
            obj.normal = vertex0.normal + bar.x * (vertex1.normal - vertex0.normal) + bar.y * (vertex2.normal - vertex0.normal);
            obj.position = vertex0.position + bar.x * (vertex1.position - vertex0.position) + bar.y * (vertex2.position - vertex0.position);
            
            obj.position = mul(worldMatrix, float4(obj.position, 1));
            obj.normal = normalize(mul(worldMatrix, float4(obj.normal, 0)));
            
            Material mat = materials[NonUniformResourceIndex(obj.matIndex)];
            if (mat.normalMapIndex != -1)
            {
                float3 normalFromMap = normalMap[NonUniformResourceIndex(mat.normalMapIndex)].Sample(anisotropicSampler, obj.uv).xyz;
                float3 tangent = vertex0.tangent + bar.x * (vertex1.tangent - vertex0.tangent) + bar.y * (vertex2.tangent - vertex0.tangent);
                float3 biTangent = vertex0.biTangent + bar.x * (vertex1.biTangent - vertex0.biTangent) + bar.y * (vertex2.biTangent - vertex0.biTangent);
                
                tangent = normalize(mul(worldMatrix, float4(tangent, 0)));
                biTangent = normalize(mul(worldMatrix, float4(biTangent, 0)));
                
                obj.normal = NormalMap(normalFromMap, tangent, biTangent, obj.normal);
            }
        }
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
    
    Material mat = materials[NonUniformResourceIndex(input.materialIndex)];
    float3 normal = input.normal.xyz;
    if (mat.normalMapIndex != -1)
    {
        float3 normalFromMap = normalMap[NonUniformResourceIndex(mat.normalMapIndex)].Sample(anisotropicSampler, input.uv).xyz;
        normal = NormalMap(normalFromMap, float3(0, 0, 1), float3(1, 0, 0), normal);
    }
    
    MaterialValues matVal = SampleMaterials(input.uv, mat);
    outputColor = CalcLightForTexturedMaterial(input.posWorld.xyz, normal, matVal);
    
    int bounceCount = 0;
    float3 origin = input.posWorld.xyz;
    float3 dir = reflect(normalize(input.posWorld.xyz - cameraPosition), normal);
    while (bounceCount < numberOfBounces)
    {
        RayTracedObject obj = RayTrace(origin, dir);
        
        if (obj.hit)
        {
            Material mat = materials[NonUniformResourceIndex(obj.matIndex)];
            MaterialValues reflectedObjMatVal = SampleMaterials(obj.uv, mat);
            
            //pbr calculations out of context with some random values changed out to other
            float3 F0 = float3(0.04, 0.04, 0.04);
            F0 = lerp(F0, matVal.albedo.rgb, matVal.metallic);
            float3 madeUpBlendValue = (F0 + (max(F0, 1.0 - matVal.roughness) - F0) * pow(1.0 - saturate(dot(-dir, normal)), 5.0));
            outputColor.rgb = lerp(outputColor.rgb, CalcLightForTexturedMaterial(obj.position, obj.normal, reflectedObjMatVal).rgb, madeUpBlendValue);
            //should not shade first intersection, later bounce might overwrite the color
            origin = obj.position;
            dir = reflect(dir, obj.normal);
            matVal = reflectedObjMatVal;
            normal = obj.normal;
        }
        else
        {
            break;
        }
        bounceCount++;
    }
    return outputColor;
}