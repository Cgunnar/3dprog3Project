Texture2D renderTargetInput : register(t0);
SamplerState fileter : register(s0);

struct vs_out
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

float4 main(vs_out input) : SV_TARGET
{
    float4 color = renderTargetInput.Sample(fileter, input.uv);
    
    //color.rgb = color.rgb / (color.rgb + float3(1, 1, 1));
    return float4(color.rgb, 1.0f);
}