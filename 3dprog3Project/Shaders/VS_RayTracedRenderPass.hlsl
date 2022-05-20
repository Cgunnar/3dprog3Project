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
	int materialID : MATERIAL_ID;
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
	int materialID;
};

cbuffer MaterialBuffer : register(b2, space3)
{
	int startOffset;
    uint ibIndex;
    uint vbIndex;
}

ConstantBuffer<InstancedData> transformIns[] : register(b0, space1);


StructuredBuffer<Vertex> vertices[] : register(t0, space4);
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
    Vertex vertex = vertices[vbIndex][indices[ibIndex][input.vertexID]];
    //Vertex vertex = vertices[vbIndex][indices[ibIndex][input.vertexID]];
	output.posWorld = mul(insData.worldMatrix, float4(vertex.position, 1.0f));
	output.normal = normalize(mul(insData.worldMatrix, float4(vertex.normal, 0.0f)));
	output.position = mul(viewProjectionMatrix, output.posWorld);
	output.uv = vertex.uv;
	output.materialID = insData.materialID;
	return output;
}