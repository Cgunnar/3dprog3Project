
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

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    output.position = float4(0, 0, 0, 0);
	return output;
}