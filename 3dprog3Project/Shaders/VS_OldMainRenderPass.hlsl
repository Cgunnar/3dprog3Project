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
	float4 normal : NORMAL;
	float2 uv : UV;
};

cbuffer CameraCB : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 viewMatrix;
	float4x4 viewProjectionMatrix;
	float3 cameraPosition;
}

cbuffer TransformCB : register(b1)
{
	float4x4 worldMatrix;
}

StructuredBuffer<Vertex> vertices : register(t0);
StructuredBuffer<unsigned int> indices : register(t1);

VS_OUT main(uint vertexID : SV_VERTEXID)
{
	VS_OUT output;
	Vertex vertex = vertices[indices[vertexID]];
	output.posWorld = mul(worldMatrix, float4(vertex.position, 1.0f));
	output.normal = normalize(mul(worldMatrix, float4(vertex.normal, 0.0f)));
	output.position = mul(viewProjectionMatrix, output.posWorld);
	output.uv = vertex.uv;
	return output;
}