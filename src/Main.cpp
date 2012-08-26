// Copyright 2012 Stephen Cakebread

#include "Pch.h"
#include "Common.h"

int kWinWidth	= 288;
int kWinHeight	= 160;

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
	RECT	rcWin	= { 0, 0, kWinWidth * 4, kWinHeight * 4 };

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
			/* flags */					0,
			/* sdk version */			D3D10_SDK_VERSION,
			/* swap chain desc */		&scd,
			/* swap chain */			&sc,
			/* device */				&gDevice
		)))
	{
		Panic("D3D CreateDevice failed - do you have D3D10 installed?");
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

	D3D10_BLEND_DESC bd = { 0 };

	bd.AlphaToCoverageEnable = FALSE;
	bd.BlendEnable[0] = TRUE;
	bd.SrcBlend = D3D10_BLEND_SRC_ALPHA;
	bd.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
	bd.BlendOp = D3D10_BLEND_OP_ADD;
	bd.SrcBlendAlpha = D3D10_BLEND_ONE;
	bd.DestBlendAlpha = D3D10_BLEND_ONE;
	bd.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	bd.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

	ID3D10BlendState* blendState = 0;
	if (FAILED(gDevice->CreateBlendState(&bd, &blendState)))
	{
		Panic("CreateBlendState failed");
	}

	gpu::Init(rtv, blendState);

	// Audio

	SoundInit();

	// Keys

	int locKeyW = MapVirtualKey(0x11, MAPVK_VSC_TO_VK);
	int locKeyS = MapVirtualKey(0x1F, MAPVK_VSC_TO_VK);
	int locKeyA = MapVirtualKey(0x1E, MAPVK_VSC_TO_VK);
	int locKeyD = MapVirtualKey(0x20, MAPVK_VSC_TO_VK);
	int locKeyZ = MapVirtualKey(0x2C, MAPVK_VSC_TO_VK);

	// Main

	void RenderInit();
	RenderInit();

	void GameInit();
	GameInit();

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
				void RenderPreUpdate();
				RenderPreUpdate();

				gKeyUp		= gHasFocus && (((GetAsyncKeyState(VK_UP) & 0x8000) != 0) || ((GetAsyncKeyState(locKeyW) & 0x8000) != 0));
				gKeyDown	= gHasFocus && (((GetAsyncKeyState(VK_DOWN) & 0x8000) != 0) || ((GetAsyncKeyState(locKeyS) & 0x8000) != 0));
				gKeyLeft	= gHasFocus && (((GetAsyncKeyState(VK_LEFT) & 0x8000) != 0) || ((GetAsyncKeyState(locKeyA) & 0x8000) != 0));
				gKeyRight	= gHasFocus && (((GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0) || ((GetAsyncKeyState(locKeyD) & 0x8000) != 0));
				gKeyFire	= gHasFocus && (((GetAsyncKeyState(VK_SPACE) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RETURN) & 0x8000) != 0) || ((GetAsyncKeyState(locKeyZ) & 0x8000) != 0));

				void GameUpdate();
				GameUpdate();

				void RenderGame();
				RenderGame();

				sc->Present(1, 0);
			}

			Sleep(gHasFocus ? 0 : 250);
		}
	}
	
	gDevice->ClearState();
	sc->SetFullscreenState(FALSE, 0);

	SoundShutdown();

	void RenderShutdown();
	RenderShutdown();

	rtv->Release();
	sc->Release();
	gDevice->Release();

	DestroyWindow(gMainWnd);

	return 0;
}