struct Vertex
{
	float3 position;
	float p;
	float2 uv;
};

struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 uv : uv;
};

StructuredBuffer<Vertex> vertices : register(t0, space0);

VS_OUT main( uint vertexID : SV_VERTEXID )
{
	VS_OUT output;
	output.position = float4(vertices[vertexID].position, 1.0f);
	output.uv = vertices[vertexID].uv;
	return output;
}