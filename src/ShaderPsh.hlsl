struct VS_OUTPUT
{
	float4 position	: SV_POSITION;
	float4 colour	: COLOR0;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
	return input.colour;
}