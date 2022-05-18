Texture2D renderTargetInput : register(t0);
SamplerState fileter : register(s0);

struct vs_out
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

//fix compute shader later
float4 main(vs_out input) : SV_TARGET
{
    //what kind of sampler should be used when scaling? does it depend on if we are up scaling or down scaling?
    float4 color = renderTargetInput.Sample(fileter, input.uv);

    //use that the src is hdr and values can be greater then 1
    //if color > some threshold we can have fancy bloom

    //replace Reinhard with ACES 
    color.rgb = color.rgb / (color.rgb + float3(1, 1, 1));
    //have a user editable value instead of 2.2
    color.rgb = pow(color.rgb, 1.0f / 2.2f);
    return float4(color.rgb, 1.0f);
}