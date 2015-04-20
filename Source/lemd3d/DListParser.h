#ifndef DLISTPARSER_H
#define DLISTPARSER_H

void DetectMicroCode(U32 uIndex);
typedef struct {
	U32 PC;
	U32 RogueStartDL;
	U32 RogueOtherDL;
} TStack;
extern LPDIRECT3DSURFACE9 NormalSurface;
extern TStack Stack[10];
extern U32 uStackIndex;

extern bool UCode_NoN;
extern U32 Segment[16];
typedef union {
	struct {
		U32 lo;
		U32 hi;
	} raw;
	struct {
		U32 lo;
		unsigned hipad   : 24;
		unsigned command : 8;
	} general;
	struct {
		U32 lo;
		unsigned hipad   : 24;
		unsigned command : 6;
		unsigned hipad2  : 2;
	} rdpgeneral;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad0    : 4;
		unsigned len     : 16;
		unsigned branch  : 8;
		unsigned cmd     : 8;
	} dl;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad0    : 4;
		unsigned len     : 16;
		unsigned v0      : 4;
		unsigned n       : 4;
		unsigned cmd     : 8;
	} vtx_gbi1;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad0    : 4;
		unsigned length  : 10;
		unsigned n       : 6;
		unsigned pad1    : 1;
		unsigned v0      : 6;
		unsigned pad     : 1;
		unsigned cmd     : 8;
	} vtx_gbi2;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad0    : 4;
		
		unsigned length  : 9;
		unsigned n       : 6;
		unsigned pad     : 1;
		unsigned v0      : 6;
		unsigned pad2    : 2;
		unsigned cmd     : 8;
	} vtx_gbi4;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad0    : 4;
		unsigned pad1    : 1;
		unsigned v0n     : 7;
		unsigned pad2    : 4;
		unsigned n       : 8;
		unsigned pad3    : 4;
		unsigned cmd     : 8;
	} vtx_gbi3;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad0    : 4;
		
		unsigned n       : 12;
		unsigned pad     : 12;
		unsigned cmd     : 8;
	} vtx_gbi5;
	struct {
		union {
			unsigned val     : 32;
			struct {
				signed NewS : 16;
				signed NewT : 16;
			} NewST;
		} val;
		unsigned pad     : 1;
		unsigned vtx     : 15;
		unsigned where   : 8;
		unsigned cmd     : 8;
	} modifyvtx;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad0    : 4;
		
		unsigned len     : 8;
		unsigned pad1    : 8;
		unsigned type    : 1;
		unsigned load    : 1;
		unsigned push    : 1;
		unsigned pad2    : 5;
		unsigned cmd     : 8;
	} mtx_gbi12;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad     : 4;

		unsigned push    : 1;
		unsigned load    : 1;
		unsigned type    : 1;
		unsigned pad1    : 13;
		unsigned len     : 4;
		unsigned pad2    : 4;
		unsigned cmd     : 8;
	} mtx_gbi3;
	struct {
		unsigned data    : 32;
		unsigned number  : 8;
		unsigned mw_index: 8;
		unsigned pad0    : 8;
		unsigned cmd     : 8;
	} moveword_gbi12;
	struct {
		unsigned data    : 32;
		unsigned mw_index: 8;
		unsigned pad     : 8;
		unsigned number  : 8;
		unsigned cmd     : 8;
	} moveword_gbi3;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad1    : 4;

		unsigned len     : 16;
		unsigned par     : 8;
		unsigned cmd     : 8;
	} movemem;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad1    : 4;

		unsigned idx     : 8;
		unsigned ofs     : 8;
		unsigned len     : 8;
		unsigned cmd     : 8;
	} movemem_gbi3;
	struct {
		unsigned v2      : 8;
		unsigned v1      : 8;
		unsigned v3      : 8;
		unsigned flag    : 8;
		unsigned pad     : 24;
		unsigned cmd     : 8;
	} tri1_gbi1, tri1_gbi4;
	struct {
		unsigned pad1    : 1;
		unsigned v1      : 7;
		unsigned pad2    : 1;
		unsigned v2      : 7;
		unsigned pad3    : 1;
		unsigned v3      : 7;
		unsigned pad4    : 8;
		unsigned pad5    : 24;
		unsigned cmd     : 8;
	} tri1_gbi2;
	struct {
		unsigned pad     : 32;
		unsigned pad1    : 1;
		unsigned v1      : 7;
		unsigned pad2    : 1;
		unsigned v2      : 7;
		unsigned pad3    : 1;
		unsigned v3      : 7;
		unsigned cmd     : 8;
	} tri1_gbi3;
	struct {
		unsigned v11     : 4;  // 0
		unsigned v12     : 4;  // 4
		unsigned v21     : 4;  // 8
		unsigned v22     : 4;  // 12
		unsigned v31     : 4;  // 16
		unsigned v32     : 4;  // 20
		unsigned v41     : 4;  // 24
		unsigned v42     : 4;  // 28
		
		unsigned v13     : 4;  // 0
		unsigned v23     : 4;  // 4
		unsigned v33     : 4;  // 8
		unsigned v43     : 4;  // 12
		unsigned pad     : 8;  // 16
		unsigned cmd     : 8;  // 24
	} tri4_gbi1;
/*
	v1 = BITS(lo, 4,0xF);
	v0 = BITS(lo, 0,0xF);
	v2 = BITS(hi, 0,0xF);
	
	v1 = BITS(lo,12,0xF);
	v0 = BITS(lo, 8,0xF);  
	v2 = BITS(hi, 4,0xF);
	
	v1 = BITS(lo,20,0xF);
	v0 = BITS(lo,16,0xF);  
	v2 = BITS(hi, 8,0xF);

	v1 = BITS(lo,28,0xF);
	v0 = BITS(lo,24,0xF);  
	v2 = BITS(hi,12,0xF);
*/
	struct {
		unsigned v21     : 5;  // 0
		unsigned v22     : 5;  // 5
		unsigned v23     : 5;  // 10
		unsigned v11     : 5;  // 15
		unsigned v12     : 5;  // 20
		unsigned v13     : 5;  // 25
		unsigned v41lo   : 2;  // 32
		
		unsigned v31     : 5;  // 0
		unsigned v32     : 5;  // 5
		unsigned v33     : 5;  // 10
		unsigned v41hi   : 3;  // 15
		unsigned v42     : 5;  // 18
		unsigned v43     : 5;  // 23
		unsigned cmd     : 4;  // 28  // v43 is partially stored in cmd (so cmd is 0x10..0x1F)
	} tri4_gbi3;
	struct {
		unsigned pad1    : 1;
		unsigned v1      : 7;
		unsigned pad2    : 1;
		unsigned v2      : 7;
		unsigned pad3    : 1;
		unsigned v3      : 7;
		unsigned pad4    : 8;
		
		unsigned pad5    : 1;
		unsigned v4      : 7;
		unsigned pad6    : 1;
		unsigned v5      : 7;
		unsigned pad7    : 1;
		unsigned v6      : 7;
		unsigned cmd     : 8;
	} tri2_gbi2, tri2_gbi3;
	struct {
		unsigned v1      : 4;
		unsigned v2      : 4;
		unsigned v4      : 4;
		unsigned v5      : 4;
		unsigned v7      : 4;
		unsigned v8      : 4;
		unsigned v10     : 4;
		unsigned v11     : 4;
		
		unsigned v3      : 4;
		unsigned v6      : 4;
		unsigned v9      : 4;
		unsigned v12     : 4;
		unsigned pad     : 8;
		unsigned cmd     : 8;
	} tri4_gbi7;  // Perfect Dark
	struct {
		unsigned pad1    : 1;
		unsigned v1      : 7;
		unsigned pad2    : 1;
		unsigned v2      : 7;
		unsigned pad3    : 1;
		unsigned v3      : 7;
		unsigned pad4    : 1;
		unsigned v4      : 7;
		
		unsigned pad5    : 24;
		unsigned cmd     : 8;
	} quad_gbi2;
	struct {
		unsigned v2      : 8;
		unsigned v1      : 8;
		unsigned v3      : 8;
		unsigned v4      : 8;
		unsigned pad     : 24;
		unsigned cmd     : 8;
	} quad_gbi4;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad0    : 4;
		unsigned padhi   : 24;
		unsigned cmd     : 8;
	} background;
	struct {
		unsigned color   : 32;
		unsigned lod     : 8;
		unsigned min_lev : 8;
		unsigned pad     : 8;
		unsigned cmd     : 8;
	} setcolor;
	struct {
		unsigned y1frac  : 2;
		unsigned y1      : 10;
		unsigned x1frac  : 2;
		unsigned x1      : 10;
		unsigned pad     : 8;
		unsigned y0frac  : 2;
		unsigned y0      : 10;
		unsigned x0frac  : 2;
		unsigned x0      : 10;
	} fillrect;
	struct {
		unsigned thfrac  : 2;
		unsigned th      : 10;
		unsigned shfrac  : 2;
		unsigned sh      : 10;
		unsigned tile    : 3;
		unsigned pad     : 5;
		unsigned tlfrac  : 2;
		unsigned tl      : 10;
		unsigned slfrac  : 2;
		unsigned sl      : 10;
		unsigned cmd     : 8;
	} loadtile;
	struct {
		unsigned pad1    : 14;
		unsigned count   : 10;
		unsigned tile    : 3;
		unsigned pad2    : 5;
		unsigned pad3    : 24;
		unsigned cmd     : 8;
	} loadtlut;
	struct {
		unsigned dxt     : 12;
		unsigned lrs     : 12;
		unsigned tile    : 3;
		unsigned pad     : 5;
		unsigned ult     : 12;
		unsigned uls     : 12;
		unsigned cmd     : 8;
	} loadblock;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad0    : 4;
		unsigned width   : 12;
		unsigned pad     : 7;
		unsigned size    : 2;
		unsigned fmt     : 3;
		unsigned cmd     : 8;
	} setimage;
	struct {
		unsigned shifts  : 4;  // 0
		unsigned masks   : 4;  // 4
		unsigned cms     : 2;  // 8
		unsigned shiftt  : 4;  // 10
		unsigned maskt   : 4;  // 14
		unsigned cmt     : 2;  // 18
		unsigned palette : 4;  // 20
		unsigned tile    : 3;  // 24
		unsigned pad1    : 5;  // 27
		unsigned tmem    : 9;  // 32 + 0
		unsigned line    : 9;  // 32 + 9
		unsigned pad0    : 1;  // 32 + 18
		unsigned siz     : 2;  // 32 + 19
		unsigned fmt     : 3;  // 32 + 21
		unsigned cmd     : 8;  // 32 + 24
	} settile;
	struct {
		unsigned lrt_frac: 2;
		unsigned lrt     : 10; // 0
		unsigned lrs_frac: 2;
		unsigned lrs     : 10; // 12
		unsigned tile    : 3;  // 24
		unsigned pad     : 5;  // 27
		unsigned ult_frac: 2;
		unsigned ult     : 10; // 32 + 0
		unsigned uls_frac: 2;
		unsigned uls     : 10; // 32 + 12
		unsigned cmd     : 8;  // 32 + 24
	} settilesize;
	struct{
		unsigned t       : 16; // 0
		unsigned s       : 16; // 16
		unsigned on      : 8;  // 32 + 0
		unsigned tile    : 3;  // 32 + 8
		unsigned pad1    : 5;  // 32 + 11
		unsigned lodscale: 8;  // 32 + 16
		unsigned cmd     : 8;  // 32 + 24
	} texture;
	struct {
		unsigned yh      : 12; // Y coordinate of lower right
		unsigned xh      : 12; // X coordinate of lower right
		unsigned tile    : 3;  // Tile descriptor index
		unsigned pad1    : 5;  // Padding
		unsigned yl      : 12; // Y coordinate of upper left
		unsigned xl      : 12; // X coordinate of upper left
		unsigned cmd     : 8;  // command
	} texrect_1;
	struct {
		unsigned t       : 16; // T texture coord at top left
		unsigned s       : 16; // S texture coord at top left
		unsigned pad     : 24;
		unsigned cmd     : 8;
	} texrect_2;
	struct {
		unsigned dtdy    : 16; // Change in T per change in Y
		unsigned dsdx    : 16; // Change in S per change in X
		unsigned pad     : 24;
		unsigned cmd     : 8;
	} texrect_3;
	struct {
		unsigned dtdy    : 16; // Change in T per change in Y
		unsigned dsdx    : 16; // Change in S per change in X
		unsigned t       : 16; // T texture coord at top left
		unsigned s       : 16; // S texture coord at top left
	} texrect_2_rogue;
	struct {
		unsigned data    : 32; // 0
		unsigned len     : 8;  // 32 + 0
		unsigned sft     : 8;  // 32 + 8
		unsigned pad0    : 8;  // 32 + 16
		unsigned cmd     : 8;  // 32 + 24
	} setothermode;
	struct {
		signed k5        : 9;
		signed k4        : 9;
		signed k3        : 9;
		signed k2lo      : 5;
		signed k2hi      : 4;
		signed k1        : 9;
		signed k0        : 9;
		unsigned pad     : 3;
		unsigned cmd     : 8;
	} setconvert;

	struct {
		unsigned end     : 10;  // divide by 40, then add(?) 1
		unsigned pad1    : 22;

		unsigned start   : 10;  // divide by 40
		unsigned pad2    : 14;
		unsigned cmd     : 8;
	} cull_gbi3;

	struct {
		unsigned zvalue  : 32;

		unsigned vtx2    : 12;  // divide by 2
		unsigned vtx5    : 12;  // divide by 5
		unsigned cmd     : 8;
	} branchz;

	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad1    : 4;

		unsigned len     : 9;
		unsigned pad     : 7;
		unsigned par     : 8;
		unsigned cmd     : 8;
	} diddy1_gbi5;
	struct {
		unsigned addr    : 24;
		unsigned segment : 4;
		unsigned pad1    : 4;

		unsigned pad2    : 4;
		unsigned count   : 8;
		unsigned pad3    : 12;
		unsigned cmd     : 8;
	} tri_gbi5;
} TCommand;

/*
Shift parameter value Shift amount 
0 No shift 
1 >> 1 
2 >> 2 
... 
9 >> 9 
10 >> 10 
11 << 5 
12 << 4 
13 << 3 
14 << 2 
15 << 1 
*/
const float ShiftTable[16] = {
		1.0f,
		1.0f/2.0f,
		1.0f/4.0f, 
		1.0f/8.0f,
		1.0f/16.0f,
		1.0f/32.0f,
		1.0f/64.0f,
		1.0f/128.0f,
		1.0f/256.0f,
		1.0f/512.0f,
		1.0f/1024.0f,
		32.0f,
		16.0f,
		8.0f,
		4.0f,
		2.0f};

extern TCommand Command;
extern bool bFirstDisplayList;

void ProcessDisplayList();

#endif