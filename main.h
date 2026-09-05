#include "memory.hpp"
#include <set>

static int EspMode = 0;
static const char* EspModes[] = { "Off", "Box", "Corner"};

static int FillMode = 0;
static const char* FillModes[] = { "Off", "Default", "Gradient" };

bool Draw_Tracers = false;
bool Draw_Distance = false;


static ImU32 DrawEspColor = IM_COL32(255, 64, 128, 255);
ImVec4 EspTemp = ImVec4(1.0f, 0.25f, 0.5f, 1.0f);

static ImU32 FillMainColor = IM_COL32(255, 64, 128, 96);
ImVec4 FillMainTemp = ImVec4(1.0f, 0.25f, 0.5f, 0.37f);
static ImU32 FillSecColor = IM_COL32(0, 196, 128, 96);
ImVec4 FillSecTemp = ImVec4(0.0f, 0.75f, 0.5f, 0.37f);

static ImU32 DrawTracersColor = IM_COL32(255, 64, 128, 255);
ImVec4 TracersTemp = ImVec4(1.0f, 0.25f, 0.5f, 1.0f);

static ImU32 DrawTextColor = IM_COL32(255, 64, 128, 255);
ImVec4 TextTemp = ImVec4(1.0f, 0.25f, 0.5f, 1.0f);

//viewport
UINT vps = 1;
D3D11_VIEWPORT viewport;
float ScreenCenterX;
float ScreenCenterY;
//create rendertarget
ID3D11RenderTargetView* RenderTargetView = NULL;
//wndproc
HWND window = nullptr;
bool ShowMenu = false;
static WNDPROC OriginalWndProcHandler = nullptr;
#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }
HRESULT hr;
bool greetings = true;
bool firstTime = true;
