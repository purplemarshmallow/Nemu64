#ifndef _H_GLOBALS_HEADER
#define _H_GLOBALS_HEADER

typedef unsigned __int64	U64;
typedef unsigned long		U32;
typedef unsigned short		U16;
typedef unsigned char		U8;
typedef signed __int64		S64;
typedef signed long			S32;
typedef signed short		S16;
typedef signed char			S8;

#include "dllinterface.h"
#include "stdio.h"
#include "s2dex.h"
#include "combtree.h"
#if _DEBUG
#include "DListParser.h"
#endif

#define COMBINEDEBUGGER 1
#define DoOwnGeometry 0

// Change this to 256 for improved gfx and lower speed
// 512 looks a bit better
#define YoshiBGWidth 512
#define YoshiBGHeight 256

// disable deprecated warning
#pragma warning(disable : 4996)

// UCode Groups...
extern U32 uMicroCodeID;

#define _microcode_unknown			0
#define _microcode_rsp_sw			1
#define _microcode_f3dex			2
#define _microcode_f3dex2			3
#define _microcode_waveraceus		4
#define _microcode_dkr				5
#define _microcode_unsupported		6
#define _microcode_pd				7
#define _microcode_conker			8
#define _microcode_naboo			9
#define _microcode_s2dex			10
#define _microcode_jfg				11
#define _microcode_rogue			12
#define _microcode_re2				13
#define _microcode_s2dex2			14

#define VERTEX_BUFFER_SIZE	1500
#define TMEMSize			0x2000
#define MaxTextureCount		500
#if DoOwnGeometry
#define MY_VERTEX_TYPE		(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | _D3DTEX)
#else
#define MY_VERTEX_TYPE		(D3DFVF_XYZ | D3DFVF_DIFFUSE | _D3DTEX)
#endif
#define D3DVertexCacheSize	1000

#define _RDRAM Global_Gfx_Info.RDRAM
// maximum number of texture stages
#define _TEXTURES 2
#define _TEX1 (_TEXTURES==1)
#define _TEX2 (_TEXTURES==2)
#define _TEX3 (_TEXTURES==3)
#define _TEX4 (_TEXTURES==4)
#define _TEX5 (_TEXTURES==5)
#define _TEX6 (_TEXTURES==6)
#define _TEX7 (_TEXTURES==7)
#define _TEX8 (_TEXTURES==8)
#define _D3DTEX ((long)_TEXTURES<<8)

#if _TEX1
#define _TexAssignU(D3DVertexBuffer) D3DVertexBuffer->tex[0].u
#define _TexAssignV(D3DVertexBuffer) D3DVertexBuffer->tex[0].v
#endif

#if _TEX2
#define _TexAssignU(D3DVertexBuffer) D3DVertexBuffer->tex[0].u = D3DVertexBuffer->tex[1].u
#define _TexAssignV(D3DVertexBuffer) D3DVertexBuffer->tex[0].v = D3DVertexBuffer->tex[1].v
#endif

#if _TEX3
#define _TexAssignU(D3DVertexBuffer) D3DVertexBuffer->tex[0].u = D3DVertexBuffer->tex[1].u = D3DVertexBuffer->tex[2].u
#define _TexAssignV(D3DVertexBuffer) D3DVertexBuffer->tex[0].v = D3DVertexBuffer->tex[1].v = D3DVertexBuffer->tex[2].v
#endif

#if _TEX4
#define _TexAssignU(D3DVertexBuffer) D3DVertexBuffer->tex[0].u = D3DVertexBuffer->tex[1].u = D3DVertexBuffer->tex[2].u = D3DVertexBuffer->tex[3].u
#define _TexAssignV(D3DVertexBuffer) D3DVertexBuffer->tex[0].v = D3DVertexBuffer->tex[1].v = D3DVertexBuffer->tex[2].v = D3DVertexBuffer->tex[3].v
#endif

#if _TEX5
#define _TexAssignU(D3DVertexBuffer) D3DVertexBuffer->tex[0].u = D3DVertexBuffer->tex[1].u = D3DVertexBuffer->tex[2].u = D3DVertexBuffer->tex[3].u = D3DVertexBuffer->tex[4].u
#define _TexAssignV(D3DVertexBuffer) D3DVertexBuffer->tex[0].v = D3DVertexBuffer->tex[1].v = D3DVertexBuffer->tex[2].v = D3DVertexBuffer->tex[3].v = D3DVertexBuffer->tex[4].v
#endif

#if _TEX6
#define _TexAssignU(D3DVertexBuffer) D3DVertexBuffer->tex[0].u = D3DVertexBuffer->tex[1].u = D3DVertexBuffer->tex[2].u = D3DVertexBuffer->tex[3].u = D3DVertexBuffer->tex[4].u = D3DVertexBuffer->tex[5].u
#define _TexAssignV(D3DVertexBuffer) D3DVertexBuffer->tex[0].v = D3DVertexBuffer->tex[1].v = D3DVertexBuffer->tex[2].v = D3DVertexBuffer->tex[3].v = D3DVertexBuffer->tex[4].v = D3DVertexBuffer->tex[5].v
#endif

#if _TEX7
#define _TexAssignU(D3DVertexBuffer) D3DVertexBuffer->tex[0].u = D3DVertexBuffer->tex[1].u = D3DVertexBuffer->tex[2].u = D3DVertexBuffer->tex[3].u = D3DVertexBuffer->tex[4].u = D3DVertexBuffer->tex[5].u = D3DVertexBuffer->tex[6].u
#define _TexAssignV(D3DVertexBuffer) D3DVertexBuffer->tex[0].v = D3DVertexBuffer->tex[1].v = D3DVertexBuffer->tex[2].v = D3DVertexBuffer->tex[3].v = D3DVertexBuffer->tex[4].v = D3DVertexBuffer->tex[5].v = D3DVertexBuffer->tex[6].v
#endif

#if _TEX8
#define _TexAssignU(D3DVertexBuffer) D3DVertexBuffer->tex[0].u = D3DVertexBuffer->tex[1].u = D3DVertexBuffer->tex[2].u = D3DVertexBuffer->tex[3].u = D3DVertexBuffer->tex[4].u = D3DVertexBuffer->tex[5].u = D3DVertexBuffer->tex[6].u = D3DVertexBuffer->tex[7].u
#define _TexAssignV(D3DVertexBuffer) D3DVertexBuffer->tex[0].v = D3DVertexBuffer->tex[1].v = D3DVertexBuffer->tex[2].v = D3DVertexBuffer->tex[3].v = D3DVertexBuffer->tex[4].v = D3DVertexBuffer->tex[5].v = D3DVertexBuffer->tex[6].v = D3DVertexBuffer->tex[7].v
#endif

#define BitM1			   0x1
#define BitM2			   0x3
#define BitM3			   0x7
#define BitM4			   0xF
#define BitM5			  0x1F
#define BitM6			  0x3F
#define BitM7			  0x7F
#define BitM8			  0xFF
#define BitM9			 0x1FF
#define BitM10			 0x3FF
#define BitM11			 0x7FF
#define BitM12			 0xFFF
#define BitM13			0x1FFF
#define BitM14			0x3FFF
#define BitM15			0x7FFF
#define BitM16			0xFFFF
#define BitM17		   0x1FFFF
#define BitM18		   0x3FFFF
#define BitM19		   0x7FFFF
#define BitM20		   0xFFFFF
#define BitM21		  0x1FFFFF
#define BitM22		  0x3FFFFF
#define BitM23		  0x7FFFFF
#define BitM24		  0xFFFFFF
#define BitM25		 0x1FFFFFF
#define BitM26		 0x3FFFFFF
#define BitM27		 0x7FFFFFF
#define BitM28		 0xFFFFFFF
#define BitM29		0x1FFFFFFF
#define BitM30		0x3FFFFFFF
#define BitM31		0x7FFFFFFF
#define BitM32		0xFFFFFFFF

#define Bit1			0x1
#define Bit2			0x2
#define Bit3			0x4
#define Bit4			0x8
#define Bit5			0x10
#define Bit6			0x20
#define Bit7			0x40
#define Bit8			0x80
#define Bit9			0x100
#define Bit10			0x200
#define Bit11			0x400
#define Bit12			0x800
#define Bit13			0x1000
#define Bit14			0x2000
#define Bit15			0x4000
#define Bit16			0x8000
#define Bit17			0x10000
#define Bit18			0x20000
#define Bit19			0x40000
#define Bit20			0x80000
#define Bit21			0x100000
#define Bit22			0x200000
#define Bit23			0x400000
#define Bit24			0x800000
#define Bit25			0x1000000
#define Bit26			0x2000000
#define Bit27			0x4000000
#define Bit28			0x8000000
#define Bit29			0x10000000
#define Bit30			0x20000000
#define Bit31			0x40000000
#define Bit32			0x80000000

#define BITS(val, shift, mask) ((val>>shift)&((1<<mask)-1))

#define KB				(1024)
#define MB				(1024*KB)
#define GB				(1024*MB)

typedef struct {
	FLOAT		x, y, z;
#if DoOwnGeometry
	FLOAT rhw;
#endif

	DWORD		color;
	struct {
		FLOAT u, v;
	} tex [_TEXTURES];
} D3DTNTVERTEX;

typedef struct{
	unsigned pad1 : 8;
	unsigned b    : 8;
	unsigned g    : 8;
	unsigned r    : 8;

	unsigned pad2 : 8;
	unsigned bcopy: 8;
	unsigned gcopy: 8;
	unsigned rcopy: 8;

	signed   pad3 : 8;
	signed   z    : 8;
	signed   y    : 8;
	signed   x    : 8;

	signed   pad4 : 32;
} TLight;

typedef struct{
	float tx, ty, tz;  // transformed
	float lx, ly, lz;  // untransformed - simply loaded
	S32 r, g, b;
} TLightLoaded;

// Blend mode values
#define B_PM_COMB		0
#define B_PM_MEM		1
#define B_PM_BLEND		2
#define B_PM_FOG		3

#define B_A_COMB		0
#define B_A_FOG			1
#define B_A_SHADE		2
#define B_A_0			3

#define B_B_INVA		0
#define B_B_MEM			1
#define B_B_1			2
#define B_B_0			3


#define BuildBlendMode1(a, p, b, m) (((B_PM_##p) << 30) | ((B_A_##a) << 26) | ((B_PM_##m) << 22) | ((B_B_##b) << 18))
#define BuildBlendMode2(a1, p1, b1, m1, a2, p2, b2, m2) (BuildBlendMode1(a1, p1, b1, m1)\
|(((B_PM_##p2) << 28) | ((B_A_##a2) << 24) | ((B_PM_##m2) << 20) | ((B_B_##b2) << 16)))


// My Blend mode extras
#define BLENDMODEEXTRA_IGNORETRANSPARENCY Bit1
#define BLENDMODEEXTRA_DONTDRAW           Bit2
#define BLENDMODEEXTRA_WAVERACE           Bit3


// Combine mode values
#define A_CombinedColor	0
#define A_Tex0Color		1
#define A_Tex1Color		2
#define A_PrimColor		3
#define A_ShadeColor	4
#define A_EnvColor		5
#define A_1				6
#define A_Noise			7
#define A_0				8
#define A_0_MIN			8

#define B_CombinedColor	0
#define B_Tex0Color		1
#define B_Tex1Color		2
#define B_PrimColor		3
#define B_ShadeColor	4
#define B_EnvColor		5
#define B_KeyCenter		6
#define B_ConertK4		7
#define B_0				8
#define B_0_MIN			8

#define C_CombinedColor 0
#define C_Tex0Color		1
#define C_Tex1Color		2
#define C_PrimColor		3
#define C_ShadeColor	4
#define C_EnvColor		5
#define C_KeyScale		6
#define C_CombinedAlpha	7
#define C_Tex0Alpha		8
#define C_Tex1Alpha		9
#define C_PrimAlpha		10
#define C_ShadeAlpha	11
#define C_EnvAlpha		12
#define C_LODFraction	13
#define C_PrimLODFrac	14
#define C_K5			15
#define C_0				16
#define C_0_MIN			16

#define D_CombinedColor	0
#define D_Tex0Color		1
#define D_Tex1Color		2
#define D_PrimColor		3
#define D_ShadeColor	4
#define D_EnvColor		5
#define D_1				6
#define D_0				7

#define AA_CombinedAlpha	0
#define AA_Tex0Alpha		1
#define AA_Tex1Alpha		2
#define AA_PrimAlpha		3
#define AA_ShadedAlpha		4
#define AA_EnvAlpha			5
#define AA_1				6
#define AA_0				7

#define BA_CombinedAlpha	0
#define BA_Tex0Alpha		1
#define BA_Tex1Alpha		2
#define BA_PrimAlpha		3
#define BA_ShadedAlpha		4
#define BA_EnvAlpha			5
#define BA_1				6
#define BA_0				7

#define CA_LODFraction		0
#define CA_Tex0Alpha		1
#define CA_Tex1Alpha		2
#define CA_PrimAlpha		3
#define CA_ShadedAlpha		4
#define CA_EnvAlpha			5
#define CA_PrimLODFrac		6
#define CA_0				7

#define DA_CombinedAlpha	0
#define DA_Tex0Alpha		1
#define DA_Tex1Alpha		2
#define DA_PrimAlpha		3
#define DA_ShadedAlpha		4
#define DA_EnvAlpha			5
#define DA_1				6
#define DA_0				7

#define BuildCombineMode2(a0, b0, c0, d0, Aa0, Ab0, Ac0, Ad0, a1, b1, c1, d1, Aa1, Ab1, Ac1, Ad1) \
		(__int64(DA_##Ad1) | (__int64(BA_##Ab1)<<3) | (__int64(D_##d1)<<6) | (__int64(DA_##Ad0)<<9) | (__int64(BA_##Ab0)<<12) | (__int64(D_##d0)<<15) | (__int64(CA_##Ac1)<<18) | (__int64(AA_##Aa1)<<21) | (__int64(B_##b1)<<24) | (__int64(B_##b0)<<28) |\
		(__int64(C_##c1)<<32) | (__int64(A_##a1)<<37) | (__int64(CA_##Ac0)<<41) | (__int64(AA_##Aa0)<<44) | (__int64(C_##c0)<<47) | (__int64(A_##a0)<<52))

#define BuildCombineMode1(a0, b0, c0, d0, Aa0, Ab0, Ac0, Ad0) \
		BuildCombineMode2(a0, b0, c0, d0, Aa0, Ab0, Ac0, Ad0, a0, b0, c0, d0, Aa0, Ab0, Ac0, Ad0)

/*
__forceinline U32 SubtractColorEntry(U32 Color1, U32 Color2)
{
	if (Color1>=Color2){
		return Color1-Color2;
	} else {
		return 0;
	}
}
*/
// clamp?
__forceinline U32 SubtractColor(U32 Color1, U32 Color2)
{
	return  (((BITS(Color1,  0, 8)-BITS(Color2,  0, 8))&BitM8)) |
			(((BITS(Color1,  8, 8)-BITS(Color2,  8, 8))&BitM8)<<8) |
			(((BITS(Color1, 16, 8)-BITS(Color2, 16, 8))&BitM8)<<16) |
			(((BITS(Color1, 24, 8)-BITS(Color2, 24, 8))&BitM8)<<24);
}

__forceinline U32 SubtractColorClamp(U32 Color1, U32 Color2)
{
	U32 c11, c12, c13, c14;
	U32 c21, c22, c23, c24;
	c11 = BITS(Color1,  0, 8);
	c12 = BITS(Color1,  8, 8);
	c13 = BITS(Color1, 16, 8);
	c14 = BITS(Color1, 24, 8);
	c21 = BITS(Color2,  0, 8);
	c22 = BITS(Color2,  8, 8);
	c23 = BITS(Color2, 16, 8);
	c24 = BITS(Color2, 24, 8);
	if (c11>=c21){
		c11 -= c21;
	} else {
		c11 = 0;
	}
	if (c12>=c22){
		c12 -= c22;
	} else {
		c12 = 0;
	}
	if (c13>=c23){
		c13 -= c23;
	} else {
		c13 = 0;
	}
	if (c14>=c24){
		c14 -= c24;
	} else {
		c14 = 0;
	}
	return D3DCOLOR_ARGB(c14, c13, c12, c11);
}

// clamp here too?
__forceinline U32 AddColor(U32 Color1, U32 Color2)
{
	return ((BITS(Color1,  0, 8)+BITS(Color2,  0, 8))&BitM8) |
			(((BITS(Color1,  8, 8)+BITS(Color2,  8, 8))&BitM8)<<8) |
			(((BITS(Color1, 16, 8)+BITS(Color2, 16, 8))&BitM8)<<16) |
			(((BITS(Color1, 24, 8)+BITS(Color2, 24, 8))&BitM8)<<24);
}

__forceinline U32 AddColorClamp(U32 Color1, U32 Color2)
{
	U32 c11, c12, c13, c14;
	U32 c21, c22, c23, c24;
	c11 = BITS(Color1,  0, 8);
	c12 = BITS(Color1,  8, 8);
	c13 = BITS(Color1, 16, 8);
	c14 = BITS(Color1, 24, 8);
	c21 = BITS(Color2,  0, 8);
	c22 = BITS(Color2,  8, 8);
	c23 = BITS(Color2, 16, 8);
	c24 = BITS(Color2, 24, 8);
	
	c11 = min(c11+c21, 255);
	c12 = min(c12+c22, 255);
	c13 = min(c13+c23, 255);
	c14 = min(c14+c24, 255);
	return D3DCOLOR_ARGB(c14, c13, c12, c11);
}

__forceinline U32 ReplicateAlpha(U32 Color)
{
	U32 Alpha;
	Alpha = BITS(Color,  24, 8);
	return D3DCOLOR_RGBA(Alpha, Alpha, Alpha, Alpha);
}

__forceinline U32 ComplementColor(U32 Color)
{
	return   ((255-BITS(Color,  0, 8))&BitM8) |
			(((255-BITS(Color,  8, 8))&BitM8)<<8) |
			(((255-BITS(Color, 16, 8))&BitM8)<<16) |
			(((255-BITS(Color, 24, 8))&BitM8)<<24);
}

__forceinline U32 MultColor(U32 Color1, U32 Color2)
{
	// Multiplication: Each color part is multiplied: ((c1+1)*c2/256)
	return (((((BITS(Color1,  0, 8)+1)*(BITS(Color2,  0, 8)))>>8))&BitM8) |
			((((((BITS(Color1,  8, 8)+1)*(BITS(Color2,  8, 8)))>>8))&BitM8)<<8) |
			((((((BITS(Color1, 16, 8)+1)*(BITS(Color2, 16, 8)))>>8))&BitM8)<<16) |
			((((((BITS(Color1, 24, 8)+1)*(BITS(Color2, 24, 8)))>>8))&BitM8)<<24);
}

typedef union {
	struct {
		unsigned lo    : 32;
		unsigned hi    : 32;
	} raw;
	U64 raw64;
	struct {
		unsigned Ad1   : 3;
		unsigned Ab1   : 3;
		unsigned d1    : 3;
		unsigned Ad0   : 3;
		unsigned Ab0   : 3;
		unsigned d0    : 3;
		unsigned Ac1   : 3;
		unsigned Aa1   : 3;
		unsigned b1    : 4;
		unsigned b0    : 4;
		
		unsigned c1    : 5;
		unsigned a1    : 4;
		unsigned Ac0   : 3;
		unsigned Aa0   : 3;
		unsigned c0    : 5;
		unsigned a0    : 4;
		unsigned padhi : 8;
	} fields;
} TCombine;

typedef union {
	struct {
		U32 lo;
		U32 hi;
	} raw;
	struct {
		unsigned alphacompare  : 2;
		unsigned zsrcsel       : 1;
		
		unsigned rendermodeaa  : 1;
		unsigned rendermodezcmp: 1;
		unsigned rendermodezupd: 1;  // allow write to z buffer
		unsigned rendermodeimrd: 1;
		unsigned rendermodeclroncvg: 1;
		unsigned rendermodecvgdst: 2;
		unsigned rendermodezmode: 3;
		unsigned rendermodecvgxalpha : 1;
		unsigned rendermodealphacvgsel : 1;
		unsigned rendermodeforcebl : 1;
		
		unsigned c2b           : 2;
		unsigned c1b           : 2;
		unsigned c2m           : 2;
		unsigned c1m           : 2;
		unsigned c2a           : 2;
		unsigned c1a           : 2;
		unsigned c2p           : 2;
		unsigned c1p           : 2;

		unsigned blendmask     : 4;   // unsupported
		unsigned alphadither   : 2;
		unsigned rgbdither     : 2;
		unsigned combkey       : 1;
		unsigned textconv      : 3;
		unsigned textfilt      : 2;
		unsigned textlut       : 2;   // 0=NONE, 2=RGBA16, 3=IA16
		unsigned textlod       : 1;
		unsigned textdetail    : 2;
		unsigned textpersp     : 1;
		unsigned cycletype     : 2;   // 0=1 cycle, 1=2 cycle, 2=copy, 3=fill
		unsigned colordither   : 1;   // unsupported
		unsigned pipeline      : 1;
		unsigned padhi         : 8;
	} fields;
} TOthermode;

typedef struct {
	// SetTile
	U32 palette;
	U32 line;
	U32 fmt;
	U32 size;
	U32 tmem;
	U32 cmt;
	U32 maskt;
	U32 shiftt;
	U32 cms;
	U32 masks;
	U32 shifts;
	// SetTileSize
	float uls;
	float ult;
	float lrs;
	float lrt;
	// if uls>lrs
	S32 Width, Height;
	bool MirrorX;
	bool MirrorY;
	U32 D3DWidth, D3DHeight;
	D3DTEXTUREADDRESS D3DAddressU, D3DAddressV;
} TTile;

typedef enum {
	_2ColTex = 1,                    // 2 Colors that blend based on the texture color. Usually used for I/IA textures
	_ModulateAddTexColor = 2,        // Texture*Color1 + Color2
	_ModulateTexColor = 3,           // Texture*Color1
	_ModulateSubTexColor = 4,        // Texture*Color1 - Color2
	_SubtractTexColor = 5,           // Texture-Color
	_ModulateColorReplaceAlpha = 6,  // (Texture*Color)&0x00FFFFFF)|(Alpha&0xFF000000)
	_ModulateAlphaReplaceColor = 7,  // (Texture*Alpha)&0xFF000000)|(Color&0x00FFFFFF0
	_ModulateOneMinusTexColor = 8	 // (1-Texture)*Color1
} TPrerenderType;

typedef enum {
	_LoadBlock = 1,
	_LoadTile = 2,
	_LoadTLUT = 3
} eLoadType;

typedef struct {
    union {
        struct {
            float        _11, _12, _13;
            float        _21, _22, _23;
            float        _31, _32, _33;

        };
        float m[3][3];
    };
} TMatrix2D;

typedef enum {
	Full, Game, None
} TViewportMode;

typedef struct{
	D3DVIEWPORT9 Viewport;
	U32 ScaleX, ScaleY, ScaleZ;
	U32 TranslateX, TranslateY, TranslateZ;
	TViewportMode ViewportMode;
	U32 WindowWidth, WindowHeight;

	D3DXMATRIX World[128];
	D3DXMATRIX *WorldMatrix;
	D3DXMATRIX Identity;
	D3DXMATRIX TempMatrix; // force matrix
	BOOL bForceMtx; // set on the MOVEWORD command, reset when loading a mtx
	U32 WorldIndex;
	D3DXMATRIX Projection, ProjectionModified;
	D3DXMATRIX ProjectionBackupS2DEX;
	bool ProjectionChanged;
	// Force matrix
	U32 MoveMemMatrixAddress;

	U32 FillColor, FogColor, EnvColor, BlendColor, PrimColor;
	float FogStart, FogEnd;
	U32 Lod;
	bool ZBufferWrite, ZEnabled, AlphaTest;
	U32 BranchZAddress;
	union {
		U32 raw;
		struct {
			unsigned ZBuffer       : 1;
			unsigned TextureEnable : 1;
			unsigned Shade         : 1;
			unsigned Pad1          : 6;
			unsigned SmoothShading : 1;
			unsigned Pad2          : 2;
			unsigned CullFront     : 1;
			unsigned CullBack      : 1;
			unsigned Pad3          : 2;
			
			unsigned Fog           : 1;
			unsigned Lighting      : 1;
			unsigned Texture_Gen   : 1;
			unsigned Texture_Gen_Linear : 1;
			unsigned Lod           : 1; // not implemented
			unsigned Pad4          : 2;
			unsigned Clipping      : 1;
		} fields;
	} GeometryMode;
	union {
		U32 raw;
		struct {
			unsigned ZBuffer       : 1;
			unsigned Pad1          : 1;
			unsigned Shade         : 1;
			unsigned Pad2          : 6;
			unsigned CullFront     : 1;
			unsigned CullBack      : 1;
			unsigned Pad3          : 5;

			unsigned Fog           : 1;
			unsigned Lighting      : 1;
			unsigned Texture_Gen   : 1;
			unsigned Texture_Gen_Linear : 1;
			unsigned Lod           : 1; // not implemented
			unsigned SmoothShading : 1;
			unsigned Pad4          : 1;
			unsigned Clipping      : 1;
		} fields;
	} GeometryMode_gbi3;
	TOthermode Othermode;
	bool DecalMode;
	U32 BlendMode;
	U32 BlendExtras;  // 1=
	TCombine Combine;
	bool CombineChanged;
	struct {
		U32 address;
		U32 fmt;
		U32 size;
		U32 width;
	} timg, zimg, cimg;
	TTile Tiles[8];
	struct {
		U32 fmt;
	} Palettes[16];
	D3DTNTVERTEX D3DVertexCache[D3DVertexCacheSize];
	U32 D3DVertexCacheIndex;
	U32 TextureCount;
	struct {
		LPDIRECT3DTEXTURE9 D3DObject;
		U32 UTI;    // unique texture identifier :-) | Highest bit = 1--> prerendered texture
		U32 Age, Width, Height;
		union {
			struct {
				U32 RDPFormat;
				U32 ByLoadBlock;
				U32 DXTUsed;
				U32 RDPPitch;
				U32 cms, cmt, masks, maskt;
			}; // for usual textures
			struct {
				TPrerenderType PrerenderType;
				U32 Flag1, Flag2, Flag3;
			}; // for prerendered textures
		};
	} Textures[MaxTextureCount];
	bool LastLoadBlock;
	struct {
		TTile *tile;
		float TextureScaleS;
		float TextureScaleT;
		U32 MipMapLevels;
	} texture;
	struct {
		U32 RDRAMAddress;
		eLoadType LoadType;
		U32 TSize;
		U32 Size;                          // for LoadBlock
		BOOL DXTUsed;                      // for LoadBlock yes/no, LoadTile/TLUT always yes
		U32 Line;
		U32 Width, Height, StartX, StartY; // for LoadTile
		U32 RDRAMStride, TMEMStride;       // for LoadTile
	} TMEM[TMEMSize];
	bool TextureChanged;
	U32 LastTextureIndex;
	TTile *LastTile;
	D3DTEXTUREADDRESS Address1U, Address1V, Address2U, Address2V;
	D3DTEXTUREFILTERTYPE TextureFilter;

	// Lighting
	U32 NumLights;
	BOOL XFormLights;
	TLightLoaded Lights[16], LookAtX, LookAtY;


	// SetConvert
	struct {
		S32 K0, K1, K2, K3, K4, K5;
	} Convert;


	// cfb and zelda background...
	U32 uLastTexCRC[3];
	U32 LastBGIndex;

	// Battle for Naboo
	U32 NabooPCAfterTexRect;

	// Diddy Kong Racing
	U32 SomeDKRStuff;

	// Jet Force Gemini
	U32 JFGMTXAdd;

	// S2DEX
	U32 S2DEXStatus[4];
	TMatrix2D Matrix2D;
	BOOL bSoftmode;  // if this is true TEXRECTs will go directly to rdram - required for BG


	// cimg tracking
	U32 FirstCImgTriAddress;

	D3DRECT Scissor;

	// Pixelshaders...
	LPDIRECT3DPIXELSHADER9 ps_highlight;    // constant color for debuggin
	LPDIRECT3DPIXELSHADER9 ps_diffuseonly;	// only difuse (mov r0,diffuse)
	LPDIRECT3DPIXELSHADER9 ps_textureonly;  // only texture (tex t0|mov r0, t0)
	LPDIRECT3DPIXELSHADER9 ps_texcoloronly; // only texture color opaque
	LPDIRECT3DPIXELSHADER9 ps_texalphaonly; // only texture alpha opaque
} TState;

typedef struct {
	BOOL bCImgTracing;
	BOOL bDisableTextureCache;
	BOOL bCommandLog;
	BOOL bVRMLExport;
	BOOL bDoCulling;
	BOOL bCullStats;
	BOOL bForceZClear;
	BOOL bForceZBuffer;
	BOOL bWireframe;
	BOOL bBufferClear;
	BOOL bDisablePixelshader;
} TOptions;

#if _DEBUG
typedef struct {
	U32 DLs;
	U32 TexturesLoadedFromCache;
	U32 TexturesCached;
} TStats;
#endif

typedef union {
	struct {
		signed ob2    : 16; /* y */
		signed ob1    : 16; /* x */
		unsigned flag : 16;
		signed ob3    : 16; /* z */
		signed tc2    : 16;
		signed tc1    : 16;
		unsigned alpha: 8; /* alpha */
		unsigned c3   : 8; /* color */
		unsigned c2   : 8; /* color */
		unsigned c1   : 8; /* color */
	} color;
	struct {
		signed ob2    : 16; /* y */
		signed ob1    : 16; /* x */
		unsigned flag : 16;
		signed ob3    : 16; /* z */
		signed tc2    : 16; /* texture coordination */
		signed tc1    : 16; /* texture coordination */
		unsigned a    : 8;  // Alpha
		signed   n3   : 8;  // Normal
		signed   n2   : 8;  // Normal
		signed   n1   : 8;  // Normal
	} normal;
} TVertex;

typedef union {
	struct {
		unsigned flag : 16;
		signed ob3    : 16; /* z */
		signed ob2    : 16; /* y */
		signed ob1    : 16; /* x */
		unsigned alpha: 8; /* alpha */
		unsigned c3   : 8; /* color */
	} color;
	struct {
		unsigned flag : 16;
		signed ob3    : 16; /* z */
		signed ob2    : 16; /* y */
		signed ob1    : 16; /* x */
		unsigned a    : 8;  // Alpha
		signed   n3   : 8;  // Normal
	} normal;
} TVertex_DKR;

typedef union {
	struct {
		signed ob2    : 16; /* y */
		signed ob1    : 16; /* x */
		unsigned flag : 16;
		signed ob3    : 16; /* z */
	} color;
	struct {
		signed ob2    : 16; /* y */
		signed ob1    : 16; /* x */
		unsigned flag : 16;
		signed ob3    : 16; /* z */
	} normal;
} TVertex_Rogue;

typedef union {
	struct {
		signed ob2    : 16; /* y */
		signed ob1    : 16; /* x */
		unsigned pad  : 2;
		unsigned ci   : 6;
		unsigned unknown : 8;
		signed ob3    : 16; /* z */
		signed tc2    : 16; /* texture coordination */
		signed tc1    : 16; /* texture coordination */
	} color;
	struct {
		signed ob2    : 16; /* y */
		signed ob1    : 16; /* x */
		unsigned pad  : 2;
		unsigned ci   : 6;
		unsigned unknown : 8;
		signed ob3    : 16; /* z */
		signed tc2    : 16; /* texture coordination */
		signed tc1    : 16; /* texture coordination */
	} normal;
} TVertex_PD;

extern U32 LastDXErrorCode;

#define SafeDX(CodeLine) \
	LastDXErrorCode = CodeLine;\
	if (FAILED(LastDXErrorCode))\
		DXTrace(__FILE__, __LINE__, LastDXErrorCode, "Error in Lemmy's video dll", true);
#define sqr(x) (x*x)

// DLL Interface
extern GFX_INFO Global_Gfx_Info;


// LemNemu.cpp
void DisplayInStatusPanel(char *str, long lPanel);
void ShowError(char *str);
extern FILE *fVRML;
extern U32 uVRMLFaceIndex;
extern U32 puCullDone, puCullCalled;
extern U32 uRenderWidth;
extern U32 uRenderHeight;


// Prerenderer
LPDIRECT3DTEXTURE9 Prerender_2ColTex(U32 Color1, U32 Color2, U32 TextureIndex);
LPDIRECT3DTEXTURE9 Prerender_ModulateAddTexColor(U32 Color1, U32 Color2, U32 TextureIndex);
LPDIRECT3DTEXTURE9 Prerender_ModulateTexColor(U32 Color1, U32 TextureIndex);
LPDIRECT3DTEXTURE9 Prerender_ModulateSubTexColor(U32 Color1, U32 Color2, U32 TextureIndex);
LPDIRECT3DTEXTURE9 Prerender_SubtractTexColor(U32 Color1, U32 TextureIndex);
LPDIRECT3DTEXTURE9 Prerender_ModulateColorReplaceAlpha(U32 Color1, U32 Alpha, U32 TextureIndex);
LPDIRECT3DTEXTURE9 Prerender_ModulateAlphaReplaceColor(U32 Alpha, U32 Color1, U32 TextureIndex);
LPDIRECT3DTEXTURE9 Prerender_ModulateOneMinusTexColor(U32 Color1, U32 TextureIndex);

typedef struct {
	U32 b : 8;
	U32 g : 8;
	U32 r : 8;
	U32 a : 8;
} TColor8888;
typedef struct {
	U16 b : 4;
	U16 g : 4;
	U16 r : 4;
	U16 a : 4;
} TColor4444;
typedef struct {
	U16 b : 5;
	U16 g : 5;
	U16 r : 5;
	U16 a : 1;
} TColor1555;


// Direct3d.cpp
extern LPDIRECT3D9 g_pD3D;
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
extern LPDIRECT3DVERTEXBUFFER9 g_VertexBuffer;
extern D3DCAPS9 DeviceCaps;
extern LPDIRECT3DTEXTURE9 texBackground[3];
//extern LPDIRECT3DSURFACE9 surWriteback;
typedef struct {
	LPDIRECT3DTEXTURE9 D3DObject;
	U32 Color;
	bool Used;
} TColorTextures[8];
extern TColorTextures g_ColorTexture;

int InitializeDirect3D();
void FreeTextures();
void FreeDirect3D();
extern bool bFullScreen;

#define FreeObject(Object) \
if (Object != NULL){Object->Release();Object = NULL;}


// combiner.cpp
typedef struct {
	D3DTEXTUREOP ColorOp, AlphaOp;
	U32 ColorArg1, ColorArg2, ColorArg3, AlphaArg1, AlphaArg2, AlphaArg3;
	LPDIRECT3DTEXTURE9 Texture;
	D3DTEXTUREADDRESS AddressU, AddressV;
} TTextureStage;

typedef TTextureStage TTextureStages[8];

#define MaxBlendPasses 6
typedef struct {
	U32 Count;
	struct {
		// Both
		D3DBLEND SrcBlend, DstBlend;

		// Fixed Function Pipeline
		TTextureStages Stages;
		U32 TFactor;

		// Pixel Shaders
		char strPixelShader[10000];
	} Passes[MaxBlendPasses];
} TBlendPasses;

void LoadDatFile();
void FreeDatFile();
extern TBlendPasses BlendPasses;
extern bool bHighlighted;
extern bool SecondTex[8];
extern bool ChangeTextureCoordinates;
extern bool UseTexture1, UseTexture2; // set by HandleTextures
extern char DatFilename[_MAX_PATH];
extern char IniFilename[_MAX_PATH];
extern U32 Texture1, Texture2; // indices into texture array

typedef enum {
	SC_NewPass = 0,
	SC_SetColorSources = 1,
	SC_SetAlphaSources = 2,
	SC_AssignTexture = 3,
	SC_SetTFactor = 4,
	SC_TempRegOperation = 5,
	SC_PrerenderTexture = 6,
	SC_QueryDevice = 7
} TScriptCommandType;

typedef struct {
	TScriptCommandType Command;
	U32 Param1, Param2, Param3, Param4, Param5, Param6;  // as of yet the maximum parameter count
} TScriptCommand, *PScriptCommand;

typedef struct {
	U32 Length;
	U32 lo, hi, blendextras;
	TScriptCommand Commands[40];  // for now 40 is max
} TScript, *PScript;

typedef struct {
	U32 Count;
	PScript Script;
} TCombineScripts;

extern TCombineScripts CombineScripts;
#if (_DEBUG||COMBINEDEBUGGER)
typedef struct {
#if COMBINEDEBUGGER
	U32 ReloadDatFile;
	float addx, addy, addz;  // for free camera movement
	float rotx, roty, rotz;  // for free camera movement
#endif
	U32 Count;
	struct {
		U32 lo, hi, blendextras;
		char pixelshader[300];
		char tree[300];
		LPDIRECT3DPIXELSHADER9 pixelshaderhandle;
		U32 Highlight;
		U32 Age;
		U32 EmulationMode;  // 0=hardcoded, 1=script exists, 2=universal combiner, 3=script with other blending, 4=pixelshader
	} Modes[500];
} TLoggedCombineModes, *PLoggedCombineModes;

extern TLoggedCombineModes LoggedCombineModes;
#endif

// Combiner.cpp
void ConfigureCombiner(bool TexturesEnabled);

// Blender.cpp
void ConfigureBlender1();
void ConfigureBlender2();


// 3DFunctions.cpp...
void SetLoadTileTlutInfo(eLoadType LoadType);
void LoadMatrix(U32 base, D3DMATRIX *Matrix);
void CalculateTileSize(TTile *tile);
void SendTrisToD3D(TTile *Tile, bool TexturesEnabled, D3DPRIMITIVETYPE BufferType, bool NoCombine);
void SetZBufferWrite();
void SetViewport();
void LoadMatrix(U32 base, D3DMATRIX *Matrix);
void LoadLight(S32 Index, TLight *Light);
void LoadVertices(U32 Address, U32 Number, U32 StartIndex);
void LoadVertices_GBI5(U32 Address, U32 Number, U32 StartIndex);
void LoadVertices_GBI7(U32 Address, U32 Number, U32 StartIndex);
void LoadVertices_Rogue(U32 Address, U32 Number, U32 StartIndex);
void SimulateTLoadCommand(U32 Start, U8 *TMEMFlipped, U32 Height, U32 RDPPitch, U32 TSize);
void UseMatrix(U32 Address, bool Type, bool Load, bool Push);
void Tri1(U32 v1, U32 v2, U32 v3);
void HandleTextures(bool TextureEnabled, TTile *tile);
void OthermodeChanged();
void FlushVertexCache();
void SetBlendPass(U32 Index);
void CalcModifiedProjectionMatrix();
//static void BackgroundTexture(U32 uTexAddr, float x1, float x2, float y1, float y2, float s1, float s2, float t1, float t2, U32 fmt, U32 size, U32 twidth, U32 theight);
void S2DEX_Texture(uObjTxtr *Texture);
void S2DEX_Sprite(uObjSprite *Sprite, U32 DoMatrix);  // DoMatrix: 0=yes, 1=move, 2=no
BOOL CImgBad();
U32 GetHeight(U32 Width);


#if _DEBUG
extern TStats Stats;
#endif

#define _10_5ToFloat(Value) (((float)(S32)(S16)(Value))*(1.0f/32.0f))
#define _11_5ToFloat(Value) (((float)(U32)(U16)(Value))*(1.0f/32.0f))
#define _5_10ToFloat(Value) (((float)(S32)(S16)(Value))*(1.0f/1024.0f))
__inline U32 RGBA_to_ARGB(U32 Value)
{
	U32 temp;
	__asm{
		mov eax, Value;
		ror eax, 8;
		mov temp, eax;
	}
	return temp;
}

extern TState State;
extern TOptions Options;
extern D3DTNTVERTEX VertexCache[256];
extern U32 PDColorCache[256];

// Memory reader:
__inline U16 Read16(U32 Address)
{
	return (*((U16*)&Global_Gfx_Info.RDRAM[Address ^ 2]));
}

__inline U32 Read32(U32 Address)
{
	return (*((U32*)&Global_Gfx_Info.RDRAM[Address]));
}

__inline U32 Read32SPDMEM(U32 Address)
{
	return (*((U32*)&Global_Gfx_Info.DMEM[Address]));
}

__inline U32 Read32SPIMEM(U32 Address)
{
	return (*((U32*)&Global_Gfx_Info.IMEM[Address]));
}

#if _DEBUG
#define ProfileTimesMax 100
extern S64 ProfileTimes[ProfileTimesMax];
extern S32 ProfileTimeDListCount;
__inline void ProfileStart(U32 Index){
	__asm{
		push ebx;
		mov eax, Index;
		mov ecx, DWORD PTR [ProfileTimes+eax*8]
		mov ebx, DWORD PTR [ProfileTimes+eax*8+4]
		rdtsc;
		sub ecx, eax;
		sbb ebx, edx;
		mov eax, ecx;
		mov edx, ebx;
		mov ecx, Index;
		mov DWORD PTR [ProfileTimes+ecx*8], eax;
		mov DWORD PTR [ProfileTimes+ecx*8+4], edx;
		pop ebx;
	}
}
__inline void ProfileStop(U32 Index){
	__asm{
		push ebx;
		mov eax, Index;
		mov ecx, DWORD PTR [ProfileTimes+eax*8]
		mov ebx, DWORD PTR [ProfileTimes+eax*8+4]
		rdtsc;
		add eax, ecx;
		adc edx, ebx;
		mov ecx, Index;
		mov DWORD PTR [ProfileTimes+ecx*8], eax;
		mov DWORD PTR [ProfileTimes+ecx*8+4], edx;
		pop ebx;
	}
}
#else
#define ProfileStart(i)
#define ProfileStop(i)
#endif
// Logfile...
#if _DEBUG
extern int piLevel;
extern FILE *fLogFile;
__inline void InitLogging()
{
	fLogFile = fopen("LEMMYVIDEO.LOG", "wt");
}

__inline void CloseLogging()
{
	if (fLogFile==0)
		return;
	fclose(fLogFile);
}

__inline void LogCommand(char *szString, ...)
{
	if ((fLogFile==0) || (!Options.bCommandLog))
		return;
	va_list(Arg);
	va_start(Arg, szString);

	fprintf(fLogFile, "%i %8X: %8X:%8X-", piLevel, Stack[uStackIndex].PC, Command.raw.hi, Command.raw.lo);
	vfprintf(fLogFile, szString, Arg);
	fflush(fLogFile);

	va_end(Arg);
}

__inline void PushLevel()
{
	piLevel++;
}

__inline void PopLevel()
{
	piLevel--;
	if (piLevel<0){
		piLevel = 0;
	}
}

__inline void SetLevel(int iLevel)
{
	piLevel = iLevel;
}

#else

/*
__inline void InitLogging()
{
}
__inline void CloseLogging()
{
}
__inline void PushLevel()
{
}
__inline void PopLevel()
{
}
#define SetLevel(i)
#define LogCommand 1 ? (void)0 : (void)
*/

#define InitLogging __noop
#define CloseLogging __noop
#define PushLevel __noop
#define PopLevel __noop
#define SetLevel __noop
#define LogCommand __noop
#endif

static void BackgroundTexture(U32 uTexAddr,
					   float x1,
					   float x2,
					   float y1,
					   float y2,
					   float s1,
					   float s2,
					   float t1,
					   float t2,
					   U32 fmt,
					   U32 size,
					   U32 twidth,
					   U32 theight)
{
	U32 uWidth;
	U32 uHeight;
	U32 x, y;
	U16 *ImgPtr;
	U32 uCol;
	U32 uTexCRC;

	LPDIRECT3DSURFACE9 D3DSurface0;
	D3DLOCKED_RECT Rect;

	
	FlushVertexCache();


	uWidth = (long)(x2-x1);
	uHeight = (long)(y2-y1);

	if (uWidth>512){
		uWidth = 512;
	}
	if (uHeight>256){
		uHeight = 256;
	}
	uTexCRC = 0;
	// do a background crc...
	for (x=0;x<twidth;x+=32){
		for (y=0;y<theight;y+=24){
			uTexCRC += Read32(uTexAddr + y*twidth + x);
		}
	}
	
	
	// find the crc in the 3-layer cache...
	U32 i, bgCount;
	bgCount = 0xFFFFFFFF;
	for (i=0;i<3;i++){
		if (State.uLastTexCRC[i]==uTexCRC){
			bgCount = i;
			break;
		}
	}
	if (bgCount==0xFFFFFFFF){
		// not found. put in texture cache
		bgCount = State.LastBGIndex;
		State.LastBGIndex++;
		if (State.LastBGIndex>=3){
			State.LastBGIndex = 0;
		}
	}

	x1 /= uWidth;
	x2 /= uWidth;
	y1 /= uHeight;
	y2 /= uHeight;
	// Shrink down texture coordinates s,t to Pow2 Coordinates

	if (uMicroCodeID!=10){
		s1 = s1 * 320 / YoshiBGWidth;
		t1 = t1 * 240 / YoshiBGHeight;
		s2 = s2 * 320 / YoshiBGWidth;
		t2 = t2 * 240 / YoshiBGHeight;
	}

	if ((uTexCRC!=State.uLastTexCRC[bgCount]) || (Options.bDisableTextureCache)){
		State.uLastTexCRC[bgCount] = uTexCRC;
		SafeDX(texBackground[bgCount]->GetSurfaceLevel(0, &D3DSurface0));
		SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
		ImgPtr = (U16*)Rect.pBits;
		switch (size){
		case 1:  // 8 bit
			U8 TMEMFlipped[TMEMSize];
			if (fmt==2){
				SimulateTLoadCommand(2048, TMEMFlipped, 1, 0, 1);
			}
			switch (fmt){
			case 2:
				// this is only for yoshi
				// I scale it up to YoshiBGWidthx256
				union {
					U8 *p8;
				} sourceptr;
				U16 CIndex;
				U32 sx, sy;
				U16 Col;
				for (y=0;y<YoshiBGHeight;y++){
					sy = y * theight / YoshiBGHeight;
					sourceptr.p8 = &_RDRAM[uTexAddr + sy*twidth];
					for (x=0;x<YoshiBGWidth;x++){
						sx = x * twidth / YoshiBGWidth;
						CIndex = sourceptr.p8[sx^3];
						Col = *((U16*)&TMEMFlipped[2048 + ((CIndex<<1)^2)]);
						ImgPtr[((y*Rect.Pitch)>>1) + x] = ((Col>>1)&BitM15) | ((Col&BitM1)<<15);
					}
				}
				break;
			}
			break;
		case 2:  // 16 bit
			for (x=0;x<uWidth;x=x+2){
				for (y=0;y<uHeight;y++){
					uCol = Read32(uTexAddr + ((y*uWidth + x)<<1));
					ImgPtr[((y*Rect.Pitch)>>1) + x + 1] = (U16)((uCol & BitM16)>>1) | Bit16;
					ImgPtr[((y*Rect.Pitch)>>1) + x] = (U16)((uCol >> 16)>>1) | Bit16;
				}
			}
			break;
		}
		SafeDX(D3DSurface0->UnlockRect());
		FreeObject(D3DSurface0);
	}

	State.ZBufferWrite = false;
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, State.ZBufferWrite));
	SetViewport();
	D3DXMATRIX Ortho;
	
	D3DXMatrixOrthoOffCenterRH(&Ortho, 0, 1, 0, 1, 0.0f, 1.0f);
	SafeDX(g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &Ortho));
	State.ProjectionChanged = true;

#if DoOwnGeometry
	x1 *= State.Viewport.Width  / (State.cimg.width);
	x2 *= State.Viewport.Width  / (State.cimg.width);
	y1 *= State.Viewport.Height / (State.cimg.width*3/4);
	y2 *= State.Viewport.Height / (State.cimg.width*3/4);
#endif
	D3DTNTVERTEX *VertexBuffer;
	VertexBuffer = &State.D3DVertexCache[0];
	VertexBuffer[0].color = 0xFFFFFFFF;
	VertexBuffer[0].x = (float)x1;
	VertexBuffer[0].y = (float)y1;
	VertexBuffer[0].z = 0.0f;
	_TexAssignU((VertexBuffer+0)) = s1;
	_TexAssignV((VertexBuffer+0)) = t1;
	
	VertexBuffer[1].color = 0xFFFFFFFF;
	VertexBuffer[1].x = (float)x2;
	VertexBuffer[1].y = (float)y1;
	VertexBuffer[1].z = 0.0f;
	_TexAssignU((VertexBuffer+1)) = s2;
	_TexAssignV((VertexBuffer+1)) = t1;
	
	VertexBuffer[2].color = 0xFFFFFFFF;
	VertexBuffer[2].x = (float)x1;
	VertexBuffer[2].y = (float)y2;
	VertexBuffer[2].z = 0.0f;
	_TexAssignU((VertexBuffer+2)) = s1;
	_TexAssignV((VertexBuffer+2)) = t2;
	
	VertexBuffer[3].color = 0xFFFFFFFF;
	VertexBuffer[3].x = (float)x2;
	VertexBuffer[3].y = (float)y2;
	VertexBuffer[3].z = 0.0f;
	_TexAssignU((VertexBuffer+3)) = s2;
	_TexAssignV((VertexBuffer+3)) = t2;

#if DoOwnGeometry	
	VertexBuffer[0].rhw = 1.0f;
	VertexBuffer[1].rhw = 1.0f;
	VertexBuffer[2].rhw = 1.0f;
	VertexBuffer[3].rhw = 1.0f;
#endif
	State.D3DVertexCacheIndex=4;
/*
	SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1));
	SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
	SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
	SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
	SafeDX(g_pd3dDevice->SetTexture(0, texBackground[bgCount]));
	SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP));
	SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP));

	//SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT));
*/
	if (uMicroCodeID==10){
		ConfigureBlender1();
		ConfigureCombiner(true);
		ConfigureBlender2();
	} else {
		BlendPasses.Count = 1;
		SafeDX(g_pd3dDevice->SetPixelShader(State.ps_textureonly));
/*
		BlendPasses.Passes[0].Stages[0].ColorOp, D3DTOP_SELECTARG1;
		BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
		BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
		BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
*/
	}
	BlendPasses.Passes[0].Stages[0].Texture = texBackground[bgCount];
	BlendPasses.Passes[0].Stages[0].AddressU = D3DTADDRESS_WRAP;
	BlendPasses.Passes[0].Stages[0].AddressV = D3DTADDRESS_WRAP;
	UseTexture1=true;
	UseTexture2=false;
 	D3DTNTVERTEX *locVertexBuffer;
	SafeDX(g_VertexBuffer->Lock(0, State.D3DVertexCacheIndex*sizeof(D3DTNTVERTEX), (void**)&locVertexBuffer, 0));
	memcpy(locVertexBuffer, State.D3DVertexCache, State.D3DVertexCacheIndex*sizeof(D3DTNTVERTEX));
	SafeDX(g_VertexBuffer->Unlock());
	for (i=0;i<BlendPasses.Count;i++){
		SetBlendPass(i);
		SafeDX(g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	}
	State.D3DVertexCacheIndex=0;
}

#endif
