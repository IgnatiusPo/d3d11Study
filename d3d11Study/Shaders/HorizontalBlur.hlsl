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
  //  float4 result;
  //  for (int x = -2; x < 3; ++x)
  //  {
		//float2 offset = float2(float(x), 0.f) * texelSize;
		//result += inputTexture.Sample(ClampPointSampler, input.texCoord + offset);
  //  }
    
    float4 sum = 0.f;
    sum += inputTexture.Sample(ClampPointSampler, input.texCoord - float2(4.0f * texelSize.x, 0.f)) * 0.0162162162;
    sum += inputTexture.Sample(ClampPointSampler, input.texCoord - float2(3.0f * texelSize.x, 0.f)) * 0.0540540541;
    sum += inputTexture.Sample(ClampPointSampler, input.texCoord - float2(2.0f * texelSize.x, 0.f)) * 0.1216216216;
    sum += inputTexture.Sample(ClampPointSampler, input.texCoord - float2(1.0f * texelSize.x, 0.f)) * 0.1945945946;

    sum += inputTexture.Sample(ClampPointSampler, input.texCoord ) * 0.2270270270;

    sum += inputTexture.Sample(ClampPointSampler, input.texCoord + float2(1.0f * texelSize.x, 0.f)) * 0.1945945946;
    sum += inputTexture.Sample(ClampPointSampler, input.texCoord + float2(2.0f * texelSize.x, 0.f)) * 0.1216216216;
    sum += inputTexture.Sample(ClampPointSampler, input.texCoord + float2(3.0f * texelSize.x, 0.f)) * 0.0540540541;
    sum += inputTexture.Sample(ClampPointSampler, input.texCoord + float2(4.0f * texelSize.x, 0.f)) * 0.0162162162;

    //output = result / (5.f);
    output = float4(sum.rgb, 1.f);
    return output;
}