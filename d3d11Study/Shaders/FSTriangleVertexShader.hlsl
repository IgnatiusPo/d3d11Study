struct vs_in {
    uint vertexId : SV_VertexID;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
    float2 texCoord : TEXCOORD0;
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory first
    output.texCoord = float2((input.vertexId << 1) & 2, input.vertexId & 2);
    output.position_clip = float4(output.texCoord * float2(2.f, -2.f) + float2( -1.f, 1.f), 1.0f,  1.f);

    return output;
}