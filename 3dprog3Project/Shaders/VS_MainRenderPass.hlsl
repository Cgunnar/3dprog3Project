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

struct Transform
{
	float4x4 worldMatrix;
};

struct RootConstant
{
	
};

cbuffer MaterialBuffer : register(b2, space3)
{
	int startOffset;
}
ConstantBuffer<Transform> transform[] : register(b1);
ConstantBuffer<Transform> transformIns[] : register(b0, space1);


StructuredBuffer<Vertex> vertices : register(t0);
StructuredBuffer<unsigned int> indices : register(t1);

struct VS_IN
{
	uint vertexID : SV_VERTEXID;
	uint instanceID : SV_INSTANCEID;
};

VS_OUT main(VS_IN input)
{

	VS_OUT output;
	Vertex vertex = vertices[indices[input.vertexID]];
	output.posWorld = mul(transformIns[startOffset].worldMatrix, float4(vertex.position, 1.0f));
	output.normal = normalize(mul(transformIns[startOffset].worldMatrix, float4(vertex.normal, 0.0f)));
	output.position = mul(viewProjectionMatrix, output.posWorld);
	output.uv = vertex.uv;
	return output;


	/*VS_OUT output;
	Vertex vertex = vertices[indices[input.vertexID]];
	output.posWorld = mul(transform[0].worldMatrix, float4(vertex.position, 1.0f));
	output.normal = normalize(mul(transform[0].worldMatrix, float4(vertex.normal, 0.0f)));
	output.position = mul(viewProjectionMatrix, output.posWorld);
	output.uv = vertex.uv;
	return output;*/
}