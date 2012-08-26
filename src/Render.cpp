#include "Pch.h"
#include "Common.h"
#include "ShaderPsh.h"
#include "ShaderVsh.h"

extern int kWinWidth;
extern int kWinHeight;

gpu::VertexBuffer* gRectVb;
gpu::ShaderDecl* gRectDecl;
gpu::Texture2d* gRectTex;

const int kMaxRectVerts = 64 * 1024;
Vertex gRectVerts[kMaxRectVerts];
int gRectVertCount;

void DrawRect(Vector2 p0, Vector2 p1, int sprite, int flags, Colour colour)
{
	if ((gRectVertCount + 6) > kMaxRectVerts)
	{
		DebugLn("DrawRect overflow");
		return;
	}

	Vertex* v = &gRectVerts[gRectVertCount];

	gRectVertCount += 6;

	int sx = sprite & 0xF;
	int sy = sprite >> 4;

	Vector2 uv0(sx / 16.0f, sy / 16.0f);
	Vector2 uv1((sx + 1) / 16.0f, (sy + 1) / 16.0f);
	Vector2 scale(2.0f / (float)kWinWidth, -2.0f / (float)kWinHeight);

	if (flags & kFlipX) Swap(uv0.x, uv1.x);
	if (flags & kFlipY) Swap(uv0.y, uv1.y);

	// t0

	v->pos.x	= p0.x * scale.x;
	v->pos.y	= p0.y * scale.y;
	v->uv.x		= uv0.x;
	v->uv.y		= uv0.y;
	v->colour	= colour;
	v++;

	v->pos.x	= p1.x * scale.x;
	v->pos.y	= p0.y * scale.y;
	v->uv.x		= uv1.x;
	v->uv.y		= uv0.y;
	v->colour	= colour;
	v++;

	v->pos.x	= p1.x * scale.x;
	v->pos.y	= p1.y * scale.y;
	v->uv.x		= uv1.x;
	v->uv.y		= uv1.y;
	v->colour	= colour;
	v++;

	// t1

	v->pos.x	= p0.x * scale.x;
	v->pos.y	= p0.y * scale.y;
	v->uv.x		= uv0.x;
	v->uv.y		= uv0.y;
	v->colour	= colour;
	v++;

	v->pos.x	= p1.x * scale.x;
	v->pos.y	= p1.y * scale.y;
	v->uv.x		= uv1.x;
	v->uv.y		= uv1.y;
	v->colour	= colour;
	v++;

	v->pos.x	= p0.x * scale.x;
	v->pos.y	= p1.y * scale.y;
	v->uv.x		= uv0.x;
	v->uv.y		= uv1.y;
	v->colour	= colour;
}

void DrawSprite(Vector2 pos, Vector2 scale, int sprite, int flags, Colour colour)
{
	DrawRect(pos - scale * Vector2(8.0f), pos + scale * Vector2(8.0f), sprite, flags, colour);
}

void RenderInit()
{
	gRectVb		= gpu::CreateVertexBuffer(sizeof(Vertex), kMaxRectVerts);
	gRectDecl	= gpu::CreateShaderDecl((void*)gShaderVsh, sizeof(gShaderVsh), (void*)gShaderPsh, sizeof(gShaderPsh));
	gRectTex	= gpu::LoadTexture2d("data\\tiles.png");
}

void RenderShutdown()
{
	gpu::DestroyVertexBuffer(gRectVb);
	gpu::DestroyShaderDecl(gRectDecl);
	gpu::DestroyTexture2d(gRectTex);
}

void RenderPreUpdate()
{
	gRectVertCount = 0;
}

void RenderGame()
{
	gpu::Clear(Colour(0.0f));

	if (void* data = gpu::Map(gRectVb))
	{
		memcpy(data, gRectVerts, gRectVertCount * sizeof(Vertex));
		gpu::Unmap(gRectVb);
	}

	gpu::SetTexture(gRectTex);

	gpu::Draw(gRectDecl, gRectVb, gRectVertCount);
}