/* vertex attributes go here to input to the vertex shader */
struct vs_in {
    float2 position_local : POS;
    float2 texCoord : TEXCOORD0;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
    float2 texCoord : TEXCOORD0;
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory first
    output.position_clip = float4(input.position_local, 1.f,  1.f);
    output.texCoord = input.texCoord;
    return output;
}