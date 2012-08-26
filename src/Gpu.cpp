// Copyright 2012 Stephen Cakebread

#include "Pch.h"
#include "Common.h"
#include "ShaderPsh.h"
#include "ShaderVsh.h"

extern ID3D10Device* gDevice;

namespace gpu
{

	// VertexBuffer

	struct VertexBuffer
	{
		ID3D10Buffer* vb;
	};

	VertexBuffer* CreateVertexBuffer(int elementSize, int elementCount)
	{
		VertexBuffer* vb = new VertexBuffer;

		if (!vb)
			return 0;

		D3D10_BUFFER_DESC vb_bd = { };

		vb_bd.ByteWidth				= elementSize * elementCount;
		vb_bd.Usage					= D3D10_USAGE_DYNAMIC;
		vb_bd.BindFlags				= D3D10_BIND_VERTEX_BUFFER;
		vb_bd.CPUAccessFlags		= D3D10_CPU_ACCESS_WRITE;
		vb_bd.MiscFlags				= 0;

		if (FAILED(gDevice->CreateBuffer(&vb_bd, 0, &vb->vb)))
		{
			delete vb;
			return 0;
		}

		return vb;
	}

	void DestroyVertexBuffer(VertexBuffer* vb)
	{
		if (vb)
		{
			vb->vb->Release();
			delete vb;
		}
	}

	void* Map(VertexBuffer* vb)
	{
		if (vb)
		{
			void* data = 0;

			if (SUCCEEDED(vb->vb->Map(D3D10_MAP_WRITE_DISCARD, 0, &data)))
				return data;
		}

		return 0;
	}

	void Unmap(VertexBuffer* vb)
	{
		if (vb)
			vb->vb->Unmap();
	}

	// ShaderDecl

	struct ShaderDecl
	{
		ID3D10VertexShader* vertex;
		ID3D10PixelShader* pixel;
		ID3D10InputLayout* il;
	};

	ShaderDecl* CreateShaderDecl(void* vertexShader, int vertexShaderLength, void* pixelShader, int pixelShaderLength)
	{
		ShaderDecl* decl = new ShaderDecl;

		if (!decl)
			return 0;

		if (FAILED(gDevice->CreateVertexShader(vertexShader, vertexShaderLength, &decl->vertex)))
		{
			Panic("CreateVertexShader failed");
		}

		if (FAILED(gDevice->CreatePixelShader(pixelShader, pixelShaderLength, &decl->pixel)))
		{
			Panic("CreatePixelShader failed");
		}

		D3D10_INPUT_ELEMENT_DESC ild[3];

		ild[0].SemanticName			= "POSITION";
		ild[0].SemanticIndex		= 0;
		ild[0].Format				= DXGI_FORMAT_R32G32_FLOAT;
		ild[0].InputSlot			= 0;
		ild[0].AlignedByteOffset	= (intptr_t)&((Vertex*)0)->pos;
		ild[0].InputSlotClass		= D3D10_INPUT_PER_VERTEX_DATA;
		ild[0].InstanceDataStepRate	= 0;

		ild[1].SemanticName			= "TEXCOORD";
		ild[1].SemanticIndex		= 0;
		ild[1].Format				= DXGI_FORMAT_R32G32_FLOAT;
		ild[1].InputSlot			= 0;
		ild[1].AlignedByteOffset	= (intptr_t)&((Vertex*)0)->uv;
		ild[1].InputSlotClass		= D3D10_INPUT_PER_VERTEX_DATA;
		ild[1].InstanceDataStepRate	= 0;

		ild[2].SemanticName			= "COLOR";
		ild[2].SemanticIndex		= 0;
		ild[2].Format				= DXGI_FORMAT_R32G32B32A32_FLOAT;
		ild[2].InputSlot			= 0;
		ild[2].AlignedByteOffset	= (intptr_t)&((Vertex*)0)->colour;
		ild[2].InputSlotClass		= D3D10_INPUT_PER_VERTEX_DATA;
		ild[2].InstanceDataStepRate	= 0;

		if (FAILED(gDevice->CreateInputLayout(ild, 3, vertexShader, vertexShaderLength, &decl->il)))
		{
			Panic("CreateInputLayout failed");
		}

		return decl;
	}

	void DestroyShaderDecl(ShaderDecl* decl)
	{
		if (decl)
		{
			decl->vertex->Release();
			decl->pixel->Release();
			decl->il->Release();

			delete decl;
		}
	}

	// Texture2d

	struct Texture2d
	{
		ID3D10Resource* tex;
		ID3D10SamplerState* sampler;
		ID3D10ShaderResourceView* srv;
	};

	Texture2d* LoadTexture2d(const char* path)
	{
		Texture2d* tex = new Texture2d;

		if (!tex)
			return 0;

		D3DX10_IMAGE_LOAD_INFO ili;

		ili.MipLevels = 1;
		ili.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		if (FAILED(D3DX10CreateTextureFromFileA(gDevice, path, &ili, 0, &tex->tex, 0)))
		{
			delete tex;
			return 0;
		}

		D3D10_SAMPLER_DESC sd = { };

		sd.Filter			= D3D10_FILTER_MIN_MAG_MIP_POINT;
		sd.AddressU			= D3D10_TEXTURE_ADDRESS_WRAP;
		sd.AddressV			= D3D10_TEXTURE_ADDRESS_WRAP;
		sd.AddressW			= D3D10_TEXTURE_ADDRESS_WRAP;
		sd.ComparisonFunc	= D3D10_COMPARISON_NEVER;

		if (FAILED(gDevice->CreateSamplerState(&sd, &tex->sampler)))
		{
			tex->tex->Release();
			delete tex;
			return 0;
		}

		if (FAILED(gDevice->CreateShaderResourceView(tex->tex, 0, &tex->srv)))
		{
			tex->tex->Release();
			tex->sampler->Release();
			delete tex;
			return 0;
		}

		return tex;
	}

	void DestroyTexture2d(Texture2d* tex)
	{
		if (tex)
		{
			tex->srv->Release();
			tex->sampler->Release();
			tex->tex->Release();
			delete tex;
		}
	}

	void SetTexture(Texture2d* tex)
	{
		if (tex)
		{
			gDevice->PSSetShaderResources(0, 1, &tex->srv);
			gDevice->PSSetSamplers(0, 1, &tex->sampler);
		}
	}

	// Drawing

	ID3D10RenderTargetView* gRtv;
	ID3D10BlendState* gBlendState;

	void Init(ID3D10RenderTargetView* rtv, ID3D10BlendState* blendState)
	{
		gRtv = rtv;
		gBlendState = blendState;
	}

	void Clear(const Colour& colour)
	{
		gDevice->ClearRenderTargetView(gRtv, (float*)&colour);
	}

	void Draw(ShaderDecl* decl, VertexBuffer* vb, int count)
	{
		if (!decl || !vb)
			return;

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		gDevice->OMSetBlendState(gBlendState, 0, 0xFFFFFFFF);

		gDevice->IASetInputLayout(decl->il);
		gDevice->IASetVertexBuffers(0, 1, &vb->vb, &stride, &offset);
		gDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		gDevice->VSSetShader(decl->vertex);
		gDevice->PSSetShader(decl->pixel);

		gDevice->Draw(count, 0);
	}

}