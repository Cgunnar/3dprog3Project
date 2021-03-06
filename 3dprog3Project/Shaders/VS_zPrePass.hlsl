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
        output.position = mul(viewProjectionMatrix, mul(insData.worldMatrix, float4(vertex.position, 1.0f)));
    }
    else
    {
        Vertex vertex = vertices[vbIndex][indices[ibIndex][input.vertexID + indexStart] + vertexStart];
        output.position = mul(viewProjectionMatrix, mul(insData.worldMatrix, float4(vertex.position, 1.0f)));
    }
	return output;
}