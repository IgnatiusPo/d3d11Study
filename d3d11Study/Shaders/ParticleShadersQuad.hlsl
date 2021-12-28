
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
    float4 position : SV_POSITION; // required output of VS
    float2 texCoord : TEXCOORD0;
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory first
    float4 position = float4(Particles[input.VertexID].position, 1.f);
    position = position + float4(origin, 0.f); // should be LocalToWorld matrix honestly

    position = mul(view, position);
    //output.position_view = position;
    //position = mul(projection, position);
    output.position = position;
    output.texCoord = float2(0.f, 0.f);
    return output;
}

//Geometry
vs_out CreateVertex(vs_out input, float2 offset, float2 texCoord)
{
    //vs_out output = (vs_out)0;
    //output.position = input.position;
    //output.position.xy += offset;
    //output.position = mul(projection, output.position);
    //output.texCoord = texCoord;
    //return output;

    input.position.xy += offset;
    input.position = mul(projection, input.position);
    input.texCoord = texCoord;
    return input;

}


[maxvertexcount(4)]
void gs_main(point vs_out input[1], inout TriangleStream<vs_out> stream)
{
    vs_out vertex = (vs_out)0;
    vertex.position = input[0].position;
    const float size = 0.01f;
    stream.Append(CreateVertex(vertex, float2(-1.f, -1.f) * size , float2(0.f, 1.f)));
    stream.Append(CreateVertex(vertex, float2(-1.f, 1.f) * size , float2(0.f, 0.f)));
    stream.Append(CreateVertex(vertex, float2(1.f, -1.f) * size , float2(1.f, 1.f)));
    stream.Append(CreateVertex(vertex, float2(1.f, 1.f) * size , float2(1.f, 0.f)));

    //vs_out vertex1 = (vs_out)0;
    //vs_out vertex2 = (vs_out)0;
    //vs_out vertex3 = (vs_out)0;
    //vs_out vertex4 = (vs_out)0;
    //vertex1.position = float4(-1.f, -1.f, 0.f, 1.f);
    //vertex2.position = float4(-1.f, 1.f, 0.f, 1.f);
    //vertex3.position = float4(1.f, -1.f, 0.f, 1.f);
    //vertex4.position = float4(1.f, 1.f, 0.f, 1.f);

    //stream.Append(vertex1);
    //stream.Append(vertex2);
    //stream.Append(vertex3);
    //stream.Append(vertex4);


    stream.RestartStrip();
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
  //output.color = float4(input.texCoord, 0.5, 1.0);
  return output;
}
