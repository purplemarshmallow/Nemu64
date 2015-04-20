#include "stdafx.h"

LPDIRECT3D9 g_pD3D;
LPDIRECT3DDEVICE9 g_pd3dDevice;
LPDIRECT3DVERTEXBUFFER9 g_VertexBuffer;
D3DCAPS9 DeviceCaps;

TColorTextures g_ColorTexture;
LPDIRECT3DTEXTURE9 texBackground[3];
U32 LastDXErrorCode;

bool bFullScreen;

LPDIRECT3DPIXELSHADER9 CompileShader(char *sPS)
{
	LPDIRECT3DPIXELSHADER9 pPixelShader=NULL;
	LPD3DXBUFFER pCode;
	LPD3DXBUFFER pErrors;

	if (D3DXAssembleShader(sPS, strlen(sPS), 0, NULL, 0, &pCode, &pErrors)!=D3D_OK){
		ShowError((char*)pErrors->GetBufferPointer());
	}else{
		SafeDX(g_pd3dDevice->CreatePixelShader((DWORD*)pCode->GetBufferPointer(),
											&pPixelShader));
	}
	if (pCode){
		SafeDX(pCode->Release());
		pCode = NULL;
	}
	if (pErrors){
		SafeDX(pErrors->Release());
		pErrors = NULL;
	}
	return pPixelShader;
}

int InitializeDirect3D()
{
	LPDIRECT3D9 g_pD3D;
	D3DPRESENT_PARAMETERS d3dpp; 
	D3DDISPLAYMODE d3ddm;
	
	if( NULL == ( g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))){
		ShowError("Error initializing Direct3D");
		return E_FAIL;
	}
	
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.Windowed   = !bFullScreen;
	d3dpp.EnableAutoDepthStencil = TRUE;

	// Get Backbuffer format from current settings
	SafeDX(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm ) );
	d3dpp.BackBufferFormat = d3ddm.Format;
	if (bFullScreen){
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.BackBufferWidth = d3ddm.Width;
		d3dpp.BackBufferHeight = d3ddm.Height;
		d3dpp.FullScreen_RefreshRateInHz = d3ddm.RefreshRate;
	} else {
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	}

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Global_Gfx_Info.hWnd,
								  D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED , 
								  &d3dpp, &g_pd3dDevice ))){
		char s[512];
		strcpy(s, "Direct3D Device initialization failed");
		ShowError(s);
		MessageBox(Global_Gfx_Info.hWnd, s, "Error", MB_OK);
		return E_FAIL;
	}
	// Save viewport data...
	SafeDX(g_pd3dDevice->GetViewport(&State.Viewport));
	State.Viewport.MinZ = 0.0f;
	State.Viewport.MaxZ = 1.0f;
	State.WindowWidth = State.Viewport.Width;
	State.WindowHeight = State.Viewport.Height;
	SafeDX(g_pd3dDevice->SetViewport(&State.Viewport));
	RECT r;
	GetClientRect(Global_Gfx_Info.hWnd, &r);

	SafeDX(g_pd3dDevice->GetDeviceCaps(&DeviceCaps));

	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 1));
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD));
	//SafeDX(g_pd3dDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE));

	U32 i;
	for (i=0;i<8;i++){
		SafeDX(g_pd3dDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
		SafeDX(g_pd3dDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
	}

	if (Options.bWireframe)
	{
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME));
	}

	// Create the vertex buffer
	SafeDX(g_pd3dDevice->CreateVertexBuffer(
			sizeof(D3DTNTVERTEX)*D3DVertexCacheSize,
			D3DUSAGE_WRITEONLY,
			MY_VERTEX_TYPE,
			D3DPOOL_SYSTEMMEM,
			&g_VertexBuffer,
			NULL));
	SafeDX(g_pd3dDevice->SetStreamSource(
			0,
			g_VertexBuffer,
			0,
			sizeof(D3DTNTVERTEX)));
	SafeDX(g_pd3dDevice->SetFVF(MY_VERTEX_TYPE));

	// Create color textures - we use them to fake color blending if tfactor is used
	SafeDX(g_pd3dDevice->CreateTexture(YoshiBGWidth, YoshiBGHeight, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &texBackground[0], NULL));
	SafeDX(g_pd3dDevice->CreateTexture(YoshiBGWidth, YoshiBGHeight, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &texBackground[1], NULL));
	SafeDX(g_pd3dDevice->CreateTexture(YoshiBGWidth, YoshiBGHeight, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &texBackground[2], NULL));
	for (i=0;i<8;i++){
		SafeDX(g_pd3dDevice->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g_ColorTexture[i].D3DObject, NULL));
		g_ColorTexture[i].Color = 0;
	}
//	SafeDX(g_pd3dDevice->CreateRenderTarget(512, 256, d3ddm.Format, D3DMULTISAMPLE_NONE, TRUE, &surWriteback));
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
	
	// Create some standard combiners...
	// Highlight (debug)
	State.ps_highlight = CompileShader("ps.1.1\nmov r0,c6\n");
	State.ps_diffuseonly = CompileShader("ps.1.1\nmov r0,v0");
	State.ps_textureonly = CompileShader("ps.1.1\ntex t0\nmov r0,t0");
	State.ps_texcoloronly = CompileShader("ps.1.1\ntex t0\nmov r0,t0\nmov r0.a,c5");
	State.ps_texalphaonly = CompileShader("ps.1.1\ntex t0\nmov r0,t0.a\nmov r0.a,c5");
	
	
	return NOERROR;
}

void FreeTextures()
{
	U32 i;
	if (State.TextureCount)
		for (i=0;i<State.TextureCount;i++){
			FreeObject(State.Textures[i].D3DObject);
		}
	State.TextureCount = 0;

}

void FreeDirect3D()
{
	FreeTextures();
	U32 i;
	for (i=0;i<8;i++){
		FreeObject(g_ColorTexture[i].D3DObject);
	}
	for (i=0;i<LoggedCombineModes.Count;i++){
		FreeObject(LoggedCombineModes.Modes[i].pixelshaderhandle);
	}
	FreeObject(State.ps_texalphaonly);
	FreeObject(State.ps_texcoloronly);
	FreeObject(State.ps_diffuseonly);
	FreeObject(State.ps_highlight);
	FreeObject(State.ps_textureonly);
	LoggedCombineModes.Count = 0;
//	FreeObject(surWriteback);
	FreeObject(texBackground[2]);
	FreeObject(texBackground[1]);
	FreeObject(texBackground[0]);
	FreeObject(g_VertexBuffer);
	FreeObject(g_pd3dDevice);
	FreeObject(g_pD3D);
}
