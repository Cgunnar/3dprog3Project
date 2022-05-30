
struct VS_IN
{
    uint vertexID : SV_VERTEXID;
    uint instanceID : SV_INSTANCEID;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
};

cbuffer MeshInfo : register(b0, space0)
{
    int meshIndex;
}

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

struct RenderUnit
{
	float4x4 worldMatrix;
    uint indexBufferDescriptorIndex;
    uint vertexBufferDescriptorIndex;
    uint indexStart;
    uint indexCount;
    uint vertexStart;
    int materialDescriptorIndex;
    int vertexType; // 1 == has tangents and bitangents
    int extraInt;
    uint2  subMeshID;//uint64_t
    uint2 meshID; //uint64_t
};

StructuredBuffer<RenderUnit> renderUnits : register(t1, space0);

StructuredBuffer<unsigned int> indices[] : register(t0, space1);
StructuredBuffer<Vertex> vertices[] : register(t0, space2);
StructuredBuffer<VertexT> verticesT[] : register(t0, space3);

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    output.position = float4(0, 0, 0, 0);
	return output;
}