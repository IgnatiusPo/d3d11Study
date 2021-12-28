struct vs_out {
    float4 position : SV_POSITION; // required output of VS
    float2 texCoord : TEXCOORD0;
};

// Pixel

Texture2D Texture0 : register(t0);
SamplerState ClampPointSampler : register(s0);

struct ps_out
{
    float4 color : SV_Target0;
};
ps_out ps_main(vs_out input) : SV_TARGET
{
  ps_out output = (ps_out)0;
  float4 color = Texture0.Sample(ClampPointSampler, input.texCoord);
  //output.color = float4(float3(color.x, color.y, color.x), 1.f);
  if (color.a < 0.1f)
  {
      discard;
  }
  //color = float4(1.f, 0.f, 1.f, color.a);
  output.color = color;

  return output;
}
