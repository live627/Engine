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
};

float4 FlashlightPixelShader(PixelInputType input) : SV_TARGET
{
	input.tex -= 0.5f;
	float dist = input.tex.x * input.tex.x + input.tex.y * input.tex.y;
	float distFromEdge = 0.15 - dist;  // positive when inside the circle
	float thresholdWidth = 0.25;  // a constant you'd tune to get the right level of softness
	float antialiasedCircle = saturate((distFromEdge / thresholdWidth) + 0.5);
	return lerp(0, 1, antialiasedCircle);
}

float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	return shaderTexture.Sample(SampleType, input.tex);
}

float4 FontPixelShader(PixelInputType input) : SV_TARGET
{
	return shaderTexture.Sample(SampleType, input.tex).rrrr * pixelColor;
}