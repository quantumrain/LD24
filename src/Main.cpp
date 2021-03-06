// Copyright 2012 Stephen Cakebread

#include "Pch.h"
#include "Common.h"
#include "resource.h"

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
	MessageBoxA(gMainWnd, msg, "LD24 - Super Conga Kat", MB_ICONERROR | MB_OK);
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

IDirect3D9* gD3d;
IDirect3DDevice9* gDevice;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// Window

	WNDCLASSEX wc = { sizeof(wc) };

	wc.lpszClassName = L"MainWnd";
	wc.lpfnWndProc = MainWndProc;
	wc.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_LD24));
	wc.hCursor = LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW));

	RegisterClassEx(&wc);

	DWORD	style	= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	DWORD	styleEx = WS_EX_WINDOWEDGE;
	RECT	rcWin	= { 0, 0, kWinWidth * 4, kWinHeight * 4 };

	AdjustWindowRectEx(&rcWin, style, FALSE, styleEx);
	OffsetRect(&rcWin, 100, 100);

	gMainWnd = CreateWindowEx(styleEx, wc.lpszClassName, L"LD24 - Super Conga Kat", style, rcWin.left, rcWin.top, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, 0, 0, 0, 0);

	ShowWindow(gMainWnd, SW_SHOWNORMAL);

	// Graphics

	if ((gD3d = Direct3DCreate9(D3D_SDK_VERSION)) == 0)
	{
		Panic("D3DCreate failed - do you have D3D9 installed?");
	}

	D3DPRESENT_PARAMETERS pp = { };

	pp.Windowed			= TRUE;
	pp.SwapEffect		= D3DSWAPEFFECT_DISCARD;
	pp.BackBufferWidth	= kWinWidth;
	pp.BackBufferHeight	= kWinHeight;
	pp.BackBufferFormat	= D3DFMT_A8R8G8B8;
	pp.hDeviceWindow	= gMainWnd;
	pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (FAILED(gD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, gMainWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &gDevice)))
	{
		Panic("D3D CreateDevice failed - do you have D3D9 installed?");
	}

	D3DVIEWPORT9 vp;
    vp.Width = kWinWidth;
    vp.Height = kWinHeight;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    vp.X = 0;
    vp.Y = 0;

	gDevice->SetViewport(&vp);

	gpu::Init();

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
			if (gDevice)
			{
				gDevice->BeginScene();

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

				gDevice->EndScene();
				gDevice->Present(0, 0, 0, 0);
			}

			Sleep(gHasFocus ? 0 : 250);
		}
	}
	
	gDevice->SetVertexDeclaration(0);
	gDevice->SetVertexShader(0);
	gDevice->SetPixelShader(0);
	gDevice->SetTexture(0, 0);

	SoundShutdown();

	void RenderShutdown();
	RenderShutdown();

	gDevice->Release();
	gD3d->Release();

	DestroyWindow(gMainWnd);

	return 0;
}