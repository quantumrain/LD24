#include "Pch.h"
#include "Common.h"
#include "ShaderPsh.h"
#include "ShaderVsh.h"

const int kWinWidth		= 160;
const int kWinHeight	= 90;

bool gHasFocus;
bool gKeyUp;
bool gKeyDown;
bool gKeyLeft;
bool gKeyRight;
bool gKeyFire;

void DebugLn(const char* txt, ...)
{
	char buf[512];

	va_list ap;

	va_start(ap, txt);
	_vsnprintf_s(buf, sizeof(buf), _TRUNCATE, txt, ap);
	va_end(ap);

	OutputDebugStringA(buf);
	OutputDebugStringA("\r\n");
}

HWND gMainWnd;

void Panic(const char* msg)
{
	MessageBoxA(gMainWnd, msg, "LD24", MB_ICONERROR | MB_OK);
	ExitProcess(0);
}

Colour gClearColour(0.1f, 0.1f, 0.1f, 1.0f);

void ClearColour(const Colour& clearColour)
{
	gClearColour = clearColour;
}

struct Vertex
{
	Vector2 pos;
	Colour colour;
};

const int kMaxLineVerts = 10 * 1024;
Vertex gLineVerts[kMaxLineVerts];
int gLineVertCount;

void DrawLine(Vector2 start, Vector2 end, Colour colour)
{
	if ((gLineVertCount + 2) > kMaxLineVerts)
	{
		DebugLn("DrawLine overflow");
		return;
	}

	Vertex* v = &gLineVerts[gLineVertCount];

	gLineVertCount += 2;

	Vector2 scale(2.0f / (float)kWinWidth, -2.0f / (float)kWinHeight);

	v[0].pos = start * scale;
	v[0].colour = colour;
	v[1].pos = end * scale;
	v[1].colour = colour;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_ACTIVATE:
			gHasFocus = wparam != WA_INACTIVE;
		break;

		case WM_SYSKEYDOWN:
			if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(VK_F4) & 0x8000))
			{
				PostQuitMessage(0);
			}
		return 0;

		case WM_CLOSE:
			PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

ID3D10Device* gDevice;
ID3D10Buffer* gVb;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// Window

	WNDCLASSEX wc = { sizeof(wc) };

	wc.lpszClassName = L"MainWnd";
	wc.lpfnWndProc = MainWndProc;
	wc.hCursor = LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW));

	RegisterClassEx(&wc);

	DWORD	style	= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	DWORD	styleEx = WS_EX_WINDOWEDGE;
	RECT	rcWin	= { 0, 0, kWinWidth * 8, kWinHeight * 8 };

	AdjustWindowRectEx(&rcWin, style, FALSE, styleEx);
	OffsetRect(&rcWin, 100, 100);

	gMainWnd = CreateWindowEx(styleEx, wc.lpszClassName, L"LD24", style, rcWin.left, rcWin.top, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, 0, 0, 0, 0);

	ShowWindow(gMainWnd, SW_SHOWNORMAL);

	// Graphics

	DXGI_SWAP_CHAIN_DESC scd = { };

	scd.BufferDesc.Width					= kWinWidth;
	scd.BufferDesc.Height					= kWinHeight;
	scd.BufferDesc.RefreshRate.Numerator	= 1;
	scd.BufferDesc.RefreshRate.Denominator	= 60;
	scd.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering			= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling					= DXGI_MODE_SCALING_CENTERED;

	scd.SampleDesc.Count	= 1;
	scd.SampleDesc.Quality	= 0;

	scd.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount		= 1;
	scd.OutputWindow	= gMainWnd;
	scd.Windowed		= TRUE;
	scd.SwapEffect		= DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags			= 0;

	IDXGISwapChain* sc = 0;

	if (FAILED(D3D10CreateDeviceAndSwapChain(
			/* adapter */				0,
			/* driver type */			D3D10_DRIVER_TYPE_HARDWARE,
			/* software */				0,
			/* flags */					D3D10_CREATE_DEVICE_DEBUG,
			/* sdk version */			D3D10_SDK_VERSION,
			/* swap chain desc */		&scd,
			/* swap chain */			&sc,
			/* device */				&gDevice
		)))
	{
		Panic("D3D CreateDevice failed - do you have D3D11 installed?");
	}

	ID3D10Texture2D* bb;
	sc->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&bb);

	ID3D10RenderTargetView* rtv = 0;
	gDevice->CreateRenderTargetView(bb, 0, &rtv);

	bb->Release();

	gDevice->OMSetRenderTargets(1, &rtv, 0);

    D3D10_VIEWPORT vp;
    vp.Width = kWinWidth;
    vp.Height = kWinHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gDevice->RSSetViewports(1, &vp);

	D3D10_BUFFER_DESC vb_bd = { };
	vb_bd.ByteWidth				= sizeof(Vertex) * kMaxLineVerts;
	vb_bd.Usage					= D3D10_USAGE_DYNAMIC;
	vb_bd.BindFlags				= D3D10_BIND_VERTEX_BUFFER;
	vb_bd.CPUAccessFlags		= D3D10_CPU_ACCESS_WRITE;
	vb_bd.MiscFlags				= 0;

	if (FAILED(gDevice->CreateBuffer(&vb_bd, 0, &gVb)))
	{
		Panic("CreateBuffer VB failed");
	}

	ID3D10VertexShader* vertexShader = 0;
	ID3D10PixelShader* pixelShader = 0;

	if (FAILED(gDevice->CreateVertexShader(gShaderVsh, sizeof(gShaderVsh), &vertexShader)))
	{
		Panic("CreateVertexShader failed");
	}

	if (FAILED(gDevice->CreatePixelShader(gShaderPsh, sizeof(gShaderPsh), &pixelShader)))
	{
		Panic("CreatePixelShader failed");
	}

	D3D10_INPUT_ELEMENT_DESC ild[2];

	ild[0].SemanticName			= "POSITION";
	ild[0].SemanticIndex		= 0;
	ild[0].Format				= DXGI_FORMAT_R32G32_FLOAT;
	ild[0].InputSlot			= 0;
	ild[0].AlignedByteOffset	= (intptr_t)&((Vertex*)0)->pos;
	ild[0].InputSlotClass		= D3D10_INPUT_PER_VERTEX_DATA;
	ild[0].InstanceDataStepRate	= 0;

	ild[1].SemanticName			= "COLOR";
	ild[1].SemanticIndex		= 0;
	ild[1].Format				= DXGI_FORMAT_R32G32B32A32_FLOAT;
	ild[1].InputSlot			= 0;
	ild[1].AlignedByteOffset	= (intptr_t)&((Vertex*)0)->colour;
	ild[1].InputSlotClass		= D3D10_INPUT_PER_VERTEX_DATA;
	ild[1].InstanceDataStepRate	= 0;

	ID3D10InputLayout* il = 0;
	if (FAILED(gDevice->CreateInputLayout(ild, 2, gShaderVsh, sizeof(gShaderVsh), &il)))
	{
		Panic("CreateInputLayout failed");
	}

	// Audio

	SoundInit();

	// Keys

	int locKeyW = MapVirtualKey(0x11, MAPVK_VSC_TO_VK);
	int locKeyS = MapVirtualKey(0x1F, MAPVK_VSC_TO_VK);
	int locKeyA = MapVirtualKey(0x1E, MAPVK_VSC_TO_VK);
	int locKeyD = MapVirtualKey(0x20, MAPVK_VSC_TO_VK);
	int locKeyZ = MapVirtualKey(0x2C, MAPVK_VSC_TO_VK);

	// Main

	for(;;)
	{
		MSG msg;
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (sc)
			{
				gLineVertCount = 0;

				gKeyUp		= gHasFocus && (((GetAsyncKeyState(VK_UP) & 0x8000) != 0) || ((GetAsyncKeyState(locKeyW) & 0x8000) != 0));
				gKeyDown	= gHasFocus && (((GetAsyncKeyState(VK_DOWN) & 0x8000) != 0) || ((GetAsyncKeyState(locKeyS) & 0x8000) != 0));
				gKeyLeft	= gHasFocus && (((GetAsyncKeyState(VK_LEFT) & 0x8000) != 0) || ((GetAsyncKeyState(locKeyA) & 0x8000) != 0));
				gKeyRight	= gHasFocus && (((GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0) || ((GetAsyncKeyState(locKeyD) & 0x8000) != 0));
				gKeyFire	= gHasFocus && (((GetAsyncKeyState(VK_SPACE) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RETURN) & 0x8000) != 0) || ((GetAsyncKeyState(locKeyZ) & 0x8000) != 0));

				void GameUpdate();
				GameUpdate();

				void* mappedData = 0;

				if (SUCCEEDED(gVb->Map(D3D10_MAP_WRITE_DISCARD, 0, &mappedData)))
				{
					memcpy(mappedData, gLineVerts, gLineVertCount * sizeof(Vertex));

					gVb->Unmap();
				}

				gDevice->ClearRenderTargetView(rtv, (float*)&gClearColour);

				UINT stride = sizeof(Vertex);
				UINT offset = 0;

				gDevice->IASetInputLayout(il);
				gDevice->IASetVertexBuffers(0, 1, &gVb, &stride, &offset);
				gDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

				gDevice->VSSetShader(vertexShader);
				gDevice->PSSetShader(pixelShader);

				gDevice->Draw(gLineVertCount, 0);

				sc->Present(1, 0);
			}

			Sleep(gHasFocus ? 0 : 250);
		}
	}
	
	gDevice->ClearState();

	SoundShutdown();

	sc->SetFullscreenState(FALSE, 0);

	il->Release();
	vertexShader->Release();
	pixelShader->Release();
	gVb->Release();
	rtv->Release();
	sc->Release();
	gDevice->Release();

	DestroyWindow(gMainWnd);

	return 0;
}