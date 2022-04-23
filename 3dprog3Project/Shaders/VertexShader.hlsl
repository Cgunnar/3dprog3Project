struct Vertex
{
	float3 position;
	float3 normal;
	float2 uv;
};

struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 posWorld : WORLD_POS;
	float2 uv : UV;
};

cbuffer TransformCB : register(b0)
{
	float4x4 worldMatrix;
}

cbuffer CameraCB : register(b1)
{
	float4x4 projectionMatrix;
}

StructuredBuffer<Vertex> vertices : register(t0);
StructuredBuffer<unsigned int> indices : register(t1);

VS_OUT main( uint vertexID : SV_VERTEXID )
{
	VS_OUT output;
	Vertex vertex = vertices[indices[vertexID]];
	output.posWorld = mul(worldMatrix, float4(vertex.position, 1.0f));
	output.position = float4(vertex.position, 1.0f);
	output.uv = vertex.uv;
	return output;
}