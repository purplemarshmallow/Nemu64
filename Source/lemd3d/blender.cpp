#include "stdafx.h"
#include "tchar.h"


void ConfigureBlender1()
{
	State.BlendExtras = 0;
	switch (State.Othermode.fields.cycletype){
	case 0: // 1-Cycle mode
		switch (State.Othermode.raw.lo & 0xCCCC0000){  // filter cycle 1 blend modes
		case BuildBlendMode1(COMB, COMB, INVA, MEM):
			// "Normal" blend mode. Nothing to do
			break;
		case BuildBlendMode1(COMB, COMB, MEM, MEM):
			// Disable transparency?? Do nothing for now
			break;
		case BuildBlendMode1(0, COMB, 1, COMB):
			// Transparency disabled (Ridge Racer for mountains. Otherwise they'd be transparent)
			State.BlendExtras |= 1;
			break;
		case BuildBlendMode1(FOG, FOG, INVA, MEM):
			//Something in Banjo Tooie - no clue
			break;
		case BuildBlendMode1(0, COMB, 1, MEM):
			// c1p: 0, c1a: 3, c1m: 1, c1b: 2
			// this mode makes drawing useless...?
			// but could maybe be used for z-buffer somehow
			State.BlendExtras |= 2;
			break;
		case BuildBlendMode1(COMB, COMB, INVA, BLEND):
			// c1p: 0, c1a: 3, c1m: 2, c1b: 0
			// this adds stuff to the background..wtf?
			break;
		case BuildBlendMode1(0, MEM, INVA, MEM):
			// c1p: 1, c1a: 3, c1m: 1, c1b: 0
			// this is used in a fillrect in waverace over the text in the menu
			State.BlendExtras |= BLENDMODEEXTRA_WAVERACE;
			break;
		default:
			LogCommand("Unhandled 1cycle-blend mode: c1p: %i, c1a: %i, c1m: %i, c1b: %i\n", State.Othermode.fields.c1p, State.Othermode.fields.c1a, State.Othermode.fields.c1m, State.Othermode.fields.c1b);
			break;
		}
		break;
	case 1: // 2-Cycle mode
		switch (State.Othermode.raw.lo & 0xFFFF0000){  // filter cycle 2 blend modes
		case BuildBlendMode2(SHADE, FOG, INVA, COMB, COMB, COMB, MEM, MEM):
			// Fog Mode in Mario and Zelda. Also used on Link. Caught in fog routines
			// Cycle1: [(SHADEALPHA*FOG) + (INVSHADEALPHA*COMB)]
			// Cycle2: [(COMBALPHA*COMB) + (MEMALPHA*MEM)] / (COMBALPHA+MEMALPHA)
			State.BlendExtras |= 1;
			break;
		case BuildBlendMode2(0, COMB, 1, COMB, COMB, COMB, MEM, MEM):
			// Mode used in Zelda for rotating 'N' in intro screen
			State.BlendExtras |= 1;
			break;
		case BuildBlendMode2(0, COMB, 1, COMB, COMB, COMB, INVA, MEM):
//Unhandled 2cycle-blend mode: c1p: 0, c1a: 3, c1m: 0, c1b: 2, c2p: 0, c2a: 0, c2m: 1, c2b: 0
			// Used in Zelda for some texrects like Nintendo Text in first screen
			break;
		case BuildBlendMode2(SHADE, FOG, INVA, COMB, COMB, COMB, INVA, MEM):
//Unhandled 2cycle-blend mode: c1p: 3, c1a: 2, c1m: 0, c1b: 0, c2p: 0, c2a: 0, c2m: 1, c2b: 0
			// Used in Zelda for smoke of Link's horse in intro
			break;
		case BuildBlendMode2(0, COMB, 1, COMB, 0, COMB, 1, COMB):
//Unhandled 2cycle-blend mode: c1p: 0, c1a: 3, c1m: 0, c1b: 2, c2p: 0, c2a: 3, c2m: 0, c2b: 2
			// Used in Zelda for Sky and JPEG inside
			break;
		case BuildBlendMode2(FOG, COMB, INVA, FOG, COMB, COMB, MEM, COMB):
//Unhandled 2cycle-blend mode: c1p: 0, c1a: 1, c1m: 3, c1b: 0, c2p: 0, c2a: 0, c2m: 1, c2b: 1
			// Mode for transparent objects with fog?
			break;
		case BuildBlendMode2(FOG, FOG, INVA, COMB, COMB, COMB, MEM, MEM):
//Unhandled 2cycle-blend mode: c1p: 3, c1a: 1, c1m: 0, c1b: 0, c2p: 0, c2a: 0, c2m: 1, c2b: 1
			State.BlendExtras |= 1;
			break;
		case BuildBlendMode2(FOG, COMB, INVA, FOG, COMB, COMB, MEM, MEM):
//Unhandled 2cycle-blend mode: c1p: 0, c1a: 1, c1m: 3, c1b: 0, c2p: 0, c2a: 0, c2m: 1, c2b: 1
			// Conker's intro
			break;
		default:
			LogCommand("Unhandled 2cycle-blend mode: c1p: %i, c1a: %i, c1m: %i, c1b: %i, c2p: %i, c2a: %i, c2m: %i, c2b: %i\n",
				State.Othermode.fields.c1p, State.Othermode.fields.c1a, State.Othermode.fields.c1m, State.Othermode.fields.c1b,
				State.Othermode.fields.c2p, State.Othermode.fields.c2a, State.Othermode.fields.c2m, State.Othermode.fields.c2b);
			break;
		}
	}
}

void ConfigureBlender2()
{
	U32 i;
	if (State.BlendExtras & BLENDMODEEXTRA_IGNORETRANSPARENCY){
		for (i=0;i<BlendPasses.Count;i++){
			if (BlendPasses.Passes[i].SrcBlend == D3DBLEND_SRCALPHA){
				BlendPasses.Passes[i].SrcBlend = D3DBLEND_ONE;
			}
			if (BlendPasses.Passes[i].DstBlend == D3DBLEND_INVSRCALPHA){
				BlendPasses.Passes[i].DstBlend = D3DBLEND_ZERO;
			}
		}
	}
	if (State.BlendExtras & BLENDMODEEXTRA_DONTDRAW){
		for (i=0;i<BlendPasses.Count;i++){
			BlendPasses.Passes[i].SrcBlend = D3DBLEND_ZERO;
			BlendPasses.Passes[i].DstBlend = D3DBLEND_ONE;
		}
	}
	if (State.BlendExtras & BLENDMODEEXTRA_WAVERACE){
		for (i=0;i<BlendPasses.Count;i++){
			BlendPasses.Passes[i].SrcBlend = D3DBLEND_ZERO;
			BlendPasses.Passes[i].DstBlend = D3DBLEND_INVSRCALPHA;
		}
	}
}
