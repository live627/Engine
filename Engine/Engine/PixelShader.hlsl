Texture2D shaderTexture;
SamplerState SampleType;

cbuffer PixelBuffer
{
	float4 pixelColor;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 Color : COLOR0;
};

float3 HSV2RGBPixelShader(PixelInputType input) : SV_TARGET
{
	float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	float3 p = abs(frac(input.Color.xxx + K.xyz) * 6.0 - K.www);

	return input.Color.z * lerp(K.xxx, saturate(p - K.xxx), input.Color.y);
}

float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	return shaderTexture.Sample(SampleType, input.tex);
}

float4 RGBPixelShader(PixelInputType input) : SV_TARGET
{
	return input.Color;
}

float4 FontPixelShader(PixelInputType input) : SV_TARGET
{
	return shaderTexture.Sample(SampleType, input.tex).r * pixelColor;
}

float4 SDFPixelShader(PixelInputType input) : SV_TARGET
{
	return smoothstep(0.1, 0.9, shaderTexture.Sample(SampleType, input.tex).r) * pixelColor;
}