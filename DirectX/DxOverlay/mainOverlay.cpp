// #include <d3d9.h>
////#include <d3dx9.h>
//#include <windows.h>
//
//using namespace std;
//
//// Import libraries to link with
//#pragma comment(lib, "d3d9.lib")
////#pragma comment(lib, "d3dx9.lib")
//
////  Globals var
//int overlayWidth = 256;
//int overlayHeight = 256;
//
//#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
//
//template <class T> void SafeRelease(T** ppT) {
//	if (*ppT) { (*ppT)->Release(); *ppT = NULL; }
//}
//
//HRESULT CreateHWOverlay(HWND hwnd, IDirect3D9Ex* pD3D, IDirect3DDevice9Ex** ppDevice) {
//	*ppDevice = NULL;
//
//	D3DCAPS9 caps;
//	ZeroMemory(&caps, sizeof(caps));
//
//	HRESULT hr = pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
//
//	if (FAILED(hr)) { return hr; }
//
//	// Check if overlay is supported.
//	if (!(caps.Caps & D3DCAPS_OVERLAY)) { return D3DERR_UNSUPPORTEDOVERLAY; }
//
//	D3DOVERLAYCAPS overlayCaps = { 0 };
//	IDirect3DDevice9Ex* pDevice = NULL;
//	IDirect3D9ExOverlayExtension* pOverlay = NULL;
//
//	// Check specific overlay capabilities.
//	hr = pD3D->QueryInterface(IID_PPV_ARGS(&pOverlay));
//
//	if (SUCCEEDED(hr)) {
//		hr = pOverlay->CheckDeviceOverlayType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, overlayWidth, overlayHeight, D3DFMT_X8R8G8B8, NULL, D3DDISPLAYROTATION_IDENTITY, &overlayCaps);
//	}
//
//	// Create the overlay.
//	if (SUCCEEDED(hr)) {
//		DWORD flags = D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED | D3DCREATE_SOFTWARE_VERTEXPROCESSING;
//
//		D3DPRESENT_PARAMETERS pp = { 0 };
//		pp.BackBufferWidth = overlayCaps.MaxOverlayDisplayWidth;
//		pp.BackBufferHeight = overlayCaps.MaxOverlayDisplayHeight;
//		pp.BackBufferFormat = D3DFMT_X8R8G8B8;
//		pp.SwapEffect = D3DSWAPEFFECT_OVERLAY;
//		pp.hDeviceWindow = hwnd;
//		pp.Windowed = TRUE;
//		pp.Flags = D3DPRESENTFLAG_VIDEO;
//		pp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
//		pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
//
//		hr = pD3D->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, flags, &pp, NULL, &pDevice);
//	}
//
//	if (SUCCEEDED(hr)) { (*ppDevice) = pDevice; (*ppDevice)->AddRef(); }
//
//	SafeRelease(&pD3D);
//	SafeRelease(&pDevice);
//	SafeRelease(&pOverlay);
//	return hr;
//}
//
//struct CUSTOMVERTEX
//{
//	FLOAT x, y, z, rhw;    // from the D3DFVF_XYZRHW flag
//	DWORD color;    // from the D3DFVF_DIFFUSE flag
//};
//
//
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
//	HRESULT            hr;
//	IDirect3D9Ex* g_pD3D = NULL;
//	IDirect3DDevice9Ex* g_pD3DDevice = NULL;
//	//ID3DXFont* g_font = NULL;
//
//	IDirect3DVertexBuffer9* vBuffer = nullptr;
//	CUSTOMVERTEX OurVertices[3];
//
//
//	// Finding target window
//	// HWND hwndShell = NULL; // Doesn't work without a window entirely :(
//	HWND targetWindow = FindWindowW(NULL, L"MyWindowClass");
//	if (targetWindow == NULL) {
//		MessageBox(NULL, L"Target window not found.", L"Error", MB_ICONERROR);
//		return EXIT_FAILURE;
//	}
//
//	// Getting target window sizes
//	RECT targetWindowLocation, targetWindowClientArea; // The "client area" is the area that gets drawn, this is the area of interest for the overlay
//	POINT pos00InTargetOnScreen = { 0,0 };
//	GetWindowRect(targetWindow, &targetWindowLocation); // Get coordinates of target window edges
//	int targetWindowWidth = targetWindowLocation.right - targetWindowLocation.left; // Calculate width & height from coords
//	int targetWindowHeight = targetWindowLocation.bottom - targetWindowLocation.top;
//	GetClientRect(targetWindow, &targetWindowClientArea); // Get coordinates of target client area
//	overlayWidth = targetWindowClientArea.right - targetWindowClientArea.left; // Calculate width & height from coords
//	overlayHeight = targetWindowClientArea.bottom - targetWindowClientArea.top;
//	ClientToScreen(targetWindow, &pos00InTargetOnScreen); // Converts the point specified (0,0 in this case) in the target client area into on-screen position
//
//	// Creating RECTs and D3DRECTs for DirectX functions
//	D3DRECT d3dRectOverlay = { 0, 0, overlayWidth, overlayHeight };
//	RECT rectOverlay = { 0, 0, overlayWidth, overlayHeight };
//
//	// Presentation prefs
//	D3DPRESENT_PARAMETERS pp;
//	pp.BackBufferWidth = overlayWidth;
//	pp.BackBufferHeight = overlayHeight;
//	pp.BackBufferFormat = D3DFMT_A8R8G8B8; //Back buffer format with alpha channel
//	pp.BackBufferCount = 1;
//	pp.MultiSampleType = D3DMULTISAMPLE_NONMASKABLE; // AA
//	pp.MultiSampleQuality = 0; // AA Quality
//	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
//	pp.hDeviceWindow = targetWindow; // Window
//	pp.Windowed = true; // winowed
//	pp.EnableAutoDepthStencil = true;
//	pp.AutoDepthStencilFormat = D3DFMT_D24S8; // 
//	pp.Flags = 0;
//	pp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
//	pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
//
//	// Create a Direct3D object
//	hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &g_pD3D);
//	if (hr != S_OK) {
//		MessageBox(NULL, L"Direct3DCreate9Ex failed.", L"Error", MB_ICONERROR);
//		return EXIT_FAILURE;
//	}
//
//	// Create a Direct3D device object
//	hr = g_pD3D->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, targetWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, NULL, &g_pD3DDevice);
//	if (hr != S_OK) {
//		MessageBox(NULL, L"CreateDeviceEx failed.", L"Error", MB_ICONERROR);
//		return EXIT_FAILURE;
//	}
//
//	hr = CreateHWOverlay(targetWindow, g_pD3D, &g_pD3DDevice);
//	if (hr != S_OK) {
//		MessageBox(NULL, L"CreateHWOverlay failed.", L"Error", MB_ICONERROR);
//		return EXIT_FAILURE;
//	}
//
//	hr = g_pD3DDevice->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX),
//		0,
//		CUSTOMFVF,
//		D3DPOOL_DEFAULT,
//		&vBuffer,
//		NULL);
//
//	if (hr != S_OK) {
//		MessageBox(NULL, L"CreateVertexBuffer failed.", L"Error", MB_ICONERROR);
//		return EXIT_FAILURE;
//	}
//
//	VOID* pVoid;    // the void* we were talking about
//
//	vBuffer->Lock(0, 0, (void**)&pVoid, 0);    // locks v_buffer, the buffer we made earlier
//	memcpy(pVoid, OurVertices, sizeof(OurVertices));    // copy vertices to the vertex buffer
//	vBuffer->Unlock();    // unlock v_buffer
//
//	//hr = D3DXCreateFont(g_pD3DDevice, 30, 10, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &g_font);
//	//if (hr != S_OK) {
//	//	MessageBox(NULL, L"D3DXCreateFont failed.", L"Error", MB_ICONERROR);
//	//	return EXIT_FAILURE;
//	//}
//
//	Sleep(2000); // To give the user the time to make target window active
//
//	while (true) {
//		g_pD3DDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
//		g_pD3DDevice->BeginScene();
//		// Draw things here
//		//g_font->DrawTextA(NULL, "Direct3D 9\nHardware overlay example\nBy harakirinox\nFor UnknownCheats.me", -1, &rectOverlay, DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 0, 0));
//
//
//		g_pD3DDevice->SetStreamSource(0, vBuffer, 0, sizeof(CUSTOMVERTEX));
//		g_pD3DDevice->SetFVF(CUSTOMFVF);
//		g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
//
//		g_pD3DDevice->EndScene();
//		g_pD3DDevice->PresentEx(&rectOverlay, &rectOverlay, targetWindow, NULL, 0);
//	}
//
//	return EXIT_SUCCESS;
//}
