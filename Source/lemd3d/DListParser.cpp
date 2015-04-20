#include "stdafx.h"
#include "MicroCodeDetector.h"
#include "s2dex.h"

typedef void TMicroCodeFunction();

TMicroCodeFunction *MicroCodeFunctions[256];  // functions of the display lists
TMicroCodeFunction *RDPFunctions[64];         // functions of the real rdp hardware


bool bFirstDisplayList;
bool bDListFinished;
U32 uMicroCodeID;
U32 uBaseMicroCodeID;
char msExcError[100];

// Stack for embedded display lists
TStack Stack[10];
U32 uStackIndex;

U32 Segment[16];

// Current DList Command/Opcode
TCommand Command;

void GBI_UNKNOWN()
{
	LogCommand("GBI_UNKNOWN, Command Index: 0x%X, raw data(hi:lo): 0x%X:0x%X\n", Command.general.command, Command.raw.hi, Command.raw.lo);
}

void GBI_TEXRECT()
{
	FlushVertexCache();

	TCommand Command2, Command3;
	float x1, x2, y1, y2;
	float u1, u2, v1, v2;

	Command2.raw.hi = Read32(Stack[uStackIndex].PC+8);
	Command2.raw.lo = Read32(Stack[uStackIndex].PC+12);
	Command3.raw.hi = Read32(Stack[uStackIndex].PC+16);
	Command3.raw.lo = Read32(Stack[uStackIndex].PC+20);
	x1 = (((float)Command.texrect_1.xh)/4);
	x2 = (((float)Command.texrect_1.xl)/4);
	y1 = (((float)Command.texrect_1.yh)/4);
	y2 = (((float)Command.texrect_1.yl)/4);
	// in 1/2 cycle mode increase texrect size
	if (State.Othermode.fields.cycletype>1){
		x2++;
		y2++;
	}
	State.ZBufferWrite = false;
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, false));

	if ((State.Viewport.X!=0) ||
		(State.Viewport.Y!=0) ||
		(State.Viewport.Width!=State.WindowWidth) ||
		(State.Viewport.Height!=State.WindowHeight) ||
		(State.Viewport.MinZ!=0.0f) ||
		(State.Viewport.MaxZ!=1.0f)){
		State.Viewport.X = 0;
		State.Viewport.Y = 0;
		State.Viewport.Width = State.WindowWidth;
		State.Viewport.Height = State.WindowHeight;
		State.Viewport.MinZ = 0.0f;
		State.Viewport.MaxZ = 1.0f;
		SafeDX(g_pd3dDevice->SetViewport(&State.Viewport));
	}

	D3DXMATRIX Ortho;
	
	D3DXMatrixOrthoOffCenterRH(&Ortho, 0, (float)State.cimg.width, (float)GetHeight(State.cimg.width), 0, 0.0f, 1.0f);
	SafeDX(g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &Ortho));
	State.ProjectionChanged = true;
	float s, t, dsdx, dtdy;
	if (uMicroCodeID==_microcode_rogue){
		s = (_10_5ToFloat(Command2.texrect_2_rogue.s)-State.Tiles[Command.texrect_1.tile].uls)*ShiftTable[State.Tiles[Command.texrect_1.tile].shifts];
		t = (_10_5ToFloat(Command2.texrect_2_rogue.t)-State.Tiles[Command.texrect_1.tile].ult)*ShiftTable[State.Tiles[Command.texrect_1.tile].shiftt];
		dsdx = _10_5ToFloat(Command2.texrect_2_rogue.dsdx)/32.0f;
		dtdy = _10_5ToFloat(Command2.texrect_2_rogue.dtdy)/32.0f;
		if (State.Othermode.fields.cycletype==2){
			// in copy mode, 4 pixels are copied at the same time
			dsdx /= 4;
		}
		u1 = s/(State.Tiles[Command.texrect_1.tile].D3DWidth);
		v1 = t/(State.Tiles[Command.texrect_1.tile].D3DHeight);

	} else {
		s = (_10_5ToFloat(Command2.texrect_2.s)-State.Tiles[Command.texrect_1.tile].uls)*ShiftTable[State.Tiles[Command.texrect_1.tile].shifts];
		t = (_10_5ToFloat(Command2.texrect_2.t)-State.Tiles[Command.texrect_1.tile].ult)*ShiftTable[State.Tiles[Command.texrect_1.tile].shiftt];
		dsdx = _10_5ToFloat(Command3.texrect_3.dsdx)/32.0f;
		dtdy = _10_5ToFloat(Command3.texrect_3.dtdy)/32.0f;
		if (State.Othermode.fields.cycletype==2){
			// in copy mode, 4 pixels are copied at the same time
			dsdx /= 4;
		}
		u1 = s/(State.Tiles[Command.texrect_1.tile].D3DWidth);
		v1 = t/(State.Tiles[Command.texrect_1.tile].D3DHeight);
	/*
		u1 = u1 - (int)u1;
		v1 = v1 - (int)v1;
	*/
	}

	u2 = u1 + dsdx*(x2-x1)/(State.Tiles[Command.texrect_1.tile].D3DWidth);
	v2 = v1 + dtdy*(y2-y1)/(State.Tiles[Command.texrect_1.tile].D3DHeight);
	LogCommand("GBI_TEXRECT, tile: %i, left: %f, top: %f, right: %f, bottom: %f, s: %f, t: %f, dsdx: %f, dsdy: %f\n",
		Command.texrect_1.tile, x1, y1, x2, y2, s, t, dsdx, dtdy);

	S32 X1, X2, Y1, Y2;
	if (State.bSoftmode){
		// directly emulate this cr** and render into rdram
		// CFB Width: State.cimg.width
		// Format: State.cimg.fmt/State.cimg.size
		U8 TMEMFlipped[TMEMSize];
		SimulateTLoadCommand(0, TMEMFlipped, (long)(y2-y1), (long)(x2-x1), State.Tiles[Command.texrect_1.tile].size);
		
		S32 x, y;
		X1 = (S32)x1;
		X2 = (S32)x2;
		Y1 = (S32)y1;
		Y2 = (S32)y2;
		if (Y2>State.Scissor.y2){
			// important
			Y2 = State.Scissor.y2;
		}
		if (X2>State.Scissor.x2){
			X2 = State.Scissor.x2;
		}
		if (X1<State.Scissor.x1){
			X1 = State.Scissor.x1;
		}
		if (Y1<State.Scissor.y1){
			Y1 = State.Scissor.y1;
		}
		for (y=Y1;y<Y2;y++){
			for (x=X1;x<X2;x++){
				_RDRAM[State.cimg.address + (y*State.cimg.width) + x] = TMEMFlipped[(y-(long)y1)*(long)(x2-x1) + x - (long)x1];
			}
		}
		Stack[uStackIndex].PC+=16;
		return;
	} 
	if (!CImgBad()){
	/*
		if (y2>State.Scissor.y2){
			// important
			y1 = State.Scissor.y2;
		}
		if (x2>State.Scissor.x2){
			x2 = State.Scissor.x2;
		}
		if (x1<State.Scissor.x1){
			x1 = State.Scissor.x1;
		}
		if (y1<State.Scissor.y1){
			y1 = State.Scissor.y1;
		}
	*/
	#if DoOwnGeometry
		x1 *= State.Viewport.Width  / (State.cimg.width);
		x2 *= State.Viewport.Width  / (State.cimg.width);
		y1 *= State.Viewport.Height / GetHeight(State.cimg.width);
		y2 *= State.Viewport.Height / GetHeight(State.cimg.width);
	#endif
		D3DTNTVERTEX *VertexBuffer;
		VertexBuffer = &State.D3DVertexCache[0];
		VertexBuffer[0].color = 0xFFFFFFFF;
		VertexBuffer[0].x = x1;
		VertexBuffer[0].y = y1;
		VertexBuffer[0].z = 0.0f;
		_TexAssignU((VertexBuffer+0)) = u1;
		_TexAssignV((VertexBuffer+0)) = v1;
		
		VertexBuffer[1].color = 0xFFFFFFFF;
		VertexBuffer[1].x = x2;
		VertexBuffer[1].y = y1;
		VertexBuffer[1].z = 0.0f;
		_TexAssignU((VertexBuffer+1)) = u2;
		_TexAssignV((VertexBuffer+1)) = v1;
		
		VertexBuffer[2].color = 0xFFFFFFFF;
		VertexBuffer[2].x = x1;
		VertexBuffer[2].y = y2;
		VertexBuffer[2].z = 0.0f;
		_TexAssignU((VertexBuffer+2)) = u1;
		_TexAssignV((VertexBuffer+2)) = v2;
		
		VertexBuffer[3].color = 0xFFFFFFFF;
		VertexBuffer[3].x = x2;
		VertexBuffer[3].y = y2;
		VertexBuffer[3].z = 0.0f;
		_TexAssignU((VertexBuffer+3)) = u2;
		_TexAssignV((VertexBuffer+3)) = v2;

	#if DoOwnGeometry	
		VertexBuffer[0].rhw = 1.0f;
		VertexBuffer[1].rhw = 1.0f;
		VertexBuffer[2].rhw = 1.0f;
		VertexBuffer[3].rhw = 1.0f;
	#endif
		State.D3DVertexCacheIndex=4;
		SendTrisToD3D(&State.Tiles[Command.texrect_1.tile], true, D3DPT_TRIANGLESTRIP, false);
	}

	switch (uMicroCodeID){
	case _microcode_naboo:
		Stack[uStackIndex].PC=State.NabooPCAfterTexRect&BitM24;
		break;
	case _microcode_rogue:
		Stack[uStackIndex].PC+=8;
		break;
	default:
		Stack[uStackIndex].PC+=16;
		break;
	}
}

void GBI_TEXRECTFLIP()
{
	LogCommand("GBI_TEXRECTFLIP\n");
}

void GBI_RDPLOADSYNC()
{
	LogCommand("GBI_RDPLOADSYNC\n");
}

void GBI_RDPPIPESYNC()
{
	LogCommand("GBI_RDPPIPESYNC\n");
}

void GBI_RDPTILESYNC()
{
	LogCommand("GBI_RDPTILESYNC\n");
}

void GBI_RDPFULLSYNC()
{
	LogCommand("GBI_RDPFULLSYNC\n");
}

void GBI_SETKEYGB()
{
	LogCommand("GBI_SETKEYGB\n");
}

void GBI_SETKEYR()
{
	LogCommand("GBI_SETKEYR\n");
}

void GBI_SETCONVERT()
{
	FlushVertexCache();
	State.Convert.K0 = Command.setconvert.k0;
	State.Convert.K1 = Command.setconvert.k1;
	State.Convert.K2 = (Command.setconvert.k2lo)|(Command.setconvert.k2hi<<5);
	State.Convert.K3 = Command.setconvert.k3;
	State.Convert.K4 = Command.setconvert.k4;
	State.Convert.K5 = Command.setconvert.k5;
	State.CombineChanged = true;
}

void GBI_SETSCISSOR()
{
	LogCommand("GBI_SETSCISSOR\n");
	if ((State.Scissor.x1!=Command.fillrect.x0) ||
		(State.Scissor.x2!=Command.fillrect.x1) ||
		(State.Scissor.y1!=Command.fillrect.y0) ||
		(State.Scissor.y2!=Command.fillrect.y1)){
		
		RECT r1;
		
		State.Scissor.x1 = Command.fillrect.x0;
		State.Scissor.x2 = Command.fillrect.x1;
		State.Scissor.y1 = Command.fillrect.y0;
		State.Scissor.y2 = Command.fillrect.y1;

		if (State.cimg.width!=0){
			r1.left = Command.fillrect.x0 * State.WindowWidth / (State.cimg.width);
			r1.right = Command.fillrect.x1 * State.WindowWidth / (State.cimg.width);
			r1.top = Command.fillrect.y0 * State.WindowHeight / GetHeight(State.cimg.width);
			r1.bottom = Command.fillrect.y1 * State.WindowHeight / GetHeight(State.cimg.width);
		} else {
			r1.left = 0;
			r1.right = State.WindowWidth;
			r1.top = 0;
			r1.bottom = State.WindowHeight;
		}

		SafeDX(g_pd3dDevice->SetScissorRect(&r1));
	}
}

void GBI_SETPRIMDEPTH()
{
	LogCommand("GBI_SETPRIMDEPTH\n");
}

void GBI_RDPSETOTHERMODE()
{
	LogCommand("GBI_RDPSETOTHERMODE\n");
	//FlushVertexCache();
	State.Othermode.raw.hi = Command.raw.hi&BitM24;
	State.Othermode.raw.lo = Command.raw.lo;
	OthermodeChanged();
}

void GBI_LOADTLUT()
{
	LogCommand("GBI_LOADTLUT, timg addr: %X, Count: %i, TMEM: %X\n",
		State.timg.address,
		Command.loadtlut.count+1,
		State.Tiles[Command.loadtlut.tile].tmem);
	SetLoadTileTlutInfo(_LoadTLUT);
	
/*
	FlushVertexCache();
	U32 uTMEM, uCount, t0, t1, t2, t3;
	U32 i;
	uTMEM = State.Tiles[Command.loadtlut.tile].tmem;
	uCount = Command.loadtlut.count+1;
	t0 = BITS(Command.raw.hi,16, 8);
	t1 = BITS(Command.raw.hi, 4, 8);
	t2 = BITS(Command.raw.lo,16, 8);
	t3 = BITS(Command.raw.lo, 4, 8);
	for (i=0;i<uCount/16;i++){
		State.TMEM[uTMEM+i*32].RDRAMAddress = State.timg.address+(i*32);
	}
	State.TextureChanged = true;
*/
}

void GBI_SETTILESIZE()
{
	FlushVertexCache();

	U32 i = Command.settilesize.tile;
	State.Tiles[i].uls = (float)(Command.settilesize.uls+Command.settilesize.uls_frac/4.0f);
	State.Tiles[i].ult = (float)(Command.settilesize.ult+Command.settilesize.ult_frac/4.0f);
	State.Tiles[i].lrs = (float)(Command.settilesize.lrs+Command.settilesize.lrs_frac/4.0f);
	State.Tiles[i].lrt = (float)(Command.settilesize.lrt+Command.settilesize.lrt_frac/4.0f);
	LogCommand("GBI_SETTILESIZE: uls: %f, ult: %f, tile: %i, lrs: %f, lrt: %f\n",
		State.Tiles[i].uls,
		State.Tiles[i].ult,
		Command.settilesize.tile,
		State.Tiles[i].lrs+1,
		State.Tiles[i].lrt+1);
	CalculateTileSize(&State.Tiles[i]);
	State.TextureChanged = true;
}

void GBI_LOADBLOCK()
{
	// Load texture into texture cache
	FlushVertexCache();
	// Put data from rdram to tmem
	U32 Size, xoffset, Width;
	switch (State.Tiles[Command.loadblock.tile].size){
	case 0:
		Size = (Command.loadblock.lrs+1)>>1;
		xoffset = Command.loadblock.uls>>1;
		Width = State.timg.width>>1;
		break;
	case 1:
		Size = (Command.loadblock.lrs+1);
		xoffset = Command.loadblock.uls;
		Width = State.timg.width;
		break;
	case 2:
		Size = (Command.loadblock.lrs+1)<<1;
		xoffset = Command.loadblock.uls<<1;
		Width = State.timg.width<<1;
		break;
	case 3:
		Size = (Command.loadblock.lrs+1)<<2;
		xoffset = Command.loadblock.uls<<2;
		Width = State.timg.width<<2;
		break;
	}
/*
	ProfileStart(14);
	U32 bi;
	if (((TMEMAddress+Size)<=TMEMSize) && ((RDRAMAddress+Size)<=8*MB)){
		// Load into TMEM
		if (((TMEMAddress&BitM2)==0) && ((RDRAMAddress&BitM2)==0) && ((Size&BitM2)==0)){
			// It is a nicely aligned copy
			memcpy(&State.TMEM[TMEMAddress], &Global_Gfx_Info.RDRAM[RDRAMAddress], Size);
		} else {
			for (bi=0;bi<Size;bi++){
				State.TMEM[(TMEMAddress+bi)^3] = Global_Gfx_Info.RDRAM[(RDRAMAddress+bi)^3];
			}
		}
	} else {
		ShowError("Invalid data in LoadBlock");
	}
	ProfileStop(14);
*/
	// possible error: Dunno if this is correct
	State.Tiles[Command.loadblock.tile].line = (Command.loadblock.dxt*Size>>3)>>11;
	U32 TMEMAddress;
	U32 RDRAMAddress;
	TMEMAddress = (State.Tiles[Command.loadblock.tile].tmem);
	RDRAMAddress = State.timg.address;
	State.TMEM[TMEMAddress].RDRAMAddress = State.timg.address;
	State.TMEM[TMEMAddress].RDRAMStride = Width;
	State.TMEM[TMEMAddress].LoadType = _LoadBlock;
	State.TMEM[TMEMAddress].Size = Size;
	State.TMEM[TMEMAddress].TSize = State.Tiles[Command.loadblock.tile].size;
	State.TMEM[TMEMAddress].Line = State.Tiles[Command.loadblock.tile].line;
	State.TMEM[TMEMAddress].StartX = xoffset;
	State.TMEM[TMEMAddress].StartY = Command.loadblock.ult;
	State.TMEM[TMEMAddress].DXTUsed = Command.loadblock.dxt!=0;
	
	if ((TMEMAddress+Size>2048) && (TMEMAddress<2048)){
		State.TMEM[2048].RDRAMAddress = State.timg.address+(2048-TMEMAddress);
		State.TMEM[2048].RDRAMStride = Width;
		State.TMEM[2048].LoadType = _LoadBlock;
		State.TMEM[2048].Size = Size;
		State.TMEM[2048].TSize = State.Tiles[Command.loadblock.tile].size;
		State.TMEM[2048].Line = State.Tiles[Command.loadblock.tile].line;
		State.TMEM[2048].StartX = xoffset;
		State.TMEM[2048].StartY = Command.loadblock.ult;
		State.TMEM[2048].DXTUsed = Command.loadblock.dxt!=0;
	}
	State.TextureChanged = true;
	State.LastLoadBlock = true;
	LogCommand("GBI_LOADBLOCK: uls: %i, ult: %i, tile: %i, lrs: %i, dxt: %i, tmem: 0x%x, size: %i\n",
		Command.loadblock.uls,
		Command.loadblock.ult,
		Command.loadblock.tile,
		Command.loadblock.lrs,
		Command.loadblock.dxt,
		TMEMAddress,
		Size);
}

void GBI_LOADTILE()
{
	LogCommand("GBI_LOADTILE: sl: %i, tl: %i, tile: %i, sh: %i, th: %i\n",
		Command.loadtile.sl,
		Command.loadtile.tl,
		Command.loadtile.tile,
		Command.loadtile.sh,
		Command.loadtile.th);
	SetLoadTileTlutInfo(_LoadTile);
}

void GBI_SETTILE()
{
	// Set information about tile - don't load it here
	FlushVertexCache();
	U32 i = Command.settile.tile;

	State.Tiles[i].palette	= Command.settile.palette;
	State.Tiles[i].line		= Command.settile.line;
	State.Tiles[i].fmt		= Command.settile.fmt;
	State.Tiles[i].size		= Command.settile.siz;
	State.Tiles[i].tmem		= Command.settile.tmem<<3;
	State.Tiles[i].cmt		= Command.settile.cmt;
	State.Tiles[i].maskt	= Command.settile.maskt;
	State.Tiles[i].shiftt	= Command.settile.shiftt;
	State.Tiles[i].cms		= Command.settile.cms;
	State.Tiles[i].masks	= Command.settile.masks;
	State.Tiles[i].shifts	= Command.settile.shifts;
	CalculateTileSize(&State.Tiles[i]);
	LogCommand("GBI_SETTILE: shifts: %i, masks: %i, cms: %i, shiftt: %i, maskt: %i, cmt: %i, palette: %i, tile: %i, tmem: %i, line: %i, size: %i, fmt: %i\n",
		Command.settile.shifts,
		Command.settile.masks,
		Command.settile.cms,
		Command.settile.shiftt,
		Command.settile.maskt,
		Command.settile.cmt,
		Command.settile.palette,
		Command.settile.tile,
		Command.settile.tmem,
		Command.settile.line,
		Command.settile.siz,
		Command.settile.fmt);
	State.TextureChanged = true;
}

void GBI_FILLRECT()
{
	LogCommand("GBI_FILLRECT\n");
	if (State.cimg.width<2){
		return;
	}
	FlushVertexCache();
	D3DRECT N64Rect, Rect;
	long Temp;
	N64Rect.x1 = Command.fillrect.x0;
	N64Rect.x2 = Command.fillrect.x1;
	N64Rect.y1 = Command.fillrect.y0;
	N64Rect.y2 = Command.fillrect.y1;
	if (N64Rect.x2 < N64Rect.x1){
		Temp = N64Rect.x1;
		N64Rect.x1 = N64Rect.x2;
		N64Rect.x2 = Temp;
	}
	if (N64Rect.y2 < N64Rect.y1){
		Temp = N64Rect.y1;
		N64Rect.y1 = N64Rect.y2;
		N64Rect.y2 = Temp;
	}
	if (State.Othermode.fields.cycletype==3){
		// fill mode
		N64Rect.x2++;
		N64Rect.y2++;
	}
	Rect.x1 = N64Rect.x1 * State.WindowWidth / (State.cimg.width);
	Rect.x2 = N64Rect.x2 * State.WindowWidth / (State.cimg.width);
	Rect.y1 = N64Rect.y1 * State.WindowHeight / GetHeight(State.cimg.width);
	Rect.y2 = N64Rect.y2 * State.WindowHeight / GetHeight(State.cimg.width);
	
	if ((State.Viewport.X!=0) ||
		(State.Viewport.Y!=0) ||
		(State.Viewport.Width!=State.WindowWidth) ||
		(State.Viewport.Height!=State.WindowHeight) ||
		(State.Viewport.MinZ!=0.0f) ||
		(State.Viewport.MaxZ!=1.0f)){
		State.Viewport.X = 0;
		State.Viewport.Y = 0;
		State.Viewport.Width = State.WindowWidth;
		State.Viewport.Height = State.WindowHeight;
		State.Viewport.MinZ = 0.0f;
		State.Viewport.MaxZ = 1.0f;
		SafeDX(g_pd3dDevice->SetViewport(&State.Viewport));
	}

	if (State.cimg.address == State.zimg.address){
		// Clear Z-Buffer
		g_pd3dDevice->Clear(1, &Rect, D3DCLEAR_ZBUFFER, State.FillColor, 1.0f, 0);
	} else {
		if (CImgBad()){
			return;
		}
		U32 Color;
		Color = State.FillColor;
		bool Yup;
		Yup = true;
		ConfigureBlender1();
		if ((State.Othermode.fields.cycletype==2) || (State.Othermode.fields.cycletype==3)){
			Color = State.FillColor;
		} else {
			switch (State.Combine.raw64){
			case BuildCombineMode1(0, 0, 0, ShadeColor, 0, 0, 0, ShadedAlpha):
				Color = State.FillColor;
				break;
			case BuildCombineMode1(0, 0, 0, EnvColor, 0, 0, 0, EnvAlpha):
				Color = State.EnvColor;
				break;
			case BuildCombineMode1(0, 0, 0, PrimColor, 0, 0, 0, PrimAlpha):
				Color = State.PrimColor;
				break;
			default:
				Yup = false;
				break;
			}
		}
		if (State.Othermode.fields.cycletype==3){
			Color |= 0xFF000000;
		}
		if (State.BlendExtras&Bit2){
			Color &= 0x00FFFFFF;
		}
		if (State.BlendExtras&BLENDMODEEXTRA_WAVERACE){
			// hack
			return;
		}

		if (Yup && ((Color&0xFF000000)==0xFF000000)){
			g_pd3dDevice->Clear(1, &Rect, D3DCLEAR_TARGET, Color, 1.0f, 0);
			LogCommand("That's it !!! Drawing filled rectangle, RDRAM: %X, CycleType: %i\n", State.cimg.address, State.Othermode.fields.cycletype);
			LogCommand("              topleft: %i:%i, bottomright: %i:%i\n", N64Rect.x1, N64Rect.y1, N64Rect.x2, N64Rect.y2);
		} else {
			D3DTNTVERTEX *VertexBuffer;
			State.ZBufferWrite = false;
			SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, false));
			D3DXMATRIX Ortho;
			D3DXMatrixOrthoOffCenterRH(&Ortho, 0, (float)State.WindowWidth, (float)State.WindowHeight, 0, 0.0f, 1.0f);
			SafeDX(g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &Ortho));
			State.ProjectionChanged = true;
			SafeDX(g_VertexBuffer->Lock(0, 4*sizeof(D3DTNTVERTEX), (void**)&VertexBuffer, 0));
			VertexBuffer[0].color = State.FillColor;
			VertexBuffer[0].x = (float)Rect.x1;
			VertexBuffer[0].y = (float)Rect.y1;
			VertexBuffer[0].z = 0.0f;
			
			VertexBuffer[1].color = State.FillColor;
			VertexBuffer[1].x = (float)Rect.x2;
			VertexBuffer[1].y = (float)Rect.y1;
			VertexBuffer[1].z = 0.0f;
			
			VertexBuffer[2].color = State.FillColor;
			VertexBuffer[2].x = (float)Rect.x1;
			VertexBuffer[2].y = (float)Rect.y2;
			VertexBuffer[2].z = 0.0f;
			
			VertexBuffer[3].color = State.FillColor;
			VertexBuffer[3].x = (float)Rect.x2;
			VertexBuffer[3].y = (float)Rect.y2;
			VertexBuffer[3].z = 0.0f;
			SafeDX(g_VertexBuffer->Unlock());

/*
			BlendPasses.Count = 1;
			BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
			BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
			BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
			BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
			BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_DISABLE;
			BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_DISABLE;
			BlendPasses.Passes[0].TFactor = Color;
*/
			SafeDX(g_pd3dDevice->SetPixelShader(State.ps_highlight));
			D3DXVECTOR4 vColor;
			vColor.w = (FLOAT)BITS(Color, 24, 8)/255.0f;
			vColor.x = (FLOAT)BITS(Color, 16, 8)/255.0f;
			vColor.y = (FLOAT)BITS(Color,  8, 8)/255.0f;
			vColor.z = (FLOAT)BITS(Color,  0, 8)/255.0f;
			SafeDX(g_pd3dDevice->SetPixelShaderConstantF(6, (float*)&vColor, 1));
			SafeDX(g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
			SafeDX(g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
			SafeDX(g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
		}
	}
}

void GBI_SETFILLCOLOR()
{
	LogCommand("GBI_SETFILLCOLOR\n");
	U32 NewColor;
	NewColor = (Command.setcolor.color&1?0xFF000000:0) | (((Command.setcolor.color&0x3E)<<2) | ((Command.setcolor.color&0x7C0)<<5) | ((Command.setcolor.color&0xF800)<<8));
//	NewColor = RGBA_to_ARGB(Command.setcolor.color);
	State.FillColor =  NewColor;
	State.CombineChanged = true;
}

void GBI_SETFOGCOLOR()
{
	LogCommand("GBI_SETFOGCOLOR\n");
	U32 NewColor;
//	NewColor = (Command.setcolor.color&0x0001?0xFF000000:0x0) | ((Command.setcolor.color&0x003E)<<2 | (Command.setcolor.color&0x07C0)<<5) | ((Command.setcolor.color&0xF800)<<8);
	NewColor = RGBA_to_ARGB(Command.setcolor.color);
	if (NewColor != State.FogColor){
		FlushVertexCache();
		State.FogColor = NewColor;
		State.CombineChanged = true;
	}
}

void GBI_SETBLENDCOLOR()
{
	LogCommand("GBI_SETBLENDCOLOR\n");
	U32 NewColor;
//	NewColor = (Command.setcolor.color&0x0001?0xFF000000:0x0) | ((Command.setcolor.color&0x003E)<<2 | (Command.setcolor.color&0x07C0)<<5) | ((Command.setcolor.color&0xF800)<<8);
	NewColor = RGBA_to_ARGB(Command.setcolor.color);
	if (NewColor != State.BlendColor){
		FlushVertexCache();
		State.BlendColor = NewColor;
		State.CombineChanged = true;
	}
}

void GBI_SETPRIMCOLOR()
{
	LogCommand("GBI_SETPRIMCOLOR\n");
	U32 NewColor;
	NewColor = RGBA_to_ARGB(Command.setcolor.color);
	if ((NewColor != State.PrimColor) || (Command.setcolor.lod!=State.Lod)){
		FlushVertexCache();
		State.PrimColor = NewColor;
		State.Lod = Command.setcolor.lod;
		State.CombineChanged = true;
	}
}

void GBI_SETENVCOLOR()
{
	LogCommand("GBI_SETENVCOLOR\n");
	U32 NewColor;
	NewColor = RGBA_to_ARGB(Command.setcolor.color);
	if (NewColor != State.EnvColor){
		FlushVertexCache();
		State.EnvColor = NewColor;
		State.CombineChanged = true;
	}
}

void GBI_SETCOMBINE()
{
	FlushVertexCache();
	LogCommand("GBI_SETCOMBINE\n");
	TCombine NewValue;
	NewValue.raw.lo = Command.raw.lo;
	NewValue.raw.hi = Command.raw.hi & BitM24;
	// simplify combiner
	if (NewValue.fields.a0>A_0_MIN) NewValue.fields.a0 = A_0;
	if (NewValue.fields.a1>A_0_MIN) NewValue.fields.a1 = A_0;

	if (NewValue.fields.b0>B_0_MIN) NewValue.fields.b0 = B_0;
	if (NewValue.fields.b1>B_0_MIN) NewValue.fields.b1 = B_0;

	if (NewValue.fields.c0>C_0_MIN) NewValue.fields.c0 = C_0;
	if (NewValue.fields.c1>C_0_MIN) NewValue.fields.c1 = C_0;

	// Detect cycle2: 0, 0, 0, CombinedColor, 0, 0, 0, CombinedAlpha
	if ((NewValue.fields.a1 == A_0) && (NewValue.fields.b1 == B_0) && (NewValue.fields.c1 == C_0) && (NewValue.fields.d1 == D_CombinedColor) &&
		(NewValue.fields.Aa1 == AA_0) && (NewValue.fields.Ab1 == BA_0) && (NewValue.fields.Ac1 == CA_0) && (NewValue.fields.Ad1 == DA_CombinedAlpha)){
		// copy first cycle
		NewValue.fields.a1 = NewValue.fields.a0;
		NewValue.fields.b1 = NewValue.fields.b0;
		NewValue.fields.c1 = NewValue.fields.c0;
		NewValue.fields.d1 = NewValue.fields.d0;
		NewValue.fields.Aa1 = NewValue.fields.Aa0;
		NewValue.fields.Ab1 = NewValue.fields.Ab0;
		NewValue.fields.Ac1 = NewValue.fields.Ac0;
		NewValue.fields.Ad1 = NewValue.fields.Ad0;
	}
	if ((State.Combine.raw.lo!=NewValue.raw.lo) || (State.Combine.raw.hi!=NewValue.raw.lo)){
		State.Combine.raw.lo = NewValue.raw.lo;
		State.Combine.raw.hi = NewValue.raw.hi;
		State.CombineChanged = true;
	}
}

void GBI_SETTIMG()
{
	LogCommand("GBI_SETTIMG\n");
	State.timg.address = Command.setimage.addr + Segment[Command.setimage.segment];
	State.timg.fmt = Command.setimage.fmt;
	State.timg.size = Command.setimage.size;
	State.timg.width = Command.setimage.width+1;
}

void GBI_SETZIMG()
{
	LogCommand("GBI_SETZIMG\n");
	State.zimg.address = Command.setimage.addr + Segment[Command.setimage.segment];
	State.zimg.fmt = Command.setimage.fmt;
	State.zimg.size = Command.setimage.size;
	State.zimg.width = Command.setimage.width+1;
/*
	if (State.zimg.address!=0){
		// black out memory
		if (State.cimg.width==320){
			memset(&Global_Gfx_Info.RDRAM[State.zimg.address], 0, 320*240*2);
		} else if (State.cimg.width==640){
			memset(&Global_Gfx_Info.RDRAM[State.zimg.address], 0, 640*480*2);
			memset(&Global_Gfx_Info.RDRAM[State.cimg.address], 0, 640*480*2);
		}
	}
*/
}

/*
void ReleaseCustomSurface()
{
	uCustomSurface = 0;
	FlushVertexCache();
	LogCommand("Disabling BackBuffer\n");
	SafeDX(g_pd3dDevice->GetDepthStencilSurface(&DepthStencilSurface));
	SafeDX(g_pd3dDevice->EndScene());
	SafeDX(g_pd3dDevice->SetRenderTarget(NormalSurface, DepthStencilSurface));
	SafeDX(g_pd3dDevice->BeginScene());
	// Write back contents of Surface to RDRAM
	D3DLOCKED_RECT Rect;
	U32 *ImgPtr;
	U32 x, y;

	SafeDX(surWriteback->LockRect(&Rect, 0, D3DLOCK_READONLY));
	U32 sx, sy;
	for (y=0;y<uCustomSurfaceHeight;y+=10){
		sy = y*256/uCustomSurfaceHeight;
		ImgPtr = (U32*)((U8*)Rect.pBits + Rect.Pitch*sy);
		for (x=0;x<uCustomSurfaceWidth;x+=20){
			sx = x*512/uCustomSurfaceWidth;
			U32 r, g, b;
			U16 Col;
			r = BITS(ImgPtr[sx], 16, 8);
			g = BITS(ImgPtr[sx],  8, 8);
			b = BITS(ImgPtr[sx],  0, 8);
			r >>= 3;
			g >>= 3;
			b >>= 3;
			Col = (U16)(b|(g<<5)|(r<<10));
			*((U16*)&_RDRAM[uCustomSurfaceAddress + x*2 + (y*uCustomSurfaceWidth*2)]) =
					Col;
			sx = (x + 1)*512/uCustomSurfaceWidth;
			r = BITS(ImgPtr[sx], 16, 8);
			g = BITS(ImgPtr[sx],  8, 8);
			b = BITS(ImgPtr[sx],  0, 8);
			r >>= 3;
			g >>= 3;
			b >>= 3;
			Col = (U16)(b|(g<<5)|(r<<10));
			*((U16*)&_RDRAM[uCustomSurfaceAddress + x*2 + (y*uCustomSurfaceWidth*2) + 2]) =
					Col;
		}
	}
	SafeDX(surWriteback->UnlockRect());
}
*/
void GBI_SETCIMG()
{
	FlushVertexCache();
	State.cimg.address = Command.setimage.addr + Segment[Command.setimage.segment];
	State.cimg.fmt = Command.setimage.fmt;
	State.cimg.size = Command.setimage.size;
	State.cimg.width = (Command.setimage.width&0x3FF)+1;

	LogCommand("GBI_SETCIMG, RDRAM: 0x%X, Width: %i\n", State.cimg.address, State.cimg.width);
	if ((uMicroCodeID==10)){
		// yoshi's story hack
		if ((Command.raw.lo!=0x0f000000) && (uMicroCodeID==10) && (Command.raw.lo!=0x802078c0)){
			State.bSoftmode = true;
		} else {
			State.bSoftmode = false;
		}
	}
}


// Microcode 1 only
void GBI1_VTX()
{
	LogCommand("GBI1_VTX");
	LoadVertices(Command.vtx_gbi1.addr + Segment[Command.vtx_gbi1.segment], Command.vtx_gbi1.n+1, Command.vtx_gbi1.v0);
}

void GBI1_TRI1()
{
	LogCommand("GBI1_TRI1\n");
	Tri1(Command.tri1_gbi1.v1/10, Command.tri1_gbi1.v2/10, Command.tri1_gbi1.v3/10);
}

void GBI1_TRI4()
{
	LogCommand("GBI1_TRI4\n");
	Tri1(Command.tri4_gbi1.v11, Command.tri4_gbi1.v12, Command.tri4_gbi1.v13);
	Tri1(Command.tri4_gbi1.v21, Command.tri4_gbi1.v22, Command.tri4_gbi1.v23);
	Tri1(Command.tri4_gbi1.v31, Command.tri4_gbi1.v32, Command.tri4_gbi1.v33);
	Tri1(Command.tri4_gbi1.v41, Command.tri4_gbi1.v42, Command.tri4_gbi1.v43);
}

void GBI1_RDPHALFCONT()
{
	LogCommand("GBI1_RDPHALFCONT\n");
}

void GBI1_QUAD()
{
	LogCommand("GBI1_QUAD\n");
	ShowError("GBI1_Quad");
}

// Microcode 1 and 2 functions...
void GBI12_SPNOOP()
{
	LogCommand("GBI12_SPNOOP\n");
}

void GBI12_RESERVED0()
{
	LogCommand("GBI12_RESERVED0, raw data (hi:lo): %X:%X\n", Command.raw.hi, Command.raw.lo);
}

void GBI12_MTX()
{
	LogCommand("GBI12_MTX: %s, %s, %s\n",
		(Command.mtx_gbi12.type?"projection":"world"),
		(Command.mtx_gbi12.load?"load":"multiply"),
		(Command.mtx_gbi12.push?"push":"don't push"));
	UseMatrix(Command.mtx_gbi12.addr + Segment[Command.mtx_gbi12.segment], (bool)Command.mtx_gbi12.type, (bool)Command.mtx_gbi12.load, (bool)Command.mtx_gbi12.push);
}

void GBI12_MOVEMEM()
{
	switch (Command.movemem.par){
	case 0x80: // Viewport
		FlushVertexCache();
		State.ScaleX     = Read32(Command.movemem.addr + Segment[Command.movemem.segment]    )>>17;
		State.ScaleY     =(Read32(Command.movemem.addr + Segment[Command.movemem.segment]    )&BitM16)>>1;
		State.ScaleZ     = Read32(Command.movemem.addr + Segment[Command.movemem.segment] + 4)>>17;
		State.TranslateX = Read32(Command.movemem.addr + Segment[Command.movemem.segment] + 8)>>17;
		State.TranslateY =(Read32(Command.movemem.addr + Segment[Command.movemem.segment] + 8)&BitM16)>>1;
		State.TranslateZ = Read32(Command.movemem.addr + Segment[Command.movemem.segment] + 12)>>17;
		LogCommand("GBI12_MOVEMEM: Viewport\n");
		break;
	case 0x82: // LookAtY
		LoadLight(-1, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]]);
		LogCommand("GBI12_MOVEMEM: LookAtY\n");
		break;
	case 0x84: // LookAtX
		LoadLight(-2, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]]);
		LogCommand("GBI12_MOVEMEM: LookAtX\n");
		break;
	case 0x86: // Light 0
		LoadLight(0, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]]);
		LogCommand("GBI12_MOVEMEM: Light 0\n");
		break;
	case 0x88: // Light 1
		LoadLight(1, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]]);
		LogCommand("GBI12_MOVEMEM: Light 1\n");
		break;
	case 0x8A: // Light 2
		LoadLight(2, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]]);
		LogCommand("GBI12_MOVEMEM: Light 2\n");
		break;
	case 0x8C: // Light 3
		LoadLight(3, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]]);
		LogCommand("GBI12_MOVEMEM: Light 3\n");
		break;
	case 0x8E: // Light 4
		LoadLight(4, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]]);
		LogCommand("GBI12_MOVEMEM: Light 4\n");
		break;
	case 0x90: // Light 5
		LoadLight(5, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]]);
		LogCommand("GBI12_MOVEMEM: Light 5\n");
		break;
	case 0x92: // Light 6
		LoadLight(6, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]]);
		LogCommand("GBI12_MOVEMEM: Light 6\n");
		break;
	case 0x94: // Light 7
		LoadLight(7, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]]);
		LogCommand("GBI12_MOVEMEM: Light 7\n");
		break;
	default:
		LogCommand("GBI12_MOVEMEM: Unhandled subopcode %i\n", Command.movemem.par);
		break;
	}
}

void GBI12_RESERVED1()
{
	LogCommand("GBI12_RESERVED1, raw data (hi:lo): %X:%X\n", Command.raw.hi, Command.raw.lo);
}

void GBI12_DL()
{
	LogCommand("GBI12_DL\n");
	if (uStackIndex<10){
		if (Command.dl.branch==0){  // Jump or Branch
			// Do jump
			uStackIndex++;
			PushLevel();  // increase level for logfile
		}
		Stack[uStackIndex].PC = Segment[Command.dl.segment] + Command.dl.addr - 8; // -8 because in the mainloop I increment the counter right after this command
		// why?
		//State.ProjectionChanged = true;
	} else {
		ShowError("Error: Display list stack too small");
	}
	//Log("%X: DL to %X\n", Stack[uStackIndex-1].PC, Stack[uStackIndex-1].PC);
}

void GBI12_RESERVED2()
{
	LogCommand("GBI12_RESERVED2, raw data (hi:lo): %X:%X\n", Command.raw.hi, Command.raw.lo);
}

void GBI12_RESERVED3()
{
	LogCommand("GBI12_RESERVED3, raw data (hi:lo): %X:%X\n", Command.raw.hi, Command.raw.lo);
}

void GBI12_SPRITE2D_BASE()
{
	LogCommand("GBI12_SPRITE2D_BASE\n");
}

void GBI12_RDPHALF2()
{
	LogCommand("GBI12_RDPHALF2, raw data (hi:lo): %X:%X\n", Command.raw.hi, Command.raw.lo);
}

void GBI12_RDPHALF1()
{
	// used for BranchZ and LoadUCode
	LogCommand("GBI12_RDPHALF1, raw data (hi:lo): %X:%X\n", Command.raw.hi, Command.raw.lo);
	State.BranchZAddress = Command.movemem.addr + Segment[Command.movemem.segment];
}

void GBI12_CLEARGEOMMODE()
{
	LogCommand("GBI12_CLEARGEOMMODE\n");
	U32 NewGeoMode;
	NewGeoMode = State.GeometryMode.raw & ~Command.raw.lo;
	if (NewGeoMode!=State.GeometryMode.raw){
		FlushVertexCache();
		State.GeometryMode.raw = NewGeoMode;
	}
}

void GBI12_SETGEOMMODE()
{
	LogCommand("GBI12_SETGEOMMODE\n");
	U32 NewGeoMode;
	NewGeoMode = State.GeometryMode.raw | Command.raw.lo;
	if (NewGeoMode!=State.GeometryMode.raw){
		FlushVertexCache();
		State.GeometryMode.raw = NewGeoMode;
	}
}

void GBI12_ENDDL()
{
	LogCommand("GBI12_ENDDL\n");
	PopLevel();  // increase level for logfile
	if ((uMicroCodeID==_microcode_jfg) && (State.WorldIndex)){
		//ShowError("reseting worldindex in enddl");
		FlushVertexCache();
		State.WorldIndex = 0;
		State.WorldMatrix = &State.World[State.WorldIndex];
	}
	if (uStackIndex==0){
		bDListFinished = true;
	} else {
		uStackIndex--;
	}
}

void GBI12_SETOTHERMODELO()
{
	LogCommand("GBI12_SETOTHERMODELO\n");
	FlushVertexCache();
	U32 Mask;
	Mask = (((1<<Command.setothermode.len)-1)<<Command.setothermode.sft);
	State.Othermode.raw.lo &= ~Mask;
	State.Othermode.raw.lo |= Command.setothermode.data;
	OthermodeChanged();
}

void GBI12_SETOTHERMODEHI()
{
	LogCommand("GBI12_SETOTHERMODEHI\n");
	FlushVertexCache();
	U32 Mask;
	Mask = (((1<<Command.setothermode.len)-1)<<Command.setothermode.sft);
	State.Othermode.raw.hi &= ~Mask;
	State.Othermode.raw.hi |= Command.setothermode.data;
	OthermodeChanged();
}

void GBI12_TEXTURE()
{
	LogCommand("GBI12_TEXTURE: Enabled: %i, tile: %i, s: %i, t: %i, lodscale: %i\n", Command.texture.on, Command.texture.tile, Command.texture.s, Command.texture.t, Command.texture.lodscale);
	FlushVertexCache();
	State.GeometryMode.fields.TextureEnable			= Command.texture.on!=0;
	State.texture.tile				= &State.Tiles[Command.texture.tile];
	switch (Command.texture.s){
	case 0x8000:
		State.texture.TextureScaleS		= 0.5f;
		break;
	case 0xFFFF:
		State.texture.TextureScaleS		= 1.0f;
		break;
	default:
		State.texture.TextureScaleS		= (Command.texture.s)/65535.0f;
		break;
	}
	switch (Command.texture.t){
	case 0x8000:
		State.texture.TextureScaleT		= 0.5f;
		break;
	case 0xFFFF:
		State.texture.TextureScaleT		= 1.0f;
		break;
	default:
		State.texture.TextureScaleT		= (Command.texture.t)/65535.0f;
		break;
	}
	State.texture.MipMapLevels		= Command.texture.lodscale;   // hwnd does this differently
	State.TextureChanged = true;
}

void GBI12_MOVEWORD()
{
	FlushVertexCache();
	switch (Command.moveword_gbi12.number){
	case 0x02: // NumLights
		State.NumLights = (Command.moveword_gbi12.data&0xfff)/32-1;
		State.XFormLights = (Command.raw.lo&Bit32)!=0;
		LogCommand("GBI12_MOVEWORD: Numlights: %i\n", State.NumLights);
		break;
	case 0x04: // clip
		LogCommand("GBI12_MOVEWORD: Clip unhandled\n");
		break;
	case 0x06: // segment
		//Log("%X: Segment %i, Data: %X\n", Stack[uStackIndex].PC, Command.moveword.mw_index>>2, Command.moveword.base);
		Segment[Command.moveword_gbi12.mw_index>>2] = Command.moveword_gbi12.data&BitM24;
		LogCommand("GBI12_MOVEWORD: Segment\n");
		break;
	case 0x08: // Fog min max
		if (uMicroCodeID==_microcode_s2dex){
			State.S2DEXStatus[Command.moveword_gbi12.mw_index>>2] =
				Command.moveword_gbi12.data;
			LogCommand("GBI12_MOVEWORD: STATUS, SID: %i, Value: 0x%X\n", Command.moveword_gbi12.mw_index>>2, Command.moveword_gbi12.data);
		} else {
/*
			float a, b;
			a = (float)(S16)(Command.moveword_gbi3.data>>16);
			b = (float)(S16)(Command.moveword_gbi3.data&BitM16);

			float Min, Max, Diff;
			Diff = 128000.0f/a;
			Min = 500-(b*Diff/256);
			Max = (128000/a) + Min;
			State.FogStart = Min;
			State.FogEnd = Max;
			LogCommand("GBI12_FOG\n");
*/
			U32 a, b, min, max;
			a = Command.moveword_gbi3.data>>16;
			b = Command.moveword_gbi3.data&BitM16;
			min = b-a;
			max = b+a;
			State.FogStart = min*(16384.0f/65536.0f);
			State.FogEnd = max*(16384.0f/65536.0f);
			LogCommand("GBI12_MOVEWORD: FOG\n");
		}
		break;
	case 0x0A: // Lightcol
		if (Command.moveword_gbi12.mw_index%32==0){
			State.Lights[Command.moveword_gbi12.mw_index/32].r = BITS(Command.moveword_gbi12.data, 24, 8);
			State.Lights[Command.moveword_gbi12.mw_index/32].g = BITS(Command.moveword_gbi12.data, 16, 8);
			State.Lights[Command.moveword_gbi12.mw_index/32].b = BITS(Command.moveword_gbi12.data, 8, 8);
		}
		LogCommand("GBI12_MOVEWORD: Light color, Index: %i, b?: %i, color:%X\n", Command.moveword_gbi12.mw_index/32, Command.moveword_gbi12.mw_index%32!=0, Command.moveword_gbi12.data);
		break;
	case 0x0E:
		LogCommand("GBI12_MOVEWORD: Perpsnorm unhandled\n");
		break;
	default:
		LogCommand("GBI12_MOVEWORD: Unhandled subopcode %i\n", Command.moveword_gbi12.number);
		break;
	}
}

void GBI12_POPMTX()
{
	LogCommand("GBI12_POPMTX\n");
	FlushVertexCache();
	if (State.WorldIndex>0){
		State.WorldIndex--;
		State.WorldMatrix = &State.World[State.WorldIndex];
	} else {
		// Matrix underflow
		ShowError("Matrix underflow");
	}
}

void GBI12_CULLDL()
{
	LogCommand("GBI12_CULLDL\n");
	if (Options.bDoCulling){
	}
}

void GBI12_NOOP()
{
	LogCommand("GBI12_NOOP\n");
}

// Microcode 2 only...

void GBI2_VTX()
{
	LogCommand("GBI2_VTX");
	LoadVertices(Command.vtx_gbi2.addr + Segment[Command.vtx_gbi2.segment], Command.vtx_gbi2.n, Command.vtx_gbi2.v0);
}

void GBI2_LOADUCODE()
{
	U32 uOldMicrocodeID;
	uOldMicrocodeID = uMicroCodeID;
	LogCommand("GBI2_LOADUCODE\n");
	(*((U32*)&Global_Gfx_Info.DMEM[0xFC0 + 16])) = State.BranchZAddress;
	DetectMicroCode(0xFFFFFFFF);
	if ((uMicroCodeID==_microcode_s2dex) && (uOldMicrocodeID==_microcode_f3dex)){
		// Initialize Projection Matrix to Identity
		State.ProjectionChanged = true;
		State.ProjectionBackupS2DEX = State.Projection;
		State.Projection = State.Identity;
		State.WorldMatrix = &State.Identity;
	}
	if ((uMicroCodeID==_microcode_f3dex) && (uOldMicrocodeID==_microcode_s2dex)){
		// Initialize Projection Matrix to Identity
		State.ProjectionChanged = true;
		State.Projection = State.ProjectionBackupS2DEX;
		State.WorldMatrix = &State.World[State.WorldIndex];
	}

	if (uMicroCodeID==0){
		DetectMicroCode(uOldMicrocodeID);
	}
}

void GBI2_BRANCHZ()
{
	LogCommand("GBI2_BRANCHZ\n");
}

void GBI2_TRI2()
{
	LogCommand("GBI2_TRI2\n");
	Tri1(Command.tri2_gbi2.v1, Command.tri2_gbi2.v2, Command.tri2_gbi2.v3);
	Tri1(Command.tri2_gbi2.v4, Command.tri2_gbi2.v5, Command.tri2_gbi2.v6);
}

void GBI2_QUAD()
{
	LogCommand("GBI2_QUAD\n");
	Tri1(Command.quad_gbi2.v1, Command.quad_gbi2.v2, Command.quad_gbi2.v3);
	Tri1(Command.quad_gbi2.v1, Command.quad_gbi2.v3, Command.quad_gbi2.v4);
}

void GBI2_TRI1()
{
	LogCommand("GBI2_TRI1\n");
	Tri1(Command.tri1_gbi2.v1, Command.tri1_gbi2.v2, Command.tri1_gbi2.v3);
}

// WaveRace US commands
void GBI4_VTX()
{
	LogCommand("GBI4_VTX");
	LoadVertices(Command.vtx_gbi4.addr + Segment[Command.vtx_gbi4.segment], Command.vtx_gbi4.n, Command.vtx_gbi4.v0);
}

void GBI4_TRI1()
{
	LogCommand("GBI4_TRI1\n");
	Tri1(Command.tri1_gbi4.v1/5, Command.tri1_gbi4.v2/5, Command.tri1_gbi4.v3/5);
}

void GBI4_QUAD()
{
	LogCommand("GBI4_QUAD\n");
	Tri1(Command.quad_gbi4.v1/5, Command.quad_gbi4.v2/5, Command.quad_gbi4.v3/5);
	Tri1(Command.quad_gbi4.v1/5, Command.quad_gbi4.v3/5, Command.quad_gbi4.v4/5);
}

// Microcode 2 and 3 function(s)...

void GBI23_MODIFYVTX()
{
	LogCommand("GBI23_MODIFYVTX\n");
	FlushVertexCache();
	//ShowError("Modify vertex");
	switch (Command.modifyvtx.where){
	case 0x14: // Texture Coordinate Value
		_TexAssignU((VertexCache+Command.modifyvtx.vtx)) = _10_5ToFloat(Command.modifyvtx.val.NewST.NewS);
		_TexAssignV((VertexCache+Command.modifyvtx.vtx)) = _10_5ToFloat(Command.modifyvtx.val.NewST.NewT);
		break;
	case 0x10: // RGBA
	case 0x18: // XYSCREEN
	case 0x1C: // ZSCREEN
	default:
		//ShowError("Modify Vertex: unhandled value");
		break;
	}
}

// Microcode 3 only
void GBI3_NOOP()
{
	LogCommand("GBI3_NOOP\n");
}

void GBI3_VTX()
{
	LogCommand("GBI3_VTX\n");
	//if (Command.vtx_gbi3.v0n > Command.vtx_gbi3.n){
		LoadVertices(Command.vtx_gbi3.addr + Segment[Command.vtx_gbi3.segment], Command.vtx_gbi3.n, Command.vtx_gbi3.v0n - Command.vtx_gbi3.n);
	//}
}

void GBI3_CULLDL()
{
	LogCommand("GBI3_CULLDL from %i to %i...\n", Command.cull_gbi3.start, Command.cull_gbi3.end + 1);
	if (Options.bDoCulling){
		U32 uStart, uEnd, u;
		float x, y, z, rhw;
		D3DXMATRIX *Matrix;

		uStart = Command.cull_gbi3.start;
		uEnd = Command.cull_gbi3.end + 1;
		Matrix = &State.Projection;

		puCullCalled++;
		for (u=uStart;u<uEnd;u++){
			z = (VertexCache[u].x*Matrix->_13 + VertexCache[u].y*Matrix->_23 + VertexCache[u].z*Matrix->_33 + Matrix->_43);
			if ((z>=0.0f) && (z<=32767.0f)){
				// check x, y
				x = (VertexCache[u].x*Matrix->_11 + VertexCache[u].y*Matrix->_21 + VertexCache[u].z*Matrix->_31 + Matrix->_41);
				rhw = (VertexCache[u].x*Matrix->_14 + VertexCache[u].y*Matrix->_24 + VertexCache[u].z*Matrix->_34 + Matrix->_44);
				if (rhw!=1.0f){
					rhw = 1.0f/rhw;
					x *= rhw;
				}
				
				if ((x>=-1.0f) && (x<=1.0f)){
					y = (VertexCache[u].x*Matrix->_12 + VertexCache[u].y*Matrix->_22 + VertexCache[u].z*Matrix->_32 + Matrix->_42);
					y *= rhw;
					if ((y>=-1.0f) && (y<=1.0f)){
						return;
					}
				}
				return;
			}
		}
		
		puCullDone++;
		// do same as ENDDL
		if (uStackIndex==0){
			bDListFinished = true;
		} else {
			uStackIndex--;
			PopLevel();
		}
	}
}

void GBI3_BRANCHZ()
{
	LogCommand("GBI3_BRANCHZ\n");
	float z;
	U32 u;
	u = Command.branchz.vtx2>>1;
	D3DXMATRIX *Matrix;
	Matrix = &State.Projection;
	z = (VertexCache[u].x*Matrix->_13 + VertexCache[u].y*Matrix->_23 + VertexCache[u].z*Matrix->_33 + Matrix->_43);
	if ((z <= Command.branchz.zvalue)){
		if (uStackIndex<64){
			// Do jump
			uStackIndex++;
			Stack[uStackIndex].PC = State.BranchZAddress - 8; // -8 because in the mainloop I increment the counter right after this command
		} else {
			ShowError("Error: Display list stack too small");
		}
	}
}

void GBI3_TRI1()
{
	LogCommand("GBI3_TRI1\n");
	Tri1(Command.tri1_gbi3.v1, Command.tri1_gbi3.v2, Command.tri1_gbi3.v3);
}

void GBI3_TRI2()
{
	LogCommand("GBI3_TRI2\n");
	Tri1(Command.tri2_gbi3.v1, Command.tri2_gbi3.v2, Command.tri2_gbi3.v3);
	Tri1(Command.tri2_gbi3.v4, Command.tri2_gbi3.v5, Command.tri2_gbi3.v6);
}

void GBI3_QUAD()
{
	LogCommand("GBI3_QUAD\n");
	Tri1(Command.tri2_gbi3.v1, Command.tri2_gbi3.v2, Command.tri2_gbi3.v3);
	Tri1(Command.tri2_gbi3.v4, Command.tri2_gbi3.v5, Command.tri2_gbi3.v6);
}

void GBI3_LINE3D()
{
	LogCommand("GBI3_LINE3D\n");
}

void GBI3_BACKGROUND()
{
	// used in zelda
	LogCommand("GBI3_BACKGROUND\n");

	U32 uTexAddr;
	U32 uRDRAMAddr;
	uRDRAMAddr = Command.background.addr + Segment[Command.background.segment];
	uTexAddr = Read32(uRDRAMAddr + 16);
	BackgroundTexture((uTexAddr&BitM24) + (Segment[BITS(uTexAddr, 24, 4)]),
		0.0f,                        // Left
		(float)(Read32(uRDRAMAddr) >> 2),  // Width
		0.0f,                        // Top
		(float)(Read32(uRDRAMAddr+8) >> 2),// Height
		0.0f,
		1.0f,
		1.0f,
		0.0f,
		0,    // RGBA
		2,    // 16 bit
		320,
		240);   
}

void GBI3_TRI4()
{
	// used in conker's bad fur day and maybe others
	LogCommand("GBI3_TRI4\n");
	Tri1(Command.tri4_gbi3.v11, Command.tri4_gbi3.v12, Command.tri4_gbi3.v13);
	Tri1(Command.tri4_gbi3.v21, Command.tri4_gbi3.v22, Command.tri4_gbi3.v23);
	Tri1(Command.tri4_gbi3.v31, Command.tri4_gbi3.v32, Command.tri4_gbi3.v33);
	Tri1((Command.tri4_gbi3.v41lo)|(Command.tri4_gbi3.v41hi<<2), Command.tri4_gbi3.v42, Command.tri4_gbi3.v43);
}

void GBI3_SPECIAL3()
{
	LogCommand("GBI3_SPECIAL3\n");
}

void GBI3_SPECIAL2()
{
	LogCommand("GBI3_SPECIAL2\n");
}

void GBI3_SPECIAL1()
{
	LogCommand("GBI3_SPECIAL1\n");
}

void GBI3_DMAIO()
{
	LogCommand("GBI3_DMAIO\n");
}

void GBI3_TEXTURE()
{
	LogCommand("GBI3_TEXTURE: Enabled: %i, tile: %i, s: %i, t: %i, lodscale: %i\n", Command.texture.on, Command.texture.tile, Command.texture.s, Command.texture.t, Command.texture.lodscale);
	//Log("Texture: Enabled: %i, tile: %i, s: %i, t: %i, lodscale: %i\n", Command.texture.on, Command.texture.tile, Command.texture.s, Command.texture.t, Command.texture.lodscale);
	FlushVertexCache();
	State.GeometryMode.fields.TextureEnable	= Command.texture.on!=0;
	State.texture.tile				= &State.Tiles[Command.texture.tile];
	switch (Command.texture.s){
	case 0x8000:
		State.texture.TextureScaleS		= 0.5f;
		break;
	case 0xFFFF:
		State.texture.TextureScaleS		= 1.0f;
		break;
	default:
		State.texture.TextureScaleS		= (Command.texture.s)/65535.0f;
		break;
	}
	switch (Command.texture.t){
	case 0x8000:
		State.texture.TextureScaleT		= 0.5f;
		break;
	case 0xFFFF:
		State.texture.TextureScaleT		= 1.0f;
		break;
	default:
		State.texture.TextureScaleT		= (Command.texture.t)/65535.0f;
		break;
	}
	State.texture.MipMapLevels		= Command.texture.lodscale;   // hwnd does this differently
	State.TextureChanged = true;
}

void GBI3_POPMATRIX()
{
	LogCommand("GBI3_POPMATRIX\n");
	FlushVertexCache();
	if (State.WorldIndex>0){
		State.WorldIndex--;
		State.WorldMatrix = &State.World[State.WorldIndex];
	} else {
		// Matrix underflow
		//ShowError("Matrix underflow");
	}
}

void GBI3_GEOMETRYMODE()
{
	FlushVertexCache();
	State.GeometryMode_gbi3.raw = State.GeometryMode_gbi3.raw & (Command.raw.hi&BitM24) | Command.raw.lo;
	LogCommand("GBI3_GEOMETRYMODE changed to 0x%X\n", State.GeometryMode_gbi3.raw);
	//State.GeometryMode.fields.TextureEnable = 1;
	State.GeometryMode.fields.ZBuffer     = State.GeometryMode_gbi3.fields.ZBuffer;
	State.GeometryMode.fields.Shade       = State.GeometryMode_gbi3.fields.Shade;
	State.GeometryMode.fields.CullFront   = State.GeometryMode_gbi3.fields.CullFront;
	State.GeometryMode.fields.CullBack    = State.GeometryMode_gbi3.fields.CullBack;
	State.GeometryMode.fields.Fog         = State.GeometryMode_gbi3.fields.Fog;
	State.GeometryMode.fields.Lighting    = State.GeometryMode_gbi3.fields.Lighting;
	State.GeometryMode.fields.Texture_Gen = State.GeometryMode_gbi3.fields.Texture_Gen;
	State.GeometryMode.fields.Texture_Gen_Linear = State.GeometryMode_gbi3.fields.Texture_Gen_Linear;
	State.GeometryMode.fields.Lod         = State.GeometryMode_gbi3.fields.Lod;
	State.GeometryMode.fields.Clipping    = State.GeometryMode_gbi3.fields.Clipping;
	State.GeometryMode.fields.SmoothShading = State.GeometryMode_gbi3.fields.SmoothShading;
}

void GBI3_MTX()
{
	LogCommand("GBI3_MTX: %s, %s, %s\n",
		(Command.mtx_gbi3.type?"projection":"world"),
		(Command.mtx_gbi3.load?"load":"multiply"),
		(!Command.mtx_gbi3.push?"push":"don't push"));
	UseMatrix(Command.mtx_gbi3.addr + Segment[Command.mtx_gbi3.segment], (bool)Command.mtx_gbi3.type, (bool)Command.mtx_gbi3.load, !(bool)Command.mtx_gbi3.push);
}

void GBI3_MOVEWORD()
{
	FlushVertexCache();
	switch (Command.moveword_gbi3.number){
	case 0x02: // NumLights
		if (uMicroCodeID==_microcode_conker){
			State.NumLights = (Command.moveword_gbi3.data&0xfff)/48;
		} else {
			State.NumLights = (Command.moveword_gbi3.data&0xfff)/24;
		}
		State.XFormLights = true;//(Command.raw.lo&Bit32!=0);
		//Log("Moveword: Numlight %i\n", State.NumLights);
		LogCommand("GBI3_MOVEWORD: Numlights: %i\n", State.NumLights);
		break;
	case 0x04: // Clip
		LogCommand("GBI3_MOVEWORD: Clip, raw data: %X:%X\n", Command.raw.hi, Command.raw.lo);
		break;
	case 0x06: // segment
		//Log("%X: Segment %i, Data: %X\n", Stack[uStackIndex].PC, Command.moveword.mw_index>>2, Command.moveword.base);
		Segment[Command.moveword_gbi3.mw_index>>2] = Command.moveword_gbi3.data&BitM24;
		LogCommand("GBI3_MOVEWORD: Segment: %i is %X\n", Command.moveword_gbi3.mw_index>>2, Segment[Command.moveword_gbi3.mw_index>>2]);
		break;
	case 0x08: // Fog min max
		long a, b, min, max;
/*
		a = (float)(S16)(Command.moveword_gbi3.data>>16);
		b = (float)(S16)(Command.moveword_gbi3.data&BitM16);

		float Min, Max, Diff;
		Diff = 128000.0f/a;
		Min = 500-(b*Diff/256);
		Max = Min + Diff;
		State.FogStart = Min;//*32,767;
		State.FogEnd = Max;//*32,767;
*/
		a = Command.moveword_gbi3.data>>16;
		b = Command.moveword_gbi3.data&BitM16;
		min = b-a;
		max = b+a;
		State.FogStart = min*(1024.0f/65536.0f);
		State.FogEnd = max*(4*1024.0f/65536.0f);
		LogCommand("GBI3_FOG\n");
		break;
	case 0x0A: // Lightcol
		if (Command.moveword_gbi3.mw_index%24==0){
			State.Lights[Command.moveword_gbi3.mw_index/24].r = BITS(Command.moveword_gbi3.data, 24, 8);
			State.Lights[Command.moveword_gbi3.mw_index/24].g = BITS(Command.moveword_gbi3.data, 16, 8);
			State.Lights[Command.moveword_gbi3.mw_index/24].b = BITS(Command.moveword_gbi3.data, 8, 8);
		}
		LogCommand("GBI3_MOVEWORD: Lightcol, Index: %i, b?: %i, color:%X\n", Command.moveword_gbi3.mw_index/24, Command.moveword_gbi3.mw_index%24!=0, Command.moveword_gbi3.data);
		break;
	case 0x0C:
		State.WorldMatrix = &State.Identity;
		//SafeDX(g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &State.TempMatrix));
		State.bForceMtx = true;
		State.ProjectionChanged = true;
		LogCommand("GBI3_MOVEWORD: Force matrix used\n");
		break;
	case 0x0E: // Perspnorm
		LogCommand("GBI3_MOVEWORD: Perspnorm, raw data: %X:%X\n", Command.raw.hi, Command.raw.lo);
		break;
	case 0x10: // ????? used in Conker's Bad Fur Day
		LogCommand("GBI3_MOVEWORD: 0x10 Conker command, raw data: %X:%X\n", Command.raw.hi, Command.raw.lo);
		break;
	default:
		LogCommand("GBI3_MOVEWORD: Unhandled subopcode: 0x%X\n", Command.moveword_gbi3.number);
		break;
	}
}

void GBI3_MOVEMEM()
{
	switch (Command.movemem_gbi3.idx){
	case 8: // Viewport
		FlushVertexCache();
		State.ScaleX     = Read32(Command.movemem_gbi3.addr + Segment[Command.movemem_gbi3.segment]    )>>17;
		State.ScaleY     =(Read32(Command.movemem_gbi3.addr + Segment[Command.movemem_gbi3.segment]    )&BitM16)>>1;
		State.ScaleZ     = Read32(Command.movemem_gbi3.addr + Segment[Command.movemem_gbi3.segment] + 4)>>17;
		State.TranslateX = Read32(Command.movemem_gbi3.addr + Segment[Command.movemem_gbi3.segment] + 8)>>17;
		State.TranslateY =(Read32(Command.movemem_gbi3.addr + Segment[Command.movemem_gbi3.segment] + 8)&BitM16)>>1;
		State.TranslateZ = Read32(Command.movemem_gbi3.addr + Segment[Command.movemem_gbi3.segment] + 12)>>17;
		LogCommand("GBI3_MOVEMEM: Viewport\n");
		break;
	case 10: // Light
		if (uMicroCodeID==_microcode_conker){
			LoadLight(((S32)Command.movemem_gbi3.ofs/6)-2, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem_gbi3.addr + Segment[Command.movemem_gbi3.segment]]);
		} else {
			LoadLight(((S32)Command.movemem_gbi3.ofs/3)-2, (TLight*)&Global_Gfx_Info.RDRAM[Command.movemem_gbi3.addr + Segment[Command.movemem_gbi3.segment]]);
		}
		LogCommand("GBI3_MOVEMEM: Light\n");
		break;
	case 12: // Point
		LogCommand("GBI3_MOVEMEM: Point\n");
		break;
	case 14: // Matrix
		//FlushVertexCache();
		LoadMatrix(Command.movemem_gbi3.addr + Segment[Command.movemem_gbi3.segment], &State.TempMatrix);
		//State.WorldMatrix = &State.TempMatrix;
		LogCommand("GBI3_MOVEMEM: Matrix\n");
		break;
	default:
		LogCommand("GBI3_MOVEMEM: Unhandled subopcode: 0x%X\n", Command.movemem_gbi3.idx);
		break;
	}
}

void GBI3_LOADUCODE()
{
	LogCommand("GBI3_LOADUCODE\n");
}

void GBI3_DL()
{
	LogCommand("GBI3_DL\n");
	if (uStackIndex<64){
		if (Command.dl.branch==0){  // Jump or Branch
			// Do jump
			uStackIndex++;
			PushLevel();  // increase level for logfile
		}
		Stack[uStackIndex].PC = Segment[Command.dl.segment] + Command.dl.addr - 8; // -8 because in the mainloop I increment the counter right after this command
	} else {
		ShowError("Error: Display list stack too small");
	}
}

void GBI3_ENDDL()
{
	LogCommand("GBI3_ENDDL\n");
	if (uStackIndex==0){
		//Log("%X: EndDL finishes Displaylist\n", Stack[uStackIndex].PC);
		bDListFinished = true;
	} else {
		//Log("%X: EndDL, Jumping back to %X\n", Stack[uStackIndex].PC, Stack[uStackIndex-1].PC+8);
		uStackIndex--;
		PopLevel();
	}
}

void GBI3_SPNOOP()
{
	LogCommand("GBI3_SPNOOP\n");
}

void GBI3_RDPHALF1()
{
	State.BranchZAddress = Command.movemem.addr + Segment[Command.movemem.segment];
	LogCommand("GBI3_RDPHALF1\n");
}

void GBI3_SETOTHERMODELO()
{
	LogCommand("GBI3_SETOTHERMODELO\n");
	U32 Mask;
	Mask = (((1<<(Command.setothermode.len+1))-1)<<(32-(Command.setothermode.len+1)-Command.setothermode.sft));
	State.Othermode.raw.lo &= ~Mask;
	State.Othermode.raw.lo |= Command.setothermode.data;
	OthermodeChanged();
}

void GBI3_SETOTHERMODEHI()
{
	LogCommand("GBI3_SETOTHERMODEHI\n");
	U32 Mask;
	Mask = (((1<<(Command.setothermode.len+1))-1)<<(32-(Command.setothermode.len+1)-Command.setothermode.sft));
	State.Othermode.raw.hi &= ~Mask;
	State.Othermode.raw.hi |= Command.setothermode.data;
	OthermodeChanged();
}

void GBI3_RDPHALF2()
{
	LogCommand("GBI3_RDPHALF2\n");
}


// Diddy Kong Racing additions to GBI2
void GBI5_MTX()
{
	// does not have a real matrix stack - just 3 world matrices
	
	LogCommand("GBI5_MTX, Index: %i, RDRAM: %X+%X == %X\n", BITS(Command.raw.hi, 22, 2), Command.vtx_gbi1.addr, Segment[Command.vtx_gbi1.segment], Command.mtx_gbi12.addr + Segment[Command.mtx_gbi12.segment]);

	
	FlushVertexCache();

	State.WorldIndex = BITS(Command.raw.hi, 22, 2);

	State.WorldMatrix = &State.World[State.WorldIndex];

	LoadMatrix(Command.mtx_gbi12.addr + Segment[Command.mtx_gbi12.segment], State.WorldMatrix);
	State.ProjectionChanged = true;
}

void GBI5_VTX()
{
	U32 A1; // Count
	U32 A2; // Start
	U32 Address;
	U32 Add;
	if (uMicroCodeID==_microcode_jfg){
		// (BITS(Command.raw.hi, 16, 8)&6)
		Add = State.JFGMTXAdd;
		A1 = BITS(Command.raw.hi, 16, 8)>>3;
		A2 = BITS(Command.raw.hi,  8, 8)>>1;
	} else {
		Add = 0;
		A2 = 0;
		A1 = ((Command.raw.hi&0xfff)-8)/18+1;
	}
	Address = Add + Command.vtx_gbi1.addr + Segment[Command.vtx_gbi1.segment];

	LogCommand("GBI5_VTX, RDRAM: %X+%X+%X == %X\n", Add, Command.vtx_gbi1.addr, Segment[Command.vtx_gbi1.segment], Address);
	PushLevel();
	LoadVertices_GBI5(Address, A1, A2);
	PopLevel();
}

void GBI5_RAWRDP()
{
	// Looks like this function directly passes rdp commands
	LogCommand("GBI5_RAWRDP:\n");
	PushLevel();
	U32 OldPC;
	U32 RDPCounter;
	U32 length;

	OldPC = Stack[uStackIndex].PC;
	Stack[uStackIndex].PC = Command.diddy1_gbi5.addr + Segment[Command.diddy1_gbi5.segment];
	length = Command.diddy1_gbi5.par;
	for (RDPCounter=0;RDPCounter<length;RDPCounter++){
		Command.raw.hi = Read32(Stack[uStackIndex].PC);
		Command.raw.lo = Read32(Stack[uStackIndex].PC + 4);
		RDPFunctions[Command.rdpgeneral.command]();
		Stack[uStackIndex].PC+=8;
	}
	PopLevel();
	Stack[uStackIndex].PC = OldPC;
}

void GBI5_TRI()
{
	U32 Count;
	U32 addr, tridata, v0, v1, v2;

	if (State.SomeDKRStuff){
		return;
	}
	Count = (Command.tri_gbi5.count);
	LogCommand("GBI5_TRI, loading %i tris:\n", Count);
	PushLevel();

	// Diddys Triangle command. Unlike other gbis this has the triangle info in rdram
	addr = Command.tri_gbi5.addr + Segment[Command.tri_gbi5.segment];
	while (Count>0){
		tridata = Read32(addr);
		v2 = BITS(tridata,  0, 8);
		v1 = BITS(tridata,  8, 8);
		v0 = BITS(tridata, 16, 8);
		tridata = Read32(addr+4);
		VertexCache[v0].tex[0].u = _10_5ToFloat(tridata>>16);
		VertexCache[v0].tex[0].v = _10_5ToFloat(tridata&BitM16);
		tridata = Read32(addr+8);
		VertexCache[v1].tex[0].u = _10_5ToFloat(tridata>>16);
		VertexCache[v1].tex[0].v = _10_5ToFloat(tridata&BitM16);
		tridata = Read32(addr+12);
		VertexCache[v2].tex[0].u = _10_5ToFloat(tridata>>16);
		VertexCache[v2].tex[0].v = _10_5ToFloat(tridata&BitM16);
		Tri1(v0, v1, v2);
		addr+=16;
		Count--;
	};
	PopLevel();
}

void GBI5_MOVEWORD()
{
	switch (Command.moveword_gbi12.number){
	case 0x02: // NumLights
		State.SomeDKRStuff = Command.raw.lo;
		State.XFormLights = ((Command.raw.lo&Bit32)!=0);
		LogCommand("GBI5_MOVEWORD: SomeDKRStuff %i\n", State.SomeDKRStuff);
		break;
	case 0x04: // clip
		LogCommand("GBI5_MOVEWORD: Clip unhandled\n");
		break;
	case 0x06: // segment
		Segment[Command.moveword_gbi12.mw_index>>2] = Command.moveword_gbi12.data&BitM24;
		LogCommand("GBI5_MOVEWORD: Segment\n");
		break;
	case 0x08: // Fog min max
		FlushVertexCache();
		if (uMicroCodeID==_microcode_s2dex){
			State.S2DEXStatus[Command.moveword_gbi12.mw_index>>2] =
				Command.moveword_gbi12.data;
			LogCommand("GBI5_MOVEWORD: STATUS, SID: %i, Value: 0x%X\n", Command.moveword_gbi12.mw_index>>2, Command.moveword_gbi12.data);
		} else {
			float a, b;
			a = (float)(S16)(Command.moveword_gbi3.data>>16);
			b = (float)(S16)(Command.moveword_gbi3.data&BitM16);

			float Min, Max, Diff;
			Diff = 128000.0f/a;
			Min = 500-(b*Diff/256);
			Max = (128000/a) + Min;
			State.FogStart = Min;
			State.FogEnd = Max;
			LogCommand("GBI5_FOG\n");
		}
		break;
	case 0x0A: // World Index
		if (State.WorldIndex != BITS(Command.raw.lo, 6, 3)){
			FlushVertexCache();
			State.WorldIndex = BITS(Command.raw.lo, 6, 3);
			State.WorldMatrix = &State.World[State.WorldIndex];
			State.ProjectionChanged = true;
		}
		LogCommand("GBI5_MOVEWORD: Set World Matrix Index: %i\n", State.WorldIndex);
		break;
	case 0x0E:
		LogCommand("GBI5_MOVEWORD: Perpsnorm unhandled\n");
		break;
	default:
		LogCommand("GBI5_MOVEWORD: Unhandled subopcode %i\n", Command.moveword_gbi12.number);
		break;
	}
}

void GBI5_DIDDY2()
{
	FlushVertexCache();
	// this command is only one rsp command
	// it writes the lower 16 bits of Command.raw.lo to spmem[0x112]
	// 0x114 is the place where the TEXTURE command stores its info

	// so far this command always has a lower hw of 0x8 ??
	LogCommand("GBI5_DIDDY2\n");
	State.GeometryMode.fields.TextureEnable = Command.raw.lo==0x8;
}

// Perfect Dark
void GBI7_VTXCOLOR()
{
	// this loads the colors into a seperate buffer
	// which is addressed by vtx command
	LogCommand("GBI7_VTXCOLOR from: 0 to %i\n",
		 BITS(Command.raw.hi, 18, 6));
	U32 i;
	typedef struct {
		unsigned alpha: 8; /* alpha */
		unsigned c3   : 8; /* color */
		unsigned c2   : 8; /* color */
		unsigned c1   : 8; /* color */
	} TColorRDRAM;
	TColorRDRAM *Color;

	Color = (TColorRDRAM*)&Global_Gfx_Info.RDRAM[Command.vtx_gbi1.addr + Segment[Command.vtx_gbi1.segment]];
	for (i=0;i<=BITS(Command.raw.hi, 18, 6);i++){
		PDColorCache[i] = (Color[i].alpha<<24) | (Color[i].c1<<16) | (Color[i].c2<<8) | (Color[i].c3);
	}
}

void GBI7_VTX()
{
	// Loads the Vertex data except color
	LogCommand("GBI7_VTX");
	LoadVertices_GBI7(Command.vtx_gbi1.addr + Segment[Command.vtx_gbi1.segment],
		 BITS(Command.raw.hi, 20, 4)+1,
		 BITS(Command.raw.hi, 16, 4));
}

void GBI7_TRI4()
{
	LogCommand("GBI7_TRI4\n");
	//Tri1(3, 0, 4);
	Tri1(Command.tri4_gbi7.v1, Command.tri4_gbi7.v2, Command.tri4_gbi7.v3);
	Tri1(Command.tri4_gbi7.v4, Command.tri4_gbi7.v5, Command.tri4_gbi7.v6);
	Tri1(Command.tri4_gbi7.v7, Command.tri4_gbi7.v8, Command.tri4_gbi7.v9);
	Tri1(Command.tri4_gbi7.v10, Command.tri4_gbi7.v11, Command.tri4_gbi7.v12);
	if (Command.tri4_gbi7.pad){
		ShowError("PerfectDark Tri4: useless bits are not zero.");
	}
}

// Battle for Naboo

void GBI9_DL()
{
	// differs from the other DL commands because it does skip the first command
	// the first 32 bits are stored, because they are
	// used as branch target address in the command in the QUAD "slot"
	
	LogCommand("GBI9_DL\n");
	if (uStackIndex<9){
		if (Command.dl.branch!=0){
			ShowError("Error: Branch bit set in GBI9_DL");
		}
		PushLevel();  // increase level for logfile
		uStackIndex++;
		Stack[uStackIndex].PC = Segment[Command.dl.segment] + Command.dl.addr;
		Stack[uStackIndex].RogueStartDL = Read32(Stack[uStackIndex].PC) & BitM24;
		Stack[uStackIndex].RogueOtherDL = Read32(Stack[uStackIndex].PC+4) & BitM24;
	} else {
		ShowError("Error: Display list stack too small");
	}
	//Log("%X: DL to %X\n", Stack[uStackIndex-1].PC, Stack[uStackIndex-1].PC);
}

void GBI9_BRANCHDL()
{
	// differs from the other DL commands because it does skip the first command
	// the first 32 bits are stored, because they are
	// used as branch target address in the command in the QUAD "slot"
	
	LogCommand("GBI9_BRANCHDL\n");
	if (uStackIndex<10){
		Stack[uStackIndex].PC = Segment[Command.dl.segment] + Command.dl.addr;
		Stack[uStackIndex].RogueStartDL = Read32(Stack[uStackIndex].PC) & BitM24;
		Stack[uStackIndex].RogueOtherDL = Read32(Stack[uStackIndex].PC+4) & BitM24;
	} else {
		ShowError("Error: Display list stack too small");
	}
	//Log("%X: DL to %X\n", Stack[uStackIndex-1].PC, Stack[uStackIndex-1].PC);
}

void GBI9_MOVEWORD()
{
	FlushVertexCache();
	switch (Command.raw.hi&BitM24){
	case 0x58C: // This PC is used after a texrect in naboo
		State.NabooPCAfterTexRect = Segment[Command.dl.segment] + Command.dl.addr;
		LogCommand("Moveword: NabooPCAfterTexRect %x\n", State.NabooPCAfterTexRect);
		break;
	case 0x04: // clip
		LogCommand("GBI12_MOVEWORD: Clip unhandled\n");
		break;
	case 0x06: // segment
		//Log("%X: Segment %i, Data: %X\n", Stack[uStackIndex].PC, Command.moveword.mw_index>>2, Command.moveword.base);
		Segment[Command.moveword_gbi12.mw_index>>2] = Command.moveword_gbi12.data&BitM24;
		LogCommand("GBI12_MOVEWORD: Segment\n");
		break;
	case 0x08: // Fog min max
		float a, b;
		a = (float)(S16)(Command.moveword_gbi3.data>>16);
		b = (float)(S16)(Command.moveword_gbi3.data&BitM16);

		float Min, Max, Diff;
		Diff = 128000.0f/a;
		Min = 500-(b*Diff/256);
		Max = (128000/a) + Min;
		State.FogStart = Min;
		State.FogEnd = Max;
		LogCommand("GBI12_FOG\n");
		break;
	case 0x0A: // Lightcol
		if (Command.moveword_gbi12.mw_index%32==0){
			State.Lights[Command.moveword_gbi12.mw_index/32].r = BITS(Command.moveword_gbi12.data, 24, 8);
			State.Lights[Command.moveword_gbi12.mw_index/32].g = BITS(Command.moveword_gbi12.data, 16, 8);
			State.Lights[Command.moveword_gbi12.mw_index/32].b = BITS(Command.moveword_gbi12.data, 8, 8);
		}
		LogCommand("GBI12_MOVEWORD: Light color, Index: %i, b?: %i, color:%X\n", Command.moveword_gbi12.mw_index/32, Command.moveword_gbi12.mw_index%32!=0, Command.moveword_gbi12.data);
		break;
	case 0x0E:
		LogCommand("GBI12_MOVEWORD: Perpsnorm unhandled\n");
		break;
	default:
		LogCommand("GBI12_MOVEWORD: Unhandled subopcode %i\n", Command.moveword_gbi12.number);
		break;
	}
}

// S2DEX - Yoshi's Story
void GBI10_BG_1CYC()
{
	FlushVertexCache();

	uObjBg *BG;
	BG = (uObjBg*)&_RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]];

	float x1, x2, y1, y2;
	float s1, s2, t1, t2;

	x1 = (BG->s.frameX)/4.0f-1.0f;
	x2 = x1 + (BG->s.frameW+1.0f)/4.0f;
	y1 = (BG->s.frameY)/4.0f-1.0f;
	y2 = y1 + (BG->s.frameH+1.0f)/4.0f;

	float scalex, scaley;
	scalex = (float)BG->s.scaleW/1024.0f;
	scaley = (float)BG->s.scaleH/1024.0f;

	s1 = ((float)scalex*BG->s.imageX/32.0f)/(BG->s.imageW/4.0f)*((float)BG->s.frameW/(float)scalex/(float)BG->s.imageW);
	t1 = ((float)scaley*BG->s.imageY/32.0f)/(BG->s.imageH/4.0f)*((float)BG->s.frameH/(float)scaley/(float)BG->s.imageH);


	s2 = (s1 + scalex);
	t2 = (t1 + scaley);
	s1 = (s1);// + 0.5f - scalex*0.5f);
	t1 = (t1);// + 0.5f - scaley*0.5f);

/*
	s1 = s1 - scalex;
	t1 = t1 - scaley;
*/

	if (x1<0.0f){
		x1 = 0.0f;
	}
	if (y1<0.0f){
		y1 = 0.0f;
	}
	if (x2>320.0f){
		x2 = 320.0f;
	}
	if (y2>240.0f){
		y2 = 240.0f;
	}

	LogCommand("GBI10_BG_1CYC, topleft: %3.2f:%3.2f, bottomright: %3.2f:%3.2f\n", x1, y1, x2, y2);

	BackgroundTexture(BG->s.imagePtr&BitM24 + Segment[BITS(BG->s.imagePtr, 24, 4)] + 0,/*(((BG->s.imageY>>2)+1)*(BG->s.imageW>>2))/0,*/
		x1,
		x2,
		y1,
		y2,
/*
		(float)BG->s.imageX/8.0f/(float)BG->s.imageW + 0.5f - scalex*0.5f,
		(float)BG->s.imageX/8.0f/(float)BG->s.imageW + 0.5f + scalex*0.5f,
		(float)BG->s.imageY/8.0f/(float)BG->s.imageH + 0.5f + scaley*0.5f,
		(float)BG->s.imageY/8.0f/(float)BG->s.imageH + 0.5f - scaley*0.5f,
*/
		s1,
		s2,
		t2,
		t1,
		BG->s.imageFmt,
		BG->s.imageSiz,
		BG->s.imageW>>2,
		BG->s.imageH>>2);

/*
	float u1, u2, v1, v2;
	
	u1 = 0.0f;
	v1 = 0.0f;
	u2 = 1.0f;//(float)tile.Width/tile.D3DWidth;
	v2 = 1.0f;//(float)tile.Height/tile.D3DHeight;
	
	SetViewport();
	
	SetZBufferWrite();

	D3DTNTVERTEX *VertexBuffer;
	State.ZBufferWrite = false;
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, false));
	D3DXMATRIX Ortho;
	D3DXMatrixOrthoOffCenterRH(&Ortho, 0, (float)State.cimg.width, (float)GetHeight(State.cimg.width), 0, 0.0f, 1.0f);
	SafeDX(g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &Ortho));
	State.ProjectionChanged = true;
	SafeDX(g_VertexBuffer->Lock(0, 4*sizeof(D3DTNTVERTEX), (U8**)&VertexBuffer, 0));
	VertexBuffer[0].color = State.FillColor;
	VertexBuffer[0].x = (float)x1;
	VertexBuffer[0].y = (float)y1;
	VertexBuffer[0].z = 0.0f;
	_TexAssignU((&VertexBuffer[0])) = u1;
	_TexAssignV((&VertexBuffer[0])) = v1;
	
	VertexBuffer[1].color = State.FillColor;
	VertexBuffer[1].x = (float)x2;
	VertexBuffer[1].y = (float)y1;
	VertexBuffer[1].z = 0.0f;
	_TexAssignU((&VertexBuffer[1])) = u2;
	_TexAssignV((&VertexBuffer[1])) = v1;
	
	VertexBuffer[2].color = State.FillColor;
	VertexBuffer[2].x = (float)x1;
	VertexBuffer[2].y = (float)y2;
	VertexBuffer[2].z = 0.0f;
	_TexAssignU((&VertexBuffer[2])) = u1;
	_TexAssignV((&VertexBuffer[2])) = v2;
	
	VertexBuffer[3].color = State.FillColor;
	VertexBuffer[3].x = (float)x2;
	VertexBuffer[3].y = (float)y2;
	VertexBuffer[3].z = 0.0f;
	_TexAssignU((&VertexBuffer[3])) = u2;
	_TexAssignV((&VertexBuffer[3])) = v2;
	SafeDX(g_VertexBuffer->Unlock());

	U32 i;
	for (i=0;i<4;i++){
		Transform2D(&VertexBuffer[i]);
	}
	BlendPasses.Count = 1;
	BlendPasses.Passes[0].Stages[0].Texture = State.Textures[Texture1].D3DObject;
	BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
	BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
	BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
	BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
	BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_DISABLE;
	BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_DISABLE;
	BlendPasses.Passes[0].TFactor = 0x30FF00FF;

	SetBlendPass(0);
	SafeDX(g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
*/
}

void GBI10_BG_COPY()
{
	LogCommand("GBI10_BG_COPY unhandled\n");
}

void GBI10_OBJ_RECTANGLE()
{
	LogCommand("GBI10_OBJ_RECTANGLE unhandled\n");
}

void GBI10_OBJ_SPRITE()
{
	uObjSprite *Sprite;

	Sprite = (uObjSprite*)&_RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]];
	S2DEX_Sprite(Sprite, 0);
	LogCommand("GBI10_OBJ_SPRITE RDRAM: 0x%X\n", Command.movemem.addr + Segment[Command.movemem.segment]);
}

void GBI10_OBJ_MOVEMEM()
{
	switch (Command.movemem.par){
	case 23:
		// Load 2D-Matrix
		LogCommand("GBI10_OBJ_MOVEMEM: Matrix\n");
		uObjMtx *Matrix;
		Matrix = (uObjMtx*)&_RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]];
		State.Matrix2D._11 = Matrix->m.A/65536.0f;
		State.Matrix2D._21 = Matrix->m.B/65536.0f;
		State.Matrix2D._12 = Matrix->m.C/65536.0f;
		State.Matrix2D._22 = Matrix->m.D/65536.0f;

		State.Matrix2D._31 = Matrix->m.X/4.0f;
		State.Matrix2D._32 = Matrix->m.Y/4.0f;
		State.Matrix2D._13 = Matrix->m.BaseScaleX/1024.0f;
		State.Matrix2D._23 = Matrix->m.BaseScaleY/1024.0f;
		State.Matrix2D._33 = 1.0f;
		break;
	case 7:
		// Load 2D-SubMatrix
		LogCommand("GBI10_OBJ_MOVEMEM: SubMatrix\n");
		uObjSubMtx *SubMatrix;
		SubMatrix = (uObjSubMtx*)&_RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]];
		State.Matrix2D._31 = SubMatrix->m.X/4.0f;
		State.Matrix2D._32 = SubMatrix->m.Y/4.0f;
		State.Matrix2D._13 = SubMatrix->m.BaseScaleX/1024.0f;
		State.Matrix2D._23 = SubMatrix->m.BaseScaleY/1024.0f;
		State.Matrix2D._33 = 1.0f;  // maybe 1 ?
		break;
	default:
		LogCommand("GBI10_OBJ_MOVEMEM: Unknown\n");
		break;
	}

}

void GBI10_SELECT_DL()
{
	LogCommand("GBI10_SELECT_DL unhandled\n");
}

void GBI10_OBJ_RENDERMODE()
{
	LogCommand("GBI10_OBJ_RENDERMODE unhandled\n");
}

void GBI10_OBJ_RECTANGLE_R()
{
/*
	uObjTxSprite *Rectangle;

	Sprite = (uObjTxSprite*)&_RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]];
	S2DEX_Texture(&Sprite->txtr);
	S2DEX_Sprite(&Sprite->sprite);
*/
}

void GBI10_OBJ_LOADTXTR()
{
	uObjTxtr *Texture;

	Texture = (uObjTxtr*)&_RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]];
	S2DEX_Texture(Texture);
	LogCommand("GBI10_OBJ_LOADTXTR RDRAM: 0x%X\n", Command.movemem.addr + Segment[Command.movemem.segment]);
}

void GBI10_OBJ_LDTX_SPRITE()
{
	uObjTxSprite *Sprite;

	Sprite = (uObjTxSprite*)&_RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]];
	S2DEX_Texture(&Sprite->txtr);
	S2DEX_Sprite(&Sprite->sprite, 0);
	LogCommand("GBI10_OBJ_LDTX_SPRITE RDRAM: 0x%X\n", Command.movemem.addr + Segment[Command.movemem.segment]);
}

void GBI10_OBJ_LDTX_RECT()
{
	uObjTxSprite *Sprite;

	Sprite = (uObjTxSprite*)&_RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]];
	S2DEX_Texture(&Sprite->txtr);
	S2DEX_Sprite(&Sprite->sprite, 2);
	LogCommand("GBI10_OBJ_LDTX_RECT RDRAM: 0x%X\n", Command.movemem.addr + Segment[Command.movemem.segment]);
}

void GBI10_OBJ_LDTX_RECT_R()
{
	uObjTxSprite *Sprite;

	Sprite = (uObjTxSprite*)&_RDRAM[Command.movemem.addr + Segment[Command.movemem.segment]];
	S2DEX_Texture(&Sprite->txtr);
	S2DEX_Sprite(&Sprite->sprite, 1);
	LogCommand("GBI10_OBJ_LDTX_RECT_R RDRAM: 0x%X\n", Command.movemem.addr + Segment[Command.movemem.segment]);
}

void GBI10_RDPHALF_0()
{
	LogCommand("GBI10_RDPHALF_0 unhandled\n");
}

void GBI11_MTX()
{
	// jet force gemini matrix
	// does not have a real matrix stack - just 4 world matrix slots
	FlushVertexCache();

// AT = BITS(Command.raw.hi, 16, 8)

// laden:     ((AT & 0x38)<<3) / 64
// schreiben: ((AT & 7) << 6) / 64

	U32 AT, LoadIndex, WriteIndex;
	AT = BITS(Command.raw.hi, 16, 8);
	LoadIndex = ((AT & 0x38)<<3) / 64;
	WriteIndex = ((AT & 7) << 6) / 64;
	
	State.WorldIndex = WriteIndex;
	State.WorldMatrix = &State.World[State.WorldIndex];
	LogCommand("GBI11_MTX, LoadIndex: %i, WriteIndex: %i, Mult: %s, JFGAdd: %X\n", LoadIndex, WriteIndex, (Command.raw.hi&0x800000?"yes":"no"), State.JFGMTXAdd);
	if (Command.raw.hi&0x800000){
		// multiply
		D3DXMATRIX Matrix;
		LoadMatrix(State.JFGMTXAdd + Command.mtx_gbi12.addr + Segment[Command.mtx_gbi12.segment], &Matrix);
		*State.WorldMatrix = Matrix*State.World[LoadIndex];
	} else {
		// don't multiply
		LoadMatrix(State.JFGMTXAdd + Command.mtx_gbi12.addr + Segment[Command.mtx_gbi12.segment], &State.World[WriteIndex]);
	}
	State.ProjectionChanged = true;
}

void GBI11_MTXPROJECTION()
{
	// jet force gemini matrix
	// kann nicht multiplizieren. Lädt einfach nur eine Matrix
	// Wahrscheinlich Projection
	LogCommand("GBI11_MTXPROJECTION\n");
	
/*
	FlushVertexCache();

	LoadMatrix(Command.mtx_gbi12.addr + Segment[Command.mtx_gbi12.segment], &State.Projection);
	State.ProjectionChanged = true;
*/
}

void GBI11_SETMTXADD()
{
	// stores some value - on the real thing this is stored in 0x214
	State.JFGMTXAdd = (Command.raw.lo&BitM24);
	LogCommand("GBI11_SETMTXADD to: 0x%X\n", State.JFGMTXAdd);
}



// RogueSquadron

void GBI_ROGUE_MOVEMEM()
{
	U32 addr;
	addr = Stack[uStackIndex].PC+8;
	switch (Command.movemem.par){
	case 0x80: // Viewport
		FlushVertexCache();
		State.ScaleX     = Read32(addr    )>>17;
		State.ScaleY     =(Read32(addr    )&BitM16)>>1;
		State.ScaleZ     = Read32(addr + 4)>>17;
		State.TranslateX = Read32(addr + 8)>>17;
		State.TranslateY =(Read32(addr + 8)&BitM16)>>1;
		State.TranslateZ = Read32(addr + 12)>>17;
		LogCommand("GBI_ROGUE_MOVEMEM: Viewport\n");
		break;
	case 0x82: // LookAtY
		LoadLight(-2, (TLight*)&Global_Gfx_Info.RDRAM[addr]);
		LogCommand("GBI_ROGUE_MOVEMEM: LookAtY\n");
		break;
	case 0x84: // LookAtX
		LoadLight(-1, (TLight*)&Global_Gfx_Info.RDRAM[addr]);
		LogCommand("GBI_ROGUE_MOVEMEM: LookAtX\n");
		break;
	case 0x86: // Light 0
		LoadLight(0, (TLight*)&Global_Gfx_Info.RDRAM[addr]);
		LogCommand("GBI_ROGUE_MOVEMEM: Light 0\n");
		break;
	case 0x88: // Light 1
		LoadLight(1, (TLight*)&Global_Gfx_Info.RDRAM[addr]);
		LogCommand("GBI_ROGUE_MOVEMEM: Light 1\n");
		break;
	case 0x8A: // Light 2
		LoadLight(2, (TLight*)&Global_Gfx_Info.RDRAM[addr]);
		LogCommand("GBI_ROGUE_MOVEMEM: Light 2\n");
		break;
	case 0x8C: // Light 3
		LoadLight(3, (TLight*)&Global_Gfx_Info.RDRAM[addr]);
		LogCommand("GBI_ROGUE_MOVEMEM: Light 3\n");
		break;
	case 0x8E: // Light 4
		LoadLight(4, (TLight*)&Global_Gfx_Info.RDRAM[addr]);
		LogCommand("GBI_ROGUE_MOVEMEM: Light 4\n");
		break;
	case 0x90: // Light 5
		LoadLight(5, (TLight*)&Global_Gfx_Info.RDRAM[addr]);
		LogCommand("GBI_ROGUE_MOVEMEM: Light 5\n");
		break;
	case 0x92: // Light 6
		LoadLight(6, (TLight*)&Global_Gfx_Info.RDRAM[addr]);
		LogCommand("GBI_ROGUE_MOVEMEM: Light 6\n");
		break;
	case 0x94: // Light 7
		LoadLight(7, (TLight*)&Global_Gfx_Info.RDRAM[addr]);
		LogCommand("GBI_ROGUE_MOVEMEM: Light 7\n");
		break;
	default:
		LogCommand("GBI_ROGUE_MOVEMEM: Unhandled subopcode %i\n", Command.movemem.par);
		break;
	}
	Stack[uStackIndex].PC+=16;
}

void GBI_ROGUE_JUMP2()
{
	LogCommand("GBI_ROGUE_JUMP2\n");
	Stack[uStackIndex].PC = Stack[uStackIndex].RogueStartDL;
	Stack[uStackIndex].RogueStartDL = Read32(Stack[uStackIndex].PC) & BitM24;
	Stack[uStackIndex].RogueOtherDL = Read32(Stack[uStackIndex].PC+4) & BitM24;
}

void GBI_ROGUE_TRI2()
{
	LogCommand("GBI_ROGUE_TRI2, extra: %X:%X\n", Read32(Stack[uStackIndex].PC+8), Read32(Stack[uStackIndex].PC+12));
	U32 D1, D2, D3, D4, NextCommand;
	U32 t1, t2, t3, t4;
	NextCommand = Read32(Stack[uStackIndex].PC+8);
	D1 = (Command.raw.lo >> 13 & 0x7F8)/40;
	D2 = (Command.raw.lo >>  5 & 0x7F8)/40;
	D3 = (Command.raw.lo <<  3 & 0x7F8)/40;
	D4 = (Command.raw.lo >> 21 & 0x7F8)/40;
	VertexCache[D1].color = PDColorCache[BITS(NextCommand, 16, 8)>>2];
	VertexCache[D2].color = PDColorCache[BITS(NextCommand,  8, 8)>>2];
	VertexCache[D3].color = PDColorCache[BITS(NextCommand,  0, 8)>>2];
	VertexCache[D4].color = PDColorCache[BITS(NextCommand, 24, 8)>>2];
	
	
	if (Command.raw.hi & 2){
		// does something more here....real rsp code loads some vectors
		t1 = Read32(Stack[uStackIndex].PC+16);
		t2 = Read32(Stack[uStackIndex].PC+20);
		t3 = Read32(Stack[uStackIndex].PC+24);
		t4 = Read32(Stack[uStackIndex].PC+28);
		VertexCache[D1].tex[0].u = _10_5ToFloat(BITS(t1, 16, 16));
		VertexCache[D1].tex[0].v = _10_5ToFloat(BITS(t1,  0, 16));
		VertexCache[D2].tex[0].u = _10_5ToFloat(BITS(t2, 16, 16));
		VertexCache[D2].tex[0].v = _10_5ToFloat(BITS(t2,  0, 16));
		VertexCache[D3].tex[0].u = _10_5ToFloat(BITS(t3, 16, 16));
		VertexCache[D3].tex[0].v = _10_5ToFloat(BITS(t3,  0, 16));
		VertexCache[D4].tex[0].u = _10_5ToFloat(BITS(t4, 16, 16));
		VertexCache[D4].tex[0].v = _10_5ToFloat(BITS(t4,  0, 16));
		
		LogCommand("  texture data: %X, %X, %X, %X\n", t1, t2, t3, t4);
		Stack[uStackIndex].PC += 16;
	}
	Tri1(D1, D2, D3);
	Tri1(D1, D3, D4);
	Stack[uStackIndex].PC += 8;
}

void GBI_ROGUE_TRI1()
{
	LogCommand("GBI_ROGUE_TRI1, extra: %X:%X\n", Read32(Stack[uStackIndex].PC+8), Read32(Stack[uStackIndex].PC+12));
	U32 D1, D2, D3, NextCommand;
	U32 t1, t2, t3;
	NextCommand = Read32(Stack[uStackIndex].PC+8);
	D1 = (Command.raw.lo >> 13 & 0x7F8)/40;
	D2 = (Command.raw.lo >>  5 & 0x7F8)/40;
	D3 = (Command.raw.lo <<  3 & 0x7F8)/40;
	VertexCache[D1].color = PDColorCache[BITS(NextCommand, 16, 8)>>2];
	VertexCache[D2].color = PDColorCache[BITS(NextCommand,  8, 8)>>2];
	VertexCache[D3].color = PDColorCache[BITS(NextCommand,  0, 8)>>2];
	
	
	if (Command.raw.hi & 2){
		// does something more here....real rsp code loads some vectors
		t1 = Read32(Stack[uStackIndex].PC+16);
		t2 = Read32(Stack[uStackIndex].PC+20);
		t3 = Read32(Stack[uStackIndex].PC+24);
		VertexCache[D1].tex[0].u = _10_5ToFloat(BITS(t1, 16, 16));
		VertexCache[D1].tex[0].v = _10_5ToFloat(BITS(t1,  0, 16));
		VertexCache[D2].tex[0].u = _10_5ToFloat(BITS(t2, 16, 16));
		VertexCache[D2].tex[0].v = _10_5ToFloat(BITS(t2,  0, 16));
		VertexCache[D3].tex[0].u = _10_5ToFloat(BITS(t3, 16, 16));
		VertexCache[D3].tex[0].v = _10_5ToFloat(BITS(t3,  0, 16));
		
		LogCommand("  texture data: %X, %X, %X\n", t1, t2, t3);
		Stack[uStackIndex].PC += 16;
	}
	Tri1(D1, D2, D3);
	Stack[uStackIndex].PC += 8;
}

void GBI_ROGUE_VTXCOLOR()
{
	LogCommand("GBI_ROGUE_VTXCOLOR\n");
	U32 uCrap0, c;
	uCrap0 = BITS(Command.raw.hi, 0, 16)>>2;
	union {
		struct {
			unsigned alpha: 8; /* alpha */
			unsigned c3   : 8; /* color */
			unsigned c2   : 8; /* color */
			unsigned c1   : 8; /* color */
		} color;
		U32 raw;
	} col;

	for (c=0;c<uCrap0;c++){
		col.raw = Read32(Command.dl.addr + (c<<2));
		PDColorCache[c] = (col.color.alpha<<24) | (col.color.c1<<16) | (col.color.c2<<8) | (col.color.c3);
	}

}

void ROGUE_FD4()
{
	// VADDC v1, v1, v0
	// SDV v1[0], 0x0000 (V1)
}

void GBI_ROGUE_JUMP3()
{
	LogCommand("GBI_ROGUE_JUMP3\n");
	// seems to be similar to HEIGHTFIELD, but calls actual function with A1=0xC

#if this_is_the_real_thing
	U32 A0, V0;
	U32 S1;  // current pc

	S1 = Stack[uStackIndex].PC + 8;

	A0 = (U16)Read16(S1 + 8); // 8 correct? first execution in the state should returns 0xFFF8

	if (Command.raw.hi & 0x200){  // at 0xDB0, jumps to 0xF64
		V0 = (U16)Read16(S1 + 0x1E);  // returns 0x200 at first execution
		V1 = (S16)Read16(S1 + 0x1A)<<4;  // returns 0 at first execution

		// LSV v0[0], 0x018 (S1)
		// LSV v0[4], 0x01C (S1)

		// MTC2 V1, v0[2]
		// VSUBC v2, v2, v2      // zero out
		// MTC2 V0, v2[0]
		// VSUBC v3, v3, v3      // zero out
		// MTC2 V0, v3[4]
		// VSUBC v1, v1, v1      // zero out
		
		
		// LSV v1[2], 0x0FC (S1)

		// now:
		// V0 = 00000200; V1 = 00000000
		// $v0 = 3000 0000 1200 0000 - 7E00 7E00 7E00 7E00
		// $v1 = 0000 0000 0000 0000 - 0000 0000 0000 0000
		// $v2 = 0200 0000 0000 0000 - 0000 0000 0000 0000
		// $v3 = 0000 0000 0200 0000 - 0000 0000 0000 0000

		V1 = 0x280;
		ROGUE_FD4();

		// VADDC v1, v2, v30[0]; // v30[0] = 0x0000
		// LSV v1[2], 0x0FE (S1)
		V1 = 0x288;
		ROGUE_FD4();

		// VADDC v1, v3, v30[0]; // v30[0] = 0x0000
		// LSV v1[2], 0x000 (S1)
		V1 = 0x290;
		ROGUE_FD4();

		// VADDC v1, v2, v3;
		// LSV v1[2], 0x002 (S1)
		V1 = 0x298;
		ROGUE_FD4();

		A1 = 0x24;
		// J DMA Code (0x268)
	}
#endif
	Stack[uStackIndex].PC = Stack[uStackIndex].RogueOtherDL;
	Stack[uStackIndex].RogueStartDL = Read32(Stack[uStackIndex].PC) & BitM24;
	Stack[uStackIndex].RogueOtherDL = Read32(Stack[uStackIndex].PC+4) & BitM24;
}

void GBI_ROGUE_HEIGHTFIELD()
{
	// seems to be similar to JUMP3, but calls actual function with A1=0x2C
	// it *might* need the same jump/branch code as JUMP3
	LogCommand("GBI_ROGUE_HEIGHTFIELD\n");
	Stack[uStackIndex].PC += 16;
}

void GBI_ROGUE_SETOTHERMODEHI_EX()
{
	LogCommand("GBI_ROGUE_SETOTHERMODEHI_EX\n");
	State.Othermode.raw.hi &= Read32(Stack[uStackIndex].PC+8);
	State.Othermode.raw.hi |= Command.raw.lo;
	Stack[uStackIndex].PC += 8;
}

void GBI_ROGUE_VTX()
{
	LogCommand("GBI_ROGUE_VTX");
	LoadVertices_Rogue(Command.vtx_gbi1.addr + Segment[Command.vtx_gbi1.segment],
		(Command.raw.hi>>0xA)&0x3F, 0);
}

void GBI_S2DEX2_UNKNOWN1()
{
	// reversed in 11th display list of Evangelion
	// RSP position: 0xA40010E8
	
	// command does the following:
	// Write8 (0x268, Command.raw.lo & 0xFF)
	// Write16(0x244, 0x248 + (Command.raw.lo & 0x08))
	// Write16(0x246, 0x258 + (Command.raw.hi & 0x08))
	// v1 = Read32(0x234 + ((Command.raw.lo & 0x18)>>1))
	// v0 = Read32(0x214 + ((Command.raw.lo & 0x70)>>2))
	// Write32(0x1FC, v1)
	// Write32(0x1F8, v0)
}

// Microcode "opcodes" finished, general code follows

void DetectMicroCode(U32 uIndex)
{
	U32 i;
	// Fill General functions...
	for (i=0;i<256;i++) {
		MicroCodeFunctions[i] = GBI_UNKNOWN;
	}

	if (uIndex==0xFFFFFFFF){
		// Detect Microcode here
		uMicroCodeID = MicroCodeDetect(Global_Gfx_Info.RDRAM, Read32SPDMEM(0xFC0 + 16));
	} else {
		uMicroCodeID = uIndex;
	}

	if (0&&UCode_NoN){
		SafeDX(g_pd3dDevice->SetRenderState(D3DRS_CLIPPING, FALSE));
	} else {
		SafeDX(g_pd3dDevice->SetRenderState(D3DRS_CLIPPING, TRUE));
	}
	char szMsg[100];
	if (uMicroCodeID==0){
		sprintf(szMsg, UCodeNames[uMicroCodeID], UCodeCRC);
//		MessageBox(Global_Gfx_Info.hStatusBar, szMsg, "Unable to detect ucode", MB_OK|MB_ICONERROR);
		SendMessage(Global_Gfx_Info.hStatusBar, SB_SETTEXT, 0, (LPARAM)szMsg);
		return;
	} else {
		sprintf(szMsg, UCodeNames[uMicroCodeID]);
		if (bFirstDisplayList){
			SendMessage(Global_Gfx_Info.hStatusBar, SB_SETTEXT, 0, (LPARAM)szMsg);
		}
		LogCommand("Changed UCode: %s\n", szMsg);
	}



	MicroCodeFunctions[0xE4] = GBI_TEXRECT;
	MicroCodeFunctions[0xE5] = GBI_TEXRECTFLIP;
	MicroCodeFunctions[0xE6] = GBI_RDPLOADSYNC;
	MicroCodeFunctions[0xE7] = GBI_RDPPIPESYNC;
	MicroCodeFunctions[0xE8] = GBI_RDPTILESYNC;
	MicroCodeFunctions[0xE9] = GBI_RDPFULLSYNC;
	MicroCodeFunctions[0xEA] = GBI_SETKEYGB;
	MicroCodeFunctions[0xEB] = GBI_SETKEYR;
	MicroCodeFunctions[0xEC] = GBI_SETCONVERT;
	MicroCodeFunctions[0xED] = GBI_SETSCISSOR;
	MicroCodeFunctions[0xEE] = GBI_SETPRIMDEPTH;
	MicroCodeFunctions[0xEF] = GBI_RDPSETOTHERMODE;
	MicroCodeFunctions[0xF0] = GBI_LOADTLUT;
	
	MicroCodeFunctions[0xF2] = GBI_SETTILESIZE;
	MicroCodeFunctions[0xF3] = GBI_LOADBLOCK;
	MicroCodeFunctions[0xF4] = GBI_LOADTILE;
	MicroCodeFunctions[0xF5] = GBI_SETTILE;
	MicroCodeFunctions[0xF6] = GBI_FILLRECT;
	MicroCodeFunctions[0xF7] = GBI_SETFILLCOLOR;
	MicroCodeFunctions[0xF8] = GBI_SETFOGCOLOR;
	MicroCodeFunctions[0xF9] = GBI_SETBLENDCOLOR;
	MicroCodeFunctions[0xFA] = GBI_SETPRIMCOLOR;
	MicroCodeFunctions[0xFB] = GBI_SETENVCOLOR;
	MicroCodeFunctions[0xFC] = GBI_SETCOMBINE;
	MicroCodeFunctions[0xFD] = GBI_SETTIMG;
	MicroCodeFunctions[0xFE] = GBI_SETZIMG;
	MicroCodeFunctions[0xFF] = GBI_SETCIMG;

	switch (uMicroCodeID) {
	case 1:
		MicroCodeFunctions[0x00] = GBI12_SPNOOP;
		MicroCodeFunctions[0x01] = GBI12_MTX;
		MicroCodeFunctions[0x02] = GBI12_RESERVED0;
		MicroCodeFunctions[0x03] = GBI12_MOVEMEM;
		MicroCodeFunctions[0x04] = GBI1_VTX;
		MicroCodeFunctions[0x05] = GBI12_RESERVED1;
		MicroCodeFunctions[0x06] = GBI12_DL;
		MicroCodeFunctions[0x07] = GBI12_RESERVED2;
		MicroCodeFunctions[0x08] = GBI12_RESERVED3;
		MicroCodeFunctions[0x09] = GBI12_SPRITE2D_BASE;

		MicroCodeFunctions[0xB1] = GBI1_TRI4;
		MicroCodeFunctions[0xB2] = GBI1_RDPHALFCONT;
		MicroCodeFunctions[0xB3] = GBI12_RDPHALF2;
		MicroCodeFunctions[0xB4] = GBI12_RDPHALF1;
		MicroCodeFunctions[0xB5] = GBI1_QUAD;
		MicroCodeFunctions[0xB6] = GBI12_CLEARGEOMMODE;
		MicroCodeFunctions[0xB7] = GBI12_SETGEOMMODE;
		MicroCodeFunctions[0xB8] = GBI12_ENDDL;
		MicroCodeFunctions[0xB9] = GBI12_SETOTHERMODELO;
		MicroCodeFunctions[0xBA] = GBI12_SETOTHERMODEHI;
		MicroCodeFunctions[0xBB] = GBI12_TEXTURE;
		MicroCodeFunctions[0xBC] = GBI12_MOVEWORD;
		MicroCodeFunctions[0xBD] = GBI12_POPMTX;
		MicroCodeFunctions[0xBE] = GBI12_CULLDL;
		MicroCodeFunctions[0xBF] = GBI1_TRI1;

		MicroCodeFunctions[0xC0] = GBI12_NOOP;
		break; 
	case _microcode_f3dex:
	case _microcode_waveraceus:
		MicroCodeFunctions[0x00] = GBI12_SPNOOP;
		MicroCodeFunctions[0x01] = GBI12_MTX;
		MicroCodeFunctions[0x02] = GBI12_RESERVED0;
		MicroCodeFunctions[0x03] = GBI12_MOVEMEM;
		if (uMicroCodeID==_microcode_waveraceus){
			MicroCodeFunctions[0x04] = GBI4_VTX;
		} else {
			MicroCodeFunctions[0x04] = GBI2_VTX;
		}
		MicroCodeFunctions[0x05] = GBI12_RESERVED1;
		MicroCodeFunctions[0x06] = GBI12_DL;
		MicroCodeFunctions[0x07] = GBI12_RESERVED2;
		MicroCodeFunctions[0x08] = GBI12_RESERVED3;
		MicroCodeFunctions[0x09] = GBI12_SPRITE2D_BASE;

		MicroCodeFunctions[0xAF] = GBI2_LOADUCODE;
		MicroCodeFunctions[0xB0] = GBI2_BRANCHZ;
		
		MicroCodeFunctions[0xB1] = GBI2_TRI2;
		MicroCodeFunctions[0xB2] = GBI23_MODIFYVTX;
		MicroCodeFunctions[0xB3] = GBI12_RDPHALF2;
		MicroCodeFunctions[0xB4] = GBI12_RDPHALF1;
		if (uMicroCodeID==_microcode_waveraceus){
			MicroCodeFunctions[0xB5] = GBI4_QUAD;
		} else {
			MicroCodeFunctions[0xB5] = GBI2_QUAD;
		}
		MicroCodeFunctions[0xB6] = GBI12_CLEARGEOMMODE;
		MicroCodeFunctions[0xB7] = GBI12_SETGEOMMODE;
		MicroCodeFunctions[0xB8] = GBI12_ENDDL;
		MicroCodeFunctions[0xB9] = GBI12_SETOTHERMODELO;
		MicroCodeFunctions[0xBA] = GBI12_SETOTHERMODEHI;
		MicroCodeFunctions[0xBB] = GBI12_TEXTURE;
		MicroCodeFunctions[0xBC] = GBI12_MOVEWORD;
		MicroCodeFunctions[0xBD] = GBI12_POPMTX;
		MicroCodeFunctions[0xBE] = GBI12_CULLDL;
		if (uMicroCodeID==_microcode_waveraceus){
			MicroCodeFunctions[0xBF] = GBI4_TRI1;
		} else {
			MicroCodeFunctions[0xBF] = GBI2_TRI1;
		}
		MicroCodeFunctions[0xC0] = GBI12_NOOP;
		break;
	case _microcode_f3dex2:
	case _microcode_conker:
		MicroCodeFunctions[0x00] = GBI3_NOOP;
		MicroCodeFunctions[0x01] = GBI3_VTX;
		MicroCodeFunctions[0x02] = GBI23_MODIFYVTX;
		MicroCodeFunctions[0x03] = GBI3_CULLDL;
		MicroCodeFunctions[0x04] = GBI3_BRANCHZ;
		MicroCodeFunctions[0x05] = GBI3_TRI1;
		MicroCodeFunctions[0x06] = GBI3_TRI2;
		MicroCodeFunctions[0x07] = GBI3_QUAD;
		MicroCodeFunctions[0x08] = GBI3_LINE3D;

		MicroCodeFunctions[0x0A] = GBI3_BACKGROUND;

		MicroCodeFunctions[0x10] = GBI3_TRI4;
		MicroCodeFunctions[0x11] = GBI3_TRI4;
		MicroCodeFunctions[0x12] = GBI3_TRI4;
		MicroCodeFunctions[0x13] = GBI3_TRI4;
		MicroCodeFunctions[0x14] = GBI3_TRI4;
		MicroCodeFunctions[0x15] = GBI3_TRI4;
		MicroCodeFunctions[0x16] = GBI3_TRI4;
		MicroCodeFunctions[0x17] = GBI3_TRI4;
		MicroCodeFunctions[0x18] = GBI3_TRI4;
		MicroCodeFunctions[0x19] = GBI3_TRI4;
		MicroCodeFunctions[0x1A] = GBI3_TRI4;
		MicroCodeFunctions[0x1B] = GBI3_TRI4;
		MicroCodeFunctions[0x1C] = GBI3_TRI4;
		MicroCodeFunctions[0x1D] = GBI3_TRI4;
		MicroCodeFunctions[0x1E] = GBI3_TRI4;
		MicroCodeFunctions[0x1F] = GBI3_TRI4;

		MicroCodeFunctions[0xD3] = GBI3_SPECIAL3;
		MicroCodeFunctions[0xD4] = GBI3_SPECIAL2;
		MicroCodeFunctions[0xD5] = GBI3_SPECIAL1;
		MicroCodeFunctions[0xD6] = GBI3_DMAIO;
		MicroCodeFunctions[0xD7] = GBI3_TEXTURE;
		MicroCodeFunctions[0xD8] = GBI3_POPMATRIX;
		MicroCodeFunctions[0xD9] = GBI3_GEOMETRYMODE;
		MicroCodeFunctions[0xDA] = GBI3_MTX;
		MicroCodeFunctions[0xDB] = GBI3_MOVEWORD;
		MicroCodeFunctions[0xDC] = GBI3_MOVEMEM;
		MicroCodeFunctions[0xDD] = GBI3_LOADUCODE;
		MicroCodeFunctions[0xDE] = GBI3_DL;
		MicroCodeFunctions[0xDF] = GBI3_ENDDL;
		MicroCodeFunctions[0xE0] = GBI3_SPNOOP;
		MicroCodeFunctions[0xE1] = GBI3_RDPHALF1;
		MicroCodeFunctions[0xE2] = GBI3_SETOTHERMODELO;
		MicroCodeFunctions[0xE3] = GBI3_SETOTHERMODEHI;
		MicroCodeFunctions[0xF1] = GBI3_RDPHALF2;
		break;
	case _microcode_dkr: // Diddy Kong Racing
		MicroCodeFunctions[0x00] = GBI12_SPNOOP;
		MicroCodeFunctions[0x01] = GBI5_MTX;
		MicroCodeFunctions[0x02] = GBI12_RESERVED0;
		MicroCodeFunctions[0x03] = GBI12_MOVEMEM;
		MicroCodeFunctions[0x04] = GBI5_VTX;
		MicroCodeFunctions[0x05] = GBI5_TRI;
		MicroCodeFunctions[0x06] = GBI12_DL;
		MicroCodeFunctions[0x07] = GBI5_RAWRDP;

		MicroCodeFunctions[0xAF] = GBI2_LOADUCODE;
		MicroCodeFunctions[0xB0] = GBI2_BRANCHZ;
		MicroCodeFunctions[0xB1] = GBI2_TRI2;
		MicroCodeFunctions[0xB2] = GBI23_MODIFYVTX;
		MicroCodeFunctions[0xB3] = GBI12_RDPHALF2;
		MicroCodeFunctions[0xB4] = GBI5_DIDDY2;
		MicroCodeFunctions[0xB5] = GBI2_QUAD;
		MicroCodeFunctions[0xB6] = GBI12_CLEARGEOMMODE;
		MicroCodeFunctions[0xB7] = GBI12_SETGEOMMODE;
		MicroCodeFunctions[0xB8] = GBI12_ENDDL;
		MicroCodeFunctions[0xB9] = GBI12_SETOTHERMODELO;
		MicroCodeFunctions[0xBA] = GBI12_SETOTHERMODEHI;
		MicroCodeFunctions[0xBB] = GBI12_TEXTURE;
		MicroCodeFunctions[0xBC] = GBI5_MOVEWORD;
		MicroCodeFunctions[0xBD] = GBI12_POPMTX;
		MicroCodeFunctions[0xBE] = GBI12_CULLDL;
		break;
	case _microcode_jfg:
		// Jet Force Gemini, Moveword ist vielleicht auch modifiziert ... ? VTX scheint einen Startindex zu haben !
		MicroCodeFunctions[0x00] = GBI12_SPNOOP;
		MicroCodeFunctions[0x01] = GBI11_MTX;
		MicroCodeFunctions[0x02] = GBI11_MTXPROJECTION;
		MicroCodeFunctions[0x03] = GBI12_MOVEMEM;
		MicroCodeFunctions[0x04] = GBI5_VTX;
		MicroCodeFunctions[0x05] = GBI5_TRI;
		MicroCodeFunctions[0x06] = GBI12_DL;
		MicroCodeFunctions[0x07] = GBI5_RAWRDP;

		MicroCodeFunctions[0xAF] = GBI2_LOADUCODE;
		MicroCodeFunctions[0xB0] = GBI2_BRANCHZ;
		MicroCodeFunctions[0xB3] = GBI12_RDPHALF2;
		MicroCodeFunctions[0xB4] = GBI5_DIDDY2;
		MicroCodeFunctions[0xB6] = GBI12_CLEARGEOMMODE;
		MicroCodeFunctions[0xB7] = GBI12_SETGEOMMODE;
		MicroCodeFunctions[0xB8] = GBI12_ENDDL;
		MicroCodeFunctions[0xB9] = GBI12_SETOTHERMODELO;
		MicroCodeFunctions[0xBA] = GBI12_SETOTHERMODEHI;
		MicroCodeFunctions[0xBB] = GBI12_TEXTURE;
		MicroCodeFunctions[0xBC] = GBI5_MOVEWORD;
		MicroCodeFunctions[0xBE] = GBI12_CULLDL;
		MicroCodeFunctions[0xBF] = GBI11_SETMTXADD;
		break;
	case _microcode_pd:
		MicroCodeFunctions[0x00] = GBI12_SPNOOP;
		MicroCodeFunctions[0x01] = GBI12_MTX;
		MicroCodeFunctions[0x02] = GBI12_RESERVED0;
		MicroCodeFunctions[0x03] = GBI12_MOVEMEM;
		MicroCodeFunctions[0x04] = GBI7_VTX;
		MicroCodeFunctions[0x06] = GBI12_DL;
		MicroCodeFunctions[0x07] = GBI7_VTXCOLOR;
		MicroCodeFunctions[0x09] = GBI12_SPRITE2D_BASE;

		MicroCodeFunctions[0xAF] = GBI2_LOADUCODE;
		MicroCodeFunctions[0xB0] = GBI2_BRANCHZ;
		MicroCodeFunctions[0xB1] = GBI7_TRI4;
		MicroCodeFunctions[0xB2] = GBI23_MODIFYVTX;
		MicroCodeFunctions[0xB3] = GBI12_RDPHALF2;
		MicroCodeFunctions[0xB4] = GBI12_RDPHALF1;
		MicroCodeFunctions[0xB6] = GBI12_CLEARGEOMMODE;
		MicroCodeFunctions[0xB7] = GBI12_SETGEOMMODE;
		MicroCodeFunctions[0xB8] = GBI12_ENDDL;
		MicroCodeFunctions[0xB9] = GBI12_SETOTHERMODELO;
		MicroCodeFunctions[0xBA] = GBI12_SETOTHERMODEHI;
		MicroCodeFunctions[0xBB] = GBI12_TEXTURE;
		MicroCodeFunctions[0xBC] = GBI12_MOVEWORD;
		MicroCodeFunctions[0xBD] = GBI12_POPMTX;
		MicroCodeFunctions[0xBE] = GBI12_CULLDL;
		break;
	case _microcode_rogue:
		MicroCodeFunctions[0x00] = GBI12_SPNOOP;
		MicroCodeFunctions[0x01] = GBI12_MTX;
		MicroCodeFunctions[0x02] = GBI_ROGUE_VTXCOLOR; 
		MicroCodeFunctions[0x03] = GBI_ROGUE_MOVEMEM;
		MicroCodeFunctions[0x04] = GBI_ROGUE_VTX;
		MicroCodeFunctions[0x05] = GBI_ROGUE_JUMP3;
		MicroCodeFunctions[0x06] = GBI9_DL;
		MicroCodeFunctions[0x07] = GBI9_BRANCHDL;

		MicroCodeFunctions[0xB4] = GBI_ROGUE_TRI2;
		MicroCodeFunctions[0xB5] = GBI_ROGUE_JUMP2;
		MicroCodeFunctions[0xB6] = GBI12_CLEARGEOMMODE;
		MicroCodeFunctions[0xB7] = GBI12_SETGEOMMODE;
		MicroCodeFunctions[0xB8] = GBI12_ENDDL;
		MicroCodeFunctions[0xB9] = GBI12_SETOTHERMODELO;
		MicroCodeFunctions[0xBA] = GBI12_SETOTHERMODEHI;
		MicroCodeFunctions[0xBB] = GBI12_TEXTURE;
		MicroCodeFunctions[0xBC] = GBI9_MOVEWORD;
		MicroCodeFunctions[0xBD] = GBI_ROGUE_HEIGHTFIELD;
		MicroCodeFunctions[0xBE] = GBI_ROGUE_SETOTHERMODEHI_EX;
		MicroCodeFunctions[0xBF] = GBI_ROGUE_TRI1;

		MicroCodeFunctions[0xC0] = GBI12_NOOP;
		break;
	case _microcode_naboo:
		MicroCodeFunctions[0x00] = GBI12_SPNOOP;
		MicroCodeFunctions[0x01] = GBI12_MTX;
		MicroCodeFunctions[0x02] = GBI12_RESERVED0;
		MicroCodeFunctions[0x03] = GBI12_MOVEMEM;
		MicroCodeFunctions[0x04] = GBI1_VTX;
		MicroCodeFunctions[0x05] = GBI12_RESERVED1;
		MicroCodeFunctions[0x06] = GBI9_DL;
		MicroCodeFunctions[0x07] = GBI12_RESERVED2;
		MicroCodeFunctions[0x08] = GBI12_RESERVED3;
		MicroCodeFunctions[0x09] = GBI12_SPRITE2D_BASE;

		MicroCodeFunctions[0xB1] = GBI1_TRI4;
		MicroCodeFunctions[0xB2] = GBI1_RDPHALFCONT;
		MicroCodeFunctions[0xB3] = GBI12_RDPHALF2;
		MicroCodeFunctions[0xB4] = GBI12_RDPHALF1;
		MicroCodeFunctions[0xB5] = GBI1_QUAD;
		MicroCodeFunctions[0xB6] = GBI12_CLEARGEOMMODE;
		MicroCodeFunctions[0xB7] = GBI12_SETGEOMMODE;
		MicroCodeFunctions[0xB8] = GBI12_ENDDL;
		MicroCodeFunctions[0xB9] = GBI12_SETOTHERMODELO;
		MicroCodeFunctions[0xBA] = GBI12_SETOTHERMODEHI;
		MicroCodeFunctions[0xBB] = GBI12_TEXTURE;
		MicroCodeFunctions[0xBC] = GBI9_MOVEWORD;
		MicroCodeFunctions[0xBD] = GBI12_POPMTX;
		MicroCodeFunctions[0xBE] = GBI12_CULLDL;
		MicroCodeFunctions[0xBF] = GBI1_TRI1;

		MicroCodeFunctions[0xC0] = GBI12_NOOP;
		break; 
	case _microcode_s2dex:
		MicroCodeFunctions[0x00] = GBI12_SPNOOP;
		MicroCodeFunctions[0x01] = GBI10_BG_1CYC;
		MicroCodeFunctions[0x02] = GBI10_BG_COPY;
		MicroCodeFunctions[0x03] = GBI10_OBJ_RECTANGLE;
		MicroCodeFunctions[0x04] = GBI10_OBJ_SPRITE;
		MicroCodeFunctions[0x05] = GBI10_OBJ_MOVEMEM;
		MicroCodeFunctions[0x06] = GBI12_DL;
		MicroCodeFunctions[0x07] = GBI12_RESERVED2;
		MicroCodeFunctions[0x08] = GBI12_RESERVED3;
		MicroCodeFunctions[0x09] = GBI10_OBJ_SPRITE;

		MicroCodeFunctions[0xAF] = GBI2_LOADUCODE;
		MicroCodeFunctions[0xB0] = GBI10_SELECT_DL;
		MicroCodeFunctions[0xB1] = GBI10_OBJ_RENDERMODE;
		MicroCodeFunctions[0xB2] = GBI10_OBJ_RECTANGLE_R;
		MicroCodeFunctions[0xB4] = GBI12_RDPHALF1;
		MicroCodeFunctions[0xB8] = GBI12_ENDDL;
		MicroCodeFunctions[0xB9] = GBI12_SETOTHERMODELO;
		MicroCodeFunctions[0xBA] = GBI12_SETOTHERMODEHI;
		MicroCodeFunctions[0xBB] = GBI12_TEXTURE;
		MicroCodeFunctions[0xBC] = GBI12_MOVEWORD;
		MicroCodeFunctions[0xC0] = GBI12_NOOP;
		MicroCodeFunctions[0xC1] = GBI10_OBJ_LOADTXTR;
		MicroCodeFunctions[0xC2] = GBI10_OBJ_LDTX_SPRITE;
		MicroCodeFunctions[0xC3] = GBI10_OBJ_LDTX_RECT;
		MicroCodeFunctions[0xC4] = GBI10_OBJ_LDTX_RECT_R;
		break;
	case _microcode_s2dex2:
		MicroCodeFunctions[0x00] = GBI3_NOOP;
		MicroCodeFunctions[0x01] = GBI3_VTX;
		MicroCodeFunctions[0x02] = GBI23_MODIFYVTX;
		MicroCodeFunctions[0x03] = GBI3_CULLDL;
		MicroCodeFunctions[0x04] = GBI3_BRANCHZ;
		MicroCodeFunctions[0x05] = GBI3_TRI1;
		MicroCodeFunctions[0x06] = GBI3_TRI2;
		MicroCodeFunctions[0x07] = GBI3_QUAD;
		MicroCodeFunctions[0x08] = GBI3_LINE3D;
		MicroCodeFunctions[0x09] = GBI10_OBJ_SPRITE;

		MicroCodeFunctions[0x0A] = GBI3_BACKGROUND;
		MicroCodeFunctions[0x0B] = GBI_S2DEX2_UNKNOWN1;

		MicroCodeFunctions[0x10] = GBI3_TRI4;
		MicroCodeFunctions[0x11] = GBI3_TRI4;
		MicroCodeFunctions[0x12] = GBI3_TRI4;
		MicroCodeFunctions[0x13] = GBI3_TRI4;
		MicroCodeFunctions[0x14] = GBI3_TRI4;
		MicroCodeFunctions[0x15] = GBI3_TRI4;
		MicroCodeFunctions[0x16] = GBI3_TRI4;
		MicroCodeFunctions[0x17] = GBI3_TRI4;
		MicroCodeFunctions[0x18] = GBI3_TRI4;
		MicroCodeFunctions[0x19] = GBI3_TRI4;
		MicroCodeFunctions[0x1A] = GBI3_TRI4;
		MicroCodeFunctions[0x1B] = GBI3_TRI4;
		MicroCodeFunctions[0x1C] = GBI3_TRI4;
		MicroCodeFunctions[0x1D] = GBI3_TRI4;
		MicroCodeFunctions[0x1E] = GBI3_TRI4;
		MicroCodeFunctions[0x1F] = GBI3_TRI4;

		MicroCodeFunctions[0xD3] = GBI3_SPECIAL3;
		MicroCodeFunctions[0xD4] = GBI3_SPECIAL2;
		MicroCodeFunctions[0xD5] = GBI3_SPECIAL1;
		MicroCodeFunctions[0xD6] = GBI3_DMAIO;
		MicroCodeFunctions[0xD7] = GBI3_TEXTURE;
		MicroCodeFunctions[0xD8] = GBI3_POPMATRIX;
		MicroCodeFunctions[0xD9] = GBI3_GEOMETRYMODE;
		MicroCodeFunctions[0xDA] = GBI3_MTX;
		MicroCodeFunctions[0xDB] = GBI3_MOVEWORD;
		MicroCodeFunctions[0xDC] = GBI3_MOVEMEM;
		MicroCodeFunctions[0xDD] = GBI3_LOADUCODE;
		MicroCodeFunctions[0xDE] = GBI3_DL;
		MicroCodeFunctions[0xDF] = GBI3_ENDDL;
		MicroCodeFunctions[0xE0] = GBI3_SPNOOP;
		MicroCodeFunctions[0xE1] = GBI3_RDPHALF1;
		MicroCodeFunctions[0xE2] = GBI3_SETOTHERMODELO;
		MicroCodeFunctions[0xE3] = GBI3_SETOTHERMODEHI;
		MicroCodeFunctions[0xF1] = GBI3_RDPHALF2;
		break;
	}
	// Set RDP Hardware commands
	for (i=0;i<64;i++) {
		RDPFunctions[i] = GBI_UNKNOWN;
	}
	RDPFunctions[0x26] = GBI_RDPLOADSYNC;
	RDPFunctions[0x27] = GBI_RDPPIPESYNC;
	RDPFunctions[0x28] = GBI_RDPTILESYNC;
	RDPFunctions[0x29] = GBI_RDPFULLSYNC;
	RDPFunctions[0x2A] = GBI_SETKEYGB;
	RDPFunctions[0x2B] = GBI_SETKEYR;
	RDPFunctions[0x2C] = GBI_SETCONVERT;
	RDPFunctions[0x2D] = GBI_SETSCISSOR;
	RDPFunctions[0x2E] = GBI_SETPRIMDEPTH;
	RDPFunctions[0x2F] = GBI_RDPSETOTHERMODE;
	RDPFunctions[0x30] = GBI_LOADTLUT;
	
	RDPFunctions[0x32] = GBI_SETTILESIZE;
	RDPFunctions[0x33] = GBI_LOADBLOCK;
	RDPFunctions[0x34] = GBI_LOADTILE;
	RDPFunctions[0x35] = GBI_SETTILE;
	RDPFunctions[0x36] = GBI_FILLRECT;
	RDPFunctions[0x37] = GBI_SETFILLCOLOR;
	RDPFunctions[0x38] = GBI_SETFOGCOLOR;
	RDPFunctions[0x39] = GBI_SETBLENDCOLOR;
	RDPFunctions[0x3A] = GBI_SETPRIMCOLOR;
	RDPFunctions[0x3B] = GBI_SETENVCOLOR;
	RDPFunctions[0x3C] = GBI_SETCOMBINE;
	RDPFunctions[0x3D] = GBI_SETTIMG;
	RDPFunctions[0x3E] = GBI_SETZIMG;
	RDPFunctions[0x3F] = GBI_SETCIMG;
}


int ProcessException(LPEXCEPTION_POINTERS lpEP)
{
	switch(lpEP->ExceptionRecord->ExceptionCode){
	case EXCEPTION_ACCESS_VIOLATION:
		strcpy(msExcError, "EXCEPTION_ACCESS_VIOLATION");
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		strcpy(msExcError, "EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
		break;
	case EXCEPTION_BREAKPOINT:
		strcpy(msExcError, "EXCEPTION_BREAKPOINT");
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		strcpy(msExcError, "EXCEPTION_DATATYPE_MISALIGNMENT");
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		strcpy(msExcError, "EXCEPTION_FLT_DENORMAL_OPERAND");
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		strcpy(msExcError, "EXCEPTION_FLT_DIVIDE_BY_ZERO");
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		strcpy(msExcError, "EXCEPTION_FLT_INEXACT_RESULT");
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		strcpy(msExcError, "EXCEPTION_FLT_INVALID_OPERATION");
		break;
	case EXCEPTION_FLT_OVERFLOW:
		strcpy(msExcError, "EXCEPTION_FLT_OVERFLOW");
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		strcpy(msExcError, "EXCEPTION_FLT_STACK_CHECK");
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		strcpy(msExcError, "EXCEPTION_FLT_UNDERFLOW");
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		strcpy(msExcError, "EXCEPTION_ILLEGAL_INSTRUCTION");
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		strcpy(msExcError, "EXCEPTION_IN_PAGE_ERROR");
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		strcpy(msExcError, "EXCEPTION_INT_DIVIDE_BY_ZERO");
		break;
	case EXCEPTION_INT_OVERFLOW:
		strcpy(msExcError, "EXCEPTION_INT_OVERFLOW");
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		strcpy(msExcError, "EXCEPTION_INVALID_DISPOSITION");
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		strcpy(msExcError, "EXCEPTION_NONCONTINUABLE_EXCEPTION");
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		strcpy(msExcError, "EXCEPTION_PRIV_INSTRUCTION");
		break;
	case EXCEPTION_SINGLE_STEP:
		strcpy(msExcError, "EXCEPTION_SINGLE_STEP");
		break;
	case EXCEPTION_STACK_OVERFLOW:
		strcpy(msExcError, "EXCEPTION_STACK_OVERFLOW");
		break;
	default:
		strcpy(msExcError, "Unknown Exception");
		break;
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

void ProcessDisplayList()
{
#if _DEBUG
	ProfileTimeDListCount++;
	Stats.DLs++;
#endif

	if (bFirstDisplayList){
		DetectMicroCode(0xFFFFFFFF);
		bFirstDisplayList = false;
		uBaseMicroCodeID = uMicroCodeID;
	} else {
		// if a custom ucode was loaded (for example via LoadUCode) restore the
		// original one here
		if (uBaseMicroCodeID != uMicroCodeID){
			DetectMicroCode(uBaseMicroCodeID);
		}
	}
	ProfileStart(0);
	
	
	State.FirstCImgTriAddress = 0;
	
	SetLevel(0);
	// increase age of cached textures. free those that are too old (>40 frames)
	U32 i, j;
	
	Stack[0].PC = Read32SPDMEM(0xFC0 + 48);
#if _DEBUG
	Command.raw.hi = 0;
	Command.raw.lo = 0;
	LogCommand("*** New display list (%i)...***\n", Stats.DLs);
#endif
	i = 0;
	while (i<State.TextureCount){
		State.Textures[i].Age++;
		if (State.Textures[i].Age>10){
			// remove texture, move back all behind by 1
			FreeObject(State.Textures[i].D3DObject);
			// Fill gap by moving all entries behind by one
			State.TextureCount--;
			if (i!=State.TextureCount){
				for (j=i;j<State.TextureCount;j++){
					State.Textures[j] = State.Textures[j+1];
				}
			}
		}
		i++;
	}
	i = 0;
#if COMBINEDEBUGGER
	for (i=0;i<LoggedCombineModes.Count;i++){
		LoggedCombineModes.Modes[i].Age++;
	}
	if (LoggedCombineModes.ReloadDatFile){
		LoggedCombineModes.ReloadDatFile = 0;
		FreeDatFile();
		LoadDatFile();
	}
#endif
	U32 Counter = 0; // remove the counter later, just to prevent crashing keep it for now
	uStackIndex = 0;
	if (uMicroCodeID==_microcode_rogue){
		// Read first 64 bits of this dlist
		Stack[uStackIndex].RogueStartDL = Read32(Stack[uStackIndex].PC) & BitM24;
		Stack[uStackIndex].RogueOtherDL = Read32(Stack[uStackIndex].PC+4) & BitM24;
		Stack[uStackIndex].PC += 8;
	}

	if (uMicroCodeID==9){
		// Battle for Naboo
		Stack[0].PC += 8;
	}
	State.WorldIndex = 0;
	State.WorldMatrix = &State.World[State.WorldIndex];
	State.D3DVertexCacheIndex = 0;

	// Initialize reflection stuff...
	State.LookAtX.r = 0x00;
	State.LookAtX.g = 0x80;
	State.LookAtX.b = 0x00;
	State.LookAtX.lx = 1;
	State.LookAtX.ly = 0;
	State.LookAtX.lz = 0;

	State.LookAtY.r = 0x00;
	State.LookAtY.g = 0x00;
	State.LookAtY.b = 0x00;
	State.LookAtY.lx = 0;
	State.LookAtY.ly = 1;
	State.LookAtY.lz = 0;

	if (Options.bForceZClear){
		g_pd3dDevice->Clear(1, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	}

	SetLevel(1);
	bDListFinished = false;
	__try{
		do{
			Command.raw.hi = Read32(Stack[uStackIndex].PC);
			Command.raw.lo = Read32(Stack[uStackIndex].PC+4);
			MicroCodeFunctions[Command.general.command]();
			Stack[uStackIndex].PC+=8;
			Counter++;
		} while ((!bDListFinished) && (Counter < 100000));
	} __except(ProcessException(GetExceptionInformation())){
		bDListFinished=true;
		ShowError(msExcError);
		return;
	}
	FlushVertexCache();
	if (bDListFinished == false){
		ShowError("Error: Too many commands executed. Stopping dlist processing");
	}
	ProfileStop(0);
}
