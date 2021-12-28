
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
    float2 texCoord : TEXCOORD0;
};

float4 ps_main(vs_out input) : SV_TARGET{
    return float4(0.1f, .1f, 0.1f, 1.0); // must return an RGBA colour
    //  return float4(input.texCoord, 0.1f, 1.f);
}