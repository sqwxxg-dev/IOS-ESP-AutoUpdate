
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <vector>
#include <iostream>
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")

//imgui
#include "imgui\imgui.h"
#include "imgui\imgui_impl_win32.h"
#include "imgui\imgui_impl_dx11.h"

//detours
#include "detours.h"
#if defined _M_X64
#pragma comment(lib, "detours.X64/detours.lib")
#elif defined _M_IX86
#pragma comment(lib, "detours.X86/detours.lib")
#endif

#pragma warning( disable : 4244 )


typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(__stdcall *D3D11ResizeBuffersHook) (IDXGISwapChain *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

D3D11PresentHook phookD3D11Present = NULL;
D3D11ResizeBuffersHook phookD3D11ResizeBuffers = NULL;

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;

DWORD_PTR* pSwapChainVtable = NULL;
DWORD_PTR* pContextVTable = NULL;
DWORD_PTR* pDeviceVTable = NULL;

#include "main.h" //helper funcs

int width;
int height;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	POINT mPos;
	GetCursorPos(&mPos);
	ScreenToClient(window, &mPos);
	ImGui::GetIO().MousePos.x = mPos.x;
	ImGui::GetIO().MousePos.y = mPos.y;

	if (uMsg == WM_KEYUP)
	{
		if (wParam == VK_F8)
		{
			if(ShowMenu)
				io.MouseDrawCursor = true;
			else
				io.MouseDrawCursor = false;
		}
	}

	if (ShowMenu)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return true;
	}

	return CallWindowProc(OriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall hookD3D11ResizeBuffers(IDXGISwapChain *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	ImGui_ImplDX11_InvalidateDeviceObjects();
	if (nullptr != RenderTargetView) { RenderTargetView->Release(); RenderTargetView = nullptr; }

	HRESULT toReturn = phookD3D11ResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	ImGui_ImplDX11_CreateDeviceObjects();
	
	return toReturn;
}


void DrawMenu() {
	
	ImGui::SetNextWindowSize(ImVec2(520.0f, 440.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(width * 0.5f, height * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (!ImGui::Begin("##PornoSiskiPiski", nullptr,
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar ))
	{
		ImGui::End();
		return;
	}

	if (ImGui::BeginTabBar("##MainTabBar", ImGuiTabBarFlags_NoTooltip))
	{
		// ================= VISUALS =================
		if (ImGui::BeginTabItem("Visuals"))
		{
			ImGui::Spacing();

			ImGui::Text("Esp Mode: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.0f);
			ImGui::Combo("##ESPMode", &EspMode, EspModes, IM_ARRAYSIZE(EspModes));
			ImGui::SameLine();
			if (ImGui::ColorEdit4("##ColorofEsp", (float*)&EspTemp, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs)) {
				DrawEspColor = ImGui::ColorConvertFloat4ToU32(EspTemp);
			}
			ImGui::Spacing();

			ImGui::Text("Fill Mode: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.0f);
			ImGui::Combo("##FILLMode", &FillMode, FillModes, IM_ARRAYSIZE(FillModes));
			ImGui::SameLine();
			if (ImGui::ColorEdit4("Main color", (float*)&FillMainTemp, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs)) {
				FillMainColor = ImGui::ColorConvertFloat4ToU32(FillMainTemp);
			}
			ImGui::SameLine();
			if (ImGui::ColorEdit4("Second color", (float*)&FillSecTemp, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs)) {
				FillSecColor = ImGui::ColorConvertFloat4ToU32(FillSecTemp);
			}
			ImGui::Spacing();

			ImGui::Checkbox("Draw tracers", &Draw_Tracers);
			ImGui::SameLine();
			if (ImGui::ColorEdit4("##ColorofTracers", (float*)&TracersTemp, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs)) {
				DrawTracersColor = ImGui::ColorConvertFloat4ToU32(TracersTemp);
			}
			ImGui::Spacing();

			ImGui::Checkbox("Show distance", &Draw_Distance);
			ImGui::SameLine();
			if (ImGui::ColorEdit4("##ColorofText", (float*)&TextTemp, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs)) {
				DrawTextColor = ImGui::ColorConvertFloat4ToU32(TextTemp);
			}
			ImGui::Spacing();

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

std::vector<unsigned int> MatrixOffsets = { 0x0, 0x18, 0x100 };
uintptr_t ReadPointer(uintptr_t ptr, const std::vector<unsigned int>& offsets)
{
	uintptr_t addr = ptr;

	__try {
		for (unsigned int offset : offsets)
		{
			if (addr < 0x10000) return 0; 
			addr = *(uintptr_t*)addr;
			addr += offset;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return 0;
	}

	return addr;
}
bool WorldToScreen(const Vec3& pos, Vec2& screen, const Matrix16& matrix)
{
	Vec4 clip;
	clip.x = pos.x * matrix.a + pos.y * matrix.e + pos.z * matrix.i + matrix.m;
	clip.y = pos.x * matrix.b + pos.y * matrix.f + pos.z * matrix.j + matrix.n;
	clip.z = pos.x * matrix.c + pos.y * matrix.g + pos.z * matrix.k + matrix.o;
	clip.w = pos.x * matrix.d + pos.y * matrix.h + pos.z * matrix.l + matrix.p;

	const float epsilon = 1e-6f;
	if (clip.w <= epsilon)
		return false;

	float invW = 1.0f / clip.w;
	float ndcX = clip.x * invW;
	float ndcY = clip.y * invW;

	screen.x = (ndcX + 1.0f) * 0.5f * width;
	screen.y = (1.0f - ndcY) * 0.5f * height;

	return true;
}

void Draw() {
	if (!b_HookEnemyCords || !b_HookLocalCords || PlayerDataBuffer == nullptr || LocalData == nullptr) return;

	PlayerData LocalPlayer = *(PlayerData*)LocalData;
	Vec3 MyPos = (LocalPlayer.Coords).ToFloat();

	static const uintptr_t MatrixBase = (uintptr_t)p_UnityPlayer + 0x1C22D00;
	uintptr_t MatrixAddr = ReadPointer(MatrixBase, MatrixOffsets);
	if (MatrixAddr == 0) return;
	Matrix16 vpMatrix = *(Matrix16*)MatrixAddr;

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
	//список отрисованных за кадр
	std::set<uintptr_t> drawnEntities;

	//Очистка буфера, фикс остающихся в меню боксов и остающихся после манекенов в тренировке боксов
	static DWORD lastTime = timeGetTime();
	DWORD timePassed = timeGetTime() - lastTime;
	if (timePassed > 1500)
	{
		lastTime = timeGetTime();
		memset(PlayerDataBuffer, 0, 320);
	}

	for (int i = 0; i < 10; i++) {

		PlayerData EnemyData = *(PlayerData*)(PlayerDataBuffer + i * 32);
		//проверка на самого себя и пустоту
		if (LocalPlayer.baseaddr == EnemyData.baseaddr || EnemyData.baseaddr == 0) continue;
		//проверка рисовался ли уже этот игрок
		if (drawnEntities.find(EnemyData.baseaddr) != drawnEntities.end()) continue;

		Vec2 ScreenPos;
		Vec3 EnemyPos = (EnemyData.Coords).ToFloat();

		if (WorldToScreen(EnemyPos, ScreenPos, vpMatrix)) {
			float distance = (EnemyPos - MyPos).Length();
			if (distance > 0.5f) {
				float scale = 10.0f / distance;
				float fW = 70.0f * scale;
				float fH = 140.0f * scale;
				ImVec2 box_min = ImVec2(ScreenPos.x - fW * 0.5, ScreenPos.y - fH * 0.5);
				ImVec2 box_max = ImVec2(ScreenPos.x + fW * 0.5, ScreenPos.y + fH * 0.5);
				if (FillMode != 0) {
					if (FillMode == 1) {
						draw_list->AddRectFilled(box_min, box_max, FillMainColor, 2.0f, 0);
					}
					if (FillMode == 2) {
						draw_list->AddRectFilledMultiColor(box_min, box_max, FillSecColor, FillSecColor, FillMainColor, FillMainColor);
					}
				}
				if (EspMode != 0) {
					if (EspMode == 1) {
						draw_list->AddRect(box_min, box_max, DrawEspColor, 0.0f, 0, 1.5f);
					}
					if (EspMode == 2) {
						float lineOffset = fW * 0.25f;
						static float thickness = 1.5f;
						draw_list->AddLine(ImVec2(box_min.x, box_min.y), ImVec2(box_min.x + lineOffset, box_min.y), DrawEspColor, thickness);
						draw_list->AddLine(ImVec2(box_min.x, box_min.y), ImVec2(box_min.x, box_min.y + lineOffset), DrawEspColor, thickness);
						draw_list->AddLine(ImVec2(box_max.x, box_min.y), ImVec2(box_max.x - lineOffset, box_min.y), DrawEspColor, thickness);
						draw_list->AddLine(ImVec2(box_max.x, box_min.y), ImVec2(box_max.x, box_min.y + lineOffset), DrawEspColor, thickness);
						draw_list->AddLine(ImVec2(box_min.x, box_max.y), ImVec2(box_min.x + lineOffset, box_max.y), DrawEspColor, thickness);
						draw_list->AddLine(ImVec2(box_min.x, box_max.y), ImVec2(box_min.x, box_max.y - lineOffset), DrawEspColor, thickness);
						draw_list->AddLine(ImVec2(box_max.x, box_max.y), ImVec2(box_max.x - lineOffset, box_max.y), DrawEspColor, thickness);
						draw_list->AddLine(ImVec2(box_max.x, box_max.y), ImVec2(box_max.x, box_max.y - lineOffset), DrawEspColor, thickness);
					}
				}
				if (Draw_Distance) {
					char textBuffer[64];
					snprintf(textBuffer, sizeof(textBuffer), "%.1f", distance);

					float baseFontSize = ImGui::GetFontSize();
					float dynamicTextSize = floorf(fmaxf(10.0f, fminf(250.0f, baseFontSize * scale)));

					ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(dynamicTextSize, FLT_MAX, 0.0f, textBuffer);

					float centerX = (box_min.x + box_max.x) * 0.5f;
					ImVec2 textPos = ImVec2(centerX - textSize.x * 0.5f, box_min.y - textSize.y - 3.0f);

					ImVec2 bgMin = ImVec2(textPos.x - 2.0f, textPos.y);
					ImVec2 bgMax = ImVec2(textPos.x + textSize.x + 2.0f, textPos.y + textSize.y);
					draw_list->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, 150), 2.0f);
					draw_list->AddText(ImGui::GetFont(), dynamicTextSize, textPos, DrawTextColor, textBuffer);
				}
				if (Draw_Tracers) {
					draw_list->AddLine(ImVec2((box_min.x + box_max.x) * 0.5f, box_max.y), ImVec2(ScreenCenterX, height), DrawTracersColor, 1.5f);
				}
				//занос отрисованного игрока в список отрисованных за кадр
				drawnEntities.insert(EnemyData.baseaddr);
			}
		}
	}
}

HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (firstTime)
	{
		firstTime = false; //only once

		//get device
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&pDevice)))
		{
			pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
			pDevice->GetImmediateContext(&pContext);
		}
		
		//imgui
		DXGI_SWAP_CHAIN_DESC sd;
		pSwapChain->GetDesc(&sd);
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard; //control menu with mouse
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		io.WantSaveIniSettings = false;
		io.IniFilename = NULL;
		window = sd.OutputWindow;
		//wndprochandler
		OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)hWndProc);
		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX11_Init(pDevice, pContext);
		ImGui::GetMainViewport()->PlatformHandleRaw = window;

	}

	//create rendertarget
	if (RenderTargetView == NULL)
	{
		//viewport
		pContext->RSGetViewports(&vps, &viewport);
		ScreenCenterX = viewport.Width / 2.0f;
		ScreenCenterY = viewport.Height / 2.0f;
		width = viewport.Width;
		height = viewport.Height;

		//get backbuffer
		ID3D11Texture2D* backbuffer = NULL;
		hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
		if (FAILED(hr)) {
			return hr;
		}

		//create rendertargetview
		hr = pDevice->CreateRenderTargetView(backbuffer, NULL, &RenderTargetView);
		backbuffer->Release();
		if (FAILED(hr)) {
			return hr;
		}
	}
	else //call before you draw
		pContext->OMSetRenderTargets(1, &RenderTargetView, NULL);
		

	//imgui
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();

	//info
	if (greetings)
	{
		ImGui::Begin("title0", &greetings, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
		ImGui::Text("Dll loaded, press F8 to open menu.");
		ImGui::End();

		static DWORD lastTime = timeGetTime();
		DWORD timePassed = timeGetTime() - lastTime;
		if (timePassed > 5000)
		{
			greetings = false;
			lastTime = timeGetTime();
		}
	}

	//mouse cursor on in menu
	if (ShowMenu)
		ImGui::GetIO().MouseDrawCursor = 1;
	else
		ImGui::GetIO().MouseDrawCursor = 0;
	
	if (ShowMenu) {
		DrawMenu();
	}
	if (EspMode != 0 || Draw_Tracers || Draw_Distance || FillMode != 0) {
		Draw();
	}
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (GetAsyncKeyState(VK_F8) & 1)
	{
		ShowMenu = !ShowMenu;
	}

	return phookD3D11Present(pSwapChain, SyncInterval, Flags);
}

const int MultisampleCount = 1;
LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hwnd, uMsg, wParam, lParam); }
DWORD __stdcall InitHooks(LPVOID)
{
	HMODULE hDXGIDLL = 0;
	do
	{
		hDXGIDLL = GetModuleHandle("dxgi.dll");
		Sleep(4000);
	} while (!hDXGIDLL);
	Sleep(100);

	IDXGISwapChain* pSwapChain;

	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

	D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtainedLevel;
	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dContext = nullptr;

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = MultisampleCount;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

	scd.BufferDesc.Width = 1;
	scd.BufferDesc.Height = 1;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;

	UINT createFlags = 0;
#ifdef _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	IDXGISwapChain* d3dSwapChain = 0;

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		requestedLevels,
		sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&scd,
		&pSwapChain,
		&pDevice,
		&obtainedLevel,
		&pContext)))
	{
		return NULL;
	}


	pSwapChainVtable = (DWORD_PTR*)pSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

	pContextVTable = (DWORD_PTR*)pContext;
	pContextVTable = (DWORD_PTR*)pContextVTable[0];

	pDeviceVTable = (DWORD_PTR*)pDevice;
	pDeviceVTable = (DWORD_PTR*)pDeviceVTable[0];

	phookD3D11Present = (D3D11PresentHook)(DWORD_PTR*)pSwapChainVtable[8];
	phookD3D11ResizeBuffers = (D3D11ResizeBuffersHook)(DWORD_PTR*)pSwapChainVtable[13];

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(LPVOID&)phookD3D11Present, (PBYTE)hookD3D11Present);
	DetourAttach(&(LPVOID&)phookD3D11ResizeBuffers, (PBYTE)hookD3D11ResizeBuffers);
	DetourTransactionCommit();

	DWORD dwOld;
	VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dwOld);

	CreateThread(NULL, 0, CheatThread, NULL, 0, NULL);

	while (true) {
		Sleep(10);
	}

	pDevice->Release();
	pContext->Release();
	pSwapChain->Release();

	return NULL;
}

BOOL __stdcall DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH: // A process is loading the DLL.
		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, 0, InitHooks, NULL, 0, NULL);
		break;

	case DLL_PROCESS_DETACH: // A process unloads the DLL.
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(LPVOID&)phookD3D11Present, (PBYTE)hookD3D11Present);
		DetourDetach(&(LPVOID&)phookD3D11ResizeBuffers, (PBYTE)hookD3D11ResizeBuffers);
		DetourTransactionCommit();
		break;
	}
	return TRUE;
}
