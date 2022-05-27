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

struct InstancedData
{
	float4x4 worldMatrix;
	int materialIndex;
};

cbuffer MaterialBuffer : register(b2, space3)
{
	int startOffset;
    uint ibIndex;
    uint vbIndex;
    uint indexStart;
    uint vertexStart;
    int hasTangents;
}

ConstantBuffer<InstancedData> transformIns[] : register(b0, space1);


StructuredBuffer<Vertex> vertices[] : register(t0, space4);
StructuredBuffer<VertexT> verticesT[] : register(t0, space5);
StructuredBuffer<unsigned int> indices[] : register(t0, space2);

struct VS_IN
{
	uint vertexID : SV_VERTEXID;
	uint instanceID : SV_INSTANCEID;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output;
    InstancedData insData = transformIns[NonUniformResourceIndex(startOffset + input.instanceID)];
    if (hasTangents)
    {
        VertexT vertex = verticesT[vbIndex][indices[ibIndex][input.vertexID + indexStart] + vertexStart];
        output.posWorld = mul(insData.worldMatrix, float4(vertex.position, 1.0f));
        output.normal.xyz = normalize(mul(insData.worldMatrix, float4(vertex.normal, 0.0f)).xyz);
        output.tangent.xyz = normalize(mul(insData.worldMatrix, float4(vertex.tangent, 0.0f)).xyz);
        output.biTangent.xyz = normalize(mul(insData.worldMatrix, float4(vertex.biTangent, 0.0f)).xyz);
        output.position = mul(viewProjectionMatrix, output.posWorld);
        output.uv = vertex.uv;
    }
    else
    {
        Vertex vertex = vertices[vbIndex][indices[ibIndex][input.vertexID + indexStart] + vertexStart];
        output.posWorld = mul(insData.worldMatrix, float4(vertex.position, 1.0f));
        output.normal.xyz = normalize(mul(insData.worldMatrix, float4(vertex.normal, 0.0f)).xyz);
        output.position = mul(viewProjectionMatrix, output.posWorld);
        output.uv = vertex.uv;
    }
    
    output.materialIndex = insData.materialIndex;
	return output;
}