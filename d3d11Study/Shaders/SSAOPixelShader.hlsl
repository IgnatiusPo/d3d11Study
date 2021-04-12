Texture2D normalTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D noiseTexture : register(t2);
SamplerState ClampPointSampler : register(s0);
SamplerState WrapPointSampler : register(s1);
cbuffer View : register(b0)
{
    matrix view;
    matrix projection;
    float4 viewWorldPos;
};
#define kernelSize 64

cbuffer Kernel
{
    float4 kernelSamples[kernelSize];
};
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
    float2 texCoord : TEXCOORD0;
};

float4 ps_main(vs_out input) : SV_TARGET{
    float4 output;
    float4 normal = normalTexture.Sample(ClampPointSampler, input.texCoord);
    float4 viewPosition = positionTexture.Sample(ClampPointSampler, input.texCoord);
    
    float2 textureSize;
    positionTexture.GetDimensions(textureSize.x, textureSize.y);
    float2 noiseScale = textureSize / 4.f;
    float3 random = (float3)noiseTexture.Sample(WrapPointSampler, input.texCoord * noiseScale);

    float3 tangent = normalize(random - normal * dot(random, normal));
    float3 bitangent = cross(tangent, normal);
    float3x3 TBN = transpose(float3x3(tangent, bitangent, (float3)normal));
   
    float occlusion = 0.0;
    float radius = 10.f;
    float bias = 0.005f;
    for (int i = 0; i < kernelSize; ++i)
    {
        float3 samplePosition = mul(TBN, (float3)kernelSamples[i]);
        samplePosition = viewPosition + samplePosition * radius;

        float4 samplePositionScreenSpace = float4(samplePosition, 1.f);
        samplePositionScreenSpace = mul(projection, samplePositionScreenSpace);
        samplePositionScreenSpace.xyz /= samplePositionScreenSpace.w; //perspective division
        samplePositionScreenSpace.xyz = samplePositionScreenSpace.xyz * 0.5f + 0.5f; // 0.0 - 1.0
        samplePositionScreenSpace.y = 1.f - samplePositionScreenSpace.y;

        float4 sampleFromTexture = positionTexture.Sample(ClampPointSampler, samplePositionScreenSpace.xy);
        float rangeCheck = 1.f - step(radius / 2.f, abs(sampleFromTexture.z - samplePosition.z));

        occlusion += ( (sampleFromTexture.z <= samplePosition.z - bias)  ? 1.0f : 0.f) * rangeCheck;
    }
    occlusion = 1.0f - (occlusion / kernelSize);
    output = float4(occlusion, occlusion, occlusion, 1.f);

    return output;
}