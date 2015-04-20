#include "stdafx.h"

GFX_INFO Global_Gfx_Info;

BOOL ErrorMsg; // only show one error message per frame to speed up
BOOL bSceneRendered;
BOOL bPaused;
HANDLE hInst;
POINT TopLeft;
FILE *fVRML;
U32 uVRMLFaceIndex;
U32 uRenderWidth = 640;
U32 uRenderHeight = 480;
#define numofwindowedModes 10
struct
{
	WORD width, height;
	char *description;
} windowedModes[numofwindowedModes] = {
	{ 640, 480, "640 x 480" },
	{ 800, 600, "800 x 600" },
	{ 1024, 768, "1024 x 768" },
	{ 1280, 960, "1280 x 960" },
	{ 1400, 1050, "1400 x 1050" },
	{ 1440, 1080, "1440 x 1080" },
	{ 1600, 1200, "1600 x 1200" },
	{ 2048, 1536, "2048 × 1536" },
	{ 2800, 2100, "2800 × 2100" },
	{ 3200, 2400, "3200 × 2400" } 
};

#if _DEBUG
FILE *fLogFile;
int piLevel;


S64 ProfileTimes[100];
S32 ProfileTimeDListCount;
#endif

U32 puCullDone, puCullCalled;

U32 DebugTextureID;
BOOL bDebugTextureBackground;
HWND hDebugVRMLCheck;
HWND hDebugCIMGTracing;
HWND hDebugCullStats;
HWND hDebugDoCulling;
HWND hDebugDisableTextureCache;
HWND hDebugForceZClear;
HWND hDebugForceZBuffer;
HWND hDebugWireframe;
HWND hDebugBufferClear;
HWND hDebugDisablePixelshader;
HWND hDebugLstStats, hDebugResetStats;
HWND hDebugWindow;
HWND hDebugWindowTextureID;
HWND hDebugWindowTextureDetails;
HWND hDebugTextureBackground;
HWND hDebugWindowProfileButton;
HWND hDebugCommandLog;
HWND hProfileTimesListbox;


void ReadSpecialSettings (const char * name)
{
  if (strstr(name, (const char *)"Killer Instinct Gold"))
    Options.bBufferClear = TRUE;
  if (strstr(name, (const char *)"Rogue Squadron"))
    Options.bDisablePixelshader=TRUE;
}

void ResizeWindow()
{
int X=100, Y=100;
HDC hdc = GetDC(NULL);
	if (hdc)
	{
		int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
		int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
		X = dpiX*100/96;
		Y = dpiY*100/96;
	}
	SetWindowPos( Global_Gfx_Info.hWnd, NULL, 0, 0,	uRenderWidth+16*X/100,
						uRenderHeight+107*Y/100, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE );
}

void DebugDrawTexture(U32 TextureIndex, bool Zoom)
{
	D3DSURFACE_DESC Desc;
	SafeDX(State.Textures[TextureIndex].D3DObject->GetLevelDesc(0, &Desc));

	// Set Viewport...
	D3DVIEWPORT9 Viewport;
	Viewport.X = 0;
	Viewport.Y = 0;
	Viewport.Width = State.WindowWidth;
	Viewport.Height = State.WindowHeight;
	Viewport.MinZ = 0;
	Viewport.MaxZ = 1.0f;
	SafeDX(g_pd3dDevice->SetViewport(&Viewport));

	D3DRECT Rect;
	Rect.x1 = 0;
	Rect.y1 = 0;
	if (Zoom){
		Rect.x2 = (Desc.Width*2)+1;
		Rect.y2 = (Desc.Height*2)+1;
	} else {
		Rect.x2 = Desc.Width+1;
		Rect.y2 = Desc.Height+1;
	}
	g_pd3dDevice->Clear(1, &Rect, D3DCLEAR_TARGET, 0xFFFFFFFF, 1.0f, 0);
	if (Zoom){
		Rect.x2 = Desc.Width*2;
		Rect.y2 = Desc.Height*2;
	} else {
		Rect.x2 = Desc.Width;
		Rect.y2 = Desc.Height;
	}
	g_pd3dDevice->Clear(1, &Rect, D3DCLEAR_TARGET, 0, 1.0f, 0);
	SafeDX(g_pd3dDevice->BeginScene());
	
	// Set Matrices...
	D3DXMATRIX OrthoMatrix;
	D3DXMatrixOrthoOffCenterRH(&OrthoMatrix, 0, (float)State.WindowWidth/3, (float)State.WindowHeight/3, 0, 0.0f, 1.0f);
	SafeDX(g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &OrthoMatrix));
	SafeDX(g_pd3dDevice->SetTransform(D3DTS_WORLD, &State.Identity));

	// Set texture...
	SafeDX(g_pd3dDevice->SetTexture(0, State.Textures[TextureIndex].D3DObject));
	SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1));
	if (bDebugTextureBackground){
		SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE|D3DTA_ALPHAREPLICATE));
	} else {
		SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
	}
	SafeDX(g_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE));
	SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE));
	if (bDebugTextureBackground){
		SafeDX(g_pd3dDevice->SetPixelShader(State.ps_texalphaonly));
	} else {
		SafeDX(g_pd3dDevice->SetPixelShader(State.ps_texcoloronly));
	}
	
	// Set up vertex buffer...
	D3DTNTVERTEX *VertexBuffer;
	SafeDX(g_VertexBuffer->Lock(0, 4*sizeof(D3DTNTVERTEX), (void**)&VertexBuffer, 0));
	VertexBuffer[0].color = 0xFFFFFFFF;
	VertexBuffer[0].x = 0.0f;
	VertexBuffer[0].y = 0.0f;
	VertexBuffer[0].z = 0.0f;
	VertexBuffer[0].tex[0].u = 0.0f;
	VertexBuffer[0].tex[0].v = 0.0f;

	VertexBuffer[1].color = 0xFFFFFFFF;
	VertexBuffer[1].x = (float)Desc.Width;
	VertexBuffer[1].y = 0.0f;
	VertexBuffer[1].z = 0.0f;
	VertexBuffer[1].tex[0].u = 1.0f;
	VertexBuffer[1].tex[0].v = 0.0f;

	VertexBuffer[2].color = 0xFFFFFFFF;
	VertexBuffer[2].x = 0.0f;
	VertexBuffer[2].y = (float)Desc.Height;
	VertexBuffer[2].z = 0.0f;
	VertexBuffer[2].tex[0].u = 0.0f;
	VertexBuffer[2].tex[0].v = 1.0f;

	VertexBuffer[3].color = 0xFFFFFFFF;
	VertexBuffer[3].x = (float)Desc.Width;
	VertexBuffer[3].y = (float)Desc.Height;
	VertexBuffer[3].z = 0.0f;
	VertexBuffer[3].tex[0].u = 1.0f;
	VertexBuffer[3].tex[0].v = 1.0f;
	SafeDX(g_VertexBuffer->Unlock());

	SafeDX(g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	SafeDX(g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	SafeDX(g_pd3dDevice->EndScene());
}

void DebugShowTexture()
{
	char str[100], strdetails[200], strformat[20];
	ShowWindow(hDebugWindowTextureDetails, SW_HIDE);
	if (State.TextureCount==0){
		sprintf(str, "No textures loaded");
	} else {
		if (DebugTextureID==0xFFFFFFFF){
			sprintf(str, "Showing no texture");
		} else {
			ShowWindow(hDebugWindowTextureDetails, SW_SHOW);
			switch (State.Textures[DebugTextureID].RDPFormat&BitM2){
			case 0:
				strcpy(strformat, "4 bit ");
				break;
			case 1:
				strcpy(strformat, "8 bit ");
				break;
			case 2:
				strcpy(strformat, "16 bit ");
				break;
			case 3:
				strcpy(strformat, "32 bit ");
				break;
			}
			switch ((State.Textures[DebugTextureID].RDPFormat>>2)&BitM3){
			case 0:
				strcat(strformat, "RGBA");
				break;
			case 1:
				strcat(strformat, "YUV");
				break;
			case 2:
				strcat(strformat, "CI");
				break;
			case 3:
				strcat(strformat, "IA");
				break;
			case 4:
				strcat(strformat, "I");
				break;
			}
			sprintf(str, "Showing texture %i of %i", DebugTextureID+1, State.TextureCount);
			sprintf(strdetails, "Format: %s, Width: %i, Height: %i, UTI: %X, RDPPitch: %i\
, cms: %s, cmt: %s, masks: %i, maskt: %i",
				strformat,
				State.Textures[DebugTextureID].Width,
				State.Textures[DebugTextureID].Height,
				State.Textures[DebugTextureID].UTI,
				State.Textures[DebugTextureID].RDPPitch,
				(State.Textures[DebugTextureID].cms==0?"Wrap":(State.Textures[DebugTextureID].cms==1?"Mirror":(State.Textures[DebugTextureID].cms==2?"Clamp and Wrap":"Clamp and Mirror"))),
				(State.Textures[DebugTextureID].cmt==0?"Wrap":(State.Textures[DebugTextureID].cmt==1?"Mirror":(State.Textures[DebugTextureID].cmt==2?"Clamp and Wrap":"Clamp and Mirror"))),
				1<<State.Textures[DebugTextureID].masks,
				1<<State.Textures[DebugTextureID].maskt);

			if (State.Textures[DebugTextureID].ByLoadBlock){
				if (State.Textures[DebugTextureID].DXTUsed){
					strcat(strdetails, ", loaded by LoadBlock with DXT!=0");
				} else {
					strcat(strdetails, ", loaded by LoadBlock with DXT==0");
				}
			} else {
				strcat(strdetails, ", loaded by LoadTile");
			}
			DebugDrawTexture(DebugTextureID, true);
			RECT rDstRect;
			rDstRect.left = 0;
			rDstRect.top = 27;
			rDstRect.right = rDstRect.left + 640;
			rDstRect.bottom = rDstRect.top + 480;
			SafeDX(g_pd3dDevice->Present(0, &rDstRect, 0, 0));
			SendMessage(hDebugWindowTextureDetails, WM_SETTEXT, 0, (LPARAM)strdetails);
		}
	}
	SendMessage(hDebugWindowTextureID, WM_SETTEXT, 0, (LPARAM)str);
}

LRESULT CALLBACK DlgProcSettings(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int	 i;
	DEVMODE deviceMode;
	switch (message){
	case WM_INITDIALOG:
		switch (lParam){
		case IDD_SETTINGS:
			// Main settings dialog

			// Fill windowed mode resolution
			EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &deviceMode );
			for (i = 0; i < numofwindowedModes; i++)
			{
				if ((deviceMode.dmPelsWidth > windowedModes[i].width) &&
					(deviceMode.dmPelsHeight > windowedModes[i].height))
				{
					SendDlgItemMessage( hWnd, IDC_WINDOWEDRES, CB_ADDSTRING, 0, (LPARAM)windowedModes[i].description );
					if ((uRenderWidth == windowedModes[i].width) &&
					    (uRenderHeight == windowedModes[i].height))
						SendDlgItemMessage( hWnd, IDC_WINDOWEDRES, CB_SETCURSEL, i, 0 );
				}
			}		

			DebugTextureID = 0xFFFFFFFF;
			hDebugWindow = hWnd;
			hDebugWindowTextureID = FindWindowEx(hDebugWindow, 0, "Static", "Showing no texture");
			hDebugWindowTextureDetails = FindWindowEx(hDebugWindow, 0, "Static", "Texture Details");
			hDebugTextureBackground = FindWindowEx(hDebugWindow, 0, "Button", "Show Alpha");
			hDebugWindowProfileButton = FindWindowEx(hDebugWindow, 0, "Button", "Profile times...");
			hDebugCommandLog = FindWindowEx(hDebugWindow, 0, "Button", "Log Commands");
			hDebugVRMLCheck = FindWindowEx(hDebugWindow, 0, "Button", "Export VRML");
			hDebugCIMGTracing = FindWindowEx(hDebugWindow, 0, "Button", "CIMG Tracing");
			hDebugCullStats = FindWindowEx(hDebugWindow, 0, "Button", "Show culling stats");
			hDebugDoCulling = FindWindowEx(hDebugWindow, 0, "Button", "Emulate cull commands");
			hDebugDisableTextureCache = FindWindowEx(hDebugWindow, 0, "Button", "Disable texture cache");
			hDebugForceZClear = FindWindowEx(hDebugWindow, 0, "Button", "Force Z Clear");
			hDebugForceZBuffer = FindWindowEx(hDebugWindow, 0, "Button", "Force Z Buffer");
			hDebugWireframe = FindWindowEx(hDebugWindow, 0, "Button", "Wireframe mode");
			hDebugBufferClear = FindWindowEx(hDebugWindow, 0, "Button", "Force Buffer Clear");
			hDebugDisablePixelshader = FindWindowEx(hDebugWindow, 0, "Button", "Disable pixel shader (requires LemD3DCombine.dat)");
			hDebugLstStats = FindWindowEx(hDebugWindow, 0, "Listbox", "");
			hDebugResetStats = FindWindowEx(hDebugWindow, 0, "Button", "Reset");		
			SendMessage(hDebugTextureBackground, BM_SETCHECK, BST_UNCHECKED, 0);
			SendMessage(hDebugCommandLog, BM_SETCHECK, (U32)Options.bCommandLog, 0);
			SendMessage(hDebugVRMLCheck, BM_SETCHECK, (U32)Options.bVRMLExport, 0);
			SendMessage(hDebugCullStats, BM_SETCHECK, (U32)Options.bCullStats, 0);
			SendMessage(hDebugDoCulling, BM_SETCHECK, (U32)Options.bDoCulling, 0);
			SendMessage(hDebugCIMGTracing, BM_SETCHECK, (U32)Options.bCImgTracing, 0);
			SendMessage(hDebugDisableTextureCache, BM_SETCHECK, (U32)Options.bDisableTextureCache, 0);
			SendMessage(hDebugForceZClear, BM_SETCHECK, (U32)Options.bForceZClear, 0);
			SendMessage(hDebugForceZBuffer, BM_SETCHECK, (U32)Options.bForceZBuffer, 0);
			SendMessage(hDebugWireframe, BM_SETCHECK, (U32)Options.bWireframe, 0);
			SendMessage(hDebugBufferClear, BM_SETCHECK, (U32)Options.bBufferClear, 0);
			SendMessage(hDebugDisablePixelshader, BM_SETCHECK, (U32)Options.bDisablePixelshader, 0);
#if _DEBUG
			char str[255];
			sprintf(str, "DLs: %i", Stats.DLs);
			SendMessage(hDebugLstStats, LB_ADDSTRING, 0, (long)str);
			sprintf(str, "T loaded: %i", Stats.TexturesCached);
			SendMessage(hDebugLstStats, LB_ADDSTRING, 0, (long)str);
			sprintf(str, "T fast:: %i", Stats.TexturesLoadedFromCache );
			SendMessage(hDebugLstStats, LB_ADDSTRING, 0, (long)str);
#endif
			bDebugTextureBackground=false;
			DebugShowTexture();
#ifndef _DEBUG
			ShowWindow(hDebugWindowProfileButton, SW_HIDE);
			ShowWindow(hDebugCommandLog, SW_HIDE);
			ShowWindow(hDebugCIMGTracing, SW_HIDE);
			ShowWindow(hDebugCullStats, SW_HIDE);
			ShowWindow(hDebugDoCulling, SW_HIDE);
			ShowWindow(hDebugDisableTextureCache, SW_HIDE);
			ShowWindow(hDebugForceZClear, SW_HIDE);
			ShowWindow(hDebugForceZBuffer, SW_HIDE);
			ShowWindow(hDebugLstStats, SW_HIDE);
			ShowWindow(hDebugResetStats, SW_HIDE);
#endif
			break;
#if _DEBUG
		case IDD_PROFILETIMES:
			hProfileTimesListbox = FindWindowEx(hWnd, 0, "Listbox", 0);
			U32 i;
			char str2[100];
			do {
				i = SendMessage(hProfileTimesListbox, LB_DELETESTRING, 0, 0);
			} while ((i>0) && (i!=LB_ERR));
			for (i=0;i<ProfileTimesMax;i++){
				if (ProfileTimes[i]!=0){
					sprintf(str2, "%i: %I64i Cycles per DList (%I64i%%)", i, ProfileTimes[i]/ProfileTimeDListCount, (ProfileTimes[i]*100/ProfileTimes[0]));
					SendMessage(hProfileTimesListbox, LB_ADDSTRING, 0, (LPARAM)&str2[0]);
				}
			}
			break;
#endif
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL){
			EndDialog(hWnd, LOWORD(wParam));
			FreeDirect3D();
			InitializeDirect3D();
			ResizeWindow();
			return TRUE;
		}

		i = SendDlgItemMessage( hWnd, IDC_WINDOWEDRES, CB_GETCURSEL, 0, 0 );
		uRenderWidth = windowedModes[i].width;
		uRenderHeight = windowedModes[i].height;

		if (LOWORD(wParam) == IDC_BUTTONSHOWPROFILER){
			DialogBoxParam((HINSTANCE)hInst, (LPCTSTR)IDD_PROFILETIMES, hWnd, (DLGPROC)DlgProcSettings, IDD_PROFILETIMES);
		}
#if _DEBUG
		if (LOWORD(wParam) == IDC_BUTTONRESET){
			memset(&Stats, 0, sizeof(TStats));
		}

		if (LOWORD(wParam) == IDC_BUTTONPROFILERESET){
			U32 i;
			do {
				i = SendMessage(hProfileTimesListbox, LB_DELETESTRING, 0, 0);
			} while ((i>0) && (i!=LB_ERR));
			memset(ProfileTimes, 0, sizeof(ProfileTimes));
			ProfileTimeDListCount=0;
		}
#endif
		if (LOWORD(wParam) == IDC_BUTTONTEXTURENEXT){
			if (State.TextureCount==0){
				DebugTextureID=0xFFFFFFFF;
			} else {
				DebugTextureID++;
				if (DebugTextureID>=State.TextureCount){
					DebugTextureID = State.TextureCount-1;
				}
			}
			DebugShowTexture();
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_BACKGROUND){
			bDebugTextureBackground = SendMessage(hDebugTextureBackground, BM_GETCHECK, 0, 0)==BST_CHECKED;
			DebugShowTexture();
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_COMMANDLOG){
			Options.bCommandLog = SendMessage(hDebugCommandLog, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_CHECKVRML){
			Options.bVRMLExport = SendMessage(hDebugVRMLCheck, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_CHECKCIMGTRACING){
			Options.bCImgTracing = SendMessage(hDebugCIMGTracing, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_CHECKCULLSTATS){
			Options.bCullStats = SendMessage(hDebugCullStats, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_CHECKDOCULLING){
			Options.bDoCulling = SendMessage(hDebugDoCulling, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_CHECKDISABLETEXTURECACHE){
			Options.bDisableTextureCache = SendMessage(hDebugDisableTextureCache, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_CHKFORCEZCLEAR){
			Options.bForceZClear = SendMessage(hDebugForceZClear, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_CHKFORCEZBUFFER){
			Options.bForceZBuffer = SendMessage(hDebugForceZBuffer, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_WIREFRAME){
			Options.bWireframe = SendMessage(hDebugWireframe, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_CHKBUFFERCLEAR){
			Options.bBufferClear = SendMessage(hDebugBufferClear, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_DISABLEPIXELSHADER){
			Options.bDisablePixelshader = SendMessage(hDebugDisablePixelshader, BM_GETCHECK, 0, 0)==BST_CHECKED;
			return TRUE;
		}
		
		if (LOWORD(wParam) == IDC_BUTTONTEXTUREPREVIOUS){
			if (State.TextureCount==0){
				DebugTextureID=0xFFFFFFFF;
			} else {
				if (DebugTextureID==0xFFFFFFFF){
					DebugTextureID=0;
				} else {
					if (DebugTextureID!=0){
						DebugTextureID--;
					}
				}
			}
			DebugShowTexture();
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void DisplayInStatusPanel(long lPanel, char *szString, ...)
{
	char str[512];
	va_list(Arg);
	va_start(Arg, szString);

	vsprintf(str, szString, Arg);

	va_end(Arg);

	SendMessage(Global_Gfx_Info.hStatusBar, SB_SETTEXT, lPanel, (LPARAM)str);
}

void ShowError(char *str)
{
#if _DEBUG
	//MessageBox(0/*Global_Gfx_Info.hWnd*/, str, "Error", MB_OK | MB_ICONERROR);
	if (!ErrorMsg){
		ErrorMsg = true;
		SendMessage(Global_Gfx_Info.hStatusBar, SB_SETTEXT, 0, (LPARAM)str);
	}
	LogCommand(str);
#endif
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	hInst = hModule;
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			InitLogging();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			CloseLogging();
			FreeDirect3D();
			break;
    }
    return TRUE;
}

EXPORT void CALL ChangeWindow (void)
{
	bPaused = true;
	Sleep(100);// wait for Call to UpdateScreen
	bFullScreen = !bFullScreen;
	if (g_pd3dDevice!=NULL)
	{
		FreeDirect3D();
		InitializeDirect3D();
		if (!bFullScreen)
		{
			ShowCursor( TRUE );
			ResizeWindow();
		}
		else
			ShowCursor( FALSE );
	}
	bPaused = false;
}

EXPORT void CALL CloseDLL (void)
{
	FreeDirect3D();
}

EXPORT void CALL DllAbout ( HWND hParent )
{
	MessageBox(hParent, "Lemmy's Nemu64 Direct3D9 Graphics Plugin", "About", MB_OK | MB_ICONINFORMATION);
}

EXPORT void CALL DllConfig ( HWND hParent )
{
	bPaused = true;
	DialogBoxParam((HINSTANCE)hInst, (LPCTSTR)IDD_SETTINGS, hParent, (DLGPROC)DlgProcSettings, IDD_SETTINGS);
	bPaused = false;
}

EXPORT void CALL DrawScreen (void)
{
}

EXPORT void CALL GetDllInfo ( PLUGIN_INFO * PluginInfo )
{
	PluginInfo->MemoryBswaped = true;
	strcpy(PluginInfo->Name, "Nemu64 Graphics");
	PluginInfo->NormalMemory = false;
	PluginInfo->Type = PLUGIN_TYPE_GFX;
	PluginInfo->Version = 0x0102;
}

EXPORT BOOL CALL InitiateGFX (GFX_INFO Gfx_Info)
{
	ErrorMsg = false;
	GetCurrentDirectory(_MAX_PATH, &DatFilename[0]);
	if ((strlen(DatFilename)!=0) && (DatFilename[strlen(DatFilename) - 1] != '\\'))
		strcat(DatFilename, "\\");
	strcpy(IniFilename, DatFilename);
	strcat(DatFilename, "LemD3DCombine.dat");
	strcat(IniFilename, "config.ini");
	Global_Gfx_Info = Gfx_Info;	
	memset(&State, 0, sizeof(TState));
	State.texture.tile = &State.Tiles[0];
	return TRUE;
}

EXPORT void CALL MoveScreen (int xpos, int ypos)
{
}

EXPORT void CALL ProcessDList(void)
{
	while (bPaused)
		Sleep(100);
	if (Options.bVRMLExport){
		CreateDirectory("vrml",0);
		char sFileName[256];
		sprintf(sFileName, "vrml\\output.wrl");
		fVRML = fopen(sFileName, "w");
		uVRMLFaceIndex = 1;
		fprintf(fVRML, "#VRML V2.0 utf8\n");
		fprintf(fVRML, "# Produced by Nemu64\n");
		fprintf(fVRML, "\n");
		fprintf(fVRML, "Viewpoint {\n");
		fprintf(fVRML, "	position 0 0 0\n");
		fprintf(fVRML, "	orientation 0 0 1 0\n");
		fprintf(fVRML, "}\n");
		fprintf(fVRML, "PointLight {\n");
		fprintf(fVRML, "	intensity 1\n");
		fprintf(fVRML, "	location 0 0 0\n");
		fprintf(fVRML, "	color 1 1 1\n");
		fprintf(fVRML, "	ambientIntensity 0.5\n");
		fprintf(fVRML, "	on TRUE\n");
		fprintf(fVRML, "}\n");
		
		fprintf(fVRML, "DEF NemuModel Transform {\n");
		fprintf(fVRML, "	children [\n");
	}

	puCullDone = 0;
	puCullCalled = 0;
	bSceneRendered = true;

	g_pd3dDevice->BeginScene();
	ProcessDisplayList();
	g_pd3dDevice->EndScene();
	
	// set the interrupt bit
	*(Global_Gfx_Info.MI_INTR_REG) |= 0x20;
	Global_Gfx_Info.CheckInterrupts();
	if (Options.bVRMLExport){
		fprintf(fVRML, "	]\n");
		fprintf(fVRML, "}\n");
		fclose(fVRML);
		TCHAR dir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, dir);
		MessageBox(Global_Gfx_Info.hStatusBar, dir, "VRML successfully exported", MB_OK);
		Options.bVRMLExport=0;
	}

	if (Options.bCullStats){
		if (puCullCalled){
			static U32 lLastCullPercent;
			U32 lNew;
			lNew = puCullDone*100/puCullCalled;
			if (lLastCullPercent != lNew){
				DisplayInStatusPanel(0, "Cull stats: %i of %i (%i%%) culled", puCullDone, puCullCalled, lNew);
				lLastCullPercent = lNew;
			}
		}
	}
}

// Close Direct3d
EXPORT void CALL RomClosed(void)
{
	FreeDatFile();
	FreeDirect3D();
}

// Init Direct3d
EXPORT void CALL RomOpen(void)
{
	InitializeDirect3D();
	CombineScripts.Script = 0;
	LoadDatFile();
	State.CombineChanged = true;
#if (COMBINEDEBUGGER|_DEBUG)
	LoggedCombineModes.Count = 0;
#endif
#if _DEBUG
	memset(ProfileTimes, 0, sizeof(ProfileTimes));
	ProfileTimeDListCount = 0;
#endif
	bFirstDisplayList = true;
	bSceneRendered = false;

	D3DXMatrixIdentity(&State.Identity);
	D3DXMatrixIdentity(&State.Projection);
	//State.Projection._43 = -256;
	//State.Projection._44 = 0;
	State.ProjectionModified = State.Projection;
	D3DXMatrixIdentity(&State.World[0]);
	State.WorldMatrix = &State.World[0];
	g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	
	// get the name of the ROM
	char name[21];
	for (int i=0; i<20; i++)
		name[i] = Global_Gfx_Info.HEADER[(32+i)^3];
	name[20] = 0;
	ReadSpecialSettings(name);

	ResizeWindow();
}

EXPORT void CALL ShowCFB (void)
{
}

EXPORT void CALL UpdateScreen (void)
{
	while (bPaused)
		Sleep(100);
	ErrorMsg = false;
	if (bSceneRendered){
		RECT rDstRect;
		if (bFullScreen){
			rDstRect.left = 0;
			rDstRect.top = 0;
			rDstRect.right = rDstRect.left + uRenderWidth;// State.Viewport.Width;
			rDstRect.bottom = rDstRect.top + uRenderHeight;//State.Viewport.Height;
		} else {
			rDstRect.left = 0;
			rDstRect.top = 27;
			rDstRect.right = rDstRect.left + uRenderWidth;
			rDstRect.bottom = rDstRect.top + uRenderHeight;
		}
		//SafeDX(g_pd3dDevice->Present(NULL, &rDstRect, NULL, NULL));
		g_pd3dDevice->Present(NULL, &rDstRect, NULL, NULL);
		bSceneRendered = false;
	}
	if (Options.bWireframe || Options.bBufferClear)
		g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
}

EXPORT void CALL ViStatusChanged (void)
{
}

EXPORT void CALL ViWidthChanged (void)
{
}

EXPORT void CALL SetResolution (long width, long height)
{
	uRenderWidth = width;
	uRenderHeight = height;
	if (g_pd3dDevice!=NULL){
		bPaused = true;
		Sleep(100);// wait for Call to UpdateScreen
		FreeDirect3D();
		InitializeDirect3D();
		bPaused = false;
	}
}

// Passes the pointer for the combine mode array to the exe, which passes it to the debugger
EXPORT void CALL GetCombinerPointer(void **CombineModes)
{
#if COMBINEDEBUGGER
	(*CombineModes) = (void*)&LoggedCombineModes;
#endif
}
