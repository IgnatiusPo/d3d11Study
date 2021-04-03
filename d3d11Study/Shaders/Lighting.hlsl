Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState ClampPointSampler : register(s0);
cbuffer LightBuffer
{
	float3 lightDirection;
};
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
    float2 texCoord : TEXCOORD0;
};

float4 ps_main(vs_out input) : SV_TARGET{
    //float3 lightDirection = float3(0.f, 0.f, 1.f);
    float4 color;
    float4 normal;
    float3 lightDir;
    float lightIntensity;
    float4 output;
    color = colorTexture.Sample(ClampPointSampler, input.texCoord);
    normal = normalTexture.Sample(ClampPointSampler, input.texCoord);
    
    lightDir = -lightDirection;
    lightIntensity = saturate(dot(normal.xyz, lightDir));
    output = saturate(color * lightIntensity);
    return output;
}