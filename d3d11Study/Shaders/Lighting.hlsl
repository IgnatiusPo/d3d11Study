Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D positionTexture : register(t2);
Texture2D inputSSAO : register(t3);
SamplerState ClampPointSampler : register(s0);
cbuffer View : register(b0)
{
    matrix view;
    matrix projection;
    float4 viewWorldPos;
};
cbuffer LightBuffer
{
	float3 lightDirection;
};

struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
    float2 texCoord : TEXCOORD0;
};

float4 ps_main(vs_out input) : SV_TARGET{
    //float3 lightDirection = float3(0.f, 0.f, 1.f);
    float4 output;
    float4 albedo = colorTexture.Sample(ClampPointSampler, input.texCoord);
    float4 ambientColor = albedo * 0.3f;
    float4 normal = normalTexture.Sample(ClampPointSampler, input.texCoord);
    float4 viewPosition = positionTexture.Sample(ClampPointSampler, input.texCoord);
    
    float3 lightDirectionView = mul(view, lightDirection);
    float3 lightDirInverse = -lightDirectionView;
    float lightIntensity = saturate(dot(normal.xyz, lightDirInverse));
    float4 diffuseColor = saturate(albedo * lightIntensity);

    float4 viewOriginPosition = mul(view, viewWorldPos);
    float3 viewDir = (float3)normalize(viewOriginPosition - viewPosition);
    float3 halfwayDir = normalize(lightDirInverse + viewDir);
    float spec = pow(max(dot((float3)normal, halfwayDir), 0.f), 16.0);
    spec *= 0.2f;
    float4 specularColor = spec * float4(1.f, 1.f, 1.f, 1.f); // specular texture, light color 
    output = ambientColor + diffuseColor + specularColor;
    float AO = inputSSAO.Sample(ClampPointSampler, input.texCoord).r;
    output = output * AO * AO; // hack, multiply only ambient component
    return output;
}