
struct GPUParticleData
{
	float3 position;
	float3 velocity;
};

StructuredBuffer<GPUParticleData> Particles : register(t0);
cbuffer View : register(b0) // always first
{
    matrix view;
    matrix projection;
    float4 viewWorldPos;
};
cbuffer PerParticleData : register(b1)
{
    float3 origin;
};

struct vs_in {
    uint VertexID : SV_VertexID;
};

struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory first
    float4 position = float4(Particles[input.VertexID].position, 1.f);
    position = position + float4(origin, 0.f); // should be LocalToWorld matrix honestly

    position = mul(view, position);
    //output.position_view = position;
    position = mul(projection, position);
    output.position_clip = position;
    return output;
}

// Pixel
struct ps_out
{
    float4 color : SV_Target0;
};
ps_out ps_main(vs_out input) : SV_TARGET
{
  ps_out output = (ps_out)0;
  output.color = float4(.5f, .1f, 0.5, 1.0);
  return output;
}
