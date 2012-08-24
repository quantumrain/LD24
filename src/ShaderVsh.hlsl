row_major float4x4 worldViewProj;

struct VS_INPUT
{
	float2 position	: POSITION;
	float4 colour	: COLOR0;
};

struct VS_OUTPUT
{
	float4 position	: SV_POSITION;
	float4 colour	: COLOR0;
};

VS_OUTPUT main(VS_INPUT v)
{
	VS_OUTPUT output;

	//output.position	= mul(float4(v.position.x, 0.0f, v.position.y, 1.0f), worldViewProj);
	output.position	= float4(v.position, 0.0f, 1.0f);
	output.colour	= v.colour;

	return output;
}