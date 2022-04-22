struct Vertex
{
	float3 position;
	float3 normal;
	float2 uv;
};

struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 uv : UV;
};

StructuredBuffer<Vertex> vertices : register(t0);
StructuredBuffer<unsigned int> indices : register(t1);

VS_OUT main( uint vertexID : SV_VERTEXID )
{
	VS_OUT output;
	Vertex vertex = vertices[indices[vertexID]];
	output.position = float4(vertex.position, 1.0f);
	output.uv = vertex.uv;
	return output;
}