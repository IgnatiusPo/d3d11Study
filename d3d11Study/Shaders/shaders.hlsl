/* vertex attributes go here to input to the vertex shader */
//#pragma pack_matrix( row_major )
struct vs_in {
    float3 position_local : POS;
};
cbuffer View
{
    matrix view;
    matrix projection;
};
/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory first
    float4 position = float4(input.position_local, 1.0f);
    position = mul(view, position);
	position = mul(projection, position);
	//position = mul( position, view);
 //   position = mul( position, projection);
    output.position_clip = position;
    return output;
}

float4 ps_main(vs_out input) : SV_TARGET{
  return float4(1.0, 0.0, 1.0, 1.0); // must return an RGBA colour
}