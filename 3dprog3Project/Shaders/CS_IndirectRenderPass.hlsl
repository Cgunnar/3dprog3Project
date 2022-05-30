cbuffer RootConstants : register(b0, space0)
{
    int renderUnitCount;
}


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
    uint2 subMeshID; //uint64_t
    uint2 meshID; //uint64_t
};

StructuredBuffer<RenderUnit> renderUnits : register(t1, space0);

#define size 32

[numthreads(size, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint id = DTid.x + DTid.y * size + DTid.z * size * size;
    if (id < renderUnitCount)
    {
        
    }
}