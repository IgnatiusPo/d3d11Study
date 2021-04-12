Texture2D inputTexture : register(t0);
SamplerState ClampPointSampler : register(s0);
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
    float2 texCoord : TEXCOORD0;
};

float4 ps_main(vs_out input) : SV_TARGET
{
    float4 output;
    float2 textureSize;
    inputTexture.GetDimensions(textureSize.x, textureSize.y);
    float2 texelSize = 1.f / textureSize;
    float4 result;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            float2 offset = float2(float(x), float(y)) * texelSize;
            result += inputTexture.Sample(ClampPointSampler, input.texCoord + offset);
        }
    }
    output = result / (4.f * 4.f);
    return output;
}