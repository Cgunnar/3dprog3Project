struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 posWorld : WORLD_POS;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 biTangent : BITANGENT;
    float2 uv : UV;
    int materialIndex : MATERIAL_INDEX;
};


float4 main(VS_OUT input) : SV_TARGET
{
    return float4(0, 0, 0, 0);
}