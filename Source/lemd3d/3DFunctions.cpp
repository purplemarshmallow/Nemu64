#include "stdafx.h"
#include "s2dex.h"

const U32 CRC32[258] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d, 0x00706c65, 0x58757300};


D3DTNTVERTEX VertexCache[256];
U32 PDColorCache[256];
TState State;
TOptions Options;
#if _DEBUG
TStats Stats;
#endif

void SetViewport()
{
	U32 StartX, StartY, StartZ, Width, Height, Depth;

	if (State.ScaleX>State.TranslateX){
		StartX = 0;
	} else {
		StartX = ((State.TranslateX-State.ScaleX)>>1) * State.WindowWidth / (State.cimg.width);
	}
	if (StartX>State.WindowWidth){
		StartX = State.WindowWidth;
	}

	if (State.ScaleY>State.TranslateY){
		StartY = 0;
	} else {
		StartY = ((State.TranslateY-State.ScaleY)>>1) * State.WindowHeight / GetHeight(State.cimg.width);
	}
	if (StartY>State.WindowHeight){
		StartY = State.WindowHeight;
	}

	
	if (State.ScaleZ>State.TranslateZ){
		StartZ = 0;
	} else {
		StartZ = ((State.TranslateZ-State.ScaleZ)>>1);
	}
	
	Width = State.ScaleX  * State.WindowWidth / (State.cimg.width);
	if (StartX+Width>State.WindowWidth){
		Width = State.WindowWidth-StartX;
	}

	Height = State.ScaleY  * State.WindowHeight / GetHeight(State.cimg.width);
	if (StartY+Height>State.WindowHeight){
		Height = State.WindowHeight-StartY;
	}
	
	Depth = State.ScaleZ;
	if ((State.Viewport.X!=StartX) || (State.Viewport.Y!=StartY) || (State.Viewport.Width != Width) || (State.Viewport.Height != Height)){
		State.Viewport.X = StartX;
		State.Viewport.Y = StartY;
		State.Viewport.Width = Width;
		State.Viewport.Height = Height;
		State.Viewport.MinZ = 0.0f;
		State.Viewport.MaxZ = 1.0f;
		g_pd3dDevice->SetViewport(&State.Viewport);
	}
}

void LoadLight(S32 Index, TLight *Light)
{
	FlushVertexCache();

	State.XFormLights = true;
	TLightLoaded *LoadLight;

	if (Index>16){
		ShowError("Light with index > 16 loaded");
		return;
	}
	if (Index<0){
		//Log("LoadLight with Index %i\n", Index);
		// this is a LookAt Command
		switch (Index){
		case -2:
			LoadLight = &State.LookAtX;
			break;
		case -1:
			LoadLight = &State.LookAtY;
			break;
		default:
			ShowError("Light with index <-2 loaded");
			return;
		}
	} else {
		LoadLight = &State.Lights[Index];
	}
 	LoadLight->r = Light->r;
	LoadLight->g = Light->g;
	LoadLight->b = Light->b;
	LoadLight->lx = (float)Light->x;
	LoadLight->ly = (float)Light->y;
	LoadLight->lz = (float)Light->z;

	LogCommand("Light %i, r:%i, g:%i, b:%i, x:%f, y:%f, z:%f\n", Index, Light->r, Light->g, Light->b, LoadLight->lx, LoadLight->ly, LoadLight->lz);
}

__inline void xform_light(D3DXMATRIX *InverseWorld, TLightLoaded *Light)
{
	float tx, ty, tz, recplength;
	tx = Light->lx*InverseWorld->_11 + Light->ly*InverseWorld->_21 + Light->lz*InverseWorld->_31;
	ty = Light->lx*InverseWorld->_12 + Light->ly*InverseWorld->_22 + Light->lz*InverseWorld->_32;
	tz = Light->lx*InverseWorld->_13 + Light->ly*InverseWorld->_23 + Light->lz*InverseWorld->_33;

	recplength = 1.0f / sqrtf(sqr(tx)+sqr(ty)+sqr(tz));
	// Normalize
	Light->tx = tx * recplength;
	Light->ty = ty * recplength;
	Light->tz = tz * recplength;
}

void xform_lights()
{
	// transform with inverse modelview
	if (!State.XFormLights){
		return;
	}
	State.XFormLights = false;
	D3DXMATRIX InverseWorld;
	ProfileStart(1);
#if 0 // c code need 1200 cycles compared to 7000 of D3DX function
	float det;
	D3DXMatrixInverse(&InverseWorld, &det, State.WorldMatrix);
#else
	InverseWorld._11=+(State.WorldMatrix->_22*State.WorldMatrix->_33-State.WorldMatrix->_23*State.WorldMatrix->_32);
	InverseWorld._12=-(State.WorldMatrix->_12*State.WorldMatrix->_33-State.WorldMatrix->_13*State.WorldMatrix->_32);
	InverseWorld._13=+(State.WorldMatrix->_12*State.WorldMatrix->_23-State.WorldMatrix->_13*State.WorldMatrix->_22);
	InverseWorld._21=-(State.WorldMatrix->_21*State.WorldMatrix->_33-State.WorldMatrix->_23*State.WorldMatrix->_31);
	InverseWorld._22=+(State.WorldMatrix->_11*State.WorldMatrix->_33-State.WorldMatrix->_13*State.WorldMatrix->_31); 
	InverseWorld._23=-(State.WorldMatrix->_11*State.WorldMatrix->_23-State.WorldMatrix->_13*State.WorldMatrix->_21);
	InverseWorld._31=+(State.WorldMatrix->_21*State.WorldMatrix->_32-State.WorldMatrix->_22*State.WorldMatrix->_31);
	InverseWorld._32=-(State.WorldMatrix->_11*State.WorldMatrix->_32-State.WorldMatrix->_12*State.WorldMatrix->_31);
	InverseWorld._33=+(State.WorldMatrix->_11*State.WorldMatrix->_22-State.WorldMatrix->_12*State.WorldMatrix->_21); 
#endif
	ProfileStop(1);

	U32 i;
	for (i=0;i<State.NumLights;i++){
		xform_light(&InverseWorld, &State.Lights[i]);
	}
/*	
	float recp;
	recp = 1.0f / sqrtf(sqr(State.LookAtY.lx)+sqr(State.LookAtY.ly)+sqr(State.LookAtY.lz));
	State.LookAtY.tx = State.LookAtY.lx * recp;
	State.LookAtY.ty = State.LookAtY.ly * recp;
	State.LookAtY.tz = State.LookAtY.lz * recp;
	
	recp = 1.0f / sqrtf(sqr(State.LookAtX.lx)+sqr(State.LookAtX.ly)+sqr(State.LookAtX.lz));
	State.LookAtX.tx = State.LookAtX.lx * recp;
	State.LookAtX.ty = State.LookAtX.ly * recp;
	State.LookAtX.tz = State.LookAtX.lz * recp;
*/

	xform_light(&InverseWorld, &State.LookAtX);
	xform_light(&InverseWorld, &State.LookAtY);
}

typedef union {
	struct {
		S64 Acc0_15  : 16;
		S64 Acc16_31 : 16;
		S64 Acc32_47 : 16;
	};
	struct {
		S64 Acc0_31  : 32;
		S64 Acc32_47 : 16;
	};
	struct {
		S64 Acc0_15  : 16;
		S64 Acc16_47 : 32;
	};
	struct {
		S64 Acc0_47  : 48;
	};
} TRSPAccum;

TRSPAccum RSPAccum[4];

void RSP_VMULF(S16 *OutReg, S16 *R1, S16 *R2)
{
	int i;
	int unsmult;
	int prod_shift;
	int acc_shift;
	int rnd_val;
	S64 tmpacc;
	U16 MulqMask;
	U16 vd_u;
	S16 vs_s,vt_s;

	U64 ClampMIN = 0xffffffffffff8000;
	U64 ClampMAX = 0x0000000000007fff;
	S64 ClampMask = ~(((U64)0x0000000000000001 << 31) - 1); 

	unsmult=0;
	acc_shift=0;
	prod_shift=0;
	rnd_val=0;
	MulqMask=0xffff;

	rnd_val = 32768;
	acc_shift = 16;
	prod_shift = 1;

	for(i=0;i<4;i++)
	{
		vs_s=(S16)(R1[i]);
		vt_s=(S16)(R2[i]);
		tmpacc=(S64)((S64)vs_s * (S64)vt_s);

		// shift for partial products
		if (prod_shift > 0)  
		{
			tmpacc <<= prod_shift;
	    }
		else if (prod_shift < 0) 
		{
			tmpacc = (U64)tmpacc >> (-prod_shift);
		}


		// do rounding if needed
		tmpacc += rnd_val;


		// sign extend bit 47
		// do i have to do this? Can i make compiler do this easier? or will it do this already?
		// test after doing a perfect interpreter.
	    if ((tmpacc>>47) & 0x1) 
			tmpacc = 0xffff000000000000 | (tmpacc & 0xffffffffffff);
	    else
			tmpacc &= 0xffffffffffff;

	    RSPAccum[i].Acc0_47 = tmpacc;

	    //clamping
	    if (tmpacc < 0) //negative
		{
			if (unsmult) 
			{
				tmpacc = 0;
            }
            else
				if (~tmpacc & ClampMask) ///there are some 0's in the overflow area
				{
					tmpacc = ((S64)ClampMIN) << acc_shift;
				}
	    } 
		else			//positive
		{
			if (tmpacc & ClampMask) // there are some 1's in the overflow area
			{
		    
				tmpacc = ((S64)ClampMAX) << acc_shift;
			}
	    }
		
#define ExtractBits64(dword, high, low) 	\
		((dword >> low) & (0xffffffffffffffff>>(63-(high-low))))

		//vd_u=(U16)(tmpacc>>(acc_shift))&((0xffffffffffffffff>>(63-((acc_shift+16-1)-acc_shift))));
		vd_u = (U16)ExtractBits64(tmpacc, (acc_shift+16-1), acc_shift);
		vd_u &= MulqMask;
		OutReg[i] = vd_u;
	}

}

void LoadVertices(U32 Address, U32 Number, U32 StartIndex)
{
	float dotproduct;

	LogCommand(", loading vertices %i to %i from &rdram[0x%X] %s\n", StartIndex, StartIndex+Number-1, Address, (State.GeometryMode.fields.Lighting?"with lighting":"without lighting"));
	if (Address>=8*MB){
		ShowError("Loading vertices out of rdram bounds");
		return;
	}
	TVertex *RDRAMVertex;

	RDRAMVertex = (TVertex*)&Global_Gfx_Info.RDRAM[Address];
	U32 i;
	D3DTNTVERTEX *Vertex;
	Vertex = &VertexCache[StartIndex];
	float x, y, z, rhw;
	D3DMATRIX *Matrix;
	Matrix = State.WorldMatrix;
	for (i=0;i<Number;i++){
		x = (float)RDRAMVertex->color.ob1;
		y = (float)RDRAMVertex->color.ob2;
		z = (float)RDRAMVertex->color.ob3;

		// World matrix
		Vertex->x   = (x*Matrix->_11 + y*Matrix->_21 + z*Matrix->_31 + Matrix->_41);
		Vertex->y   = (x*Matrix->_12 + y*Matrix->_22 + z*Matrix->_32 + Matrix->_42);
		Vertex->z   = (x*Matrix->_13 + y*Matrix->_23 + z*Matrix->_33 + Matrix->_43);
		rhw         = (x*Matrix->_14 + y*Matrix->_24 + z*Matrix->_34 + Matrix->_44);
		if (rhw!=1.0f){
			rhw = 1.0f/rhw;
			Vertex->x *= rhw;
			Vertex->y *= rhw;
			Vertex->z *= rhw;
		}
		float n1, n2, n3;
		ProfileStart(2);
		if (State.GeometryMode.fields.Shade){
			if (State.GeometryMode.fields.Lighting){
				xform_lights();
				n1 = RDRAMVertex->normal.n1/127.0f;
				n2 = RDRAMVertex->normal.n2/127.0f;
				n3 = RDRAMVertex->normal.n3/127.0f;
				if ((U32)State.NumLights<=16){
					S32 r,g,b;

					U32 LightIndex;

					// ambient light
					r = State.Lights[State.NumLights].r;
					g = State.Lights[State.NumLights].g;
					b = State.Lights[State.NumLights].b;
					// directional
					for(LightIndex=0;LightIndex<State.NumLights;LightIndex++){
						dotproduct = (State.Lights[LightIndex].tx * n1 +
										State.Lights[LightIndex].ty * n2 +
										State.Lights[LightIndex].tz * n3);
						if (dotproduct>0){
							r+=(U32)(dotproduct*State.Lights[LightIndex].r);
							g+=(U32)(dotproduct*State.Lights[LightIndex].g);
							b+=(U32)(dotproduct*State.Lights[LightIndex].b);
						}
					}
					// range check
					r = min(255, r);
					g = min(255, g);
					b = min(255, b);
				
					// done
					Vertex->color = (RDRAMVertex->color.alpha<<24) |
									//0xFFFFFFFF |
									(r<<16) | (g<<8) | b;
				}
			} else {
				Vertex->color = (RDRAMVertex->color.alpha<<24) | (RDRAMVertex->color.c1<<16) | (RDRAMVertex->color.c2<<8) | (RDRAMVertex->color.c3);
			}
		} else {
			Vertex->color = State.PrimColor;
		}
		// texture coordinates
		if (State.GeometryMode.fields.Texture_Gen && State.GeometryMode.fields.Lighting && State.GeometryMode.fields.Shade){
			Vertex->tex[0].u = (State.LookAtX.tx * n1 +
							State.LookAtX.ty * n2 +
							State.LookAtX.tz * n3);

			Vertex->tex[0].v = (State.LookAtY.tx * n1 +
							State.LookAtY.ty * n2 +
							State.LookAtY.tz * n3);

			if (State.GeometryMode.fields.Texture_Gen_Linear){
				float s, t;
				s = Vertex->tex[0].u;
				t =	Vertex->tex[0].v;
				Vertex->tex[0].u = ((s*s*s + s*s*s*0.45347f + s*0.636627f)+1.0f)*0.5f/State.texture.TextureScaleS;
				Vertex->tex[0].v = ((t*t*t + t*t*t*0.45347f + t*0.636627f)+1.0f)*0.5f/State.texture.TextureScaleT;
			} else {
				Vertex->tex[0].u = ((Vertex->tex[0].u)+1.0f)*0.5f/State.texture.TextureScaleS;
				Vertex->tex[0].v = ((Vertex->tex[0].v)+1.0f)*0.5f/State.texture.TextureScaleT;
			}
		} else {
			Vertex->tex[0].u = _10_5ToFloat(RDRAMVertex->color.tc1)*State.texture.TextureScaleS;
			Vertex->tex[0].v = _10_5ToFloat(RDRAMVertex->color.tc2)*State.texture.TextureScaleT;
		}
		ProfileStop(2);
		LogCommand("** Vertex %i **\n", i);
		LogCommand("   x: %2.2f,  y: %2.2f,  x: %2.2f\n", x, y, z);
		LogCommand("  tx: %2.2f, ty: %2.2f, tz: %2.2f\n", Vertex->x, Vertex->y, Vertex->z);
		if (State.GeometryMode.fields.Lighting){
			LogCommand("  nx: %2.2f, ny: %2.2f, nz: %2.2f\n", n1, n2, n3);
		}
		LogCommand("  Color: 0x%X\n", Vertex->color);
		if (State.GeometryMode.fields.Texture_Gen && State.GeometryMode.fields.Lighting){
			LogCommand("  Texture generated coordinates: %4.4f:%4.4f\n", Vertex->tex[0].u, Vertex->tex[0].v);
		} else {
			LogCommand("  Raw texture coordinates: 0x%X:0x%X\n", RDRAMVertex->color.tc1, RDRAMVertex->color.tc2);
			LogCommand("  Texture coordinates: %4.4f:%4.4f\n", Vertex->tex[0].u, Vertex->tex[0].v);
		}
		Vertex++;
		RDRAMVertex++;
	}
}

void LoadVertices_GBI5(U32 Address, U32 Number, U32 StartIndex)
{
	if (Address>=8*MB){
		ShowError("Loading vertices out of rdram bounds");
		return;
	}

	U32 i;
	D3DTNTVERTEX *Vertex;
	Vertex = &VertexCache[StartIndex];

	D3DMATRIX *Matrix;
	Matrix = &State.World[State.WorldIndex];
	for (i=0;i<Number;i++){
		Vertex->color = (Read16(Address+6)<<16)|(Read16(Address+8));
		Vertex->color = ((Vertex->color&0xFF)<<24) | ((Vertex->color>>8)&0x00FFFFFF);

#if 1
		Vertex->x=(float)(S16)Read16(Address);
		Vertex->y=(float)(S16)Read16(Address+2);
		Vertex->z=(float)(S16)Read16(Address+4);
#else
		float x, y, z, rhw;
		x=(float)(S16)Read16(Address);
		y=(float)(S16)Read16(Address+2);
		z=(float)(S16)Read16(Address+4);

		Vertex->x   = (x*Matrix->_11 + y*Matrix->_21 + z*Matrix->_31 + Matrix->_41);
		Vertex->y   = (x*Matrix->_12 + y*Matrix->_22 + z*Matrix->_32 + Matrix->_42);
		Vertex->z   = (x*Matrix->_13 + y*Matrix->_23 + z*Matrix->_33 + Matrix->_43);
		rhw         = (x*Matrix->_14 + y*Matrix->_24 + z*Matrix->_34 + Matrix->_44);
		rhw = 1.0f/rhw;
		Vertex->x *= rhw;
		Vertex->y *= rhw;
		Vertex->z *= (1.0f/65536.0f);
#endif
		LogCommand("Vertex %i: x=%2.3f, y=%2.3f, z=%2.3f, col=%X\n", i, Vertex->x, Vertex->y, Vertex->z, Vertex->color);

		Vertex++;
		Address += 10;
	}
}

void LoadVertices_GBI7(U32 Address, U32 Number, U32 StartIndex)
{
	LogCommand(", loading vertices %i to %i from &rdram[0x%X]\n", StartIndex, StartIndex+Number-1, Address);
	if (Address>=8*MB){
		ShowError("Loading vertices out of rdram bounds");
		return;
	}
	TVertex_PD *RDRAMVertex;

	RDRAMVertex = (TVertex_PD*)&Global_Gfx_Info.RDRAM[Address];
	U32 i;
	D3DTNTVERTEX *Vertex;
	Vertex = &VertexCache[StartIndex];
	float x, y, z, rhw;
	D3DMATRIX *Matrix;
	Matrix = State.WorldMatrix;
	for (i=0;i<Number;i++){
		x = (float)RDRAMVertex->color.ob1;
		y = (float)RDRAMVertex->color.ob2;
		z = (float)RDRAMVertex->color.ob3;

		// World matrix
		Vertex->x   = (x*Matrix->_11 + y*Matrix->_21 + z*Matrix->_31 + Matrix->_41);
		Vertex->y   = (x*Matrix->_12 + y*Matrix->_22 + z*Matrix->_32 + Matrix->_42);
		Vertex->z   = (x*Matrix->_13 + y*Matrix->_23 + z*Matrix->_33 + Matrix->_43);
		rhw         = (x*Matrix->_14 + y*Matrix->_24 + z*Matrix->_34 + Matrix->_44);
		if (rhw!=1.0f){
			rhw = 1.0f/rhw;
			Vertex->x *= rhw;
			Vertex->y *= rhw;
			Vertex->z *= rhw;
		}
		float n1, n2, n3;
		Vertex->color = PDColorCache[RDRAMVertex->color.ci];
		if (State.GeometryMode.fields.Lighting){
			xform_lights();
			// the color values of the VertexCache are actually normals
			n1 = ((S8)BITS(Vertex->color, 16, 8))/127.0f;
			n2 = ((S8)BITS(Vertex->color,  8, 8))/127.0f;
			n3 = ((S8)BITS(Vertex->color,  0, 8))/127.0f;
			if ((U32)State.NumLights<=16){
				S32 r,g,b;
				float dotproduct;

				U32 LightIndex;

				// ambient light
				r = State.Lights[State.NumLights].r;
				g = State.Lights[State.NumLights].g;
				b = State.Lights[State.NumLights].b;
				// directional
				for(LightIndex=0;LightIndex<State.NumLights;LightIndex++){
			        dotproduct = (State.Lights[LightIndex].tx * n1 +
						          State.Lights[LightIndex].ty * n2 +
								  State.Lights[LightIndex].tz * n3);
		            if (dotproduct>0){
			            r+=(U32)(dotproduct*State.Lights[LightIndex].r);
				        g+=(U32)(dotproduct*State.Lights[LightIndex].g);
					    b+=(U32)(dotproduct*State.Lights[LightIndex].b);
					}
			    }
		        // range check
				r = min(255, r);
				g = min(255, g);
				b = min(255, b);
			
				// done - use vertex transparency
				Vertex->color = (Vertex->color & 0xFF000000) | 
								(r<<16) | (g<<8) | b;
			}
		}
		// texture coordinates
		if (State.GeometryMode.fields.Texture_Gen && State.GeometryMode.fields.Lighting){
			float s,t;
			float minx, miny, maxx, maxy;
			if (State.GeometryMode.fields.Texture_Gen_Linear){
				s   = ((n1*State.Projection._11) + (n2*State.Projection._21) + (n3*State.Projection._31));
				t   = ((n1*State.Projection._12) + (n2*State.Projection._22) + (n3*State.Projection._32));
				s   = 1.0f/cosf(s);
				t   = 1.0f/cosf(t);
			} else {
				s   = ((n1*State.Projection._11) + (n2*State.Projection._21) + (n3*State.Projection._31));
				t   = ((n1*State.Projection._12) + (n2*State.Projection._22) + (n3*State.Projection._32));
			}
			minx = -1.0f/State.texture.TextureScaleS;
			maxx =  1.0f/State.texture.TextureScaleS;
			miny = -1.0f/State.texture.TextureScaleT;
			maxy =  1.0f/State.texture.TextureScaleT;
//			minx = -1.0f;
//			maxx =  1.0f;
//			miny = -1.0f;
//			maxy =  1.0f;
			Vertex->tex[0].u = (s+minx)/(maxx-minx)*0.5f;
			Vertex->tex[0].v = (t+minx)/(maxy-miny)*0.5f;
		} else {
			Vertex->tex[0].u = _10_5ToFloat(RDRAMVertex->color.tc1)*State.texture.TextureScaleS;
			Vertex->tex[0].v = _10_5ToFloat(RDRAMVertex->color.tc2)*State.texture.TextureScaleT;
		}
		Vertex++;
		RDRAMVertex++;
	}
}

void LoadVertices_Rogue(U32 Address, U32 Number, U32 StartIndex)
{
	LogCommand(", loading vertices %i to %i from &rdram[0x%X]\n", StartIndex, StartIndex+Number-1, Address);
	if (Address>=8*MB){
		ShowError("Loading vertices out of rdram bounds");
		return;
	}
	TVertex_Rogue *RDRAMVertex;

	RDRAMVertex = (TVertex_Rogue*)&Global_Gfx_Info.RDRAM[Address];
	U32 i;
	D3DTNTVERTEX *Vertex;
	Vertex = &VertexCache[StartIndex];
	float x, y, z, rhw;
	D3DMATRIX *Matrix;
	Matrix = State.WorldMatrix;
	for (i=0;i<Number;i++){
		x = (float)RDRAMVertex->color.ob1;
		y = (float)RDRAMVertex->color.ob2;
		z = (float)RDRAMVertex->color.ob3;

		// World matrix
		Vertex->x   = (x*Matrix->_11 + y*Matrix->_21 + z*Matrix->_31 + Matrix->_41);
		Vertex->y   = -(x*Matrix->_12 + y*Matrix->_22 + z*Matrix->_32 + Matrix->_42);
		Vertex->z   = (x*Matrix->_13 + y*Matrix->_23 + z*Matrix->_33 + Matrix->_43);
		rhw         = (x*Matrix->_14 + y*Matrix->_24 + z*Matrix->_34 + Matrix->_44);
		if (rhw!=1.0f){
			rhw = 1.0f/rhw;
			Vertex->x *= rhw;
			Vertex->y *= rhw;
			Vertex->z *= rhw;
		}
		Vertex++;
		RDRAMVertex++;
	}
}

void LoadMatrix(U32 base, D3DMATRIX *Matrix)
{
	const float divisor = 1.0f/65536.0f;
	Matrix->_11 = ((long)((Read32(base)   &0xFFFF0000 ) | (Read32(base+32) >> 16 )) * divisor );
	Matrix->_12 = ((long)((Read32(base)   <<16 )        | (Read32(base+32) & 0x0000FFFF)) * divisor );
	Matrix->_13 = ((long)((Read32(base+4) &0xFFFF0000 ) | (Read32(base+36) >> 16 )) * divisor );
	Matrix->_14 = ((long)((Read32(base+4) <<16 )        | (Read32(base+36) & 0x0000FFFF)) * divisor );
	Matrix->_21 = ((long)((Read32(base+8) &0xFFFF0000 ) | (Read32(base+40) >> 16 )) * divisor );
	Matrix->_22 = ((long)((Read32(base+8) <<16 )        | (Read32(base+40) & 0x0000FFFF)) * divisor );
	Matrix->_23 = ((long)((Read32(base+12)&0xFFFF0000 ) | (Read32(base+44) >> 16 )) * divisor );
	Matrix->_24 = ((long)((Read32(base+12)<<16 )        | (Read32(base+44) & 0x0000FFFF)) * divisor );
	Matrix->_31 = ((long)((Read32(base+16)&0xFFFF0000 ) | (Read32(base+48) >> 16 )) * divisor );
	Matrix->_32 = ((long)((Read32(base+16)<<16 )        | (Read32(base+48) & 0x0000FFFF)) * divisor );
	Matrix->_33 = ((long)((Read32(base+20)&0xFFFF0000 ) | (Read32(base+52) >> 16 )) * divisor );
	Matrix->_34 = ((long)((Read32(base+20)<<16 )        | (Read32(base+52) & 0x0000FFFF)) * divisor );
	Matrix->_41 = ((long)((Read32(base+24)&0xFFFF0000 ) | (Read32(base+56) >> 16 )) * divisor );
	Matrix->_42 = ((long)((Read32(base+24)<<16 )        | (Read32(base+56) & 0x0000FFFF)) * divisor );
	Matrix->_43 = ((long)((Read32(base+28)&0xFFFF0000 ) | (Read32(base+60) >> 16 )) * divisor );
	Matrix->_44 = ((long)((Read32(base+28)<<16 )        | (Read32(base+60) & 0x0000FFFF)) * divisor );
}

void CalcModifiedProjectionMatrix()
{
	// Range before: -MinMaxZ to MinMaxZ
	// Range after: 0 to 1
	D3DXMATRIX Matrix;
	D3DXMatrixIdentity(&Matrix);
#define MinMaxZ 2.0f
	if (UCode_NoN){
		Matrix._33 = (1.0f/(MinMaxZ*4));   // multiplied
	} else {
		Matrix._33 = (1.0f/(1.0f*4));      // multiplied
	}
	Matrix._43 = 0.75f;                     // is added

	if ((uMicroCodeID==_microcode_jfg)||(uMicroCodeID==_microcode_dkr)){
		State.ProjectionModified = State.World[State.WorldIndex]*Matrix;
		//State.ProjectionModified = State.Identity;
	} else {
		State.ProjectionModified = State.Projection*Matrix;
	}
#if COMBINEDEBUGGER
	// add camera debug stuff
	D3DXMatrixTranslation(&Matrix,
					(float)LoggedCombineModes.addx,
					(float)LoggedCombineModes.addy,
					(float)LoggedCombineModes.addz);
	State.ProjectionModified = Matrix * State.ProjectionModified;
	D3DXMatrixRotationYawPitchRoll(&Matrix,
					(float)LoggedCombineModes.roty,
					(float)LoggedCombineModes.rotx,
					(float)LoggedCombineModes.rotz);
	State.ProjectionModified = Matrix * State.ProjectionModified;
#endif
}

inline D3DMATRIX MatrixMult(const D3DMATRIX a,const D3DMATRIX b)
{
	D3DMATRIX ret;
	ret._11 = a._11 * b._11 + a._21 * b._12 + a._31 * b._13 + a._41 * b._14;
	ret._12 = a._12 * b._11 + a._22 * b._12 + a._32 * b._13 + a._42 * b._14;
	ret._13 = a._13 * b._11 + a._23 * b._12 + a._33 * b._13 + a._43 * b._14;
	ret._14 = a._14 * b._11 + a._24 * b._12 + a._34 * b._13 + a._44 * b._14;
	ret._21 = a._11 * b._21 + a._21 * b._22 + a._31 * b._23 + a._41 * b._24;
	ret._22 = a._12 * b._21 + a._22 * b._22 + a._32 * b._23 + a._42 * b._24;
	ret._23 = a._13 * b._21 + a._23 * b._22 + a._33 * b._23 + a._43 * b._24;
	ret._24 = a._14 * b._21 + a._24 * b._22 + a._34 * b._23 + a._44 * b._24;
	ret._31 = a._11 * b._31 + a._21 * b._32 + a._31 * b._33 + a._41 * b._34;
	ret._32 = a._12 * b._31 + a._22 * b._32 + a._32 * b._33 + a._42 * b._34;
	ret._33 = a._13 * b._31 + a._23 * b._32 + a._33 * b._33 + a._43 * b._34;
	ret._34 = a._14 * b._31 + a._24 * b._32 + a._34 * b._33 + a._44 * b._34;
	ret._41 = a._11 * b._41 + a._21 * b._42 + a._31 * b._43 + a._41 * b._44;
	ret._42 = a._12 * b._41 + a._22 * b._42 + a._32 * b._43 + a._42 * b._44;
	ret._43 = a._13 * b._41 + a._23 * b._42 + a._33 * b._43 + a._43 * b._44;
	ret._44 = a._14 * b._41 + a._24 * b._42 + a._34 * b._43 + a._44 * b._44;
	return ret;
}

void UseMatrix(U32 Address, bool Type, bool Load, bool Push)
{
	FlushVertexCache();
	D3DXMATRIX Matrix;
	LoadMatrix(Address, &Matrix);
	if (Type){
		// Possible bug: d3d says _34 may not be <0 but it works
		if (Push){
			ShowError("Trying to push projecton matrix");
		}
		if (Load){
			State.Projection = Matrix;
		} else {
			//State.Projection = MatrixMult(State.Projection, Matrix);
			State.Projection = Matrix * State.Projection;
		}
		State.ProjectionChanged = true;
	} else {
		if (Push){
			State.WorldIndex++;
			State.WorldMatrix = &State.World[State.WorldIndex];
			if (Load){
				State.World[State.WorldIndex] = Matrix;
			} else {
				(*State.WorldMatrix) = Matrix * State.World[State.WorldIndex-1];
			}
		} else {
			if (Load){
				State.World[State.WorldIndex] = Matrix;
			} else {
				(*State.WorldMatrix) = Matrix*State.World[State.WorldIndex];
			}
		}
	}
	// the force matrix command can overwrite this.
	// Whenever this function is called set it back
	if (State.bForceMtx){
		State.WorldMatrix = &State.World[State.WorldIndex];
		State.bForceMtx = false;
		State.ProjectionChanged = true;
	}
}

void CalculateTileSize(TTile *tile)
{
	// Purpose: Calculate tile
	//          ->D3DWidth, D3DHeight: Final Texture size for D3D (Power of 2)
	//          ->Width, Height:       Final Texture size for loading
	//          ->D3DAddressU, D3DAddressV: Final texture adress
	// 
	// Input: tile->cms/cmt/masks/maskt and tile->Width/Height
	U32 tx, D3DWidth, D3DHeight;
	U32 POW2Width, POW2Height;
	U32 Width, Height;

	if (tile->masks==0){
		tile->Width = (S32)(tile->lrs-tile->uls+1);
		D3DWidth = tile->Width;
		tile->D3DAddressU = D3DTADDRESS_MIRROR;
	} else {
		POW2Width = 1<<tile->masks;
		Width = (U32)(tile->lrs-tile->uls+1);
		switch(tile->cms){
		case 0: // Wrap
			D3DWidth = POW2Width;
			if (Width>POW2Width){
				//ShowError("Wrap Warning: Width!=POW2Width");
				tile->Width = POW2Width;
			} else {
				tile->Width = Width;
			}
			tile->D3DAddressU = D3DTADDRESS_WRAP;
			break;
		case 1: // Mirror
			D3DWidth = POW2Width;
			if (Width>POW2Width){
				//ShowError("Mirror Warning: Width!=POW2Width");
				tile->Width = POW2Width;
			} else {
				tile->Width = Width;
			}
			tile->D3DAddressU = D3DTADDRESS_MIRROR;
			break;
		case 2: // Clamp & Wrap
			// we'll see how this works
			// the n64 can probably mirror a texture 10 times and then clamp
			// direct3d cannot do this - so I mirror it forever
			tile->Width = Width;
			D3DWidth = POW2Width;
			if (Width>POW2Width){
				tile->Width = POW2Width;
				tile->D3DAddressU = D3DTADDRESS_WRAP;
			} else {
				tile->D3DAddressU = D3DTADDRESS_CLAMP;
			}
			break;
		case 3: // Clamp & Mirror
			// we'll see how this works
			// the n64 can probably mirror a texture 10 times and then clamp
			// direct3d cannot do this - so I mirror it forever
			tile->Width = Width;
			D3DWidth = POW2Width;
			if (Width>POW2Width){
				tile->Width = POW2Width;
				tile->D3DAddressU = D3DTADDRESS_MIRROR;
			} else {
				tile->D3DAddressU = D3DTADDRESS_CLAMP;
			}
			break;
		}
	}
	
	if (tile->maskt==0){
		tile->Height = (S32)(tile->lrt-tile->ult+1);
		D3DHeight = tile->Height;
		tile->D3DAddressV = D3DTADDRESS_MIRROR;
	} else {
		POW2Height = 1<<tile->maskt;
		Height = (U32)(tile->lrt-tile->ult+1);
		switch(tile->cmt){
		case 0: // Wrap
			D3DHeight = POW2Height;
			if (Height>POW2Height){
				//ShowError("Wrap Warning: Height!=POW2Height");
				tile->Height = POW2Height;
			} else {
				tile->Height = Height;
			}
			tile->D3DAddressV = D3DTADDRESS_WRAP;
			break;
		case 1: // Mirror
			D3DHeight = POW2Height;
			if (Height>POW2Height){
				//ShowError("Mirror Warning: Height!=POW2Height");
				tile->Height = POW2Height;
			} else {
				tile->Height = Height;
			}
			tile->D3DAddressV = D3DTADDRESS_MIRROR;
			break;
		case 2: // Clamp & Wrap
			// we'll see how this works
			// the n64 can probably mirror a texture 10 times and then clamp
			// direct3d cannot do this - so I mirror it forever
			tile->Height = Height;
			D3DHeight = POW2Height;
			if (Height>POW2Height){
				tile->Height = POW2Height;
				tile->D3DAddressV = D3DTADDRESS_WRAP;
			} else {
				tile->D3DAddressV = D3DTADDRESS_CLAMP;
			}
			break;
		case 3: // Clamp & Mirror
			// we'll see how this works
			// the n64 can probably mirror a texture 10 times and then clamp
			// direct3d cannot do this - so I mirror it forever
			tile->Height = Height;
			D3DHeight = POW2Height;
			if (Height>POW2Height){
				tile->Height = POW2Height;
				tile->D3DAddressV = D3DTADDRESS_MIRROR;
			} else {
				tile->D3DAddressV = D3DTADDRESS_CLAMP;
			}
			break;
		}
	}
	
	for (tx=1024;tx>=1;tx>>=1) {
		if (D3DWidth<=tx)   { tile->D3DWidth  = tx;}
		if (D3DHeight<=tx)  { tile->D3DHeight = tx;}
	}
}

void SetLoadTileTlutInfo(eLoadType LoadType)
{
	// this function stores the info of LoadTile and LoadTlut in Pseudo TMEM
	FlushVertexCache();

	TTile *tile;
	tile = &State.Tiles[Command.loadtile.tile];
	tile->uls = Command.loadtile.sl+Command.loadtile.slfrac/4.0f;
	tile->ult = Command.loadtile.tl+Command.loadtile.tlfrac/4.0f;
	tile->lrs = Command.loadtile.sh+Command.loadtile.shfrac/4.0f;
	tile->lrt = Command.loadtile.th+Command.loadtile.thfrac/4.0f;

	U32 Height, Width;
	Width = Command.loadtile.sh - Command.loadtile.sl+1;
	Height = Command.loadtile.th - Command.loadtile.tl+1;

	U32 xoffset, stride, tmemstride;
	tmemstride = tile->line<<3;
	if (LoadType==_LoadTile){
		switch (tile->size) {
		case 0:
			ShowError("Loadtile with size 0");
			break;
		case 1:
			xoffset = Command.loadtile.sl;
			stride = State.timg.width;
			break;
		case 2:
			xoffset = Command.loadtile.sl<<1;
			stride = State.timg.width<<1;
			Width<<=1;
			break;
		case 3:
			xoffset = Command.loadtile.sl<<2;
			stride = State.timg.width<<2;
			Width<<=2;
			tmemstride<<=1;
			break;
		}
	} else {
		// move halfwords, not bytes
		xoffset = Command.loadtile.sl<<1;
		stride = State.timg.width<<1;
		Width<<=1;
	}

	U32 TMEMAddress;
	U32 DRAMAddress;
	DRAMAddress = State.timg.address;
	TMEMAddress = tile->tmem;
restart:
	State.TMEM[TMEMAddress].TMEMStride = tmemstride;
	State.TMEM[TMEMAddress].RDRAMAddress = DRAMAddress;
	State.TMEM[TMEMAddress].RDRAMStride = stride;
	State.TMEM[TMEMAddress].LoadType = LoadType;
	State.TMEM[TMEMAddress].StartX = xoffset;
	State.TMEM[TMEMAddress].StartY = Command.loadtile.tl;
	State.TMEM[TMEMAddress].Width = Width;
	State.TMEM[TMEMAddress].Height = Height;
	State.TMEM[TMEMAddress].TSize = tile->size;
	State.TMEM[TMEMAddress].Line = tile->line;
	State.TMEM[TMEMAddress].DXTUsed = true;
	State.TextureChanged = true;
	State.LastLoadBlock = false;
	if (LoadType==_LoadTLUT){
		DRAMAddress += 32;
		TMEMAddress += 128;
		Width -= 32;
		if ((S32)Width>0){
			goto restart;
		}
	}
}

void SimulateTLoadCommand(U32 Start, U8 *TMEMFlipped, U32 Height, U32 RDPPitch, U32 TSize)
{
	U32 x, y;
	union {
		U8* p8;
		U16* p16;
		U32* p32;
		U64* p64;
	} sourceptr, targetptr;
	U32 RDRAMPtr;
	RDRAMPtr = (U32)&_RDRAM[State.TMEM[Start].RDRAMAddress] + State.TMEM[Start].StartX + State.TMEM[Start].StartY*State.TMEM[Start].RDRAMStride;
	if (State.TMEM[Start].LoadType==_LoadBlock){
		// Simulate LoadBlock
		if (((Start&BitM2)==0) && ((State.TMEM[Start].RDRAMAddress&BitM2)==0) && ((State.TMEM[Start].Size&BitM2)==0)){
			// It is a nicely aligned copy
			memcpy(&TMEMFlipped[Start], (void*)RDRAMPtr, State.TMEM[Start].Size);
		} else {
			U32 bi;
			for (bi=0;bi<State.TMEM[Start].Size;bi++){
				TMEMFlipped[(Start+bi)^3] = *(U8*)(((U32)(RDRAMPtr+bi))^3);
			}
		}
		if (!State.TMEM[Start].DXTUsed){
			if (TSize!=3){
				// Flip odd lines
				for (y=1;y<Height;y+=2){
					sourceptr.p8 = &_RDRAM[State.TMEM[Start].RDRAMAddress + (y*RDPPitch)];
					targetptr.p8 = &TMEMFlipped[Start + (y*RDPPitch)];
					for (x=0;x<(RDPPitch>>2)-1;x+=2){
						targetptr.p32[x] = sourceptr.p32[x + 1];
						targetptr.p32[x + 1] = sourceptr.p32[x];
					}
				}
			} else {
				// Flip odd lines for 32 bit textures
				for (y=1;y<Height;y+=2){
					sourceptr.p8 = &_RDRAM[State.TMEM[Start].RDRAMAddress + (y*RDPPitch)];
					targetptr.p8 = &TMEMFlipped[Start + (y*RDPPitch)]; 
					for (x=0;x<(RDPPitch>>3)-1;x+=2){
						targetptr.p64[x] = sourceptr.p64[x + 1];
						targetptr.p64[x + 1] = sourceptr.p64[x];
					}
				}
			}
		}
	} else {
		// simulate LoadTile
		U32 j, x;
		U32 LTWidth, LTHeight, stride, xoffset, yoffset, tmemstride;
		U32 RDRAMAddress;
		RDRAMAddress = State.TMEM[Start].RDRAMAddress;
		LTWidth = State.TMEM[Start].Width;
		LTHeight = State.TMEM[Start].Height;
		stride = State.TMEM[Start].RDRAMStride;
		xoffset = State.TMEM[Start].StartX;
		yoffset = State.TMEM[Start].StartY;
		tmemstride = State.TMEM[Start].TMEMStride;
		if ((Start + LTHeight*LTWidth)>TMEMSize){
			ShowError("Loadtile out of bounds");
			LTHeight = (TMEMSize-Start)/LTWidth;
		}
		if ((stride&BitM2) || (xoffset&BitM2) || (RDRAMAddress&BitM2)){
			// unaligned
			for (j=0;j<LTHeight;j++){
				for (x=0;x<LTWidth;x++){
					TMEMFlipped[(Start + (j*tmemstride) + x)^3] = _RDRAM[(RDRAMAddress+((yoffset+j)*stride)+(xoffset+x))^3];
				}
			}
		} else {
			// aligned
			for (j=0;j<LTHeight;j++){
				memcpy(
					&TMEMFlipped[Start + (j*tmemstride)],
					&_RDRAM[RDRAMAddress+((yoffset+j)*stride)+xoffset],
					(LTWidth | 3)+1);
			}
		}
	}
}

U32 CacheTexture(TTile *tile)
{
	LPDIRECT3DTEXTURE9 D3DTexture;
	LPDIRECT3DSURFACE9 D3DSurface0;
	D3DFORMAT Format;
	D3DLOCKED_RECT Rect;
	U32 i, UTI, Index;
	U32 TempHeight;
	U32 RDRAMPtr, CRCPitch;

	if ((!State.TextureChanged) && (tile==State.LastTile)){
		return State.LastTextureIndex;
	}

	U32 Start;
	U32 Width, Height, D3DWidth, D3DHeight, RDPPitch, BytesPerLine;
	D3DWidth = tile->D3DWidth;
	D3DHeight = tile->D3DHeight;
	Start=tile->tmem;
/*
	if ((tile->masks!=0) && ((tile->cms==2) || (tile->cms==3))){
		Width = min((S32)(tile->lrs-tile->uls+1), (S32)tile->Width);
	} else {
*/
	Width = tile->Width;
//	}
/*
	if ((tile->maskt!=0) && ((tile->cmt==2) || (tile->cmt==3))){
		Height = min((S32)(tile->lrt-tile->ult+1), (S32)tile->Height);
	} else {
*/
	Height = tile->Height;
//	}
	if (Width==0){
		Width = 1;
	}
	if (Height==0){
		Height = 1;
	}
	if ((S32)Width<=0){
		Width = -(S32)Width;
	}
	if ((S32)Height<=0){
		Height = -(S32)Height;
	}

	if ((S32)Width<=0)
		Width = 1;
	if ((S32)Height<=0)
		Height = 1;

	if ((S32)D3DWidth<=0)
		D3DWidth = 1;
	if ((S32)D3DHeight<=0)
		D3DHeight = 1;

	if (D3DWidth<Width)
		Width = D3DWidth;
	
	if (D3DHeight<Height)
		Height = D3DHeight;
	
	if (tile->size==3){
		RDPPitch = tile->line<<4;
	} else {
		RDPPitch = tile->line<<3;
	}
	if (RDPPitch==0){
		// save a fake texture...
		LogCommand("Error: RDPPitch==0 in CacheTexture\n");
		SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &D3DTexture, NULL));
		if (State.TextureCount >= (MaxTextureCount-5)){
			// Delete oldest texture
			U32 tindex, oldestindex;
			oldestindex = 0;
			for (tindex=1;tindex<State.TextureCount;tindex++){
				if (State.Textures[tindex].Age>=State.Textures[oldestindex].Age){
					oldestindex = tindex;
				}
			}
			FreeObject(State.Textures[oldestindex].D3DObject);
			// We know the oldest texture, now replace it
			Index = oldestindex;
		} else {
			Index = State.TextureCount;
			State.TextureCount++;
		}
		State.Textures[Index].UTI = 0;
		State.Textures[Index].D3DObject = D3DTexture;
		State.Textures[Index].RDPFormat = tile->size | (tile->fmt<<2);
		State.Textures[Index].Width = Width;
		State.Textures[Index].Height = Height;
		State.Textures[Index].ByLoadBlock = State.LastLoadBlock;
		State.Textures[Index].DXTUsed = State.TMEM[Start].DXTUsed;
		State.Textures[Index].RDPPitch = RDPPitch;
		State.Textures[Index].cms = tile->cms;
		State.Textures[Index].cmt = tile->cmt;
		State.Textures[Index].masks = tile->masks;
		State.Textures[Index].maskt = tile->maskt;
		State.Textures[Index].Age = 0;
		State.LastTextureIndex = Index;
		State.LastTile = tile;
		return 0;
	}
	switch (tile->size){
	case 0:
		BytesPerLine = (Width)>>1;
		break;
	case 1:
		BytesPerLine = Width;
		break;
	case 2:
		BytesPerLine = (Width)<<1;
		break;
	case 3:
		BytesPerLine = (Width)<<2;
		break;
	}
	if ((Start&BitM2)){
		ShowError("Trying to cache unaligned texture");
	}
	if (RDPPitch*Height+Start>TMEMSize){
		ShowError("Trying to load texture that is out of tmem bounds");
		Start = 0;
		Height = (TMEMSize-Start)/RDPPitch;
	}
	Index = 0xFFFFFFFF;
	// Calculate UTI
	if (State.TMEM[Start].RDRAMAddress==0){
		Start = 0;
		ShowError("Error: LoadBlock or LoadTile simulation without tmem failed");
	}
	if (State.TMEM[Start].LoadType == _LoadBlock){
		RDRAMPtr = (U32)&_RDRAM[State.TMEM[Start].RDRAMAddress] + State.TMEM[Start].StartX + State.TMEM[Start].StartY*State.TMEM[Start].RDRAMStride;
		BytesPerLine = State.TMEM[Start].Size;
		TempHeight = 1;
	} else {
		CRCPitch = State.TMEM[Start].RDRAMStride;
		RDRAMPtr = (U32)&_RDRAM[State.TMEM[Start].RDRAMAddress] + State.TMEM[Start].StartX + State.TMEM[Start].StartY*State.TMEM[Start].RDRAMStride;
		TempHeight = Height;
	}
#define CRCSkip 3
	__asm{
		push ebx;
		push esi;
		mov ebx, 0;

		mov eax, RDRAMPtr;
		and eax, ~3;

UTIOuterLoopStart:
		mov ecx, BytesPerLine;
		xor esi, esi;
		shr ecx, 2;
UTILoopStart:
		mov edx, [eax+esi];
		ror ecx, 5;
		rol edx, cl;
		rol ecx, 5;
		rol edx, cl;

		// xor to UTI
		xor ebx, edx;
		add esi, CRCSkip*4;
		sub ecx, CRCSkip;
		jg UTILoopStart;

		add eax, CRCPitch;
		sub DWORD PTR [TempHeight], 1;
		jg UTIOuterLoopStart;

		mov DWORD PTR [UTI], ebx;
		pop esi;
		pop ebx;
	}
/*
	U32 _tSize;
	_tSize = BytesPerLine;
	__asm{
		push edi;
		push esi;
		push ebx;
		mov eax, 0ffffffffh;			// crc value
		mov ebx, RDRAMPtr;				// data indexer
		mov edi, TempHeight;
UTILoopOuter:
		and ebx, ~3;
		xor ecx, ecx;			// temp
		mov edx, BytesPerLine;	// counter
		lea esi, CRC32;			// table
UTILoopInner:
		mov cl, [ebx];
		xor cl, al;
		inc ebx;
		shr eax, 8;
		xor eax, [esi+(ecx*4)];
		dec edx;
		jnz UTILoopInner;

		sub ebx, BytesPerLine;
		add ebx, CRCPitch;
		dec edi;
		jnz UTILoopOuter;
		not eax;
		pop ebx;
		pop esi;
		pop edi;
	}
*/

	UTI ^= State.TMEM[Start].RDRAMAddress;
	UTI ^= Start;
	if (State.TMEM[Start].LoadType==_LoadTile){
		UTI ^= State.TMEM[Start].StartX;
		UTI ^= State.TMEM[Start].StartY<<1;
		UTI ^= State.TMEM[Start].Width<<2;
		UTI ^= State.TMEM[Start].Height<<3;
	}
	if (tile->fmt==2){
		if (tile->size==0){
			for (i=0;i<8;i++){
				UTI += Read32(State.TMEM[2048].RDRAMAddress+((U32)tile->palette)*128+(i<<2));
			}
		} else {
			for (i=0;i<128;i++){
				UTI += Read32(State.TMEM[2048].RDRAMAddress+(i<<2));
			}
		}
	}

	UTI &= BitM31; // mask out highest bit, otherwise it would be a prerendered texture

	// Search in cache table for UTI
	if (!Options.bDisableTextureCache){
		if (State.TextureCount){
			for (i=0;i<State.TextureCount;i++){
				if ((State.Textures[i].UTI == UTI) &&
					(State.Textures[i].cms==tile->cms) &&
					(State.Textures[i].cmt==tile->cmt) &&
					(State.Textures[i].masks==tile->masks) &&
					(State.Textures[i].maskt==tile->maskt) &&
					(State.Textures[i].DXTUsed==State.TMEM[Start].DXTUsed) &&
					(State.Textures[i].RDPPitch==RDPPitch) &&
					(State.Textures[i].D3DObject)){
					Index = i;
					break;
				}
			}
		}
	}
	U32 x, y;
#if _DEBUG
	if (Index==0xFFFFFFFF){
		Stats.TexturesCached++;
	} else {
		Stats.TexturesLoadedFromCache++;
	}
#endif	
	if ((Index == 0xFFFFFFFF)){
		// create new D3D Texture and copy data from TMEM
		
		D3DTexture = 0;
		union {
			U8* p8;
			U16* p16;
			U32* p32;
			U64* p64;
		} sourceptr, targetptr;

		U8 TMEMFlipped[TMEMSize];
		// simulate LoadBlock/LoadTile/LoadTLUT for the texture
		// and if it uses a palette repeat that
		SimulateTLoadCommand(Start, TMEMFlipped, Height, RDPPitch, tile->size);
		U32 CIndex;
		U32 c, in, al;  // intensity, alpha
		switch (tile->size){
		case 0: // 4 bit
			switch (tile->fmt){
			case 1: // YUV
				ShowError("Invalid texture format: 4 bit YUV");
				break;
			case 0: // RGBA
			case 2: // CI
				SimulateTLoadCommand(2048 + ((U32)tile->palette)*128, TMEMFlipped, 1, RDPPitch, tile->size);
				switch (State.Othermode.fields.textlut){
				case 0:
				case 2: // RGBA 16
					Format = D3DFMT_A1R5G5B5;
					SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
					SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
					SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
					targetptr.p16 = (U16*)Rect.pBits;
					sourceptr.p8 = &TMEMFlipped[Start];
					U16 Col;
					if ((Start&3) || (Width&7)){
						//ShowError("Unaligned load 4 bit ci texture");
					}
					for (y=0;y<Height;y++){
						for (x=0;x<((Width+1)>>1);x++){
							CIndex = (sourceptr.p8[x^3]>>4)&BitM4;
							Col = *((U16*)&TMEMFlipped[2048 + (((U32)tile->palette)*128) + ((CIndex<<1)^2)]);
							//Col = (CIndex<<2)|(CIndex<<7)|(CIndex<<12)|1;
							targetptr.p16[(x<<1)] = ((Col>>1)&BitM15) | ((Col&BitM1)<<15);
						
							CIndex = sourceptr.p8[x^3]&BitM4;
							Col = *((U16*)&TMEMFlipped[2048 + (((U32)tile->palette)*128) + ((CIndex<<1)^2)]);
							//Col = (CIndex<<2)|(CIndex<<7)|(CIndex<<12)|1;
							targetptr.p16[(x<<1)+1] = ((Col>>1)&BitM15) | ((Col&BitM1)<<15);
						}
						targetptr.p8+=Rect.Pitch;
						sourceptr.p8+=RDPPitch;
					}
					break;
				case 3:  // IA 16
					Format = D3DFMT_A8R8G8B8;
					SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
					SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
					SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
					targetptr.p16 = (U16*)Rect.pBits;
					sourceptr.p8 = &TMEMFlipped[Start];
					if ((Start&3) || (Width&7)){
						//ShowError("Unaligned load 4 bit ci texture");
					}
					for (y=0;y<Height;y++){
						for (x=0;x<((Width+1)>>1);x++){
							CIndex = (sourceptr.p8[x^3]>>4)&BitM4;
							Col = *((U16*)&TMEMFlipped[2048 + (((U32)tile->palette)*128) + ((CIndex<<1)^2)]);
							in = Col>>8;
							al = Col&BitM8;
							targetptr.p32[(x<<1)] = (U32)((al<<24) | (in<<16) | (in<<8) | (in));
						
							CIndex = sourceptr.p8[x^3]&BitM4;
							Col = *((U16*)&TMEMFlipped[2048 + (((U32)tile->palette)*128) + ((CIndex<<1)^2)]);
							in = Col>>8;
							al = Col&BitM8;
							targetptr.p32[(x<<1)+1] = (U32)((al<<24) | (in<<16) | (in<<8) | (in));
						}
						targetptr.p8+=Rect.Pitch;
						sourceptr.p8+=RDPPitch;
					}
					break;
				default:
					ShowError("Unhandled/Invalid texture format: 4 bit CI");
					break;
				}
				break;
			case 3: // IA
				Format = D3DFMT_A1R5G5B5;
				SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
				SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
				SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
				targetptr.p16 = (U16*)Rect.pBits;
				sourceptr.p8 = &TMEMFlipped[Start];
				for (y=0;y<Height;y++){
					for (x=0;x<((Width+1)>>1);x++){
						c = (sourceptr.p8[x^3]>>4);
						al = c&BitM1;
						c>>=1;
						in = (c&BitM2)|((c&BitM3)<<2);
						targetptr.p16[(x<<1)] = (U16)((al<<15) | (in<<10) | (in<<5) | (in));
						
						c = sourceptr.p8[x^3]&BitM4;
						al = c&BitM1;
						c>>=1;
						in = (c&BitM2)|((c&BitM3)<<2);
						targetptr.p16[(x<<1)+1] = (U16)((al<<15) | (in<<10) | (in<<5) | (in));
					}
					targetptr.p8+=Rect.Pitch;
					sourceptr.p8+=RDPPitch;
				}
				break;
			case 4: // I
				Format = D3DFMT_A8R8G8B8;
				SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
				SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
				SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
				targetptr.p32 = (U32*)Rect.pBits;
				sourceptr.p8 = &TMEMFlipped[Start];
				for (y=0;y<Height;y++){
					for (x=0;x<((Width+1)>>1);x++){
						in = ((sourceptr.p8[x^3]>>4)&BitM4) | (((sourceptr.p8[x^3]>>4)&BitM4)<<4);
						targetptr.p32[(x<<1)] = (in<<24) | (in<<16) | (in<<8) | in;

						in = (sourceptr.p8[x^3]&BitM4) | ((sourceptr.p8[x^3]&BitM4)<<4);
						targetptr.p32[(x<<1)+1] = (in<<24) | (in<<16) | (in<<8) | in;
					}
					targetptr.p8+=Rect.Pitch;
					sourceptr.p8+=RDPPitch;
				}
				break;
			}
			break;
		case 1: // 8 bit
			switch (tile->fmt){
			case 1: // YUV
				ShowError("Invalid texture format: 8 bit YUV");
				break;
			case 0: // RGBA
			case 2: // CI
				SimulateTLoadCommand(2048, TMEMFlipped, 1, RDPPitch, tile->size);
				switch (State.Othermode.fields.textlut){
				case 0:
				case 2: // RGBA 16
					Format = D3DFMT_A1R5G5B5;
					SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
					SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
					SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
					targetptr.p16 = (U16*)Rect.pBits;
					sourceptr.p8 = &TMEMFlipped[Start];
					U16 Col;
					for (y=0;y<Height;y++){
						for (x=0;x<Width;x++){
							CIndex = sourceptr.p8[x^3];
							Col = *((U16*)&TMEMFlipped[2048 + ((CIndex<<1)^2)]);
							//CIndex >>= 3;
							//Col = (CIndex<<1)|(CIndex<<6)|(CIndex<<11)|1;
							targetptr.p16[x] = ((Col>>1)&BitM15) | ((Col&BitM1)<<15);
						}
						targetptr.p8+=Rect.Pitch;
						sourceptr.p8+=RDPPitch;
					}
					break;
				case 3:  // IA 16
					Format = D3DFMT_A8R8G8B8;
					SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
					SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
					SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
					targetptr.p32 = (U32*)Rect.pBits;
					sourceptr.p8 = &TMEMFlipped[Start];
					U16 Col16;
					for (y=0;y<Height;y++){
						for (x=0;x<Width;x++){
							CIndex = sourceptr.p8[x^3];
							Col16 = *((U16*)&_RDRAM[State.TMEM[2048].RDRAMAddress + ((CIndex<<1)^2)]);
							in = Col16>>8;
							al = Col16&BitM8;
							targetptr.p32[x] = (U32)((al<<24) | (in<<16) | (in<<8) | (in));
						}
						targetptr.p8+=Rect.Pitch;
						sourceptr.p8+=RDPPitch;
					}
					break;
				default:
					ShowError("Invalid texture format: 8 bit CI with invalid texture palette");
					break;
				}
				break;
			case 3: // IA
				Format = D3DFMT_A4R4G4B4;
				SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
				SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
				SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
				targetptr.p16 = (U16*)Rect.pBits;
				sourceptr.p8 = &TMEMFlipped[Start];
				for (y=0;y<Height;y++){
					for (x=0;x<Width;x++){
						in = sourceptr.p8[x^3]>>4;
						al = sourceptr.p8[x^3]&BitM4;
						targetptr.p16[x] = (U16)((al<<12) | (in<<8) | (in<<4) | in);
					}
					targetptr.p8+=Rect.Pitch;
					sourceptr.p8+=RDPPitch;
				}
				break;
			case 4: // I
				Format = D3DFMT_A8R8G8B8;
				SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
				SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
				SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
				targetptr.p32 = (U32*)Rect.pBits;
				sourceptr.p8 = &TMEMFlipped[Start];
				for (y=0;y<Height;y++){
					for (x=0;x<Width;x++){
						in = sourceptr.p8[x^3];
						targetptr.p32[x] = (in<<24) | (in<<16) | (in<<8) | in;
					}
					targetptr.p8+=Rect.Pitch;
					sourceptr.p8+=RDPPitch;
				}
				break;
			}
			break;
		case 2: // 16 bit
			switch (tile->fmt){
			case 0: // RGBA
				Format = D3DFMT_A1R5G5B5;
				SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
				SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
				SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
				targetptr.p16 = (U16*)Rect.pBits;
				sourceptr.p8 = &TMEMFlipped[Start];
				for (y=0;y<Height;y++){
					for (x=0;x<Width;x++){
						targetptr.p16[x] = ((sourceptr.p16[x^1]>>1)&BitM15) | ((sourceptr.p16[x^1]&BitM1)<<15);
					}
					targetptr.p8+=Rect.Pitch;
					sourceptr.p8+=RDPPitch;
				}
				break;
			case 1: // YUV
				ShowError("Unhandled texture format: 16 bit YUV");
				break;
			case 2: // CI
				ShowError("Invalid texture format: 16 bit CI");
				break;
			case 3: // IA
				Format = D3DFMT_A8R8G8B8;
				SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
				SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
				SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
				targetptr.p32 = (U32*)Rect.pBits;
				sourceptr.p8 = &TMEMFlipped[Start];
				for (y=0;y<Height;y++){
					for (x=0;x<Width;x++){
						in = sourceptr.p16[x^1]>>8;
						al = sourceptr.p16[x^1]&BitM8;
						targetptr.p32[x] = (U32)((al<<24) | (in<<16) | (in<<8) | (in));
					}
					targetptr.p8+=Rect.Pitch;
					sourceptr.p8+=RDPPitch;
				}
				break;
			case 4: // I
				ShowError("Invalid texture format: 16 bit Intensity");
				break;
			}
			break;
		case 3: // 32 bit
			switch (tile->fmt){
			case 0: // RGBA
				Format = D3DFMT_A8R8G8B8;
				SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, Format, D3DPOOL_MANAGED, &D3DTexture, NULL));
				SafeDX(D3DTexture->GetSurfaceLevel(0, &D3DSurface0));
				SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
				targetptr.p32 = (U32*)Rect.pBits;
				sourceptr.p8 = &TMEMFlipped[Start];
				for (y=0;y<Height;y++){
					for (x=0;x<Width;x++){
						targetptr.p32[x] = RGBA_to_ARGB(sourceptr.p32[x]);
					}
					targetptr.p8+=Rect.Pitch;
					sourceptr.p8+=RDPPitch;
				}
				break;
			case 1: // YUV
				ShowError("Invalid texture format: 32 bit YUV");
				break;
			case 2: // CI
				ShowError("Invalid texture format: 32 bit CI");
				break;
			case 3: // IA
				ShowError("Invalid texture format: 32 bit intensity alpha");
				break;
			case 4: // I
				ShowError("Invalid texture format: 32 bit intensity");
				break;
			}
			break;
		}
		if (D3DTexture){
			if ((S32)D3DWidth>(S32)Width){
				// texture was changed to a POW2 texture. To improve stretching and allow clamping fill clamp the tex
				targetptr.p32 = (U32*)Rect.pBits;
				sourceptr.p8 = &TMEMFlipped[Start];
				if ((tile->masks!=0) && (tile->cms==3)){
					// mirror texture to boundary
					switch (Format){
					case D3DFMT_A8R8G8B8:
						for (y=0;y<Height;y++){
							for (x=Width;x<D3DWidth;x++){
								targetptr.p32[x] = targetptr.p32[Width-x+Width-1];
							}
							targetptr.p8+=Rect.Pitch;
						}
						break;
					case D3DFMT_A1R5G5B5:
					case D3DFMT_A4R4G4B4:
						for (y=0;y<Height;y++){
							for (x=Width;x<D3DWidth;x++){
								targetptr.p16[x] = targetptr.p16[Width-x+Width-1];
							}
							targetptr.p8+=Rect.Pitch;
						}
						break;
					default:
						ShowError("Stretch Error: Unhandled format in s");
						break;
					}
				} else {
					switch (Format){
					case D3DFMT_A8R8G8B8:
						for (y=0;y<Height;y++){
							U32 Col;
							Col = targetptr.p32[Width-1];
							for (x=Width;x<D3DWidth;x++){
								targetptr.p32[x] = Col;
							}
							targetptr.p8+=Rect.Pitch;
						}
						break;
					case D3DFMT_A1R5G5B5:
					case D3DFMT_A4R4G4B4:
						for (y=0;y<Height;y++){
							U16 Col16;
							Col16 = targetptr.p16[Width-1];
							for (x=Width;x<D3DWidth;x++){
								targetptr.p16[x] = Col16;
							}
							targetptr.p8+=Rect.Pitch;
						}
						break;
					default:
						ShowError("Stretch Error: Unhandled format in s");
						break;
					}
				}
			}
			if ((S32)D3DHeight>(S32)Height){
				// texture was changed to a POW2 texture. To improve stretching and allow clamping fill clamp the tex
				targetptr.p32 = (U32*)Rect.pBits;
				sourceptr.p8 = &TMEMFlipped[Start];
				U32 BytesPerLine;
				switch (Format){
				case D3DFMT_A8R8G8B8:
					BytesPerLine = D3DWidth<<2;
					break;
				case D3DFMT_A1R5G5B5:
				case D3DFMT_A4R4G4B4:
					BytesPerLine = D3DWidth<<1;
					break;
				default:
					ShowError("Stretch Error: Unhandled format in t");
					break;
				}
				if ((tile->maskt!=0) && (tile->cmt==3)){
					// mirror texture to boundary
					for (y=Height;y<D3DHeight;y++){
						memcpy(&targetptr.p8[Rect.Pitch*y], &targetptr.p8[Rect.Pitch*(Height-y+Height-1)], BytesPerLine);
					}
				} else {
					for (y=Height;y<D3DHeight;y++){
						memcpy(&targetptr.p8[Rect.Pitch*y], &targetptr.p8[Rect.Pitch*(Height-1)], BytesPerLine);
					}
				}
			}
			SafeDX(D3DSurface0->UnlockRect());
			FreeObject(D3DSurface0);
		} else {
			// unknown texture type
			// load black default texture
			SafeDX(g_pd3dDevice->CreateTexture(D3DWidth, D3DHeight, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &D3DTexture, NULL));
		}
		if (State.TextureCount >= (MaxTextureCount-5)){
			// Delete oldest texture
			U32 tindex, oldestindex;
			oldestindex = 0;
			for (tindex=1;tindex<State.TextureCount;tindex++){
				if (State.Textures[tindex].Age>=State.Textures[oldestindex].Age){
					oldestindex = tindex;
				}
			}
			FreeObject(State.Textures[oldestindex].D3DObject);
			// We know the oldest texture, now replace it
			Index = oldestindex;
		} else {
			Index = State.TextureCount;
			State.TextureCount++;
		}
		State.Textures[Index].UTI = UTI;
		State.Textures[Index].D3DObject = D3DTexture;
		State.Textures[Index].RDPFormat = tile->size | (tile->fmt<<2);
		State.Textures[Index].Width = Width;
		State.Textures[Index].Height = Height;
		State.Textures[Index].ByLoadBlock = State.LastLoadBlock;
		State.Textures[Index].DXTUsed = State.TMEM[Start].DXTUsed;
		State.Textures[Index].RDPPitch = RDPPitch;
		State.Textures[Index].cms = tile->cms;
		State.Textures[Index].cmt = tile->cmt;
		State.Textures[Index].masks = tile->masks;
		State.Textures[Index].maskt = tile->maskt;
	}
	State.Textures[Index].Age = 0;
	State.LastTextureIndex = Index;
	State.LastTile = tile;
	return Index;
}

void SetZBufferWrite()
{
	bool bUseZ;
	if ((State.Othermode.fields.rendermodezupd!=0) != State.ZBufferWrite){
		State.ZBufferWrite = State.Othermode.fields.rendermodezupd!=0;
		SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, State.ZBufferWrite));
	}
	if (Options.bForceZBuffer){
		bUseZ = true;
	} else {
		bUseZ = State.Othermode.fields.rendermodezcmp!=0;
	}
	if (bUseZ != State.ZEnabled){
		State.ZEnabled = bUseZ;
		SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, State.ZEnabled));
	}
/*
	if ((State.Othermode.fields.alphacompare==0) != State.AlphaTest){
		State.AlphaTest = State.Othermode.fields.alphacompare==0;
		SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, State.AlphaTest));
	}
*/
}

void HandleTextures(bool TextureEnabled, TTile *tile)
{
	if (TextureEnabled){
		// check which textures are used
		if ((State.Othermode.fields.cycletype==2)||(State.Othermode.fields.cycletype==3)){
			// Fill mode
			UseTexture1 = true;
			UseTexture2 = false;
		} else {
			if (State.CombineChanged || State.TextureChanged){
				if (State.Othermode.fields.cycletype<=1){
					UseTexture1 =	(State.Combine.fields.a0==A_Tex0Color) ||
									(State.Combine.fields.b0==B_Tex0Color) ||
									(State.Combine.fields.c0==C_Tex0Color) ||
									(State.Combine.fields.c0==C_Tex0Alpha) ||
									(State.Combine.fields.d0==D_Tex0Color) ||
									(State.Combine.fields.Aa0==AA_Tex0Alpha) ||
									(State.Combine.fields.Ab0==BA_Tex0Alpha) ||
									(State.Combine.fields.Ac0==CA_Tex0Alpha) ||
									(State.Combine.fields.Ad0==DA_Tex0Alpha);
					UseTexture2 =	(State.Combine.fields.a0==A_Tex1Color) ||
									(State.Combine.fields.b0==B_Tex1Color) ||
									(State.Combine.fields.c0==C_Tex1Color) ||
									(State.Combine.fields.c0==C_Tex1Alpha) ||
									(State.Combine.fields.d0==D_Tex1Color) ||
									(State.Combine.fields.Aa0==AA_Tex1Alpha) ||
									(State.Combine.fields.Ab0==BA_Tex1Alpha) ||
									(State.Combine.fields.Ac0==CA_Tex1Alpha) ||
									(State.Combine.fields.Ad0==DA_Tex1Alpha);
					if (State.Othermode.fields.cycletype==1){
						// 2 Cycle
						UseTexture1 |=	(State.Combine.fields.a1==A_Tex0Color) ||
										(State.Combine.fields.b1==B_Tex0Color) ||
										(State.Combine.fields.c1==C_Tex0Color) ||
										(State.Combine.fields.c1==C_Tex0Alpha) ||
										(State.Combine.fields.d1==D_Tex0Color) ||
										(State.Combine.fields.Aa1==AA_Tex0Alpha) ||
										(State.Combine.fields.Ab1==BA_Tex0Alpha) ||
										(State.Combine.fields.Ac1==CA_Tex0Alpha) ||
										(State.Combine.fields.Ad1==DA_Tex0Alpha);
						UseTexture2 |=	(State.Combine.fields.a1==A_Tex1Color) ||
										(State.Combine.fields.b1==B_Tex1Color) ||
										(State.Combine.fields.c1==C_Tex1Color) ||
										(State.Combine.fields.c1==C_Tex1Alpha) ||
										(State.Combine.fields.d1==D_Tex1Color) ||
										(State.Combine.fields.Aa1==AA_Tex1Alpha) ||
										(State.Combine.fields.Ab1==BA_Tex1Alpha) ||
										(State.Combine.fields.Ac1==CA_Tex1Alpha) ||
										(State.Combine.fields.Ad1==DA_Tex1Alpha);
					}
				} else {
					UseTexture1 = false;
					UseTexture2 = false;
				}
			}
		}
		if (UseTexture1){
			Texture1 = CacheTexture(tile);
			/*
			if (tile->masks==0){
				State.Address1U = D3DTADDRESS_CLAMP;
			} else {
				switch (tile->cms){
				case 0: // wrap
					State.Address1U = D3DTADDRESS_WRAP;
					break;
				case 1: // mirror
					State.Address1U = D3DTADDRESS_MIRROR;
					break;
				case 2: // clamp (wrap/mirror to coordinates is done in software)
				case 3:
					State.Address1U = D3DTADDRESS_CLAMP;
					break;
				}
			}
			if (tile->maskt==0){
				State.Address1V = D3DTADDRESS_CLAMP;
			} else {
				switch (tile->cmt){
				case 0: // wrap
					State.Address1V = D3DTADDRESS_WRAP;
					break;
				case 1: // mirror
					State.Address1V = D3DTADDRESS_MIRROR;
					break;
				case 2: // clamp (wrap/mirror to coordinates is done in software)
				case 3:
					State.Address1V = D3DTADDRESS_CLAMP;
					break;
				}
			}
			*/
			State.Address1U = tile->D3DAddressU;
			State.Address1V = tile->D3DAddressV;
		}
		if (UseTexture2){
			TTile *tile2;
			if (State.Othermode.fields.textlod){
				// don't support detail/sharpen textures for now
				Texture2 = Texture1;
				tile2 = tile;
			} else {
				tile2 = tile+1;
				Texture2 = CacheTexture(tile2);
			}
			State.Address2U = tile2->D3DAddressU;
			State.Address2V = tile2->D3DAddressV;
/*
			if (tile2->masks==0){
				State.Address2U = D3DTADDRESS_CLAMP;
			} else {
				switch (tile2->cms){
				case 0: // wrap
					State.Address2U = D3DTADDRESS_WRAP;
					break;
				case 1: // mirror
					State.Address2U = D3DTADDRESS_MIRROR;
					break;
				case 2: // clamp (mirroring/clamping to coordinates is done in software)
				case 3:
					State.Address2U = D3DTADDRESS_CLAMP;
					break;
				}
			}
			if (tile2->maskt==0){
				State.Address2V = D3DTADDRESS_CLAMP;
			} else {
				switch (tile2->cmt){
				case 0: // wrap
					State.Address2V = D3DTADDRESS_WRAP;
					break;
				case 1: // mirror
					State.Address2V = D3DTADDRESS_MIRROR;
					break;
				case 2: // clamp (mirroring/clamping to coordinates is done in software)
				case 3:
					State.Address2V = D3DTADDRESS_CLAMP;
					break;
				}
			}
*/
		}
		switch (State.Othermode.fields.textfilt){
		case 0: // Point
			State.TextureFilter = D3DTEXF_POINT;
			break;
		case 2: // Bilerp
			State.TextureFilter = D3DTEXF_LINEAR;
			break;
		case 3: // Average
			State.TextureFilter = D3DTEXF_LINEAR;
			break;
		}
	} else {
		UseTexture1 = false;
		UseTexture2 = false;
	}
}

void OthermodeChanged()
{
	bool NewDecalMode;
	NewDecalMode = (State.Othermode.fields.rendermodezmode==3);
	if (DeviceCaps.RasterCaps&D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS){
		float bias;
		float slope;
		if (NewDecalMode){
			bias = -0.001f;
			slope = 1.0f;
		} else {
			bias = 0.0f;
			slope = 0.0f;
		}
		SafeDX(g_pd3dDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *((DWORD*)&(slope))));
		SafeDX(g_pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *((DWORD*)&(bias))));
	} else {
		if (NewDecalMode && !State.DecalMode){
			// turn it on
			State.DecalMode = true;
			State.ProjectionChanged = true;
		} else if (!NewDecalMode && State.DecalMode){
			State.DecalMode = false;
			State.ProjectionChanged = true;
		}
	}
	State.TextureChanged = true;


/*
	if (State.Othermode.fields.zsrcsel){
		ShowError("Unsupported: Prim Src Z");
	}
*/
	U32 NewBlendMode;
	NewBlendMode = State.Othermode.raw.lo;// & BitM32;
	if (State.BlendMode != NewBlendMode){
		State.BlendMode = NewBlendMode;
	}
}

void SendTrisToD3D(TTile *Tile, bool TexturesEnabled, D3DPRIMITIVETYPE BufferType, bool NoCombine)
{
	U32 i;
	if (!NoCombine){
		HandleTextures(TexturesEnabled, Tile);
		ConfigureBlender1();
		ConfigureCombiner(TexturesEnabled);
		ConfigureBlender2();
		if (ChangeTextureCoordinates){
			// Tex1 used, change texture coordinates of the correct stages
			U32 j;
			for (i=0;i<_TEXTURES;i++){
				if (SecondTex[i]){
					for (j=0;j<State.D3DVertexCacheIndex;j++){
						State.D3DVertexCache[j].tex[i].u *= (float)Tile->D3DWidth;
						State.D3DVertexCache[j].tex[i].u /= ShiftTable[Tile->shifts];
						State.D3DVertexCache[j].tex[i].u += (Tile)->uls;
						State.D3DVertexCache[j].tex[i].u -= (Tile+1)->uls;
						State.D3DVertexCache[j].tex[i].u *= ShiftTable[(Tile+1)->shifts];
						State.D3DVertexCache[j].tex[i].u /= (float)(Tile+1)->D3DWidth;
					
						State.D3DVertexCache[j].tex[i].v *= (float)Tile->D3DHeight;
						State.D3DVertexCache[j].tex[i].v /= ShiftTable[Tile->shiftt];
						State.D3DVertexCache[j].tex[i].v += (Tile)->ult;
						State.D3DVertexCache[j].tex[i].v -= (Tile+1)->ult;
						State.D3DVertexCache[j].tex[i].v *= ShiftTable[(Tile+1)->shiftt];
						State.D3DVertexCache[j].tex[i].v /= (float)(Tile+1)->D3DHeight;
					}
				}
			}
		}
	}
#if 0
	if ((State.Othermode.fields.cycletype==1) && (State.GeometryMode.fields.Fog)){
		// Do Fog
		if ((State.Othermode.raw.lo & 0xFFFF0000) == BuildBlendMode2(SHADE, FOG, INVA, COMB, COMB, COMB, MEM, MEM)||
			(State.Othermode.raw.lo & 0xFFFF0000) == BuildBlendMode2(SHADE, FOG, INVA, COMB, COMB, COMB, INVA, MEM)){
			float Start, End;//, Density;
			//Density = 0.001f;
			Start = State.FogStart;
			End = State.FogEnd;
			// Fog Vertex Mode seems to not care about the camera position - so I'll use table mode
			//SafeDX(g_pd3dDevice->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)(&Density)));
			//SafeDX(g_pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE));
			SafeDX(g_pd3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&Start)));
			SafeDX(g_pd3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)(&End)));
			SafeDX(g_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR));
			SafeDX(g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE));
			SafeDX(g_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, State.FogColor));
		} else {
			SafeDX(g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE));
		}
	}
#endif
#if 0
	if (bHighlighted){
		State.D3DVertexCacheIndex = 0;
		return;
	}
#endif
	D3DTNTVERTEX *VertexBuffer;
	SafeDX(g_VertexBuffer->Lock(0, State.D3DVertexCacheIndex*sizeof(D3DTNTVERTEX), (void**)&VertexBuffer, 0));
	memcpy(VertexBuffer, State.D3DVertexCache, State.D3DVertexCacheIndex*sizeof(D3DTNTVERTEX));
	SafeDX(g_VertexBuffer->Unlock());
	U32 TriCount;
	switch (BufferType){
	case D3DPT_TRIANGLELIST: // normal tris
		TriCount = State.D3DVertexCacheIndex/3;
		break;
	case D3DPT_TRIANGLESTRIP:
		TriCount = State.D3DVertexCacheIndex-2;
		break;
	}
	for (i=0;i<BlendPasses.Count;i++){
		SetBlendPass(i);
		SafeDX(g_pd3dDevice->DrawPrimitive(BufferType, 0, TriCount));
	}
	State.D3DVertexCacheIndex = 0;
}

COLORREF GetSurfacePixel(D3DFORMAT Format, D3DLOCKED_RECT Rect, int x, int y){
	union {
		U8* p8;
		U16* p16;
		U32* p32;
		U64* p64;
	} ptr;

	ptr.p8 = (U8*)Rect.pBits;
	ptr.p8 += y*Rect.Pitch;
	switch (Format){
	case D3DFMT_A8R8G8B8:
		ptr.p32+=x;
		return (((*ptr.p32) & 0xFF000000) >> 24) | (((*ptr.p32) & 0x00FFFFFF) << 8);
	case D3DFMT_A1R5G5B5:
		ptr.p16+=x;
		return ((*ptr.p16)&Bit16?0x0:0xFF) | (BITS((U32)(*ptr.p16), 10, 5)<<(24+3)) | (BITS((U32)(*ptr.p16), 5, 5)<<(16+3)) | (BITS((U32)*(ptr.p16), 0, 5)<<(8+3));
	case D3DFMT_A4R4G4B4:
		ptr.p16+=x;
		return (BITS((U32)(*ptr.p16), 12, 4)<<(24+4)) | (BITS((U32)(*ptr.p16), 8, 4)<<(24+4)) | (BITS((U32)(*ptr.p16), 4, 4)<<(16+4)) | (BITS((U32)*(ptr.p16), 0, 4)<<(8+4));
	default:
		return 0;
	}
}

void SaveVRMLTexture(U32 uTextureIndex, U32 *iWidth, U32 *iHeight, bool bIsTexture1){
	D3DLOCKED_RECT Rect;
	D3DSURFACE_DESC Desc;
	char sFilenameColor[256];
	char sFilenameAlpha[256];
	FILE *fColor, *fAlpha;
	BITMAPFILEHEADER bitmapfileheader;
	BITMAPINFOHEADER bitmapinfoheader;
#if VRMLInline
	if (bIsTexture1){
		// texture
		fprintf(fVRML, "				texture PixelTexture {\n");
		IDirect3DSurface9 *Surface;
		SafeDX(State.Textures[uTextureIndex].D3DObject->GetSurfaceLevel(0, &Surface));
		SafeDX(Surface->GetDesc(&Desc));
		SafeDX(Surface->LockRect(&Rect, 0, 0));

		fprintf(fVRML, "					image %i %i 4 \n", State.Textures[uTextureIndex].Width, State.Textures[uTextureIndex].Height);
		for (U32 y = 0;y<State.Textures[uTextureIndex].Height;y++){
			fprintf(fVRML, "					");
			for (U32 x = 0;x<State.Textures[uTextureIndex].Width;x++){
				COLORREF c = GetSurfacePixel(Desc.Format, Rect, x, y);
				fprintf(fVRML, "%i ", c);
			}
			fprintf(fVRML, "\n");
		}

		SafeDX(Surface->UnlockRect());
		SafeDX(Surface->Release());
		fprintf(fVRML, "					repeatS TRUE\n");
		fprintf(fVRML, "					repeatT TRUE\n");
		fprintf(fVRML, "				}\n");
	}
#else
	if (bIsTexture1){
		// texture
		fprintf(fVRML, "				texture ImageTexture {\n");
		fprintf(fVRML, "				    url \"%X_c.bmp\"\n", State.Textures[uTextureIndex].UTI);
		fprintf(fVRML, "					repeatS TRUE\n");
		fprintf(fVRML, "					repeatT TRUE\n");
		fprintf(fVRML, "				}\n");
	}
	
	sprintf(sFilenameColor, "vrml\\%X_c.bmp", State.Textures[uTextureIndex].UTI);
	sprintf(sFilenameAlpha, "vrml\\%X_a.bmp", State.Textures[uTextureIndex].UTI);
	IDirect3DSurface9 *Surface;
	fColor = fopen(sFilenameColor, "wb");
	fAlpha = fopen(sFilenameAlpha, "wb");
	
	// Write both file headers
	bitmapfileheader.bfSize = sizeof(BITMAPFILEHEADER);
	bitmapfileheader.bfType = 'MB';  // reverse endian of "BM"
	bitmapfileheader.bfReserved1 = 0;
	bitmapfileheader.bfReserved2 = 0;
	bitmapfileheader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + State.Textures[uTextureIndex].Width*State.Textures[uTextureIndex].Height*3;
	bitmapfileheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	fwrite(&bitmapfileheader, sizeof(BITMAPFILEHEADER), 1, fColor);
	fwrite(&bitmapfileheader, sizeof(BITMAPFILEHEADER), 1, fAlpha);

	// Write both info headers
	bitmapinfoheader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfoheader.biWidth = State.Textures[uTextureIndex].Width;
	bitmapinfoheader.biHeight = State.Textures[uTextureIndex].Height;
	bitmapinfoheader.biPlanes = 1;
	bitmapinfoheader.biBitCount = 24;
	bitmapinfoheader.biCompression = BI_RGB;
	bitmapinfoheader.biSizeImage = 0;
	bitmapinfoheader.biXPelsPerMeter = 72;
	bitmapinfoheader.biYPelsPerMeter = 72;
	bitmapinfoheader.biClrUsed = 0;
	bitmapinfoheader.biClrImportant = 0;
	fwrite(&bitmapinfoheader, sizeof(BITMAPINFOHEADER), 1, fColor);
	fwrite(&bitmapinfoheader, sizeof(BITMAPINFOHEADER), 1, fAlpha);

	SafeDX(State.Textures[uTextureIndex].D3DObject->GetSurfaceLevel(0, &Surface));
	SafeDX(Surface->GetDesc(&Desc));
	SafeDX(Surface->LockRect(&Rect, 0, 0));

	for (U32 y = 0;y<State.Textures[uTextureIndex].Height;y++){
		for (U32 x = 0;x<State.Textures[uTextureIndex].Width;x++){
			COLORREF c = GetSurfacePixel(Desc.Format, Rect, x, y);
			COLORREF aonly, conly;
			conly = c>>8;
			aonly = (c&BitM8);
			//aonly = 255-aonly;
			aonly = RGB(aonly, aonly, aonly);
			fwrite(&conly, 3, 1, fColor);
			fwrite(&aonly, 3, 1, fAlpha);
		}

		// zero pad to 32 bit boundary
		int iPad = (State.Textures[uTextureIndex].Width*3) & BitM2;
		if (iPad!=0){
			U32 null = 0;
			fwrite(&null, 4-iPad, 1, fColor);
			fwrite(&null, 4-iPad, 1, fAlpha);
		}
	}

	SafeDX(Surface->UnlockRect());
	SafeDX(Surface->Release());
	fclose(fColor);
	fclose(fAlpha);
#endif
	*iWidth = Desc.Width;
	*iHeight = Desc.Height;
}

void FlushVertexCache()
{
	if (State.D3DVertexCacheIndex){
		if (Options.bVRMLExport){
			U32 i;
			U32 iVRMLWidth, iVRMLHeight, iVRMLWidth2, iVRMLHeight2;
			fprintf(fVRML, "		Shape {\n");
			fprintf(fVRML, "			appearance DEF NemuMat%i Appearance {\n", uVRMLFaceIndex);
			fprintf(fVRML, "				material Material {\n");
			fprintf(fVRML, "					ambientIntensity 1\n");
			fprintf(fVRML, "					diffuseColor ");
			// put in diffuse color of first point
			fprintf(fVRML, "%f %f %f\n",
				BITS(State.D3DVertexCache[0].color, 16, 8)/256.0f,
				BITS(State.D3DVertexCache[0].color,  8, 8)/256.0f,
				BITS(State.D3DVertexCache[0].color,  0, 8)/256.0f);
			fprintf(fVRML, "					specularColor 0 0 0\n");
			fprintf(fVRML, "					emissiveColor 0 0 0\n");
			fprintf(fVRML, "					shininess 0.0\n");
			fprintf(fVRML, "					transparency 0\n");
			fprintf(fVRML, "				}\n");

			iVRMLWidth = 0;
			iVRMLHeight = 0;
			iVRMLWidth2 = 0;
			iVRMLHeight2 = 0;
			HandleTextures(State.GeometryMode.fields.TextureEnable, State.texture.tile);
			if (UseTexture1){
				SaveVRMLTexture(Texture1, &iVRMLWidth, &iVRMLHeight, true);
			}
			if (UseTexture2){
				SaveVRMLTexture(Texture2, &iVRMLWidth2, &iVRMLHeight2, !UseTexture1);
			}
			fprintf(fVRML, "			}\n");

			fprintf(fVRML, "			geometry DEF Nemu_%X_%X_%i IndexedFaceSet {\n",
						(UseTexture1?State.Textures[Texture1].UTI:0), 
						(UseTexture2?State.Textures[Texture2].UTI:0),
						uVRMLFaceIndex);
			fprintf(fVRML, "				ccw FALSE\n");
			fprintf(fVRML, "				solid TRUE\n");
			fprintf(fVRML, "				coord Coordinate{\n");
			fprintf(fVRML, "					point [\n");
			for (i=0;i<State.D3DVertexCacheIndex;i++){
				fprintf(fVRML, "						%3.8f %3.8f %3.8f", State.D3DVertexCache[i].x/1000.0f, State.D3DVertexCache[i].y/1000.0f, State.D3DVertexCache[i].z/1000.0f);
				if (i<State.D3DVertexCacheIndex-1){
					fprintf(fVRML, ",");
				}
				fprintf(fVRML, "\n");
			}
			fprintf(fVRML, "					]\n");
			fprintf(fVRML, "				}\n");

			// Coordinate Indices...
			fprintf(fVRML, "				coordIndex [\n");
			for (i=0;i<State.D3DVertexCacheIndex;i+=3){
				fprintf(fVRML, "					%i, %i, %i, -1", i, i+1, i+2);
				if (i<State.D3DVertexCacheIndex-4){
					fprintf(fVRML, ",");
				}
				fprintf(fVRML, "\n");
			}
			fprintf(fVRML, "				]\n");

			if (UseTexture1||UseTexture2){
				// Texture coordinates...
				fprintf(fVRML, "				texCoord TextureCoordinate {\n");
				fprintf(fVRML, "					point [\n");
				for (i=0;i<State.D3DVertexCacheIndex;i++){
					float u, v;
					u = State.D3DVertexCache[i].tex[0].u*State.Textures[Texture1].Width;
					v = State.D3DVertexCache[i].tex[0].v*State.Textures[Texture1].Height;
					if (iVRMLWidth){
						u /= (S32)iVRMLWidth;
					}
					if (iVRMLHeight){
						v /= (S32)iVRMLHeight;
					}
					fprintf(fVRML, "						%3.8f %3.8f", u, v);

					if (i<State.D3DVertexCacheIndex-1){
						fprintf(fVRML, ",");
					}
					fprintf(fVRML, "\n");
				}
				fprintf(fVRML, "					]\n");
				fprintf(fVRML, "				}\n");

				// Texture Coordinate Indices...
				fprintf(fVRML, "				texCoordIndex [\n");
				for (i=0;i<State.D3DVertexCacheIndex;i+=3){
					fprintf(fVRML, "					%i, %i, %i, -1", i, i+1, i+2);
					if (i<State.D3DVertexCacheIndex-4){
						fprintf(fVRML, ",");
					}
					fprintf(fVRML, "\n");
				}
				fprintf(fVRML, "				]\n");
			} else {
				// Color...
				fprintf(fVRML, "				colorPerVertex TRUE\n");
				fprintf(fVRML, "				color Color {\n");
				fprintf(fVRML, "					color [ \n");
				for (i=0;i<State.D3DVertexCacheIndex;i++){
					fprintf(fVRML, "						%f %f %f",
						BITS(State.D3DVertexCache[i].color, 16, 8)/256.0f,
						BITS(State.D3DVertexCache[i].color,  8, 8)/256.0f,
						BITS(State.D3DVertexCache[i].color,  0, 8)/256.0f);
					if (i<State.D3DVertexCacheIndex-1){
						fprintf(fVRML, ",");
					}
					fprintf(fVRML, "\n");
				}
				fprintf(fVRML, "					]\n");
				fprintf(fVRML, "				}\n");

				// Color Coordinate Indices...
				fprintf(fVRML, "				colorIndex [\n");
				for (i=0;i<State.D3DVertexCacheIndex;i+=3){
					fprintf(fVRML, "					%i, %i, %i, -1", i, i+1, i+2);
					if (i<State.D3DVertexCacheIndex-4){
						fprintf(fVRML, ",");
					}
					fprintf(fVRML, "\n");
				}
				fprintf(fVRML, "				]\n");
			}
			fprintf(fVRML, "			}\n");
			fprintf(fVRML, "		}\n");
			uVRMLFaceIndex++;
		}
		LogCommand("Rendering %i tris in %s mode\n", State.D3DVertexCacheIndex/3, (State.Othermode.fields.cycletype==0?"1 Cycle":(State.Othermode.fields.cycletype==1?"2 Cycle":(State.Othermode.fields.cycletype==2?"Copy":"Fill"))));
		SetViewport();
		if (State.ProjectionChanged){
			CalcModifiedProjectionMatrix();
#if DoOwnGeometry
			SafeDX(g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &State.Projection));
#else
			if (State.bForceMtx){
				SafeDX(g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &State.TempMatrix));
			} else {
				SafeDX(g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &State.ProjectionModified));
			}
#endif
			State.ProjectionChanged = false;
		}
		SetZBufferWrite();

		SendTrisToD3D(State.texture.tile, State.GeometryMode.fields.TextureEnable, D3DPT_TRIANGLELIST, false);

	}
}

void CopyVtxBuffer(D3DTNTVERTEX *SourceVertex, D3DTNTVERTEX *D3DVertexBuffer, U32 ColorIndex)
{
#if DoOwnGeometry
	D3DMATRIX *Matrix;
	Matrix = &State.Projection;
	FLOAT x, y, z, rhw;
	x = SourceVertex->x;
	y = SourceVertex->y;
	z = SourceVertex->z;

	// apply projection
	D3DVertexBuffer->x   =   (x*Matrix->_11 + y*Matrix->_21 + z*Matrix->_31 + Matrix->_41);
	D3DVertexBuffer->y   =   (x*Matrix->_12 + y*Matrix->_22 + z*Matrix->_32 + Matrix->_42);
	D3DVertexBuffer->z   =   (x*Matrix->_13 + y*Matrix->_23 + z*Matrix->_33 + Matrix->_43);
	rhw                  = 1/(x*Matrix->_14 + y*Matrix->_24 + z*Matrix->_34 + Matrix->_44);

	D3DVertexBuffer->x *=  rhw;
	D3DVertexBuffer->y *=  rhw;
	D3DVertexBuffer->z /=  32767.0f;
	D3DVertexBuffer->rhw = rhw;
	
	// convert to pixel coordinates
	D3DVertexBuffer->x = (( D3DVertexBuffer->x+1) * State.Viewport.Width/2) + State.Viewport.X;
	D3DVertexBuffer->y = ((-D3DVertexBuffer->y+1) * State.Viewport.Height/2) + State.Viewport.Y;
#else
	D3DVertexBuffer->x = SourceVertex->x;
	D3DVertexBuffer->y = SourceVertex->y;
	D3DVertexBuffer->z = SourceVertex->z;
#endif
	
	float Width, Height;
	Width = (float)(State.texture.tile->D3DWidth);
	Height = (float)(State.texture.tile->D3DHeight);
	_TexAssignU(D3DVertexBuffer) = (SourceVertex->tex[0].u-State.texture.tile->uls) * ShiftTable[State.texture.tile->shifts] / Width;
	_TexAssignV(D3DVertexBuffer) = (SourceVertex->tex[0].v-State.texture.tile->ult) * ShiftTable[State.texture.tile->shiftt] / Height;
	// use which vertex?
	//if ((!State.GeometryMode.fields.Lighting) && (!State.GeometryMode.fields.SmoothShading)){
	//	D3DVertexBuffer->color = VertexCache[ColorIndex].color;
	//} else {
		D3DVertexBuffer->color = SourceVertex->color;
	//}
}

// One point is behind the camera, the other two are visible
__inline void Clip1Vtx(D3DTNTVERTEX *Visible1, D3DTNTVERTEX *Visible2, D3DTNTVERTEX *Invisible)
{
	State.D3DVertexCacheIndex-=3;
}

// Two points are behind the camera, the other one is visible
__inline void Clip2Vtx(D3DTNTVERTEX *Visible, D3DTNTVERTEX *Invisible1, D3DTNTVERTEX *Invisible2)
{
	FLOAT f;
	f = - (Visible->z / (Invisible1->z-Visible->z));
	Invisible1->x = Visible->x + f*(Invisible1->x-Visible->x);
	Invisible1->y = Visible->y + f*(Invisible1->y-Visible->y);
	Invisible1->z = 0.0f;

	f = - (Visible->z / (Invisible2->z-Visible->z));
	Invisible2->x = Visible->x + f*(Invisible2->x-Visible->x);
	Invisible2->y = Visible->y + f*(Invisible2->y-Visible->y);
	Invisible2->z = 0.0f;

}


BOOL CImgBad()
{
	if (Options.bCImgTracing){
		// returns false if cimg changed since last time it was first called
		if (State.FirstCImgTriAddress==0){
			State.FirstCImgTriAddress = State.cimg.address;
		} else {
			if (State.FirstCImgTriAddress != State.cimg.address){
				return true;
			}
		}
	}
	return false;
}

void Tri1(U32 v1, U32 v2, U32 v3)
{
	if (CImgBad()){
		return;
	}

	// happens in perfect dark
	if ((v1==v2) && (v2==v3)){
		return;
	}
	// Save tris is vertex cache
	LogCommand("  %i,%i,%i\n", v1, v2, v3);
#if DoOwnGeometry
	FLOAT z1, z2, z3;
	U32 uClipMode; // 0 = send unchanged, 1 = one point behind camera, 2 = two points behind camera(, 3 = clip completely)
	U32 uClipVisible1, uClipVisible2;
	U32 uClipInvisible1, uClipInvisible2;
	
	// Do Clipping - this check is pretty long...
	
	z1 = VertexCache[v1].z;
	z2 = VertexCache[v2].z;
	z3 = VertexCache[v3].z;
	if (z1<0.0f){
		if (z2<0.0f){
			if (z3<0.0f){
				return;
			} else {
				uClipMode = 2;
				uClipInvisible1 = 0;
				uClipInvisible2 = 1;
				uClipVisible1 = 2;
			}
		} else {
			if (z3<0.0f){
				uClipMode = 2;
				uClipInvisible1 = 0;
				uClipInvisible2 = 2;
				uClipVisible1 = 1;
			} else {
				uClipMode = 1;
				uClipInvisible1 = 0;
				uClipVisible1 = 1;
				uClipVisible2 = 2;
			}
		}
	} else {
		if (z2<0.0f){
			if (z3<0.0f){
				uClipMode = 2;
				uClipInvisible1 = 1;
				uClipInvisible2 = 2;
				uClipVisible1 = 0;
			} else {
				uClipMode = 1;
				uClipInvisible1 = 1;
				uClipVisible1 = 0;
				uClipVisible2 = 2;
			}
		} else {
			if (z3<0.0f){
				uClipMode = 1;
				uClipInvisible1 = 2;
				uClipVisible1 = 0;
				uClipVisible2 = 1;
			} else {
				uClipMode = 0;
			}
		}
	}
	if (uClipMode!=0){
		return;
	}
#endif
	CopyVtxBuffer(&VertexCache[v1], &State.D3DVertexCache[State.D3DVertexCacheIndex], v1);
	CopyVtxBuffer(&VertexCache[v2], &State.D3DVertexCache[State.D3DVertexCacheIndex+1], v1);
	CopyVtxBuffer(&VertexCache[v3], &State.D3DVertexCache[State.D3DVertexCacheIndex+2], v1);

/*
#if DoOwnGeometry
	// Clip after the tris have been sent to the D3DVertexCache
	switch (uClipMode){
	case 1:
		Clip1Vtx(&State.D3DVertexCache[State.D3DVertexCacheIndex + uClipVisible1],
			     &State.D3DVertexCache[State.D3DVertexCacheIndex + uClipVisible2],
				 &State.D3DVertexCache[State.D3DVertexCacheIndex + uClipInvisible1]);
		break;
	case 2:
		Clip2Vtx(&State.D3DVertexCache[State.D3DVertexCacheIndex + uClipVisible1],
			     &State.D3DVertexCache[State.D3DVertexCacheIndex + uClipInvisible1],
				 &State.D3DVertexCache[State.D3DVertexCacheIndex + uClipInvisible2]);
		break;
	}
#endif
*/
	State.D3DVertexCacheIndex+=3;
	if (State.D3DVertexCacheIndex>D3DVertexCacheSize-10){
		FlushVertexCache();
	}
}

void S2DEX_Texture(uObjTxtr *Texture)
{
	FlushVertexCache();

	switch (Texture->block.type)
	{
	case G_OBJLT_TXTRBLOCK:
		// Load texture into texture cache
		U32 TMEMAddress;
		U32 RDRAMAddress;
		
		State.timg.address = Segment[(Texture->block.image>>24)&BitM4] + (Texture->block.image&BitM24);
		// Put data from rdram to tmem
		U32 Size, Width;
		Size = (Texture->block.tsize+1)<<3;
		Width = 0;
		TMEMAddress = (Texture->block.tmem)<<3;
		RDRAMAddress = State.timg.address;
		State.TMEM[TMEMAddress].RDRAMAddress = State.timg.address;
		State.TMEM[TMEMAddress].RDRAMStride = Width;
		State.TMEM[TMEMAddress].LoadType = _LoadBlock;
		State.TMEM[TMEMAddress].Size = Size;
		State.TMEM[TMEMAddress].TSize = 2;
		State.TMEM[TMEMAddress].Line = Texture->block.tline;
		State.TMEM[TMEMAddress].StartX = 0;
		State.TMEM[TMEMAddress].StartY = 0;
		State.TMEM[TMEMAddress].DXTUsed = true;
		State.TextureChanged = true;
		State.LastLoadBlock = true;
		break;
	case G_OBJLT_TXTRTILE:
		State.timg.address = Segment[(Texture->tile.image>>24)&BitM4] + (Texture->tile.image&BitM24);
		// Put data from rdram to tmem
		TMEMAddress = (Texture->tile.tmem)<<3;
		RDRAMAddress = State.timg.address;
		
		State.TMEM[TMEMAddress].TMEMStride = 8;
		State.TMEM[TMEMAddress].RDRAMAddress = State.timg.address;
		State.TMEM[TMEMAddress].RDRAMStride = 8;
		State.TMEM[TMEMAddress].LoadType = _LoadTile;
		State.TMEM[TMEMAddress].StartX = 0;
		State.TMEM[TMEMAddress].StartY = 0;
		State.TMEM[TMEMAddress].Width = Texture->tile.twidth;
		State.TMEM[TMEMAddress].Height = Texture->tile.theight;
		State.TMEM[TMEMAddress].TSize = 1;
		State.TMEM[TMEMAddress].Line = 0;
		State.TMEM[TMEMAddress].DXTUsed = true;
		State.TextureChanged = true;
		State.LastLoadBlock = false;
		break;
	case G_OBJLT_TLUT:
		State.timg.address = Segment[(Texture->tlut.image>>24)&BitM4] + (Texture->tlut.image&BitM24);
		// Put data from rdram to tmem
		TMEMAddress = (Texture->tlut.phead-256+2048);
		RDRAMAddress = State.timg.address;
		State.TMEM[TMEMAddress].TMEMStride = 0;
		State.TMEM[TMEMAddress].RDRAMAddress = State.timg.address;
		State.TMEM[TMEMAddress].RDRAMStride = 0;
		State.TMEM[TMEMAddress].LoadType = _LoadTile;
		State.TMEM[TMEMAddress].StartX = 0;
		State.TMEM[TMEMAddress].StartY = 0;
		State.TMEM[TMEMAddress].Width = (Texture->tlut.pnum+1)<<1;
		State.TMEM[TMEMAddress].Height = 1;
		State.TMEM[TMEMAddress].TSize = 1;
		State.TMEM[TMEMAddress].Line = 0;
		State.TMEM[TMEMAddress].DXTUsed = true;
		State.TextureChanged = true;
		State.LastLoadBlock = false;
		break;
	}


}

__inline void Transform2D(D3DTNTVERTEX *v)
{
	// Uses the 2D Matrix for S2DEX ucodes...
	float x, y;
	x   = (v->x*State.Matrix2D._11 + v->y*State.Matrix2D._21 + State.Matrix2D._31);
	y   = (v->x*State.Matrix2D._12 + v->y*State.Matrix2D._22 + State.Matrix2D._32);
	v->x = x;
	v->y = y;
}

__inline void Transform2DMove(D3DTNTVERTEX *v)
{
	// Uses the 2D Matrix for S2DEX ucodes...
	float x, y;
	x   = (v->x/State.Matrix2D._13 + State.Matrix2D._31);
	y   = (v->y/State.Matrix2D._23 + State.Matrix2D._32);
	v->x = x;
	v->y = y;
}

void S2DEX_Sprite(uObjSprite *Sprite, U32 DoMatrix)
{
	float u1, u2, v1, v2;
	float x1, x2, y1, y2;
	TTile tile;
	S32 tx;
	
	tile.tmem = Sprite->s.imageAdrs<<3;
	tile.line = Sprite->s.imageStride;
	tile.Width = (Sprite->s.imageW)/32;
	tile.Height = (Sprite->s.imageH)/32;
	tile.fmt = Sprite->s.imageFmt;
	tile.palette = Sprite->s.imagePal;
	tile.size = Sprite->s.imageSiz;
	tile.cms = 0;
	tile.cmt = 0;
	tile.masks = 8;
	tile.maskt = 8;
	tile.shifts = 0;
	tile.shiftt = 0;
	tile.uls = 0;
	tile.ult = 0;
	tile.lrs = 128;
	tile.lrt = 128;
	tile.D3DAddressU = D3DTADDRESS_CLAMP;
	tile.D3DAddressV = D3DTADDRESS_CLAMP;
	tile.D3DWidth = 1;
	tile.D3DHeight = 1;
	for (tx=1024;tx>=1;tx>>=1) {
		if (tile.Width<=tx)   { tile.D3DWidth  = tx;}
		if (tile.Height<=tx)  { tile.D3DHeight = tx;}
	}
	u1 = 0.0f;
	v1 = 0.0f;
	u2 = (float)tile.Width/tile.D3DWidth;
	v2 = (float)tile.Height/tile.D3DHeight;
	
	HandleTextures(true, &tile);

	SetViewport();
	x1 = Sprite->s.objX/4.0f;
	y1 = Sprite->s.objY/4.0f;
	x2 = (x1 + _10_5ToFloat(Sprite->s.imageW)*_5_10ToFloat(Sprite->s.scaleW));
	y2 = (y1 + _10_5ToFloat(Sprite->s.imageH)*_5_10ToFloat(Sprite->s.scaleH));
	
	SetZBufferWrite();

	D3DTNTVERTEX *VertexBuffer;
	State.ZBufferWrite = false;
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, false));
	D3DXMATRIX Ortho;
	D3DXMatrixOrthoOffCenterRH(&Ortho, 0, (float)State.cimg.width, (float)GetHeight(State.cimg.width), 0, 0.0f, 1.0f);
	SafeDX(g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &Ortho));
	State.ProjectionChanged = true;
	SafeDX(g_VertexBuffer->Lock(0, 4*sizeof(D3DTNTVERTEX), (void**)&VertexBuffer, 0));
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
	switch (DoMatrix){
	case 0:
		for (i=0;i<4;i++){
			Transform2D(&VertexBuffer[i]);
		}
		break;
	case 1:  // Don't rotate
		for (i=0;i<4;i++){
			Transform2DMove(&VertexBuffer[i]);
		}
		break;
	case 2:  // Don't rotate
/*
		for (i=0;i<4;i++){
			VertexBuffer[i].x = VertexBuffer[i].x/2.0f + (float)GetHeight(State.cimg.width)/2.0f;
			VertexBuffer[i].y = VertexBuffer[i].y/2.0f + (float)GetHeight(State.cimg.width)/2.0f;
		}
*/
		break;
	}
	
	ConfigureBlender1();
 	ConfigureCombiner(true);
	ConfigureBlender2();

	BlendPasses.Passes[0].Stages[0].AddressU = D3DTADDRESS_MIRROR;
	BlendPasses.Passes[0].Stages[0].AddressV = D3DTADDRESS_MIRROR;

	SetBlendPass(0);
	SafeDX(g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
}

U32 GetHeight(U32 Width)
{
	switch (Width){
		case 320:
			return 240;
			break;
		case 512:  // Excite Bike
			return 240;
			break;
		case 640:
			return 480;
			break;
		default:
			return Width*3/4;
			break;
	}
}
