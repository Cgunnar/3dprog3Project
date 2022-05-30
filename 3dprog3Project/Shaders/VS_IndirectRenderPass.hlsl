
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





StructuredBuffer<unsigned int> indices[] : register(t0, space1);
StructuredBuffer<Vertex> vertices[] : register(t0, space2);
StructuredBuffer<VertexT> verticesT[] : register(t0, space3);

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    output.position = float4(0, 0, 0, 0);
	return output;
}