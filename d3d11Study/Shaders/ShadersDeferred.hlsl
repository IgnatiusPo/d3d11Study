struct vs_in {
    float3 position_local : POS;
    float3 normal : NORMAL;
};
cbuffer View : register(b0)
{
    matrix view;
    matrix projection;
};
cbuffer PerObjectData : register(b1)
{
    matrix modelMat;
}
/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
    float3 normal : NORMAL;
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory first
    float4 position = float4(input.position_local, 1.0f);
    position = mul(modelMat, position);
    position = mul(view, position);
	position = mul(projection, position);
    output.position_clip = position;
    output.normal = mul(modelMat, input.normal);
    //output.normal = input.normal;
    return output;
}

// Pixel
struct PixelOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
};
PixelOutput ps_main(vs_out input) : SV_TARGET
{
  PixelOutput output;
  output.color = float4(1.0, 0.0, 1.0, 1.0);
  output.normal = float4(input.normal, 1.f);
  return output;
}