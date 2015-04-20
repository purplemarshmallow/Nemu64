#include "stdafx.h"
#include "tchar.h"

const char *Combine_A[16] = {"CombinedColor", "Tex0Color", "Tex1Color", "PrimColor", "ShadeColor",  "EnvColor", "1",          "Noise",         "0", "0", "0", "0", "0", "0", "0", "0"};
const char *Combine_B[16] = {"CombinedColor", "Tex0Color", "Tex1Color", "PrimColor", "ShadeColor",  "EnvColor", "Key Center", "K4",            "0", "0", "0", "0", "0", "0", "0", "0"};
const char *Combine_C[32] = {"CombinedColor", "Tex0Color", "Tex1Color", "PrimColor", "ShadeColor",  "EnvColor", "Key Scale",  "CombinedAlpha", "Tex0Alpha", "Tex1Alpha", "PrimAlpha", "ShadedAlpha", "EnvAlpha", "LODFraction", "PrimLODFrac", "K5", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};
const char *Combine_D[8]  = {"CombinedColor", "Tex0Color", "Tex1Color", "PrimColor", "ShadeColor",  "EnvColor", "1",          "0"};

const char *CombinA_A[8]  = {"CombinedAlpha", "Tex0Alpha", "Tex1Alpha", "PrimAlpha", "ShadedAlpha", "EnvAlpha", "1",        "0"};
const char *CombinA_B[8]  = {"CombinedAlpha", "Tex0Alpha", "Tex1Alpha", "PrimAlpha", "ShadedAlpha", "EnvAlpha", "1",        "0"};
const char *CombinA_C[8]  = {"LODFraction",   "Tex0Alpha", "Tex1Alpha", "PrimAlpha", "ShadedAlpha", "EnvAlpha", "PrimLODFrac", "0"};
const char *CombinA_D[8]  = {"CombinedAlpha", "Tex0Alpha", "Tex1Alpha", "PrimAlpha", "ShadedAlpha", "EnvAlpha", "1",        "0"};

const char *BlendNames[14]  = {"Invalid", "ZERO", "ONE", "SRCCOLOR", "INVSRCCOLOR", "SRCALPHA", "INVSRCALPHA", "DESTALPHA", "INVDESTALPHA", "DESTCOLOR", "INVDESTCOLOR", "SRCALPHASAT", "BOTHSRCALPHA", "BOTHINVSRCALPHA"};
const char *ColorOpNames[27]  = {"Invalid", "DISABLE", "SELECTARG1", "SELECTARG2", "MODULATE", "MODULATE2X", "MODULATE4X", "ADD", "ADDSIGNED", "ADDSIGNED2X", "SUBTRACT", "ADDSMOOTH", "BLENDDIFFUSEALPHA", "BLENDTEXTUREALPHA", "BLENDFACTORALPHA", "BLENDTEXTUREALPHAPM", "BLENDCURRENTALPHA", "PREMODULATE", "MODULATEALPHA_ADDCOLOR", "MODULATECOLOR_ADDALPHA", "MODULATEINVALPHA_ADDCOLOR", "MODULATEINVCOLOR_ADDALPHA", "BUMPENVMAP", "BUMPENVMAPLUMINANCE", "DOTPRODUCT3", "MULTIPLYADD", "LERP"};
const char *ColorArgNames[64] = {"Diffuse", "Current", "Texture", "TFactor", "Specular", "Temp", "", "", "", "", "", "", "", "", "", "",
                                 "1-Diffuse", "1-Current", "1-Texture", "1-TFactor", "1-Specular", "1-Temp", "", "", "", "", "", "", "", "", "", "",
                                 "AlphaDiffuse", "AlphaCurrent", "AlphaTexture", "AlphaTFactor", "AlphaSpecular", "AlphaTemp", "", "", "", "", "", "", "", "", "", "",
                                 "1-AlphaDiffuse", "1-AlphaCurrent", "1-AlphaTexture", "1-AlphaTFactor", "1-AlphaSpecular", "1-AlphaTemp", "", "", "", "", "", "", "", "", "", ""};


TBlendPasses BlendPasses;
bool SecondTex[8];
bool bHighlighted;
bool ChangeTextureCoordinates;
char DatFilename[_MAX_PATH]="";
char IniFilename[_MAX_PATH]="";
LPDIRECT3DPIXELSHADER9 m_pPixelShader;
bool UseTexture1, UseTexture2; // set by HandleTextures
U32 Texture1, Texture2; // indices into texture array

TCombineScripts CombineScripts;
#if (_DEBUG||COMBINEDEBUGGER)
TLoggedCombineModes LoggedCombineModes;
#endif

bool FirstCombinerStage, AllowFirstStage, TFactorUsed;
U32 TFactorColor;
U32 CombinerStage;
bool FirstIsTexture;
U32 ColorTextureIndex;

void AddCombinerStage(U32 Source, D3DTEXTUREOP Operation, U32 Tex01)
{
	if (FirstCombinerStage){
		FirstIsTexture = (Source&D3DTA_SELECTMASK)==D3DTA_TEXTURE;
		BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
		BlendPasses.Passes[0].Stages[0].ColorArg1 = Source;
		FirstCombinerStage = false;
		AllowFirstStage = true;
		CombinerStage++;
	} else {
		if (CombinerStage==1){
			if ((FirstIsTexture && ((Source&D3DTA_SELECTMASK)==D3DTA_TEXTURE)) || (!AllowFirstStage)){
				BlendPasses.Passes[0].Stages[CombinerStage].ColorOp = Operation;
				BlendPasses.Passes[0].Stages[CombinerStage].ColorArg1 = D3DTA_CURRENT;
				BlendPasses.Passes[0].Stages[CombinerStage].ColorArg2 = Source;
				CombinerStage++;
			} else {
				BlendPasses.Passes[0].Stages[CombinerStage-1].ColorOp = Operation;
				BlendPasses.Passes[0].Stages[CombinerStage-1].ColorArg2 = Source;
				AllowFirstStage = false;
			}
		} else {
			BlendPasses.Passes[0].Stages[CombinerStage].ColorOp = Operation;
			BlendPasses.Passes[0].Stages[CombinerStage].ColorArg1 = D3DTA_CURRENT;
			BlendPasses.Passes[0].Stages[CombinerStage].ColorArg2 = Source;
			CombinerStage++;
		}
	}
	if (((Source&D3DTA_SELECTMASK)==D3DTA_TEXTURE) && (Tex01!=0xFFFFFFFF)){
		if (Tex01==0){
			BlendPasses.Passes[0].Stages[CombinerStage-1].Texture = State.Textures[Texture1].D3DObject;
			BlendPasses.Passes[0].Stages[CombinerStage-1].AddressU = State.Address1U;
			BlendPasses.Passes[0].Stages[CombinerStage-1].AddressV = State.Address1V;
		} else {
			BlendPasses.Passes[0].Stages[CombinerStage-1].Texture = State.Textures[Texture2].D3DObject;
			BlendPasses.Passes[0].Stages[CombinerStage-1].AddressU = State.Address2U;
			BlendPasses.Passes[0].Stages[CombinerStage-1].AddressV = State.Address2V;
		}
	}
	if (((Source&D3DTA_SELECTMASK)==D3DTA_TEXTURE) && (Tex01==0xFFFFFFFF)){
		BlendPasses.Passes[0].Stages[CombinerStage-1].Texture = g_ColorTexture[ColorTextureIndex].D3DObject;
		BlendPasses.Passes[0].Stages[CombinerStage-1].AddressU = D3DTADDRESS_WRAP;
		BlendPasses.Passes[0].Stages[CombinerStage-1].AddressV = D3DTADDRESS_WRAP;
	}
	if ((Source&D3DTA_SELECTMASK)==D3DTA_TFACTOR){
		BlendPasses.Passes[0].TFactor = TFactorColor;
	}
}

U32 GenerateColorTexture(U32 Color)
{
	U32 i;
	ColorTextureIndex = 0xFFFFFFFF;
	for (i=0;i<8;i++){
		if (g_ColorTexture[i].Color==Color){
			ColorTextureIndex = i;
			break;
		}
	}
	if (ColorTextureIndex==0xFFFFFFFF){
		ColorTextureIndex = 0;
		while (g_ColorTexture[ColorTextureIndex].Used) {
			ColorTextureIndex++;
		}
		if (ColorTextureIndex>=8){
			// shouldn't happen
			ColorTextureIndex = 0;
			ShowError("Error 1 in UseColor");
		}
		LPDIRECT3DSURFACE9 D3DSurface0;
		D3DLOCKED_RECT Rect;
		SafeDX(g_ColorTexture[ColorTextureIndex].D3DObject->GetSurfaceLevel(0, &D3DSurface0));
		SafeDX(D3DSurface0->LockRect(&Rect, 0, 0));
		*((U32*)Rect.pBits) = Color;
		SafeDX(D3DSurface0->UnlockRect());
		g_ColorTexture[ColorTextureIndex].Color = Color;
	}
	g_ColorTexture[ColorTextureIndex].Used = true;
	return ColorTextureIndex;
}

U32 UseColor(U32 Color)
{
	if (TFactorUsed){
		if ((Color==TFactorColor)){
			return D3DTA_TFACTOR;
		} else {
			GenerateColorTexture(Color);
			return D3DTA_TEXTURE;
		}
	} else {
		TFactorUsed = true;
		TFactorColor = Color;
		return D3DTA_TFACTOR;
	}
}

void DoCycle(U32 a, U32 b, U32 c, U32 d)
{
	U32 Color;
	bool DoConstantColorInA;
	DoConstantColorInA = false;
	switch (a){
	case A_ShadeColor:
		AddCombinerStage(D3DTA_DIFFUSE, D3DTOP_MODULATE, 0xFFFFFFFF);
		break;
	case A_PrimColor:
		Color = State.PrimColor;
		DoConstantColorInA = true;
		break;
	case A_EnvColor:
		Color = State.EnvColor;
		DoConstantColorInA = true;
		break;
	case A_Tex0Color:
		AddCombinerStage(D3DTA_TEXTURE, D3DTOP_MODULATE, 0);
		break;
	case A_Tex1Color:
		AddCombinerStage(D3DTA_TEXTURE, D3DTOP_MODULATE, 1);
		break;
	}
	bool bDone;
	bDone = false;
	if (DoConstantColorInA){
		// is there a constant color in b? if yes, subtract b and add to texture stages
		switch (b){
		case B_PrimColor:
			Color = D3DCOLOR_ARGB(0xFF,
				    BITS(Color,  0, 8)-BITS(State.PrimColor,  0, 8),
				    BITS(Color,  8, 8)-BITS(State.PrimColor,  8, 8),
					BITS(Color, 16, 8)-BITS(State.PrimColor, 16, 8));
			bDone = true;
			break;
		case B_EnvColor:
			Color = D3DCOLOR_ARGB(0xFF,
				    BITS(Color,  0, 8)-BITS(State.EnvColor,  0, 8),
				    BITS(Color,  8, 8)-BITS(State.EnvColor,  8, 8),
					BITS(Color, 16, 8)-BITS(State.EnvColor, 16, 8));
			bDone = true;
			break;
		}
		AddCombinerStage(UseColor(Color), D3DTOP_MODULATE, 0xFFFFFFFF);
	}
	if (!bDone){
		switch (b){
			case B_ShadeColor:
			AddCombinerStage(D3DTA_DIFFUSE, D3DTOP_SUBTRACT, 0xFFFFFFFF);
			break;
		case B_PrimColor:
			Color = UseColor(State.PrimColor);
			AddCombinerStage(Color, D3DTOP_SUBTRACT, 0xFFFFFFFF);
			break;
		case B_EnvColor:
			Color = UseColor(State.EnvColor);
			AddCombinerStage(Color, D3DTOP_SUBTRACT, 0xFFFFFFFF);
			break;
		case B_Tex0Color:
			AddCombinerStage(D3DTA_TEXTURE, D3DTOP_SUBTRACT, 0);
			break;
		case B_Tex1Color:
			AddCombinerStage(D3DTA_TEXTURE, D3DTOP_SUBTRACT, 1);
			break;
		}
	}
	switch (c){
	case C_ShadeColor:
		AddCombinerStage(D3DTA_DIFFUSE, D3DTOP_MODULATE, 0xFFFFFFFF);
		break;
	case C_PrimColor:
		Color = UseColor(State.PrimColor);
		AddCombinerStage(Color, D3DTOP_MODULATE, 0xFFFFFFFF);
		break;
	case C_EnvColor:
		Color = UseColor(State.EnvColor);
		AddCombinerStage(Color, D3DTOP_MODULATE, 0xFFFFFFFF);
		break;
	case C_PrimAlpha:
		Color = UseColor(State.PrimColor);
		AddCombinerStage(Color|D3DTA_ALPHAREPLICATE, D3DTOP_MODULATE, 0xFFFFFFFF);
		break;
	case C_LODFraction:
		Color = UseColor(D3DCOLOR_ARGB(State.Lod, State.Lod, State.Lod, State.Lod));
		AddCombinerStage(Color|D3DTA_ALPHAREPLICATE, D3DTOP_MODULATE, 0xFFFFFFFF);
		break;
	case C_EnvAlpha:
		Color = UseColor(State.EnvColor);
		AddCombinerStage(Color|D3DTA_ALPHAREPLICATE, D3DTOP_MODULATE, 0xFFFFFFFF);
		break;
	case C_Tex0Color:
		AddCombinerStage(D3DTA_TEXTURE, D3DTOP_MODULATE, 0);
		break;
	case C_Tex0Alpha:
		AddCombinerStage(D3DTA_TEXTURE|D3DTA_ALPHAREPLICATE, D3DTOP_MODULATE, 0);
		break;
	case C_Tex1Color:
		AddCombinerStage(D3DTA_TEXTURE, D3DTOP_MODULATE, 1);
		break;
	case C_Tex1Alpha:
		AddCombinerStage(D3DTA_TEXTURE|D3DTA_ALPHAREPLICATE, D3DTOP_MODULATE, 1);
		break;
	}
	switch (d){
	case D_ShadeColor:
		AddCombinerStage(D3DTA_DIFFUSE, D3DTOP_ADD, 0xFFFFFFFF);
		break;
	case D_PrimColor:
		Color = UseColor(State.PrimColor);
		AddCombinerStage(Color, D3DTOP_ADD, 0xFFFFFFFF);
		break;
	case D_EnvColor:
		Color = UseColor(State.EnvColor);
		AddCombinerStage(Color, D3DTOP_ADD, 0xFFFFFFFF);
		break;
	case D_Tex0Color:
		AddCombinerStage(D3DTA_TEXTURE, D3DTOP_ADD, 0);
		break;
	case D_Tex1Color:
		AddCombinerStage(D3DTA_TEXTURE, D3DTOP_ADD, 1);
		break;
	}
}



void HighlightCombiner(U32 Color)
{
	BlendPasses.Count = 1;
	// nice to see which combiner does what
	BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
	BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
	BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
	BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
	BlendPasses.Passes[0].TFactor = 0xFF000000|Color;
	BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
	BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
}

__forceinline void CombineSetTexture(U32 Pass, U32 Stage, U32 TextureIndex)
{
	if (TextureIndex==1){
		BlendPasses.Passes[Pass].Stages[Stage].Texture = State.Textures[Texture1].D3DObject;
		BlendPasses.Passes[Pass].Stages[Stage].AddressU = State.Address1U;
		BlendPasses.Passes[Pass].Stages[Stage].AddressV = State.Address1V;
	} else {
		BlendPasses.Passes[Pass].Stages[Stage].Texture = State.Textures[Texture2].D3DObject;
		BlendPasses.Passes[Pass].Stages[Stage].AddressU = State.Address2U;
		BlendPasses.Passes[Pass].Stages[Stage].AddressV = State.Address2V;
		SecondTex[Stage] = true;
	}
}

__forceinline void Template_FireEffect(U32 BlendFactor, bool SwapTextures, U32 AlphaModulate)
{
	// blah1 = (((PrimColor-EnvColor)*BlendFactor)&0x00FFFFFF) | ((BlendFactor*AlphaModulate)&0xFF000000)
	// Color: Tex1Color*blah1-PrimColor*blah1         +  [Tex0Color*(PrimColor-EnvColor)+EnvColor]
	// Alpha: Tex1Alpha*blah1-          blah1         +  [Tex0Alpha*AlphaModulate]

	U32 col1, col2;
	U32 Tex1, Tex2;
	Tex1 = (SwapTextures?Texture1:Texture2);
	Tex2 = (SwapTextures?Texture2:Texture1);
	BlendPasses.Count = 1;
	BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
	BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
	BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADDSIGNED;
	BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
	BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
	BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
	BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
	BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_ADDSIGNED;
	BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
	BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
	//col1 = MultColor((AlphaModulate&0xFF000000)|(SubtractColor(State.PrimColor, State.EnvColor)&0x00FFFFFF), BlendFactor);
	col1 = (MultColor(SubtractColor(State.PrimColor, State.EnvColor), BlendFactor)&0x00FFFFFF) | (MultColor(BlendFactor, AlphaModulate)&0xFF000000);
	col2 = AddColor(MultColor(col1, 0xFF000000|State.PrimColor), 0x80808080);
	BlendPasses.Passes[0].Stages[0].Texture = Prerender_ModulateSubTexColor(col1, col2, Tex1);
	BlendPasses.Passes[0].Stages[1].Texture = Prerender_ModulateAddTexColor((SubtractColor(State.PrimColor, State.EnvColor)&0x00FFFFFF)|(AlphaModulate&0xFF000000), State.EnvColor&0x00FFFFFF, Tex2);
	if (SwapTextures){
		SecondTex[1] = true;
		BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
		BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
		BlendPasses.Passes[0].Stages[1].AddressU = State.Address2U;
		BlendPasses.Passes[0].Stages[1].AddressV = State.Address2V;
	} else {
		SecondTex[0] = true;
		BlendPasses.Passes[0].Stages[0].AddressU = State.Address2U;
		BlendPasses.Passes[0].Stages[0].AddressV = State.Address2V;
		BlendPasses.Passes[0].Stages[1].AddressU = State.Address1U;
		BlendPasses.Passes[0].Stages[1].AddressV = State.Address1V;
	}
}

__forceinline void Template_BlendTextureColor(U32 Factor)
{
	// Color: Tex0Color*(Factor*(PrimColor-EnvColor)-PrimColor*Factor*(PrimColor-EnvColor))+(PrimColor-EnvColor) + EnvColor
	U32 col1;
	U32 Tex1;
	U32 Diff;
	TTile *Tile1;
	Tex1 = Texture1;
	Tile1 = State.texture.tile;
	BlendPasses.Count = 1;
	BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
	BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
	BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
	// Diff = PrimColor-EnvColor
	// Modulate texture with Diff*(Factor-PrimColor*Factor) + Diff
	// then add EnvColor
	Diff = SubtractColor(State.PrimColor, State.EnvColor);
	col1 = AddColor(Diff, MultColor(Diff, SubtractColor(Factor, MultColor(State.PrimColor, Factor))));
	BlendPasses.Passes[0].Stages[0].Texture = Prerender_ModulateAddTexColor(col1, State.EnvColor, Tex1);
	BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
	BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
}

void FreeDatFile()
{
	if (CombineScripts.Script!=0){
		free(CombineScripts.Script);
		CombineScripts.Script = 0;
	}
	CombineScripts.Count = 0;
}


int _httoi(const TCHAR *value)
{
  struct CHexMap
  {
    TCHAR chr;
    int value;
  };
  const int HexMapL = 16;
  CHexMap HexMap[HexMapL] =
  {
    {'0', 0}, {'1', 1},
    {'2', 2}, {'3', 3},
    {'4', 4}, {'5', 5},
    {'6', 6}, {'7', 7},
    {'8', 8}, {'9', 9},
    {'A', 10}, {'B', 11},
    {'C', 12}, {'D', 13},
    {'E', 14}, {'F', 15}
  };
  TCHAR *mstr = _tcsupr(_tcsdup(value));
  TCHAR *s = mstr;
  int result = 0;
  if (*s == '0' && *(s + 1) == 'X') s += 2;
  bool firsttime = true;
  while (*s != '\0')
  {
    bool found = false;
    for (int i = 0; i < HexMapL; i++)
    {
      if (*s == HexMap[i].chr)
      {
        if (!firsttime) result <<= 4;
        result |= HexMap[i].value;
        found = true;
        break;
      }
    }
    if (!found) break;
    s++;
    firsttime = false;
  }
  free(mstr);
  return result;
}

bool GetNextArgument(char **str, U32 *Value)
{
	// if next parameter is not gived, Value is set to 0xFFFFFFFF
	char newstr[100];
	if (strlen(*str)==0){
		*Value = 0xFFFFFFFF;
		return false;
	}
	if (strstr(*str, ", ")==0){
		*Value = _httoi(*str);
		*str = "";
		return true;
	} else {
		strncpy(&newstr[0], *str, strlen(*str));
		if (strstr(&newstr[0], ", ")){
			strstr(&newstr[0], ", ")[0] = 0;
		}
		*Value = _httoi(&newstr[0]);
		*str = strstr(*str, ", ")+2;
		return true;
	}
}

S32 ScriptIndex;
void HandleCommand(char *str, char *commandstr, TScriptCommandType Type)
{
	U32 i;
	bool Yup;
	Yup = true;
	for (i=0;i<strlen(commandstr);i++){
		if (commandstr[i]!=str[i]){
			Yup = false;
			break;
		}
	}
	U32 Arg;
	if (Yup){
		CombineScripts.Script[ScriptIndex].Commands[CombineScripts.Script[ScriptIndex].Length].Command = Type;
		str = &str[i+1];

		GetNextArgument(&str, &Arg);
		CombineScripts.Script[ScriptIndex].Commands[CombineScripts.Script[ScriptIndex].Length].Param1 = Arg;

		GetNextArgument(&str, &Arg);
		CombineScripts.Script[ScriptIndex].Commands[CombineScripts.Script[ScriptIndex].Length].Param2 = Arg;

		GetNextArgument(&str, &Arg);
		CombineScripts.Script[ScriptIndex].Commands[CombineScripts.Script[ScriptIndex].Length].Param3 = Arg;

		GetNextArgument(&str, &Arg);
		CombineScripts.Script[ScriptIndex].Commands[CombineScripts.Script[ScriptIndex].Length].Param4 = Arg;

		GetNextArgument(&str, &Arg);
		CombineScripts.Script[ScriptIndex].Commands[CombineScripts.Script[ScriptIndex].Length].Param5 = Arg;

		GetNextArgument(&str, &Arg);
		CombineScripts.Script[ScriptIndex].Commands[CombineScripts.Script[ScriptIndex].Length].Param6 = Arg;
	}
}

__inline bool ScriptCompareLess(U32 lo, U32 hi, U32 blendextras, TScript *Script)
{
	// returns true if hi:lo are smaller than *Script
	if (hi < Script->hi){
		return true;
	} else {
		if (hi > Script->hi){
			return false;
		} else {
			if (lo < Script->lo){
				return true;
			} else {
				if (lo > Script->hi){
					return false;
				} else {
					if (blendextras < Script->blendextras){
						return true;
					} else {
						return false;
					}
				}
			}
		}
	}
}

int ScriptCompare(const void* a, const void* b)
{
	TScript ai = *((TScript*)a), bi = *((TScript*)b);
	if(ai.hi<bi.hi){
		return -1;
	} else {
		if(ai.lo>bi.lo){
			return 1;
		} else {
			if(ai.lo<bi.lo){
				return -1;
			} else {
				if(ai.lo>bi.lo){
					return 1;
				} else {
					if (ai.blendextras>bi.blendextras){
						return 1;
					} else {
						if (ai.blendextras<bi.blendextras){
							return -1;
						} else {
							return 0;
						}
					}
				}
			}
		}
	}
}

void LoadDatFile()
{
	char str[255], strlo[9], strhi[7], strextras[9];
	char *str2;
	CombineScripts.Count = 0;

	FILE *in;
	// Count combiners
	in = fopen(DatFilename, "r");
	if (in==0){
		return;
	}
	while (!feof(in)){
		fscanf(in, "%s\n", str);
		if (str[0] == '['){
			CombineScripts.Count++;
		}
	}
	fclose(in);
	CombineScripts.Script = (TScript*)malloc(CombineScripts.Count*sizeof(TScript));
	in = fopen(DatFilename, "rt");
	ScriptIndex = -1;
	while (!feof(in)){
		if (fgets(str, 256, in)&&(strlen(str)!=0)){
			if (str[0] == '['){
				str[23] = 0;
				strncpy(strhi, &str[1], 6);
				strncpy(strlo, &str[7], 8);
				strncpy(strextras, &str[15], 8);
				strlo[8] = 0;
				strhi[6] = 0;
				strextras[8] = 0;
				ScriptIndex++;
				CombineScripts.Script[ScriptIndex].hi = (_httoi(strhi)) & BitM24;
				CombineScripts.Script[ScriptIndex].lo = _httoi(strlo);
				CombineScripts.Script[ScriptIndex].blendextras = _httoi(strextras);
				CombineScripts.Script[ScriptIndex].Length = 0;
			} else {
				HandleCommand(str, "NewPass", SC_NewPass);
				str2 = strstr(str, ":")+2;
				if (str2==(char*)2){
					str2 = str;
				}
				
				HandleCommand(str2, "SetColorSources", SC_SetColorSources);
				HandleCommand(str2, "SetAlphaSources", SC_SetAlphaSources);
				HandleCommand(str2, "AssignTexture", SC_AssignTexture);
				HandleCommand(str2, "SetTFactor", SC_SetTFactor);
				HandleCommand(str2, "TempRegOperation", SC_TempRegOperation);
				HandleCommand(str2, "PrerenderTexture", SC_PrerenderTexture);
				HandleCommand(str2, "QueryDevice", SC_QueryDevice);
				CombineScripts.Script[ScriptIndex].Length++;
			}
		}
	}
	fclose(in);


	// sort the array using qsort
	qsort(&CombineScripts.Script[0], CombineScripts.Count, sizeof(TScript), &ScriptCompare);
}

void SetBlendPass(U32 Index)
{
if(!Options.bDisablePixelshader)
{
	D3DXVECTOR4 Colors[8];

	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, BlendPasses.Passes[Index].SrcBlend));
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, BlendPasses.Passes[Index].DstBlend));
	if (UseTexture1){
/*
		SafeDX(g_pd3dDevice->SetTexture(0, State.Textures[Texture1].D3DObject));
		SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, State.Address1U));
		SafeDX(g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, State.Address1V));
*/
		SafeDX(g_pd3dDevice->SetTexture(0, BlendPasses.Passes[0].Stages[0].Texture));
		SafeDX(g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, BlendPasses.Passes[0].Stages[0].AddressU));
		SafeDX(g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, BlendPasses.Passes[0].Stages[0].AddressV));
		SafeDX(g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, State.TextureFilter));
		SafeDX(g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, State.TextureFilter));
	}
	if (UseTexture2){
/*
		SafeDX(g_pd3dDevice->SetTexture(1, State.Textures[Texture2].D3DObject));
		SafeDX(g_pd3dDevice->SetTextureStageState(1, D3DTSS_ADDRESSU, State.Address2U));
		SafeDX(g_pd3dDevice->SetTextureStageState(1, D3DTSS_ADDRESSV, State.Address2V));
*/
		SafeDX(g_pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, BlendPasses.Passes[0].Stages[1].AddressU));
		SafeDX(g_pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, BlendPasses.Passes[0].Stages[1].AddressV));
		SafeDX(g_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, State.TextureFilter));
		SafeDX(g_pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, State.TextureFilter));
		SafeDX(g_pd3dDevice->SetTexture(1, BlendPasses.Passes[0].Stages[1].Texture));

	}
	Colors[0].w = (FLOAT)BITS(State.PrimColor, 24, 8)/255.0f;
	Colors[0].x = (FLOAT)BITS(State.PrimColor, 16, 8)/255.0f;
	Colors[0].y = (FLOAT)BITS(State.PrimColor,  8, 8)/255.0f;
	Colors[0].z = (FLOAT)BITS(State.PrimColor,  0, 8)/255.0f;
	
	Colors[1].w = (FLOAT)BITS(State.EnvColor, 24, 8)/255.0f;
	Colors[1].x = (FLOAT)BITS(State.EnvColor, 16, 8)/255.0f;
	Colors[1].y = (FLOAT)BITS(State.EnvColor,  8, 8)/255.0f;
	Colors[1].z = (FLOAT)BITS(State.EnvColor,  0, 8)/255.0f;
	
	Colors[2].w = (FLOAT)State.Lod/255.0f;
	Colors[2].x = Colors[2].w;
	Colors[2].y = Colors[2].w;
	Colors[2].z = Colors[2].w;

	Colors[3].x = 0.5f;
	Colors[3].y = 0.5f;
	Colors[3].z = 0.5f;
	Colors[3].w = 0.5f;

	Colors[4].x = 0.0f;
	Colors[4].y = 0.0f;
	Colors[4].z = 0.0f;
	Colors[4].w = 0.0f;

	Colors[5].x = 1.0f;
	Colors[5].y = 1.0f;
	Colors[5].z = 1.0f;
	Colors[5].w = 1.0f;

	Colors[6].x = 1.0f;
	Colors[6].y = 0.0f;
	Colors[6].z = 1.0f;
	Colors[6].w = 1.0f;

/*
	Colors[7].w = ((FLOAT)BITS(State.PrimColor, 24, 8)-(FLOAT)BITS(State.EnvColor, 24, 8)/255.0f);
	Colors[7].x = ((FLOAT)BITS(State.PrimColor, 16, 8)-(FLOAT)BITS(State.EnvColor, 16, 8)/255.0f);
	Colors[7].y = ((FLOAT)BITS(State.PrimColor,  8, 8)-(FLOAT)BITS(State.EnvColor,  8, 8)/255.0f);
	Colors[7].z = ((FLOAT)BITS(State.PrimColor,  0, 8)-(FLOAT)BITS(State.EnvColor,  0, 8)/255.0f);
*/
	Colors[7].w = (FLOAT)State.Convert.K5/255.0f;
	Colors[7].x = Colors[7].w;
	Colors[7].y = Colors[7].w;
	Colors[7].z = Colors[7].w;
	SafeDX(g_pd3dDevice->SetPixelShaderConstantF(0, (float*)&Colors[0], 8));
}
else//DisablePixelshader
{
			SafeDX(g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, BlendPasses.Passes[Index].SrcBlend));
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, BlendPasses.Passes[Index].DstBlend));
	SafeDX(g_pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, BlendPasses.Passes[Index].TFactor));
	U32 i; // stage
	for (i=0;i<8;i++){
		SafeDX(g_pd3dDevice->SetTextureStageState(i, D3DTSS_COLOROP, BlendPasses.Passes[Index].Stages[i].ColorOp));
		SafeDX(g_pd3dDevice->SetTextureStageState(i, D3DTSS_ALPHAOP, BlendPasses.Passes[Index].Stages[i].AlphaOp));
		if (BlendPasses.Passes[Index].Stages[i].ColorOp == D3DTOP_DISABLE){
			break;
		}
		SafeDX(g_pd3dDevice->SetTextureStageState(i, D3DTSS_COLORARG1, BlendPasses.Passes[Index].Stages[i].ColorArg1));
		SafeDX(g_pd3dDevice->SetTextureStageState(i, D3DTSS_COLORARG2, BlendPasses.Passes[Index].Stages[i].ColorArg2));
		//SafeDX(g_pd3dDevice->SetTextureStageState(i, D3DTSS_COLORARG0, BlendPasses.Passes[Index].Stages[i].ColorArg3));
		SafeDX(g_pd3dDevice->SetTextureStageState(i, D3DTSS_ALPHAARG1, BlendPasses.Passes[Index].Stages[i].AlphaArg1));
		SafeDX(g_pd3dDevice->SetTextureStageState(i, D3DTSS_ALPHAARG2, BlendPasses.Passes[Index].Stages[i].AlphaArg2));
		//SafeDX(g_pd3dDevice->SetTextureStageState(i, D3DTSS_ALPHAARG0, BlendPasses.Passes[Index].Stages[i].AlphaArg3));
		SafeDX(g_pd3dDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, BlendPasses.Passes[Index].Stages[i].AddressU));
		SafeDX(g_pd3dDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, BlendPasses.Passes[Index].Stages[i].AddressV));
		SafeDX(g_pd3dDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, State.TextureFilter));
		SafeDX(g_pd3dDevice->SetSamplerState(i, D3DSAMP_MINFILTER, State.TextureFilter));
		SafeDX(g_pd3dDevice->SetTexture(i, BlendPasses.Passes[Index].Stages[i].Texture));
		if ((BlendPasses.Passes[Index].Stages[i].ColorArg1==D3DTA_TEXTURE) ||
			(BlendPasses.Passes[Index].Stages[i].ColorArg2==D3DTA_TEXTURE) ||
			(BlendPasses.Passes[Index].Stages[i].AlphaArg1==D3DTA_TEXTURE) ||
			(BlendPasses.Passes[Index].Stages[i].AlphaArg2==D3DTA_TEXTURE)){
			if (BlendPasses.Passes[Index].Stages[i].Texture==0){
				ShowError("Error SetBlendPass: Texture used but not set");
			}
		}
	}
	}
}

combtree *PSDoColorPass(combtree *firstpass, int a, int b, int c, int d)
{
	CombinerSource source;
	__int32 combconstant;
	combtree *thistree;
	bool bCombinedColor;

	combconstant = 0;
	bCombinedColor = false;
	switch(a){
	case A_ShadeColor:
		source = diffuse;
		break;
	case A_PrimColor:
		source = constant;
		combconstant = 1;
		break;
	case A_EnvColor:
		source = constant;
		combconstant = 2;
		break;
	case A_Tex0Color:
		source = tex0;
		break;
	case A_Tex1Color:
		source = tex1;
		break;
	case A_0:
		source = zero;
		break;
	case A_1:
		source = one;
		break;
	case A_Noise:
		source = zero;  // is there some way to do this in ps?
		break;
	case A_CombinedColor:
		bCombinedColor = true;
		break;
	default:
		source = zero;
		ShowError("Combiner Error: Unhandled Field in A");
		break;
	}
	if ((firstpass==NULL) || (!bCombinedColor)){
		thistree = new combtree(source, combconstant);
	} else {
		thistree = new combtree(firstpass->root);
	}

	combconstant = 0;
	bCombinedColor = false;
	switch(b){
	case B_Tex0Color:
		source = tex0;
		break;
	case B_Tex1Color:
		source = tex1;
		break;
	case B_PrimColor:
		source = constant;
		combconstant = 1;
		break;
	case B_ShadeColor:
		source = diffuse;
		break;
	case B_EnvColor:
		source = constant;
		combconstant = 2;
		break;
	case B_KeyCenter:
		source = zero;
		break;
	case B_ConertK4:
		source = zero;
		break;
	case B_0:
		source = zero;
		break;
	case B_CombinedColor:
		bCombinedColor = true;
		break;
	default:
		source = zero;
		ShowError("Combiner Error: Unhandled Field in B");
		break;
	}

	if ((firstpass==NULL) || (!bCombinedColor)){
		thistree->AddOpTop(sub, source, combconstant);
	} else {
		thistree->AddNodeTop(sub, true, firstpass->root);
	}
	
	bCombinedColor = false;
	combconstant = 0;
	switch(c){
	case C_Tex0Color:
		source = tex0;
		break;
	case C_Tex1Color:
		source = tex1;
		break;
	case C_Tex0Alpha:
		source = tex0alpha;
		break;
	case C_Tex1Alpha:
		source = tex1alpha;
		break;
	case C_PrimColor:
		source = constant;
		combconstant = 1;
		break;
	case C_ShadeColor:
		source = diffuse;
		break;
	case C_ShadeAlpha:
		source = diffusealpha;
		break;
	case C_EnvColor:
		source = constant;
		combconstant = 2;
		break;
	case C_EnvAlpha:
		source = constant;
		combconstant = -2;
		break;
	case C_PrimAlpha:
		source = constant;
		combconstant = -1;
		break;
	case C_0:
		source = zero;
		break;
	case C_PrimLODFrac:
		source = constant;
		combconstant = 3;
		break;
	case C_CombinedColor:
		bCombinedColor = true;
		break;
	case C_CombinedAlpha:
		source = combinedalpha;
		ShowError("Combiner Problem: CombinedAlpha for Color not yet supported.");
		break;
	case C_LODFraction:
		source = constant;
		combconstant = 4;
		break;
	case C_K5:
		source = constant;
		combconstant = 8;
		break;
	default:
		source = diffuse;
		ShowError("Combiner Error: Unhandled Field in C");
		break;
	}
	if ((firstpass==NULL) || (!bCombinedColor)){
		thistree->AddOpTop(mul, source, combconstant);
	} else {
		thistree->AddNodeTop(mul, true, firstpass->root);
	}

	bCombinedColor = false;
	combconstant = 0;
	switch(d){
	case D_Tex0Color:
		source = tex0;
		break;
	case D_Tex1Color:
		source = tex1;
		break;
	case D_PrimColor:
		source = constant;
		combconstant = 1;
		break;
	case D_ShadeColor:
		source = diffuse;
		break;
	case D_EnvColor:
		source = constant;
		combconstant = 2;
		break;
	case D_0:
		source = zero;
		break;
	case D_1:
		source = one;
		break;
	case D_CombinedColor:
		bCombinedColor = true;
		break;
	default:
		source = zero;
		ShowError("Combiner Error: Unhandled Field in D");
		break;
	}
	if ((firstpass==NULL) || (!bCombinedColor)){
		thistree->AddOpTop(add, source, combconstant);
	} else {
		thistree->AddNodeTop(add, true, firstpass->root);
	}
	return thistree;
}

combtree *PSDoAlphaPass(combtree *firstpass, int a, int b, int c, int d)
{
	CombinerSource source;
	__int32 combconstant;
	combtree *thistree;
	bool bCombinedAlpha;

	combconstant = 0;
	bCombinedAlpha = false;
	switch(a){
	case AA_ShadedAlpha:
		source = diffusealpha;
		break;
	case AA_PrimAlpha:
		source = constant;
		combconstant = 1;
		break;
	case AA_EnvAlpha:
		source = constant;
		combconstant = 2;
		break;
	case AA_Tex0Alpha:
		source = tex0alpha;
		break;
	case AA_Tex1Alpha:
		source = tex1alpha;
		break;
	case AA_0:
		source = zero;
		break;
	case AA_1:
		source = one;
		break;
	case AA_CombinedAlpha:
		bCombinedAlpha = true;
		break;
	default:
		source = zero;
		ShowError("Combiner Error: Unhandled Field in Alpha A");
		break;
	}
	if ((firstpass==NULL) || (!bCombinedAlpha)){
		thistree = new combtree(source, combconstant);
	} else {
		thistree = new combtree(firstpass->root);
	}

	combconstant = 0;
	bCombinedAlpha = false;
	switch(b){
	case BA_Tex0Alpha:
		source = tex0alpha;
		break;
	case BA_Tex1Alpha:
		source = tex1alpha;
		break;
	case BA_PrimAlpha:
		source = constant;
		combconstant = 1;
		break;
	case BA_ShadedAlpha:
		source = diffusealpha;
		break;
	case BA_EnvAlpha:
		source = constant;
		combconstant = 2;
		break;
	case BA_0:
		source = zero;
		break;
	case BA_1:
		source = one;
		break;
	case BA_CombinedAlpha:
		bCombinedAlpha = true;
		break;
	default:
		source = zero;
		ShowError("Combiner Error: Unhandled Field in Alpha B");
		break;
	}

	if ((firstpass==NULL) || (!bCombinedAlpha)){
		thistree->AddOpTop(sub, source, combconstant);
	} else {
		thistree->AddNodeTop(sub, true, firstpass->root);
	}
	
	bCombinedAlpha = false;
	combconstant = 0;
	switch(c){
	case CA_Tex0Alpha:
		source = tex0alpha;
		break;
	case CA_Tex1Alpha:
		source = tex1alpha;
		break;
	case CA_PrimAlpha:
		source = constant;
		combconstant = 1;
		break;
	case CA_ShadedAlpha:
		source = diffusealpha;
		break;
	case CA_EnvAlpha:
		source = constant;
		combconstant = 2;
		break;
	case CA_0:
		source = zero;
		break;
	case CA_PrimLODFrac:
		source = constant;
		combconstant = 3;
		break;
	case CA_LODFraction:
		source = constant;
		combconstant = 4;
		break;
	default:
		ShowError("Combiner Error: Unhandled Field in Alpha C");
		break;
	}
	if ((firstpass==NULL) || (!bCombinedAlpha)){
		thistree->AddOpTop(mul, source, combconstant);
	} else {
		thistree->AddNodeTop(mul, true, firstpass->root);
	}

	bCombinedAlpha = false;
	combconstant = 0;
	switch(d){
	case DA_Tex0Alpha:
		source = tex0alpha;
		break;
	case DA_Tex1Alpha:
		source = tex1alpha;
		break;
	case DA_PrimAlpha:
		source = constant;
		combconstant = 1;
		break;
	case DA_ShadedAlpha:
		source = diffusealpha;
		break;
	case DA_EnvAlpha:
		source = constant;
		combconstant = 2;
		break;
	case DA_0:
		source = zero;
		break;
	case DA_1:
		source = one;
		break;
	case DA_CombinedAlpha:
		bCombinedAlpha = true;
		break;
	default:
		source = zero;
		ShowError("Combiner Error: Unhandled Field in Alpha D");
		break;
	}
	if ((firstpass==NULL) || (!bCombinedAlpha)){
		thistree->AddOpTop(add, source, combconstant);
	} else {
		thistree->AddNodeTop(add, true, firstpass->root);
	}
	return thistree;
}

void ConfigureCombiner(bool TexturesEnabled)
{
if(!Options.bDisablePixelshader)
{
	combtree *firstpass;
	combtree *bothpasses;
	combtree *alphafirstpass;
	combtree *alphabothpasses;
	char sColor[1000];
	char sAlpha[1000];
	char sAlpha2[1000];
	LPD3DXBUFFER pCode;
	LPD3DXBUFFER pErrors;
	U32 i;
	bool bIsNew;
	U32 LogCombineModeIndex;

	if (State.CombineChanged){
		bIsNew = true;
		for (i=0;i<LoggedCombineModes.Count;i++){
			if ((LoggedCombineModes.Modes[i].lo == State.Combine.raw.lo) && (LoggedCombineModes.Modes[i].hi == State.Combine.raw.hi) && (LoggedCombineModes.Modes[i].blendextras == State.BlendExtras)){
				bIsNew = false;
				LoggedCombineModes.Modes[i].Age=0;
				LogCombineModeIndex = i;
				break;
			}
		}
		bHighlighted = false;
		if (bIsNew){
			LoggedCombineModes.Modes[LoggedCombineModes.Count].lo = State.Combine.raw.lo;
			LoggedCombineModes.Modes[LoggedCombineModes.Count].hi = State.Combine.raw.hi;
			LoggedCombineModes.Modes[LoggedCombineModes.Count].blendextras = State.BlendExtras;
			LoggedCombineModes.Modes[LoggedCombineModes.Count].Age=0;
			LoggedCombineModes.Modes[LoggedCombineModes.Count].Highlight=false;
			LoggedCombineModes.Modes[LoggedCombineModes.Count].EmulationMode = 4;
			LogCombineModeIndex = LoggedCombineModes.Count;
			LoggedCombineModes.Count++;
		}

		if ((State.Othermode.fields.cycletype==2) || (State.Othermode.fields.cycletype==3)){
			if (TexturesEnabled){
				SafeDX(g_pd3dDevice->SetPixelShader(State.ps_textureonly));
			} else {
				SafeDX(g_pd3dDevice->SetPixelShader(State.ps_diffuseonly));
			}
		} else {
			if (!bIsNew){
				if (LoggedCombineModes.Modes[LogCombineModeIndex].Highlight){
					SafeDX(g_pd3dDevice->SetPixelShader(State.ps_highlight));
				} else {
					SafeDX(g_pd3dDevice->SetPixelShader(LoggedCombineModes.Modes[LogCombineModeIndex].pixelshaderhandle));
				}
			} else {
				firstpass = PSDoColorPass(NULL,
					State.Combine.fields.a0,
					State.Combine.fields.b0,
					State.Combine.fields.c0,
					State.Combine.fields.d0);

				if (State.Othermode.fields.cycletype==1){
					bothpasses = PSDoColorPass(firstpass,
						State.Combine.fields.a1,
						State.Combine.fields.b1,
						State.Combine.fields.c1,
						State.Combine.fields.d1);
					delete firstpass;
				} else {
					bothpasses = firstpass;
				}

				alphafirstpass = PSDoAlphaPass(NULL,
					State.Combine.fields.Aa0,
					State.Combine.fields.Ab0,
					State.Combine.fields.Ac0,
					State.Combine.fields.Ad0);
				
				if (State.Othermode.fields.cycletype==1){
					if (State.Combine.fields.c1==C_CombinedAlpha){
						alphafirstpass->AddSecondPassCommand();
					}
					alphabothpasses = PSDoAlphaPass(alphafirstpass,
						State.Combine.fields.Aa1,
						State.Combine.fields.Ab1,
						State.Combine.fields.Ac1,
						State.Combine.fields.Ad1);
					delete alphafirstpass;
				} else {
					alphabothpasses = alphafirstpass;
				}
				
				char s2[3000], s3[2000];
				bothpasses->getstring(s2);
				bothpasses->Optimize();
				bothpasses->getstring(s3);
				if (strlen(s2)+strlen(s3)+5 < sizeof(LoggedCombineModes.Modes[LogCombineModeIndex].tree)){
					strcpy(LoggedCombineModes.Modes[LogCombineModeIndex].tree, s2);
					strcat(LoggedCombineModes.Modes[LogCombineModeIndex].tree, " *|* ");
					strcat(LoggedCombineModes.Modes[LogCombineModeIndex].tree, s3);
				} else {
					strcpy(LoggedCombineModes.Modes[LogCombineModeIndex].tree, "too big");
				}
				alphabothpasses->Optimize();

				bool bUseTex1, bUseTex2;
				bUseTex1 = false;
				bUseTex2 = false;
				bothpasses->getpixelshader(sColor, false, false, &bUseTex1, &bUseTex2);
				alphabothpasses->getpixelshader(sAlpha, true, true, &bUseTex1, &bUseTex2);
				if ((State.Othermode.fields.cycletype==1) && (State.Combine.fields.c1==C_CombinedAlpha)){
					alphabothpasses->getpixelshader(sAlpha2, true, false, &bUseTex1, &bUseTex2);
				} else {
					sAlpha2[0] = 0;
				}
				strcpy(BlendPasses.Passes[0].strPixelShader, "ps.1.1\n");
				if (bUseTex1){
					strcat(BlendPasses.Passes[0].strPixelShader, "tex t0\n");
				}
				if (bUseTex2){
					strcat(BlendPasses.Passes[0].strPixelShader, "tex t1\n");
				}
				strcat(BlendPasses.Passes[0].strPixelShader, sAlpha);
				strcat(BlendPasses.Passes[0].strPixelShader, "\n");
				strcat(BlendPasses.Passes[0].strPixelShader, sColor);
				if ((State.Othermode.fields.cycletype==1) && (State.Combine.fields.c1==C_CombinedAlpha)){
					strcat(BlendPasses.Passes[0].strPixelShader, "\n");
					strcat(BlendPasses.Passes[0].strPixelShader, sAlpha2);
				}
				delete bothpasses;
				delete alphabothpasses;

				if (strlen(BlendPasses.Passes[0].strPixelShader) > sizeof(LoggedCombineModes.Modes[LoggedCombineModes.Count].pixelshader)){
					ShowError("Error: Pixelshader got too big!");
				} else {
					strcpy(LoggedCombineModes.Modes[LogCombineModeIndex].pixelshader, BlendPasses.Passes[0].strPixelShader);
				}

				// Compile Pixelshader
				if (D3DXAssembleShader(
						&BlendPasses.Passes[0].strPixelShader[0],
						strlen(BlendPasses.Passes[0].strPixelShader),
						0,
						NULL,
						0,
						&pCode,
						&pErrors)!=D3D_OK){
					ShowError((char*)pErrors->GetBufferPointer());
				}else{
					SafeDX(g_pd3dDevice->CreatePixelShader((DWORD*)pCode->GetBufferPointer(),
														&m_pPixelShader));
					SafeDX(g_pd3dDevice->SetPixelShader(m_pPixelShader));
					LoggedCombineModes.Modes[LogCombineModeIndex].pixelshaderhandle = m_pPixelShader;
				}
				if (pCode){
					SafeDX(pCode->Release());
					pCode = NULL;
				}
				if (pErrors){
					SafeDX(pErrors->Release());
					pErrors = NULL;
				}
			}
		}
		BlendPasses.Count = 1;
		BlendPasses.Passes[0].SrcBlend = D3DBLEND_SRCALPHA;
		BlendPasses.Passes[0].DstBlend = D3DBLEND_INVSRCALPHA;
	}
	if (UseTexture1){
		BlendPasses.Passes[0].Stages[0].Texture = State.Textures[Texture1].D3DObject;
		BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
		BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
		SecondTex[0] = false;
	}
	if (UseTexture2){
		BlendPasses.Passes[0].Stages[1].Texture = State.Textures[Texture2].D3DObject;
		BlendPasses.Passes[0].Stages[1].AddressU = State.Address2U;
		BlendPasses.Passes[0].Stages[1].AddressV = State.Address2V;
		SecondTex[1] = true;
		ChangeTextureCoordinates = true;
	} else {
		ChangeTextureCoordinates = false;
	}
}
else//DisablePixelshader
{
	U32 i;
	bool LogCombinerOutput, Highlight=false;
#if COMBINEDEBUGGER
	U32 LogCombineModeIndex;
#endif
	if (State.CombineChanged || State.TextureChanged){
#if (_DEBUG||COMBINEDEBUGGER)
		LogCombinerOutput = true;
		for (i=0;i<LoggedCombineModes.Count;i++){
#if COMBINEDEBUGGER
			if ((LoggedCombineModes.Modes[i].lo == State.Combine.raw.lo) && (LoggedCombineModes.Modes[i].hi == State.Combine.raw.hi) && (LoggedCombineModes.Modes[i].blendextras == State.BlendExtras)){
				LogCombinerOutput = false;
				LoggedCombineModes.Modes[i].Age=0;
				if (LoggedCombineModes.Modes[i].Highlight){
					Highlight = true;
				}
				LogCombineModeIndex = i;
				break;
			}
#endif
		}
		bHighlighted = false;
		if (LogCombinerOutput){
			LoggedCombineModes.Modes[LoggedCombineModes.Count].lo = State.Combine.raw.lo;
			LoggedCombineModes.Modes[LoggedCombineModes.Count].hi = State.Combine.raw.hi;
			LoggedCombineModes.Modes[LoggedCombineModes.Count].blendextras = State.BlendExtras;
#if COMBINEDEBUGGER
			LoggedCombineModes.Modes[LoggedCombineModes.Count].Age=0;
			LoggedCombineModes.Modes[LoggedCombineModes.Count].Highlight=false;
			LogCombineModeIndex = LoggedCombineModes.Count;
#endif
			LoggedCombineModes.Count++;
		}
#else
		LogCombinerOutput = false;
#endif
		if (LogCombinerOutput){
			if (State.Othermode.fields.cycletype<=1){
				//Log("Combine Cycle1:       (%s - %s) * %s + %s\n", Combine_A[State.Combine.fields.a0],  Combine_B[State.Combine.fields.b0],  Combine_C[State.Combine.fields.c0],  Combine_D[State.Combine.fields.d0]);
				//Log("Combine Alpha Cycle1: (%s - %s) * %s + %s\n", CombinA_A[State.Combine.fields.Aa0], CombinA_B[State.Combine.fields.Ab0], CombinA_C[State.Combine.fields.Ac0], CombinA_D[State.Combine.fields.Ad0]);
				if (State.Othermode.fields.cycletype==1){
					//Log("Combine Cycle2:       (%s - %s) * %s + %s\n", Combine_A[State.Combine.fields.a1],  Combine_B[State.Combine.fields.b1],  Combine_C[State.Combine.fields.c1],  Combine_D[State.Combine.fields.d1]);
					//Log("Combine Alpha Cycle2: (%s - %s) * %s + %s\n", CombinA_A[State.Combine.fields.Aa1], CombinA_B[State.Combine.fields.Ab1], CombinA_C[State.Combine.fields.Ac1], CombinA_D[State.Combine.fields.Ad1]);
					//Log("BuildCombineMode2(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)\n", Combine_A[State.Combine.fields.a0],  Combine_B[State.Combine.fields.b0],  Combine_C[State.Combine.fields.c0],  Combine_D[State.Combine.fields.d0], CombinA_A[State.Combine.fields.Aa0], CombinA_B[State.Combine.fields.Ab0], CombinA_C[State.Combine.fields.Ac0], CombinA_D[State.Combine.fields.Ad0],
					//	Combine_A[State.Combine.fields.a1],  Combine_B[State.Combine.fields.b1],  Combine_C[State.Combine.fields.c1],  Combine_D[State.Combine.fields.d1], CombinA_A[State.Combine.fields.Aa1], CombinA_B[State.Combine.fields.Ab1], CombinA_C[State.Combine.fields.Ac1], CombinA_D[State.Combine.fields.Ad1]);
				} else {									 
					//Log("BuildCombineMode1(%s, %s, %s, %s, %s, %s, %s, %s)\n", Combine_A[State.Combine.fields.a0],  Combine_B[State.Combine.fields.b0],  Combine_C[State.Combine.fields.c0],  Combine_D[State.Combine.fields.d0], CombinA_A[State.Combine.fields.Aa0], CombinA_B[State.Combine.fields.Ab0], CombinA_C[State.Combine.fields.Ac0], CombinA_D[State.Combine.fields.Ad0]);
				}
			}
		}
		for (i=0;i<8;i++){
			g_ColorTexture[i].Used = false;
			SecondTex[i] = false;
		}
		memset(&BlendPasses, 0, sizeof(TBlendPasses));
		BlendPasses.Count = 1;
		U32 j;
		for (i=0;i<MaxBlendPasses;i++){
			for (j=0;j<8;j++){
				BlendPasses.Passes[i].Stages[j].ColorOp = D3DTOP_DISABLE;
				BlendPasses.Passes[i].Stages[j].AlphaOp = D3DTOP_DISABLE;
			}
			BlendPasses.Passes[i].SrcBlend = D3DBLEND_SRCALPHA;
			BlendPasses.Passes[i].DstBlend = D3DBLEND_INVSRCALPHA;
		}

		static bool CombinerHandled=false;
		if (!Highlight){
			if (((State.Othermode.fields.cycletype==2) || (State.Othermode.fields.cycletype==3))){
				if (TexturesEnabled){
					BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
					BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
					BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
					BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
					CombineSetTexture(0, 0, 1);
				} else {
					BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
					BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
					BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
					BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
				}
				CombinerHandled = true;
			} else {
				// look for a script
				U32 j;
				static U32 CombinerIndex=0;
				if (State.CombineChanged){
					CombinerHandled = false;
#if 0
					dashier mal fertig machen!!!
					U32 mi, le, ri;
					// Binary seach - algorythm from Delphi 2 book :)
					// The scripts are pre-sorted with qsort
					le = 0;
					ri = CombineScripts.Count-1;
					do {
						mi = (le + ri) >> 1;
						if (ScriptCompareLess(State.Combine.raw.lo, State.Combine.raw.hi, &CombineScripts.Script[mi])){
							ri = mi - 1;
						} else {
							le = mi + 1;
						}
					} while (!((le > ri) || ((State.Combine.raw.lo == CombineScripts.Script[mi].lo) && (State.Combine.raw.hi == CombineScripts.Script[mi].hi))));

					if ((State.Combine.raw.lo == CombineScripts.Script[mi].lo) && (State.Combine.raw.hi == CombineScripts.Script[mi].hi)){
						CombinerHandled = true;
						// blender mode correct?
						if (CombineScripts.Script[mi].blendextras != State.BlendExtras){
							// it's not
							// check if there is one with more accurate extra (blender) features
							// right now there are only two possibilites: transparency disabled or its not
							if ((mi>0) && (CombineScripts.Script[mi-1].blendextras == State.BlendExtras)
									   && (CombineScripts.Script[mi-1].lo == State.Combine.raw.lo)
									   && (CombineScripts.Script[mi-1].hi == State.Combine.raw.hi)){
								CombinerIndex = mi-1;
							} else {
								if ((mi<CombineScripts.Count-1)
									   && (CombineScripts.Script[mi+1].blendextras == State.BlendExtras)
									   && (CombineScripts.Script[mi+1].lo == State.Combine.raw.lo)
									   && (CombineScripts.Script[mi+1].hi == State.Combine.raw.hi)){
									CombinerIndex = mi+1;
								} else {
									CombinerIndex = mi;
								}
							}
						} else {
							CombinerIndex = mi;
						}
					}
#else
					U32 si;
					for (si=0;si<CombineScripts.Count;si++){
						if ((CombineScripts.Script[si].hi == State.Combine.raw.hi) && (CombineScripts.Script[si].lo == State.Combine.raw.lo)){
							if (!CombinerHandled){
								// First combiner found
								CombinerIndex = si;
								CombinerHandled = true;
							} else {
								// Check which one matches better
								if (CombineScripts.Script[si].blendextras == State.BlendExtras){
									CombinerIndex = si;
									break;
								}
							}
						}
					}
#endif
				}
				if (CombinerHandled){
#if COMBINEDEBUGGER
					if (CombineScripts.Script[CombinerIndex].blendextras == State.BlendExtras){
						LoggedCombineModes.Modes[LogCombineModeIndex].EmulationMode = 1;
					} else {
						LoggedCombineModes.Modes[LogCombineModeIndex].EmulationMode = 3;
					}
#endif
					{
						U32 Temp1=0, Temp2=0, Temp3=0, Temp4=0, Temp5=0, Temp6=0;
						S32 CurrentPass;
						S32 ColorStage, AlphaStage;
						BOOL bConditionFailed;
						bConditionFailed = false;
						CurrentPass = -1;
						TScriptCommand *Cmd;
						LPDIRECT3DTEXTURE9 Prerendered1, Prerendered2;
						Prerendered1 = 0;
						Prerendered2 = 0;
						for (j=0;j<CombineScripts.Script[CombinerIndex].Length;j++){
							Cmd = &CombineScripts.Script[CombinerIndex].Commands[j];
							if (bConditionFailed){
								if (Cmd->Command==SC_NewPass){
									bConditionFailed = false;
								}
							}
							if (!bConditionFailed){
								switch (Cmd->Command){
								case SC_NewPass:
									if(Cmd->Param3==0xFFFFFFFF){
										// unconditional pass
										bConditionFailed = false;
									} else {
										// 1. get source arguments
										U32 Arg1, Arg2;
										switch (Cmd->Param4){
										case 0:  // Prim Color
											Arg1 = State.PrimColor;
											break;
										case 1:  // Prim Alpha
											Arg1 = ReplicateAlpha(State.PrimColor);
											break;
										case 2:  // Env Color
											Arg1 = State.EnvColor;
											break;
										case 3:  // Env Alpha
											Arg1 = ReplicateAlpha(State.EnvColor);
											break;
										case 4:  // Prim LOD Frac
											Arg1 = D3DCOLOR_ARGB(State.Lod, State.Lod, State.Lod, State.Lod);
											break;
										case 5:  // Temp1
											Arg1 = Temp1;
											break;
										case 6:  // Temp2
											Arg1 = Temp2;
											break;
										case 7:  // Temp3
											Arg1 = Temp3;
											break;
										case 8:  // Temp4
											Arg1 = Temp4;
											break;
										case 9:  // Temp5
											Arg1 = Temp5;
											break;
										case 10:  // Temp6
											Arg1 = Temp6;
											break;
										}
										switch (Cmd->Param5){
										case 0:  // Constant
											Arg2 = Cmd->Param6;
											break;
										case 1:  // Prim Color
											Arg2 = State.PrimColor;
											break;
										case 2:  // Prim Alpha
											Arg2 = ReplicateAlpha(State.PrimColor);
											break;
										case 3:  // Env Color
											Arg2 = State.EnvColor;
											break;
										case 4:  // Env Alpha
											Arg2 = ReplicateAlpha(State.EnvColor);
											break;
										case 5:  // Prim LOD Frac
											Arg2 = D3DCOLOR_ARGB(State.Lod, State.Lod, State.Lod, State.Lod);
											break;
										case 6:  // Temp1
											Arg2 = Temp1;
											break;
										case 7:  // Temp2
											Arg2 = Temp2;
											break;
										case 8:  // Temp3
											Arg2 = Temp3;
											break;
										case 9:  // Temp4
											Arg2 = Temp4;
											break;
										case 10:  // Temp5
											Arg2 = Temp5;
											break;
										case 11:  // Temp6
											Arg2 = Temp6;
											break;
										}
										switch(Cmd->Param3){
										case 0:  // equal
											bConditionFailed = !(Arg1==Arg2);
											break;
										case 1:  // not equal
											bConditionFailed = !(Arg1!=Arg2);
											break;
										case 2:  // greater
											bConditionFailed = !(Arg1>Arg2);
											break;
										case 3:  // greater or equal
											bConditionFailed = !(Arg1>=Arg2);
											break;
										case 4:  // smaller
											bConditionFailed = !(Arg1<Arg2);
											break;
										case 5:  // smaller or equal
											bConditionFailed = !(Arg1<=Arg2);
											break;
										}
									}
									if (!bConditionFailed){
										CurrentPass++;
										ColorStage = 0;
										AlphaStage = 0;
										BlendPasses.Passes[CurrentPass].SrcBlend = (D3DBLEND)Cmd->Param1;
										BlendPasses.Passes[CurrentPass].DstBlend = (D3DBLEND)Cmd->Param2;
									}
									break;
								case SC_SetColorSources:
									BlendPasses.Passes[CurrentPass].Stages[ColorStage].ColorOp = (D3DTEXTUREOP)Cmd->Param1;
									BlendPasses.Passes[CurrentPass].Stages[ColorStage].ColorArg1 = Cmd->Param2;
									BlendPasses.Passes[CurrentPass].Stages[ColorStage].ColorArg2 = Cmd->Param3;
									BlendPasses.Passes[CurrentPass].Stages[ColorStage].ColorArg3 = Cmd->Param4;
									ColorStage++;
									break;
								case SC_SetAlphaSources:
									BlendPasses.Passes[CurrentPass].Stages[AlphaStage].AlphaOp = (D3DTEXTUREOP)Cmd->Param1;
									BlendPasses.Passes[CurrentPass].Stages[AlphaStage].AlphaArg1 = Cmd->Param2;
									BlendPasses.Passes[CurrentPass].Stages[AlphaStage].AlphaArg2 = Cmd->Param3;
									BlendPasses.Passes[CurrentPass].Stages[AlphaStage].AlphaArg3 = Cmd->Param4;
									AlphaStage++;
									break;
								case SC_AssignTexture:
									switch (Cmd->Param2){
									case 0: // First Texture
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = State.Textures[Texture1].D3DObject;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = State.Address1U;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = State.Address1V;
										break;
									case 1: // Second Texture
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = State.Textures[Texture2].D3DObject;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = State.Address2U;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = State.Address2V;
										SecondTex[Cmd->Param1] = true;
										break;
									case 2: // Prerendered First Texture
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = Prerendered1;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = State.Address1U;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = State.Address1V;
										break;
									case 3: // Prerendered Second Texture
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = Prerendered2;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = State.Address2U;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = State.Address2V;
										SecondTex[Cmd->Param1] = true;
										break;
									case 4: // Color Texture Prim Color
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = g_ColorTexture[GenerateColorTexture(State.PrimColor)].D3DObject;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = D3DTADDRESS_WRAP;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = D3DTADDRESS_WRAP;
										break;
									case 5: // Color Texture Env Color
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = g_ColorTexture[GenerateColorTexture(State.EnvColor)].D3DObject;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = D3DTADDRESS_WRAP;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = D3DTADDRESS_WRAP;
										break;
									case 6: // Color Texture Temp1
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = g_ColorTexture[GenerateColorTexture(Temp1)].D3DObject;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = D3DTADDRESS_WRAP;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = D3DTADDRESS_WRAP;
										break;
									case 7: // Color Texture Temp2
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = g_ColorTexture[GenerateColorTexture(Temp2)].D3DObject;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = D3DTADDRESS_WRAP;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = D3DTADDRESS_WRAP;
										break;
									case 8: // Color Texture Temp3
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = g_ColorTexture[GenerateColorTexture(Temp3)].D3DObject;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = D3DTADDRESS_WRAP;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = D3DTADDRESS_WRAP;
										break;
									case 9: // Color Texture Temp4
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = g_ColorTexture[GenerateColorTexture(Temp4)].D3DObject;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = D3DTADDRESS_WRAP;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = D3DTADDRESS_WRAP;
										break;
									case 10: // Color Texture Temp5
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = g_ColorTexture[GenerateColorTexture(Temp5)].D3DObject;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = D3DTADDRESS_WRAP;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = D3DTADDRESS_WRAP;
										break;
									case 11: // Color Texture Temp6
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].Texture = g_ColorTexture[GenerateColorTexture(Temp6)].D3DObject;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressU = D3DTADDRESS_WRAP;
										BlendPasses.Passes[CurrentPass].Stages[Cmd->Param1].AddressV = D3DTADDRESS_WRAP;
										break;
									}
									break;
								case SC_SetTFactor:
									switch (Cmd->Param1){
									case 0:  // Prim Color
										BlendPasses.Passes[CurrentPass].TFactor = State.PrimColor;
										break;
									case 1:  // Env Color
										BlendPasses.Passes[CurrentPass].TFactor = State.EnvColor;
										break;
									case 2:  // Prim LOD Frac
										BlendPasses.Passes[CurrentPass].TFactor = D3DCOLOR_ARGB(0, State.Lod, State.Lod, State.Lod);
										break;
									case 3:  // Temp1
										BlendPasses.Passes[CurrentPass].TFactor = Temp1;
										break;
									case 4:  // Temp2
										BlendPasses.Passes[CurrentPass].TFactor = Temp2;
										break;
									case 5:  // Temp3
										BlendPasses.Passes[CurrentPass].TFactor = Temp3;
										break;
									case 6:  // Temp4
										BlendPasses.Passes[CurrentPass].TFactor = Temp4;
										break;
									case 7:  // Temp5
										BlendPasses.Passes[CurrentPass].TFactor = Temp5;
										break;
									case 8:  // Temp6
										BlendPasses.Passes[CurrentPass].TFactor = Temp6;
										break;
									case 9:  // Prim Alpha
										BlendPasses.Passes[CurrentPass].TFactor = ReplicateAlpha(State.PrimColor);
										break;
									case 10:  // Env Alpha
										BlendPasses.Passes[CurrentPass].TFactor = ReplicateAlpha(State.EnvColor);
										break;
									}
									BlendPasses.Passes[CurrentPass].TFactor &= 0x00FFFFFF;
									switch (Cmd->Param2){
									case 0:  // Prim Alpha
										BlendPasses.Passes[CurrentPass].TFactor |= State.PrimColor&0xFF000000;
										break;
									case 1:  // Env Alpha
										BlendPasses.Passes[CurrentPass].TFactor |= State.EnvColor&0xFF000000;
										break;
									case 2:  // Prim LOD Frac
										BlendPasses.Passes[CurrentPass].TFactor |= State.Lod<<24;
										break;
									case 3:  // Temp1
										BlendPasses.Passes[CurrentPass].TFactor |= Temp1&0xFF000000;
										break;
									case 4:  // Temp2
										BlendPasses.Passes[CurrentPass].TFactor |= Temp2&0xFF000000;
										break;
									case 5:  // Temp3
										BlendPasses.Passes[CurrentPass].TFactor |= Temp3&0xFF000000;
										break;
									case 6:  // Temp4
										BlendPasses.Passes[CurrentPass].TFactor |= Temp4&0xFF000000;
										break;
									case 7:  // Temp5
										BlendPasses.Passes[CurrentPass].TFactor |= Temp5&0xFF000000;
										break;
									case 8:  // Temp6
										BlendPasses.Passes[CurrentPass].TFactor |= Temp6&0xFF000000;
										break;
									}
									break;
								case SC_TempRegOperation:
									// 1. get source arguments
									U32 Arg1, Arg2;
									switch (Cmd->Param2){
									case 0:  // Prim Color
										Arg1 = State.PrimColor;
										break;
									case 1:  // Prim Alpha
										Arg1 = ReplicateAlpha(State.PrimColor);
										break;
									case 2:  // Env Color
										Arg1 = State.EnvColor;
										break;
									case 3:  // Env Alpha
										Arg1 = ReplicateAlpha(State.EnvColor);
										break;
									case 4:  // Prim LOD Frac
										Arg1 = D3DCOLOR_ARGB(State.Lod, State.Lod, State.Lod, State.Lod);
										break;
									case 5:  // Temp1
										Arg1 = Temp1;
										break;
									case 6:  // Temp2
										Arg1 = Temp2;
										break;
									case 7:  // Temp3
										Arg1 = Temp3;
										break;
									case 8:  // Temp4
										Arg1 = Temp4;
										break;
									case 9:  // Temp5
										Arg1 = Temp5;
										break;
									case 10:  // Temp6
										Arg1 = Temp6;
										break;
									}
									switch (Cmd->Param3){
									case 0:  // Constant
										Arg2 = Cmd->Param4;
										break;
									case 1:  // Prim Color
										Arg2 = State.PrimColor;
										break;
									case 2:  // Prim Alpha
										Arg2 = ReplicateAlpha(State.PrimColor);
										break;
									case 3:  // Env Color
										Arg2 = State.EnvColor;
										break;
									case 4:  // Env Alpha
										Arg2 = ReplicateAlpha(State.EnvColor);
										break;
									case 5:  // Prim LOD Frac
										Arg2 = D3DCOLOR_ARGB(State.Lod, State.Lod, State.Lod, State.Lod);
										break;
									case 6:  // Temp1
										Arg2 = Temp1;
										break;
									case 7:  // Temp2
										Arg2 = Temp2;
										break;
									case 8:  // Temp3
										Arg2 = Temp3;
										break;
									case 9:  // Temp4
										Arg2 = Temp4;
										break;
									case 10:  // Temp5
										Arg2 = Temp5;
										break;
									case 11:  // Temp6
										Arg2 = Temp6;
										break;
									}
									switch (Cmd->Param1){
									case 0:  // Set
										Arg1 = Arg2;
										break;
									case 1:  // Replicate Alpha
										Arg1 = ReplicateAlpha(Arg2);
										break;
									case 2:  // Logical AND
										Arg1 &= Arg2;
										break;
									case 3:  // Logical OR
										Arg1 |= Arg2;
										break;
									case 4:  // Add
										Arg1 = AddColor(Arg1, Arg2);
										break;
									case 5:  // Subtract
										Arg1 = SubtractColor(Arg1, Arg2);
										break;
									case 6:  // Modulate
										Arg1 = MultColor(Arg1, Arg2);
										break;
									}
									switch (Cmd->Param5){
									case 0:  // Temp 1
										Temp1 = Arg1;
										break;
									case 1:  // Temp 2
										Temp2 = Arg1;
										break;
									case 2:  // Temp 3
										Temp3 = Arg1;
										break;
									case 3:  // Temp 4
										Temp4 = Arg1;
										break;
									case 4:  // Temp 5
										Temp5 = Arg1;
										break;
									case 5:  // Temp 6
										Temp6 = Arg1;
										break;
									}
									break;
								case SC_PrerenderTexture:
									switch (Cmd->Param3){
									case 0:  // Prim Color
										Arg1 = State.PrimColor;
										break;
									case 1:  // Env Color
										Arg1 = State.EnvColor;
										break;
									case 2:  // Env Color
										Arg1 = Temp1;
										break;
									case 3:  // Env Color
										Arg1 = Temp2;
										break;
									case 4:  // Env Color
										Arg1 = Temp3;
										break;
									case 5:  // Env Color
										Arg1 = Temp4;
										break;
									case 6:  // Env Color
										Arg1 = Temp5;
										break;
									case 7:  // Env Color
										Arg1 = Temp6;
										break;
									}
									switch (Cmd->Param4){
									case 0:  // Prim Color
										Arg2 = State.PrimColor;
										break;
									case 1:  // Env Color
										Arg2 = State.EnvColor;
										break;
									case 2:  // Env Color
										Arg2 = Temp1;
										break;
									case 3:  // Env Color
										Arg2 = Temp2;
										break;
									case 4:  // Env Color
										Arg2 = Temp3;
										break;
									case 5:  // Env Color
										Arg2 = Temp4;
										break;
									case 6:  // Env Color
										Arg2 = Temp5;
										break;
									case 7:  // Env Color
										Arg2 = Temp6;
										break;
									}
									U32 TexIndex;
									LPDIRECT3DTEXTURE9 PrerenderedIndex;
									TexIndex = ((Cmd->Param1==0)?Texture1:Texture2);
									switch (Cmd->Param2){
									case 0:
										PrerenderedIndex = Prerender_2ColTex(Arg1, Arg2, TexIndex);
										break;
									case 1:
										PrerenderedIndex = Prerender_ModulateTexColor(Arg1, TexIndex);
										break;
									case 2:
										PrerenderedIndex = Prerender_ModulateAddTexColor(Arg1, Arg2, TexIndex);
										break;
									case 3:
										PrerenderedIndex = Prerender_ModulateSubTexColor(Arg1, Arg2, TexIndex);
										break;
									case 4:
										PrerenderedIndex = Prerender_SubtractTexColor(Arg1, TexIndex);
										break;
									case 5:
										PrerenderedIndex = Prerender_ModulateColorReplaceAlpha(Arg1, Arg2, TexIndex);
										break;
									case 6:
										PrerenderedIndex = Prerender_ModulateAlphaReplaceColor(Arg1, Arg2, TexIndex);
										break;
									case 7:
										PrerenderedIndex = Prerender_ModulateOneMinusTexColor(Arg1, TexIndex);
										break;
									}
									if (Cmd->Param1==0){
										Prerendered1 = PrerenderedIndex;
									} else {
										Prerendered2 = PrerenderedIndex;
									}
									break;
								case SC_QueryDevice:
									switch (Cmd->Param1){
									case 0:
										Arg1 = DeviceCaps.MaxSimultaneousTextures;
										break;
									case 1:
										Arg1 = DeviceCaps.MaxTextureBlendStages;
										break;
									case 2:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_ADD;
										break;
									case 3:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_ADDSIGNED;
										break;
									case 4:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_ADDSIGNED2X;
										break;
									case 5:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_ADDSMOOTH;
										break;
									case 6:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_BLENDCURRENTALPHA;
										break;
									case 7:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_BLENDDIFFUSEALPHA;
										break;
									case 8:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_BLENDFACTORALPHA;
										break;
									case 9:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_BLENDTEXTUREALPHA;
										break;
									case 10:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_BLENDTEXTUREALPHAPM;
										break;
									case 11:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_BUMPENVMAP;
										break;
									case 12:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_BUMPENVMAPLUMINANCE;
										break;
									case 13:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_DISABLE;
										break;
									case 14:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_DOTPRODUCT3;
										break;
									case 15:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_LERP;
										break;
									case 16:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_MODULATE;
										break;
									case 17:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_MODULATE2X;
										break;
									case 18:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_MODULATE4X;
										break;
									case 19:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_MODULATEALPHA_ADDCOLOR;
										break;
									case 20:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_MODULATECOLOR_ADDALPHA;
										break;
									case 21:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_MODULATEINVALPHA_ADDCOLOR;
										break;
									case 22:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_MODULATEINVCOLOR_ADDALPHA;
										break;
									case 23:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_MULTIPLYADD;
										break;
									case 24:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_PREMODULATE;
										break;
									case 25:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_SELECTARG1;
										break;
									case 26:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_SELECTARG2;
										break;
									case 27:
										Arg1 = DeviceCaps.TextureCaps & D3DTOP_SUBTRACT;
										break;
									
									case 28: // not used - blank
										break;

									case 29:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_BOTHINVSRCALPHA;
										break;
									case 30:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_BOTHSRCALPHA;
										break;
									case 31:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_DESTALPHA;
										break;
									case 32:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_DESTCOLOR;
										break;
									case 33:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_INVDESTALPHA;
										break;
									case 34:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_INVDESTCOLOR;
										break;
									case 35:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_INVSRCALPHA;
										break;
									case 36:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_INVSRCCOLOR;
										break;
									case 37:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_ONE;
										break;
									case 38:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_SRCALPHA;
										break;
									case 39:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_SRCALPHASAT;
										break;
									case 40:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_SRCCOLOR;
										break;
									case 41:
										Arg1 = DeviceCaps.SrcBlendCaps & D3DBLEND_ZERO;
										break;
									
									case 42: // not used - blank
										break;

									case 43:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_BOTHINVSRCALPHA;
										break;
									case 44:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_BOTHSRCALPHA;
										break;
									case 45:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_DESTALPHA;
										break;
									case 46:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_DESTCOLOR;
										break;
									case 47:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_INVDESTALPHA;
										break;
									case 48:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_INVDESTCOLOR;
										break;
									case 49:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_INVSRCALPHA;
										break;
									case 50:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_INVSRCCOLOR;
										break;
									case 51:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_ONE;
										break;
									case 52:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_SRCALPHA;
										break;
									case 53:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_SRCALPHASAT;
										break;
									case 54:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_SRCCOLOR;
										break;
									case 55:
										Arg1 = DeviceCaps.DestBlendCaps & D3DBLEND_ZERO;
										break;
									}
									switch (Cmd->Param2){
									case 0:
										Temp1 = Arg1;
										break;
									case 1:
										Temp2 = Arg1;
										break;
									case 2:
										Temp3 = Arg1;
										break;
									case 3:
										Temp4 = Arg1;
										break;
									case 4:
										Temp5 = Arg1;
										break;
									case 5:
										Temp6 = Arg1;
										break;
									}
									break;
								}
							}
						}
						BlendPasses.Count = CurrentPass+1;
					}
				}
#if 0
				if (!CombinerHandled){
					CombinerHandled = true;
					switch (State.Combine.raw64){
					case BuildCombineMode1(PrimColor, ShadeColor, Tex0Color, ShadeColor, PrimAlpha, ShadedAlpha, Tex0Alpha, ShadedAlpha):
						// Mario head screen and used in Duke Nukem
						//(PrimColor - ShadeColor)  * Tex0Color + ShadeColor
						//(PrimAlpha - ShadedAlpha) * Tex0Alpha + ShadedAlpha
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						if (DeviceCaps.MaxSimultaneousTextures==2){
							// Hack: ShadedAlpha has to be added. We can't do that with 2 stages, so I ignore it
							BlendPasses.Count = 2;
							BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
							BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
							CombineSetTexture(1, 1, 1);
							BlendPasses.Passes[1].TFactor = State.PrimColor;
							BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						} else {
							BlendPasses.Count = 1;
							// allow transparency
							BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SUBTRACT;
							BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
							BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
							BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
							BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
							BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
							BlendPasses.Passes[0].Stages[2].ColorOp = D3DTOP_ADD;
							BlendPasses.Passes[0].Stages[2].ColorArg1 = D3DTA_CURRENT;
							BlendPasses.Passes[0].Stages[2].ColorArg2 = D3DTA_DIFFUSE;
							BlendPasses.Passes[0].Stages[2].AlphaOp = D3DTOP_ADD;
							BlendPasses.Passes[0].Stages[2].AlphaArg1 = D3DTA_CURRENT;
							BlendPasses.Passes[0].Stages[2].AlphaArg2 = D3DTA_DIFFUSE;
						}
						break;
					case BuildCombineMode1(EnvColor, ShadeColor, Tex0Color, ShadeColor, 0, 0, 0, Tex0Alpha):
						// Duke Nukem - wazzup? this is one of the easiest combines and you don't get it working??
						BlendPasses.Count = 2;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
		
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						CombineSetTexture(1, 0, 1);
						break;
					case BuildCombineMode1(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, EnvAlpha):
						// Mario peach screen, her face. Aerogauge car in selection screen
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(0, 0, 0, ShadeColor, 0, 0, 0, EnvAlpha):
						// Mario peach screen, her hair
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(ShadeColor, 0, EnvColor, 0, ShadedAlpha, 0, EnvAlpha, 0):
						// Mario text box background
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(Tex0Color, ShadeColor, Tex0Alpha, ShadeColor, 0, 0, 0, EnvAlpha):
						// Mario's head (eyes) in the game. Doesn't work properly because of clamping problems
						// (Tex0Color-ShadeColor)*Tex0Alpha+ShadeColor
						// Tex0Color*Tex0Alpha + ShadeColor*(1 - Tex0Alpha)
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_BLENDTEXTUREALPHA;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(0, 0, 0, EnvColor, Tex0Alpha, 0, EnvAlpha, 0):
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(0, 0, 0, PrimColor, 0, 0, 0, Tex0Alpha):
						// Super Smash Bros, Menu
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode1(0, 0, 0, ShadeColor, 0, 0, 0, 1):
						// Super Smash Bros
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = 0xFF000000;
						break;
					case BuildCombineMode1(0, 0, 0, Tex0Color, 0, 0, 0, ShadedAlpha):
						// Mariokart first screen and Conker background
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(0, 0, 0, Tex0Color, 0, 0, 0, EnvAlpha):
						// Mariokart first screen, when it blends
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(1, EnvColor, Tex0Color, PrimColor, PrimAlpha, 0, Tex0Alpha, 0):
						// Mariokart players
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = (State.PrimColor&0xFF000000) | (ComplementColor(State.EnvColor)&0x00FFFFFF);
						// Put PrimColor (d) into a texture
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(State.PrimColor)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode1(0, 0, 0, ShadeColor, 0, 0, 0, Tex0Alpha):
						// Zelda, when you hit someone
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, Tex0Alpha, 0, EnvAlpha, 0, CombinedColor, 0, PrimColor, 0, Tex1Alpha, 0, PrimLODFrac, CombinedAlpha):
						// Zelda, stuff in the intro
						// Color: Tex0Color*ShadeColor*PrimColor
						// Alpha: Tex1Alpha*PrimLODFrac+Tex0Alpha*EnvAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = (State.PrimColor&0x00FFFFFF)|(State.EnvColor&0xFF000000);
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = Prerender_ModulateTexColor(D3DCOLOR_RGBA(State.Lod, State.Lod, State.Lod, State.Lod), Texture2);
						BlendPasses.Passes[0].Stages[1].AddressU = State.Address2U;
						BlendPasses.Passes[0].Stages[1].AddressV = State.Address2V;
						SecondTex[1] = true;
						break;
					case BuildCombineMode2(Tex0Color, PrimColor, PrimLODFrac, Tex0Color, 0, 0, 0, 1, PrimColor, EnvColor, CombinedColor, EnvColor, 0, 0, 0, 1):
						// Deku Nuts in Store
						Template_BlendTextureColor(D3DCOLOR_ARGB(State.Lod, State.Lod, State.Lod, State.Lod));
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, PrimLODFrac, Tex0Color, Tex1Alpha, Tex0Alpha, EnvAlpha, Tex0Alpha, ShadeColor, PrimColor, CombinedColor, PrimColor, 0, 0, 0, CombinedAlpha):
						// Zelda, Hearts - Hack, I assume all ShadeColor=ShadeColorOfFirstTri. Doesn't look right :(
						// Color: (ShadeColor-PrimColor)*[(Tex1Color-Tex0Color)*PrimLODFrac+Tex0Color]+PrimColor
						// Alpha: (Tex1Alpha-Tex0Alpha)*EnvAlpha+Tex0Alpha
		
						// this is a blend between Tex1Color and Tex0Color with PrimLODFrac as blend factor
						// for the alpha blend I put EnvAlpha into the vertex colors of the tris
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].Texture = Prerender_ModulateAddTexColor(SubtractColor(State.D3DVertexCache[1].color, State.PrimColor)|0xFF000000, 0x00FFFFFF&State.PrimColor, Texture1);
						BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
					BlendPasses.Passes[0].Stages[1].Texture = Prerender_ModulateAddTexColor(SubtractColor(State.D3DVertexCache[1].color, State.PrimColor)|0xFF000000, 0x00FFFFFF&State.PrimColor, Texture2);
						BlendPasses.Passes[0].Stages[1].AddressU = State.Address2U;
						BlendPasses.Passes[0].Stages[1].AddressV = State.Address2V;
						BlendPasses.Passes[0].TFactor = State.Lod<<24;
						SecondTex[1] = true;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_BLENDDIFFUSEALPHA;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						// put EnvAlpha into the Vertex Colors
						for (j=0;j<State.D3DVertexCacheIndex;j++){
							State.D3DVertexCache[j].color = (State.D3DVertexCache[j].color&0x00FFFFFF)|(State.EnvColor&0xFF000000);
						}
						break;
					case BuildCombineMode1(PrimColor, EnvColor, Tex0Color, EnvColor, 0, 0, 0, PrimAlpha):
						// Zelda, Link's lifebar
						// Color: (PrimColor-EnvColor)*Tex0Color+EnvColor
						// Alpha: PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].Texture = Prerender_2ColTex(State.PrimColor, State.EnvColor, Texture1);
						BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 1, 1);
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, EnvAlpha, Tex0Color, Tex1Alpha, Tex0Alpha, EnvAlpha, Tex0Alpha, CombinedColor, 0, ShadeColor, 0, Tex1Alpha, 0, PrimLODFrac, CombinedAlpha):
						// Color: [(Tex1Color-Tex0Color)*EnvAlpha+Tex0Color]*ShadeColor
						// Alpha: (Tex1Alpha*PrimLODFrac) + [(Tex1Alpha-Tex0Alpha)*EnvAlpha+Tex0Alpha]
						// Zelda: Wall of great fairy fountain
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 2);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode2(Tex1Color, PrimColor, EnvAlpha, Tex0Color, Tex1Alpha, 1, EnvAlpha, Tex0Alpha, PrimColor, EnvColor, CombinedColor, EnvColor, CombinedAlpha, 0, ShadedAlpha, 0):
						// Effect when you the fairy southes your wounds - Hack
						// Color: (PrimColor-EnvColor)*[(Tex1Color-PrimColor)*EnvAlpha+Tex0Color] + EnvColor
						// Alpha: [(Tex1Alpha-1)*EnvAlpha+Tex0Alpha]*ShadedAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = (State.PrimColor&0xFF000000)|(SubtractColor(State.PrimColor, State.EnvColor)&0x00FFFFFF);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(State.EnvColor)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode2(Tex1Color, PrimColor, EnvAlpha, Tex0Color, Tex1Alpha, Tex0Alpha, EnvAlpha, Tex0Alpha, PrimColor, EnvColor, CombinedColor, EnvColor, CombinedAlpha, 0, ShadedAlpha, 0):
						// Zelda: Effect when you play Zelda's lullaby - Hack, transparency should blend between textures (same as the one in the church) and should then be modulated with DIFFUSE
						// Color  (PrimColor-EnvColor)*[(Tex1Color-PrimColor)*EnvAlpha+Tex0Color] + EnvColor
						// Alpha: [(Tex1Alpha-Tex0Alpha)*EnvAlpha+Tex0Alpha]*ShadedAlpha
						// Pass 1 - does "Tex0Color*(PrimColor-EnvColor)) + EnvColor" with alpha "Tex0Alpha*PrimAlpha"
						Template_FireEffect(ReplicateAlpha(State.EnvColor), false, State.PrimColor);
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, PrimAlpha, CombinedColor, 0, PrimColor, 0, 0, 0, 0, CombinedAlpha):
						// Zora's Domain: Ice
						// Color: Tex0Color*ShadeColor*PrimColor
						// Alpha: PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, EnvAlpha, Tex0Color, Tex1Alpha, Tex0Alpha, EnvAlpha, Tex0Alpha, CombinedColor, 0, ShadeColor, 0, CombinedAlpha, 0, ShadedAlpha, 0):
						// Zelda, waterfall at Zora's. I cannot premodulate ShadeColor, so its not possible with 2 texture stages
						// Color: [(Tex1Color-Tex0Color)*EnvAlpha+Tex0Color]*ShadeColor
						// Alpha: [(Tex1Alpha-Tex0Alpha)*EnvAlpha+Tex0Alpha]*ShadedAlpha
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 2);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, EnvAlpha, Tex0Color, 0, 0, 0, 1, CombinedColor, 0, PrimColor, 0, CombinedAlpha, 0, PrimAlpha, 0):
						// Zelda, water IN Zora's domain. We need EnvAlpha for the blend, so I multiply PrimColor directly into the textures, and set their alpha to PrimAlpha
						// Color: [(Tex1Color-Tex0Color)*EnvAlpha+Tex0Color]*PrimColor
						// Alpha: PrimAlpha
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].Texture = Prerender_ModulateColorReplaceAlpha(State.PrimColor, State.PrimColor, Texture1);
						BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = Prerender_ModulateColorReplaceAlpha(State.PrimColor, State.PrimColor, Texture2);
						BlendPasses.Passes[0].Stages[1].AddressU = State.Address2U;
						BlendPasses.Passes[0].Stages[1].AddressV = State.Address2V;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, EnvAlpha, Tex0Color, 0, 0, 0, 1, CombinedColor, 0, ShadeColor, 0, CombinedAlpha, 0, ShadedAlpha, 0):
						// Zelda, water before Zora's. I cannot premodulate ShadeColor, so its not possible with 2 texture stages
						// Color: [(Tex1Color-Tex0Color)*EnvAlpha+Tex0Color]*ShadeColor
						// Alpha: ShadedAlpha
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 2);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						break;
					case BuildCombineMode2(Tex0Color, PrimColor, PrimLODFrac, Tex0Color, 0, 0, 0, 1, PrimColor, EnvColor, CombinedColor, EnvColor, 0, 0, 0, CombinedAlpha):
						// Zelda, stuff in temple of time
						Template_BlendTextureColor(D3DCOLOR_ARGB(State.Lod, State.Lod, State.Lod, State.Lod));
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
		/*
						Template_FireEffect(ReplicateAlpha(D3DCOLOR_RGBA(State.Lod, State.Lod, State.Lod, State.Lod)), true, 0xFF000000);
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
		*/
						break;
					case BuildCombineMode2(Tex1Color, PrimColor, PrimLODFrac, Tex0Color, Tex1Alpha, Tex0Alpha, PrimLODFrac, Tex0Alpha, PrimColor, EnvColor, CombinedColor, EnvColor, CombinedAlpha, 0, ShadedAlpha, 0):
						// Effect when you put/get the master word out of the the church - A bit hacked
						Template_FireEffect(ReplicateAlpha(D3DCOLOR_RGBA(State.Lod, State.Lod, State.Lod, State.Lod)), false, 0xFF000000);
						break;
					case BuildCombineMode2(Tex1Color, PrimColor, PrimLODFrac, Tex0Color, Tex1Alpha, Tex0Alpha, PrimLODFrac, Tex0Alpha, PrimColor, EnvColor, CombinedColor, EnvColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Zelda, stuff in church - Hack, (transparency should blend between textures) but rarely needed
						Template_FireEffect(ReplicateAlpha(D3DCOLOR_RGBA(State.Lod, State.Lod, State.Lod, State.Lod)), false, State.PrimColor);
						break;
					case BuildCombineMode1(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, 1):
						// Zelda, part of Master Sword in temple of time
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(Tex0Color, PrimColor, EnvAlpha, Tex0Color, 0, 0, 0, 1, PrimColor, EnvColor, CombinedColor, EnvColor, 0, 0, 0, CombinedAlpha):
						// Zelda: Master sword in the temple of time
						Template_BlendTextureColor(ReplicateAlpha(State.EnvColor));
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(Tex1Color, PrimColor, EnvAlpha, Tex0Color, Tex1Alpha, 1, EnvAlpha, Tex0Alpha, PrimColor, EnvColor, CombinedColor, EnvColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Zelda, when loading Links sword. Same effect as the fire at the logo but uses EnvAlpha instead of PrimLODFrac
						// Color: (PrimColor-EnvColor)*[(Tex1Color-PrimColor)*EnvAlpha+Tex0Color]+EnvColor
						// Alpha: ((Tex1Alpha-1)*EnvAlpha+Tex0Alpha)*PrimAlpha
						Template_FireEffect(ReplicateAlpha(State.EnvColor), false, State.PrimColor);
						break;
					case BuildCombineMode1(PrimColor, EnvColor, Tex0Color, EnvColor, Tex0Alpha, 0, PrimAlpha, 0):
						// Mariokart flames and smoke, Zelda dots in map, hearts and some text, Tooie items on screen
						// (Prim Color - Env Color) * Tex0 Color + Env Color
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].Texture = Prerender_2ColTex(State.PrimColor, State.EnvColor, Texture1);
						BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 1, 1);
		/*
						BlendPasses.Count = 1;
						// Use 'Prim Color-Env Color' as TFACTOR. Use Env Color as color texture (for alpha too)
						
						Color = SubtractColor(State.PrimColor, State.EnvColor);
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = Color;
		
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture((State.EnvColor&0x00FFFFFF)|(State.PrimColor&0xFF000000))].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
		*/
						break;
					case BuildCombineMode1(Tex0Color, 0, PrimColor, 0, Tex0Alpha, 0, PrimAlpha, 0):
						// Mariokart player icons top left, Chameleon twist choco test - also used in conker, dunno where
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode1(0, 0, 0, ShadeColor, Tex0Alpha, 0, ShadedAlpha, 0):
						// Mariokart Text
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(0, 0, 0, PrimColor, 0, 0, 0, PrimAlpha):
						// Tarzan, Zelda link pointer on map during game
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode2(1, PrimColor, EnvColor, PrimColor, 0, 0, 0, ShadedAlpha, CombinedColor, 0, ShadeColor, 0, CombinedAlpha, 0, EnvAlpha, 0):
						// Banjo Tooie Selection Screen
						// Color: (1-Prim)*Env+Prim
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = (AddColor(MultColor(ComplementColor(State.PrimColor), State.EnvColor), State.PrimColor)&0x00FFFFFF)|(State.EnvColor);
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex0Alpha, 0, ShadedAlpha, 0, CombinedColor, 0, ShadeColor, 0, CombinedAlpha, 0, EnvAlpha, 0):
						// Banjo Tooie, multiplayer ingame
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex0Color, PrimColor, EnvColor, PrimColor, Tex0Alpha, 0, ShadedAlpha, 0, CombinedColor, 0, ShadeColor, 0, CombinedAlpha, 0, EnvAlpha, 0):
						// Banjo Tooie - Some stuff in multiplayer
						// Color: [(Tex0Color-PrimColor)*EnvColor+PrimColor] * ShadeColor
						// Alpha: Tex0Alpha*ShadedAlpha*EnvAlpha
						// [(Tex0Color-PrimColor)*EnvColor+PrimColor] * ShadeColor =
						// [(Tex0Color*(EnvColor+PrimColor)-(PrimColor*EnvColor+PrimColor)] * ShadeColor
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].Texture = Prerender_ModulateSubTexColor(0xFF000000|AddColor(State.EnvColor, State.PrimColor), 0x00FFFFFF&MultColor(State.PrimColor, AddColor(State.EnvColor, State.PrimColor)), Texture1);
						BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(0, 0, 0, EnvColor, 0, 0, 0, EnvAlpha):
						// Aerogauge
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(ShadeColor, 0, PrimColor, EnvColor, Tex0Alpha, 0, ShadedAlpha, 0):
						// TWine
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(State.EnvColor)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, Tex0Alpha, 0, PrimAlpha, 0, EnvColor, CombinedColor, EnvAlpha, CombinedColor, 0, 0, 0, CombinedAlpha):
						// Rush 2 - cars, Hack
						// Color: (EnvColor*EnvAlpha-(Tex0Color*ShadeColor*EnvAlpha))+CombinedColor
						// Alpha: Tex0Alpha*PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = ReplicateAlpha(State.PrimColor);
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(ShadeColor, 0, PrimColor, 0, 0, 0, 0, Tex0Alpha):
						// Rush 2 - cars, second mode
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, ShadedAlpha):
						// Forsaken (looks better without but must be some other problem)
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(Tex0Color, 0, ShadeColor, 0, Tex0Alpha, 0, PrimAlpha, 0):
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(Tex0Color, 0, ShadeColor, 0, Tex0Alpha, 0, ShadedAlpha, 0):
						// Conker's Bad Fur Day first screen, the light from above
						// Micromachines cars
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(0, 0, 0, PrimColor, PrimAlpha, 0, Tex0Alpha, 0):
						// Conker's Bad Fur Day letters in title screen
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode1(Tex0Color, 0, EnvColor, 0, EnvAlpha, 0, Tex0Alpha, 0):
						// Conker's Bad Fur Day image in title screen
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(Tex0Color, EnvColor, ShadeColor, PrimColor, Tex0Alpha, 0, ShadedAlpha, 0):
						// Conker's BFD - Ground in intro, lamp
						BlendPasses.Count = 2;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
		
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[1].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(1, 0, 1);
						BlendPasses.Passes[1].TFactor = State.PrimColor;
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode1(0, 0, 0, PrimColor, Tex0Alpha, 0, ShadedAlpha, 0):
		// Combine Cycle1:       (0 - 0) * 0 + Prim Color
		// Combine Alpha Cycle1: (Tex0 Alpha - 0) * Shaded Alpha + 0
						// Conker's BFD - debris in title screen
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode1(0, 0, 0, ShadeColor, 1, Tex0Alpha, ShadedAlpha, 0):
		// Combine Cycle1:       (0 - 0) * 0 + Shade Color
		// Combine Alpha Cycle1: (1.0 - Tex0 Alpha) * Shaded Alpha + 0
						// Conker's BFD - Shadows
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE|D3DTA_COMPLEMENT;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, Tex1Alpha, CombinedColor, EnvColor, ShadeColor, PrimColor, CombinedAlpha, 0, ShadedAlpha, 0):
						// Conker's BFD: Plants ingame
						//(Tex0Color*ShadeColor-EnvColor)*ShadeColor + PrimColor
						BlendPasses.Count = 2;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						// will work only on geforces/ref rasta
						BlendPasses.Passes[0].Stages[2].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[2].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[2].ColorArg2 = D3DTA_DIFFUSE;
		
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[1].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].TFactor = (State.EnvColor&0xFF000000)|(State.PrimColor&0x00FFFFFF);
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						CombineSetTexture(1, 0, 1);
						break;
					case BuildCombineMode1(Tex0Color, 0, EnvColor, 0, Tex0Alpha, 0, EnvAlpha, 0):
		// Combine Cycle1:       (Tex0 Color - 0) * Env Color + 0
		// Combine Alpha Cycle1: (Tex0 Alpha - 0) * Env Alpha + 0
						// Conker's BFD - smoke during title
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(PrimColor, EnvColor, Tex0Alpha, EnvColor, PrimAlpha, 0, Tex0Alpha, 0):
						// Conker's BFD - smoke during title
						BlendPasses.Count = 1;
						Color = SubtractColor(State.PrimColor, State.EnvColor);
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TEXTURE|D3DTA_ALPHAREPLICATE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = (State.PrimColor&0xFF000000) | (Color&0x00FFFFFF);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(State.EnvColor)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode1(0, 0, 0, 0, 0, 0, 0, Tex0Alpha):
						// Conker's bfd intro, there is a circle around him
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ZERO;
						break;
					case BuildCombineMode1(ShadeColor, EnvColor, PrimAlpha, PrimColor, 0, 0, 0, ShadedAlpha):
						// conker, its behind the doors when you enter a game. dunno if it works well
						// Color simplified: ShadeColor * PrimAlpha + (PrimColor - EnvColor * PrimAlpha)
						BlendPasses.Count = 1;
						
						Color = SubtractColor(State.PrimColor, MultColor(State.EnvColor, ReplicateAlpha(State.PrimColor)));
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].TFactor = ReplicateAlpha(State.PrimColor);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(Color)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode1(Tex0Color, EnvColor, ShadeColor, PrimColor, 0, 0, 0, Tex0Alpha):
						// Wall in Conker
		//Combine Cycle1:       (Tex0Color - EnvColor) * ShadeColor + PrimColor
		//Combine Alpha Cycle1: (0 - 0) * 0 + Tex0Alpha
						BlendPasses.Count = 2;
						
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_DIFFUSE;
						
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[1].TFactor = State.PrimColor;
						CombineSetTexture(1, 0, 1);
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode1(0, 0, 0, PrimColor, Tex0Alpha, 0, PrimAlpha, 0):
						// Conker's BFD: Shadow, Zelda intro text (in 2nd screen)
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(EnvColor, 0, ShadeColor, 0, Tex0Alpha, 0, EnvAlpha, 0):
						// Conker's BFD: Another Shadow in Conker
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(1, 0, EnvColor, 0, 0, 0, 0, EnvAlpha):
						// Conker's BFD: Dot of other player in multiplayer games' maps
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(0, 0, 0, Tex0Color, Tex0Alpha, 0, EnvAlpha, 0):
						// Conker's BFD: Multiplayer over other player when he shoots
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(Tex0Color, PrimColor, EnvColor, PrimColor, Tex0Alpha, 0, PrimAlpha, 0):
						// Conker's BFD: Multiplayer when you get hit
		//Combine Cycle1:       Tex0Color * EnvColor + PrimColor - PrimColor*EnvColor
		//Combine Alpha Cycle1: Tex0Alpha * PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = (State.EnvColor&BitM24) | (State.PrimColor&0xFF000000);
						
						Color = SubtractColor(State.PrimColor, MultColor(State.PrimColor, State.EnvColor));
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(Color)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode1(PrimColor, EnvColor, Tex0Color, EnvColor, Tex0Alpha, 0, EnvAlpha, 0):
						// Conker's BFD: Multiplayer when you get hit some smoke
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = SubtractColor(State.PrimColor, State.EnvColor);
						
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(State.EnvColor)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode1(0, 0, 0, EnvColor, EnvAlpha, 0, ShadedAlpha, 0):
						// Conker's BFD: Laser in multiplayer
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(Tex0Color, 0, ShadeColor, 0, Tex0Alpha, 0, EnvAlpha, 0):
						// Conker's BFD: blood on the floor when conker gets hit
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(Tex0Color, 0, EnvColor, PrimColor, Tex0Alpha, 0, EnvAlpha, 0):
						// Conker's BFD: Flames during intro and main menu
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(State.PrimColor)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode1(0, 0, 0, ShadeColor, PrimAlpha, 0, Tex0Alpha, 0):
						// Conker's BFD: Stuff during intro. Looks like the places when the rain hits the ground
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(Tex0Color, 0, ShadeColor, 0, PrimAlpha, 0, Tex0Alpha, 0):
						// Conker's BFD: Rain during intro
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(0, 0, 0, Tex0Color, Tex0Alpha, 0, ShadedAlpha, 0):
						// Conker's BFD: Multiplayer Beach: Laser
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(0, 0, 0, Tex0Color, 0, 0, 0, Tex0Alpha):
						// Conker's BFD: The universal combiner works fine on this one, but I want the message out of the log
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(Tex0Color, 0, EnvColor, 0, 0, 0, 0, Tex0Alpha):
						// Conker's BFD: Multiplayer Beach - Icons at the bottom of the screen
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode1(PrimColor, 0, Tex0Color, 0, PrimAlpha, 0, Tex0Alpha, 0):
						// Conker's BFD: Speech Bubbles
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode1(PrimColor, EnvColor, Tex0Color, EnvColor, PrimAlpha, 0, Tex0Alpha, 0):
						// Conker's BFD: Happens when you fall down on the floor from a high point
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = (State.PrimColor&0xFF000000)|(SubtractColor(State.PrimColor, State.EnvColor)&0x00FFFFFF);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(State.EnvColor)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode1(Tex0Color, EnvColor, ShadeColor, PrimColor, 0, 0, 0, ShadedAlpha):
						// Conker's BFD: dunno where
						BlendPasses.Count = 2;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
		
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].TFactor = State.PrimColor;
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode1(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, Tex0Alpha):
						// Conker's BFD: Main game (fence)
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(0, 0, 0, 1, Tex0Alpha, 0, EnvAlpha, 0):
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor | 0x00FFFFFF;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex1Alpha, Tex0Alpha, LODFraction, Tex0Alpha, ShadeColor, EnvColor, CombinedColor, PrimColor, CombinedAlpha, 0, EnvAlpha, 0):
						// Conker's BFD	- Part of Conker, Golden Part of Rare Logo
						// ((Tex1Color-Tex0Color)*LODFraction+Tex0Color)*(ShadeColor-EnvColor)*PrimColor
		
						//((Tex1Alpha-Tex0Alpha)*LODFraction+Tex0Alpha)*EnvAlpha
						BlendPasses.Count = 2;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
		
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						CombineSetTexture(1, 0, 1);
						BlendPasses.Passes[1].TFactor = (State.EnvColor&0xFF000000)|(State.PrimColor&0x00FFFFFF);
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(0, 0, 0, 0, 0, 0, 0, ShadedAlpha, ShadeColor, EnvColor, K5, PrimColor, CombinedAlpha, 0, EnvAlpha, 0):
		//Combine Cycle2:       (Shade Color - Env Color) * Convert K5 + Prim Color
		//Combine Alpha Cycle2: (Shaded Alpha * Env Alpha)
						// Conker's BFD - Part of his mouth - Hack (how to emulate K5)
						BlendPasses.Count = 2;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						Color = D3DCOLOR_ARGB((U8)State.Convert.K5, (U8)State.Convert.K5, (U8)State.Convert.K5, (U8)State.Convert.K5);
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(Color)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
		
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].TFactor = (State.PrimColor&0x00FFFFFF) | (State.EnvColor&0xFF000000);
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex1Alpha, Tex0Alpha, LODFraction, Tex0Alpha, CombinedColor, 0, ShadeColor, 0, CombinedAlpha, 0, EnvAlpha, 0):
		// Combine Cycle1:       (Tex1 Color - Tex0 Color) * LOD Fraction + Tex0 Color
		// Combine Alpha Cycle1: (Tex1 Alpha - Tex0 Alpha) * LOD Fraction + Tex0 Alpha
		// Combine Cycle2:       (Combined Color - 0) * Shade Color + 0
		// Combine Alpha Cycle2: (Combined Alpha - 0) * Env Alpha + 0
		// simplified with Lod=0
		// Combine Cycle2:       Tex0 Color  * Shade Color
		// Combine Alpha Cycle2: Tex0 Alpha  * Env Alpha
						// Conker's BFD - debris in title screen
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode2(0, 0, 0, Tex0Color, Tex0Alpha, 0, ShadedAlpha, 0, ShadeColor, EnvColor, CombinedColor, PrimColor, CombinedAlpha, 0, EnvAlpha, 0):
						// Conker's Bad Fur Day N-Logo in intro
						//Color: (Shade Color - Env Color) * Tex0 Color + Prim Color
						//Alpha: Shaded Alpha * Env Alpha  * Tex0 Alpha
						BlendPasses.Count = 2;
						// Pass 1
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
		
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
		
						// Pass 2
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
		
						BlendPasses.Passes[1].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[1].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						CombineSetTexture(1, 0, 1);
						BlendPasses.Passes[1].TFactor = (State.EnvColor&0xFF000000) | (State.PrimColor&0x00FFFFFF);
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex1Alpha, Tex0Alpha, LODFraction, Tex0Alpha, CombinedColor, EnvColor, ShadeColor, PrimColor, CombinedAlpha, 0, ShadedAlpha, 0):
		//Combine Cycle1:       (Tex1Color - Tex0Color) * LODFraction + Tex0Color
		//Combine Alpha Cycle1: (Tex1Alpha - Tex0Alpha) * LODFraction + Tex0Alpha
		//Combine Cycle2:       (CombinedColor - EnvColor) * ShadeColor + PrimColor
		//Combine Alpha Cycle2: (CombinedAlpha - 0) * ShadedAlpha + 0
		//simplified (LOD=0)
		//Combine Cycle2:       (Tex0Color - EnvColor) * ShadeColor + PrimColor
		//Combine Alpha Cycle2: Tex0Alpha * ShadedAlpha
						// Conker's BFD: In a multiplayer game
						BlendPasses.Count = 2;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_DIFFUSE;
						
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[1].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].TFactor = State.PrimColor;
						CombineSetTexture(1, 0, 1);
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex1Alpha, Tex0Alpha, LODFraction, Tex0Alpha, CombinedColor, 0, ShadeColor, 0, ShadedAlpha, 0, EnvAlpha, 0):
		//Combine Cycle2:       Tex0Color * ShadeColor
		//Combine Alpha Cycle2: Tex0Alpha * EnvAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, Tex0Alpha, 0, ShadedAlpha, 0, 0, 0, 0, CombinedColor, CombinedAlpha, 0, EnvAlpha, 0):
		//Combine Cycle2:       Tex0Color * ShadeColor
		//Combine Alpha Cycle2: Tex0Alpha * ShadedAlpha * EnvAlpha
						// Conker's BFD: Some weapons and stuff when you get hit
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(0, 0, 0, ShadeColor, 0, 0, 0, ShadedAlpha, 0, 0, 0, ShadeColor, ShadedAlpha, 0, EnvAlpha, 0):
						// Conker's BFD: Untested - Don't know where
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, Tex0Alpha, 0, 0, 0, CombinedColor, CombinedAlpha, 0, EnvAlpha, 0):
						// Conker's BFD: Some stuff on weapons
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(0, 0, 0, Tex0Color, 0, 0, 0, ShadedAlpha, ShadeColor, EnvColor, CombinedColor, PrimColor, CombinedAlpha, 0, EnvAlpha, 0):
						// Conker's BFD: Head and hands of guy in "more milk" screen
		//Combine Cycle2:       (ShadeColor - EnvColor) * Tex0Color + PrimColor
		//Combine Alpha Cycle2: ShadedAlpha * EnvAlpha
						BlendPasses.Count = 2;
						// Pass 1
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
		
						// Pass 2
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].TFactor = State.EnvColor;
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex1Alpha, Tex0Alpha, LODFraction, Tex0Alpha, CombinedColor, EnvColor, ShadeColor, PrimColor, 0, 0, 0, CombinedAlpha):
		//Combine Cycle2:       (Tex0Color - EnvColor) * ShadeColor + PrimColor
		//Combine Alpha Cycle2:  Tex0Alpha
						BlendPasses.Count = 2;
						// Pass 1
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SUBTRACT;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_DIFFUSE;
		
						// Pass 2
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(1, 0, 1);
						BlendPasses.Passes[1].TFactor = State.PrimColor;
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex1Alpha, Tex0Alpha, LODFraction, Tex0Alpha, CombinedColor, 0, ShadeColor, 0, CombinedAlpha, 0, ShadedAlpha, 0):
						// something in Conker's BFD
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex1Alpha, Tex0Alpha, LODFraction, Tex0Alpha, CombinedColor, 0, ShadeColor, 0, 0, 0, 0, CombinedAlpha):
						// S.F. Rush 2049 (street) and Rush 2 and somewhere in Conker's BFD
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex1Alpha, Tex0Alpha, LODFraction, Tex0Alpha, CombinedColor, 0, PrimColor, 0, CombinedAlpha, 0, PrimAlpha, 0):
						// S.F. Rush 2049 and somewhere in Conker's BFD
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode2(0, 0, 0, ShadeColor, Tex0Alpha, 0, Tex0Alpha, 0, CombinedColor, 0, PrimColor, 0, 0, 0, 0, CombinedAlpha):
						// Pokemon Stadium - Arena Floor
						// Color: ShadeColor*PrimColor
						// Alpha: Tex0Alpha*Tex0Alpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(0, 0, 0, ShadeColor, 0, 0, 0, ShadedAlpha):
						// Gex 3, Pokemon stadium, Rocket Robot
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						break;
					case BuildCombineMode1(EnvColor, PrimColor, Tex0Color, PrimColor, EnvAlpha, PrimAlpha, Tex0Alpha, PrimAlpha):
						// Gex3, first text
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = SubtractColor(State.EnvColor, State.PrimColor);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(State.PrimColor)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode2(0, 0, 0, ShadeColor, 0, 0, 0, ShadedAlpha, CombinedColor, 0, PrimColor, 0, CombinedAlpha, 0, PrimAlpha, 0):
						// Aidyn Chronicles, spinning N
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, Tex0Alpha, CombinedColor, 0, PrimColor, 0, CombinedAlpha, 0, PrimAlpha, 0):
						// Aidyn Chronicles after a few screen
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, Tex0Alpha, 0, ShadedAlpha, 0, CombinedColor, 0, PrimColor, 0, 0, 0, 0, CombinedAlpha):
						// Paper Mario
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(0, 0, 0, ShadeColor, 0, 0, 0, ShadedAlpha, CombinedColor, 0, PrimColor, 0, 0, 0, 0, CombinedAlpha):
						// Paper Mario
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode1(Tex0Color, Tex1Color, PrimAlpha, Tex1Color, 0, 0, 0, EnvAlpha):
						// Aerogauge transparent Water. The color fomula is
						// (Tex0Color - Tex1Color) * PrimAlpha + Tex1Color
						// simplified:
						// Tex0Color*PrimAlpha + (Tex1Color*(1-PrimAlpha))
						U32 Alpha;
						Alpha = D3DCOLOR_ARGB(State.EnvColor>>24, State.PrimColor >> 24, State.PrimColor >> 24, State.PrimColor >> 24);
						BlendPasses.Count = 2;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = Alpha;
		
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[1].Stages[0].ColorArg2 = D3DTA_TFACTOR | D3DTA_COMPLEMENT;
						CombineSetTexture(1, 0, 2);
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].TFactor = Alpha;
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(Tex0Color, PrimColor, EnvAlpha, Tex0Color, 0, 0, 0, 0, PrimColor, EnvColor, CombinedColor, EnvColor, 0, 0, 0, 1):
						// Zelda: Rotating N at start
						Template_BlendTextureColor(ReplicateAlpha(State.EnvColor));
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(Tex1Color, PrimColor, EnvAlpha, Tex0Color, 0, 0, 0, Tex0Alpha, PrimColor, EnvColor, CombinedColor, EnvColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Legend of zelda, text in first screen. Must be prerendered
						// Color: (PrimColor-EnvColor)*[(Tex1Color-PrimColor)*EnvAlpha+Tex0Color]+EnvColor
						// Alpha: Tex0Alpha*PrimAlpha
						Template_FireEffect(ReplicateAlpha(State.EnvColor), false, State.PrimColor);
						break;
					case BuildCombineMode2(Tex1Color, PrimColor, PrimLODFrac, Tex0Color, Tex1Alpha, 1, PrimLODFrac, Tex0Alpha, PrimColor, EnvColor, CombinedColor, EnvColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Legend of zelda, fire behind zelda logo, and poe the ghost
						Template_FireEffect(D3DCOLOR_RGBA(State.Lod, State.Lod, State.Lod, State.Lod), false, State.PrimColor);
						break;
					case BuildCombineMode2(Tex0Color, PrimColor, PrimLODFrac, Tex1Color, Tex0Alpha, 1, PrimLODFrac, Tex1Alpha, PrimColor, EnvColor, CombinedColor, EnvColor, 0, 0, 0, CombinedAlpha):
						// Zelda, fire behind tokens you get from Skultullas
						Template_FireEffect(D3DCOLOR_RGBA(State.Lod, State.Lod, State.Lod, State.Lod), true, 0xFF000000);
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, EnvAlpha, Tex0Color, Tex1Alpha, Tex0Alpha, EnvAlpha, Tex0Alpha, PrimColor, EnvColor, CombinedColor, EnvColor, CombinedAlpha, 0, PrimAlpha, 0):
		//Combine Cycle1:       (Tex1Color - Tex0Color) * EnvAlpha + Tex0Color
		//Combine Alpha Cycle1: (Tex1Alpha - Tex0Alpha) * EnvAlpha + Tex0Alpha
		//Combine Cycle2:       (PrimColor - EnvColor) * CombinedColor + EnvColor
		//Combine Alpha Cycle2: (CombinedAlpha - 0) * PrimAlpha + 0
						// Legend of zelda - Navy
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].Texture = Prerender_ModulateAddTexColor((SubtractColor(State.PrimColor, State.EnvColor)&0x00FFFFFF) | (State.PrimColor&0xFF000000), (State.EnvColor&0x00FFFFFF), Texture1);
						BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
						BlendPasses.Passes[0].Stages[1].Texture = Prerender_ModulateAddTexColor((SubtractColor(State.PrimColor, State.EnvColor)&0x00FFFFFF) | (State.PrimColor&0xFF000000), (State.EnvColor&0x00FFFFFF), Texture2);
						BlendPasses.Passes[0].Stages[1].AddressU = State.Address2U;
						BlendPasses.Passes[0].Stages[1].AddressV = State.Address2V;
						SecondTex[1] = true;
						BlendPasses.Passes[0].TFactor= State.EnvColor;
						break;
					case BuildCombineMode1(1, 0, PrimColor, 0, Tex0Alpha, 0, PrimAlpha, 0):
						// Legend of Zelda, text in map
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, EnvAlpha, Tex0Color, 0, 0, 0, 1, CombinedColor, 0, PrimColor, 0, 0, 0, 0, CombinedAlpha):
						// Zelda, in church of time its something on the wall
						BlendPasses.Count = 2;
						// Pass 1
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 2);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						// Pass 2
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].TFactor = State.PrimColor;
						BlendPasses.Passes[1].DstBlend = D3DBLEND_SRCCOLOR;
						BlendPasses.Passes[1].SrcBlend = D3DBLEND_ZERO;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, EnvAlpha, Tex0Color, 0, 0, 0, 1, CombinedColor, 0, ShadeColor, 0, 0, 0, 0, CombinedAlpha):
						// Legend of zelda - Ground. This is a hard one, luckily it doesn't use transparency
						//Combine Cycle1:       [(Tex1Color - Tex0Color) * EnvAlpha + Tex0Color] * ShadeColor
						BlendPasses.Count = 2;
						// Pass 1
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 2);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						// Pass 2
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].DstBlend = D3DBLEND_SRCCOLOR;
						BlendPasses.Passes[1].SrcBlend = D3DBLEND_ZERO;
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, 1, CombinedColor, 0, PrimColor, 0, 0, 0, 0, CombinedAlpha):
						// Zelda Link's horse while he rides on it during Intro
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, Tex0Alpha, EnvColor, 0, CombinedColor, 0, 0, 0, 0, CombinedAlpha):
						// Zelda Link's green clothes
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, Tex0Alpha, PrimColor, 0, CombinedColor, 0, 0, 0, 0, CombinedAlpha):
						// Zelda Link's skin, Cow's (too much transparency??)
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(PrimColor, EnvColor, Tex0Color, EnvColor, PrimAlpha, 0, Tex0Alpha, 0, CombinedColor, 0, ShadeColor, 0, 0, 0, 0, CombinedAlpha):
		//Combine        ((Tex0Color * (PrimColor - EnvColor)) + EnvColor) * ShadeColor
		//Combine Alpha    Tex0Alpha * PrimAlpha
						// Zelda - Smoke
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].Texture = Prerender_2ColTex(State.PrimColor, State.EnvColor, Texture1);
						BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 1, 1);
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, Tex0Alpha, 0, PrimAlpha, 0, CombinedColor, 0, PrimColor, 0, Tex1Alpha, 0, PrimLODFrac, CombinedAlpha):
						// Zelda: Wall in the well where you get the eye of truth
						// Color: Tex0Color * ShadeColor * PrimColor
						// Alpha: Tex0Alpha * PrimAlpha + Tex1Alpha * PrimLODFrac
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = Prerender_ModulateAddTexColor(D3DCOLOR_RGBA(State.Lod, State.Lod, State.Lod, State.Lod), 0, Texture2);
						BlendPasses.Passes[0].Stages[1].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[1].AddressV = State.Address1V;
						SecondTex[1] = true;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, 1, PrimColor, EnvColor, CombinedColor, EnvColor, 0, 0, 0, CombinedAlpha):
						// Zelda, Skultullas
						// Color: ((PrimColor-EnvColor)*Tex0Color*ShadeColor)+EnvColor
						// Alpha: 1
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
						BlendPasses.Passes[0].TFactor = 0xFF000000|State.EnvColor;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						CombineSetTexture(0, 0, 1);
						// change all ShadeColor so they are ShadeColor*(PrimColor-EnvColor)
						Color = SubtractColor(State.PrimColor, State.EnvColor);
						for (j=0;j<State.D3DVertexCacheIndex;j++){
							State.D3DVertexCache[j].color = MultColor(State.D3DVertexCache[j].color, Color);
						}
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, 0, 0, 0, Tex0Alpha, CombinedColor, 0, PrimColor, 0, 0, 0, 0, CombinedAlpha):
		//Combine Cycle1:       (Tex0Color - 0) * ShadeColor + 0
		//Combine Alpha Cycle1: (0 - 0) * 0 + Tex0Alpha
		//Combine Cycle2:       (CombinedColor - 0) * PrimColor + 0
		//Combine Alpha Cycle2: (0 - 0) * 0 + CombinedAlpha
						// Zelda, Flowers and trees
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, EnvAlpha, Tex0Color, 0, 0, 0, 1, CombinedColor, 0, ShadeColor, 0, CombinedAlpha, 0, PrimAlpha, 0):
						// Zelda transparent water
						// Color: [(Tex1Color-Tex0Color)*EnvAlpha+Tex0Color]*ShadeColor
						// Alpha: PrimAlpha
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 2);
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						// I multiply SRC by DIFFUSE, so multiply DIFFUSE by PrimAlpha and set its alpha to PrimAlpha. Won't work with TEXRECTs/FILLRECTs
						Color = BITS(State.PrimColor, 24, 8)<<24;//D3DCOLOR_ARGB(BITS(State.PrimColor, 24, 8), BITS(State.PrimColor, 24, 8), BITS(State.PrimColor, 24, 8));
						for (j=0;j<State.D3DVertexCacheIndex;j++){
							State.D3DVertexCache[j].color = (MultColor(State.D3DVertexCache[j].color, State.PrimColor)&0xFF000000);
						}
						break;
					case BuildCombineMode1(Tex0Color, 0, PrimColor, 0, 0, 0, 0, 1):
		//Combine Cycle1:       Tex0Color * PrimColor
		//Combine Alpha Cycle1: (0 - 0) * 0 + 1
		//Combine Cycle2:       (0 - 0) * 0 + CombinedColor
		//Combine Alpha Cycle2: (0 - 0) * 0 + CombinedAlpha
						// Legend of zelda: Rupee
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.PrimColor | 0xFF000000;
						break;
					case BuildCombineMode1(Tex1Color, Tex0Color, PrimAlpha, Tex0Color, Tex1Alpha, Tex0Alpha, PrimAlpha, Tex0Alpha):
						// Legend of zelda (jpeg?)
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = ComplementColor(State.PrimColor);
						CombineSetTexture(0, 0, 1);
						CombineSetTexture(0, 1, 2);
						break;
					case BuildCombineMode1(ShadeColor, 0, PrimColor, 0, 0, 0, 0, 1):
						// Legend of zelda - Used somehow for jpeg, Skultullas eyes
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor|0xFF000000;
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, Tex0Alpha, 0, ShadedAlpha, 0, CombinedColor, 0, PrimColor, 0, CombinedAlpha, 0, PrimAlpha, 0):
						// Legend of zelda - JPEG stuff?
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TFACTOR;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode1(PrimColor, 0, Tex0Color, 0, Tex0Alpha, 0, PrimAlpha, 0):
						// Zelda, shadow of signs
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex0Color, 0, PrimColor, 0, 0, 0, 0, 1, Tex0Color, 0, PrimColor, 0, 0, 0, 0, CombinedAlpha):
						// Zelda, marker on signs
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(0, 0, 0, PrimColor, 0, 0, 0, 1):
						// Zelda, somewhere
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor|0xFF000000;
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex1Alpha, Tex0Alpha, LODFraction, Tex0Alpha, CombinedColor, 0, ShadeColor, 0, 0, 0, 0, ShadedAlpha):
						// Color: [(Tex1Color-Tex0Color]*LODFraction+Tex0Color]*ShadeColor
						// Alpha: ShadedAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(1, 0, PrimLODFrac, Tex0Color, Tex0Alpha, 0, PrimAlpha, 0, CombinedColor, 0, ShadeColor, EnvColor, 0, 0, 0, CombinedAlpha):
						// Doom64, walls
						//Color: Tex0Color*ShadeColor+EnvColor+PrimLODFrac
						//Alpha: Tex0Alpha*PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = (State.PrimColor&0xFF000000)|(AddColor(State.EnvColor, D3DCOLOR_XRGB(State.Lod, State.Lod, State.Lod))&0x00FFFFFF);
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(PrimColor, EnvColor, Tex0Color, EnvColor, 0, 0, 0, Tex0Alpha, CombinedColor, 0, ShadeColor, 0, 0, 0, 0, CombinedAlpha):
						// Ridge Racer, Mountains (looks wrong, maybe a blender problem)
		 				BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].Texture = Prerender_2ColTex(State.PrimColor, State.EnvColor, 1);
						BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;  // ignore stage
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_TEXTURE;
						CombineSetTexture(0, 1, 1);
						break;
					case BuildCombineMode1(EnvColor, Tex0Color, EnvAlpha, Tex0Color, Tex0Alpha, 0, PrimAlpha, 0):
						// Diddy Kong Racing options
						//(EnvColor-Tex0Color)*EnvAlpha+Tex0Color
						//Tex0Alpha*PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].Texture = g_ColorTexture[GenerateColorTexture(State.PrimColor)].D3DObject;
						BlendPasses.Passes[0].Stages[1].AddressU = D3DTADDRESS_WRAP;
						BlendPasses.Passes[0].Stages[1].AddressV = D3DTADDRESS_WRAP;
						break;
					case BuildCombineMode2(Tex0Color, 0, ShadeColor, 0, Tex0Alpha, 0, ShadedAlpha, 0, CombinedColor, 0, EnvColor, CombinedColor, 0, 0, 0, CombinedAlpha):
						// Toy story 2, in game. First combine i see using CombinedColor two times
						//Tex0Color*ShadeColor*EnvColor+Tex0Color*ShadeColor
						//Tex0Alpha*ShadedAlpha
						BlendPasses.Count = 2;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 1);
		
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[1].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[1].Stages[0].AlphaArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						CombineSetTexture(1, 0, 1);
						break;
					case BuildCombineMode2(ShadeColor, 0, Tex0Color, 0, 0, 0, 0, 1, 0, 0, 0, CombinedColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Rocket Robot on Wheels
						// Color: ShadeColor*Tex0Color
						// Alpha: PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, LODFraction, Tex0Color, Tex1Alpha, Tex0Alpha, LODFraction, Tex0Alpha, ShadeColor, 0, CombinedColor, 0, 0, 0, 0, PrimAlpha):
						// Rocket Robot on Wheels
						// Color: ShadeColor*Tex0Color
						// Alpha: PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(ShadeColor, EnvColor, Tex0Color, EnvColor, Tex0Alpha, 0, ShadedAlpha, 0, 0, 0, 0, CombinedColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Rocket Robot on Wheels - do as 2 Pass, but I could prerender as well
						// Color: (ShadeColor - EnvColor) * Tex0Color + EnvColor
						//      == ShadeColor*Tex0Color + (1-Tex0Color)*EnvColor
						// Alpha: Tex0Alpha * ShadedAlpha * PrimAlpha
						BlendPasses.Count = 2;
						// Pass 1
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						// Pass 2
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TEXTURE|D3DTA_COMPLEMENT;
						BlendPasses.Passes[1].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[1].Stages[1].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[1].Stages[1].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].TFactor = (State.PrimColor&0xFF000000)|(State.EnvColor&0x00FFFFFF);
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						CombineSetTexture(1, 0, 1);
						break;
					case BuildCombineMode2(Tex1Color, Tex0Color, ShadeAlpha, Tex0Color, Tex1Alpha, Tex0Alpha, ShadedAlpha, Tex0Alpha, EnvColor, ShadeColor, CombinedColor, ShadeColor, CombinedAlpha, 0, EnvAlpha, 0):
						// Rocket Robot on Wheels - OMG
						// Color: (EnvColor-ShadeColor)*[(Tex1Color-Tex0Color)*ShadedAlpha+Tex0Color]+ShadeColor
						// Alpha: [(Tex1Alpha-Tex0Alpha)*ShadedAlpha+Tex0Alpha]*EnvAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TEXTURE;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(EnvColor, ShadeColor, Tex0Color, ShadeColor, Tex0Alpha, 0, EnvAlpha, 0, 0, 0, 0, CombinedColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Rocket Robot on Wheels - Hack: I use texturealpha instead of color for blending between Env and Shade
						// Color: (EnvColor-ShadeColor)*Tex0Color+ShadeColor
						//    ==  Tex0Color*EnvColor + (1-Tex0Color)*ShadeColor
						// Alpha: PrimAlpha*Tex0Alpha*EnvAlpha
						
						BlendPasses.Count = 2;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = (MultColor(State.PrimColor, State.EnvColor)&0xFF000000)|(State.EnvColor&0x00FFFFFF);
						CombineSetTexture(0, 0, 1);
						
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TEXTURE|D3DTA_COMPLEMENT;
						BlendPasses.Passes[1].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[1].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[1].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].TFactor = MultColor(State.PrimColor, State.EnvColor);
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						CombineSetTexture(1, 0, 1);
						break;
					case BuildCombineMode2(ShadeColor, 0, Tex0Color, 0, 0, 0, 0, Tex0Alpha, 0, 0, 0, CombinedColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Rocket Robot: Coins
						// Color: ShadeColor*Tex0Color
						// Alpha: Tex0Alpha*PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(ShadeColor, 0, Tex0Color, 0, 0, 0, 0, EnvAlpha, 0, 0, 0, CombinedColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Rocket robot, laser
						// Color: ShadeColor*Tex0Color
						// Alpha: EnvAlpha*PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = MultColor(State.PrimColor, State.EnvColor);
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode2(ShadeColor, EnvColor, Tex0Color, EnvColor, 0, 0, 0, 1, 0, 0, 0, CombinedColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Rocket Robot
						// Color: (ShadeColor-EnvColor)*Tex0Color+EnvColor
						// Alpha: PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].Texture = Prerender_2ColTex(State.PrimColor, State.EnvColor, 1);
						BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 1, 1);
						break;
					case BuildCombineMode2(0, 0, 0, ShadeColor, 0, 0, 0, ShadedAlpha, 0, 0, 0, CombinedColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Rocket Robot
						// Color: ShadeColor
						// Alpha: ShadedAlpha*PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode1(ShadeColor, EnvColor, Tex0Color, EnvColor, 0, 0, 0, 1):
						// Rocket Robot
						// Color: ShadeColor*Tex0Color + (1-Tex0Color)*EnvColor
						BlendPasses.Count = 2;
						// Pass 1
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						CombineSetTexture(0, 0, 1);
						// Pass 2
						BlendPasses.Passes[1].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[1].Stages[0].ColorArg1 = D3DTA_TEXTURE|D3DTA_COMPLEMENT;
						BlendPasses.Passes[1].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[1].DstBlend = D3DBLEND_ONE;
						BlendPasses.Passes[1].SrcBlend = D3DBLEND_ONE;
						BlendPasses.Passes[1].TFactor = State.EnvColor;
						CombineSetTexture(1, 0, 1);
						break;
					case BuildCombineMode2(ShadeColor, 0, Tex0Color, 0, 0, 0, 0, ShadedAlpha, 0, 0, 0, CombinedColor, CombinedAlpha, 0, PrimAlpha, 0):
						// Rocket Robot, light (which makes it unplayable in hwnd's plugin)
						// Color: ShadeColor*Tex0Color
						// Alpha: ShadedAlpha*PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(0, 0, 0, Tex0Color, EnvAlpha, 0, Tex1Alpha, Tex0Alpha):
						// Castlevania
						// Color: Tex0Color
						// Alpha: EnvAlpha*Tex1Alpha+Tex0Alpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.EnvColor;
						CombineSetTexture(0, 0, 2);
						CombineSetTexture(0, 1, 1);
						break;
					case BuildCombineMode2(Tex0Color, 0, PrimColor, 0, Tex0Alpha, 0, PrimAlpha, 0, CombinedColor, 0, ShadeColor, 0, 0, 0, 0, CombinedAlpha):
						// Castlevania
						// Color: Tex0Color*PrimColor+ShadeColor
						// Alpha: Tex0Alpha*PrimAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_ADD;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						CombineSetTexture(0, 0, 1);
						break;
					case BuildCombineMode1(ShadeColor, 0, PrimColor, 0, ShadedAlpha, 0, PrimAlpha, 0):
						// Castlevania
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaArg2 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode1(PrimColor, ShadeColor, PrimAlpha, ShadeColor, 0, 0, 0, ShadedAlpha):
						// Castlevania
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						break;
					case BuildCombineMode1(PrimColor, 0, ShadeColor, 0, 0, 0, 0, 1):
						// Castlevania
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_MODULATE;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TFACTOR;
						BlendPasses.Passes[0].Stages[0].ColorArg2 = D3DTA_DIFFUSE;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						BlendPasses.Passes[0].DstBlend = D3DBLEND_ZERO;
						BlendPasses.Passes[0].SrcBlend = D3DBLEND_ONE;
						break;
					case BuildCombineMode2(Tex0Color, Tex1Color, PrimAlpha, Tex1Color, Tex0Alpha, Tex1Alpha, PrimAlpha, Tex1Alpha, PrimColor, EnvColor, CombinedColor, EnvColor, CombinedAlpha, 0, EnvAlpha, 0):
						// Castlevania fire, OMG
						// Color: [(Tex0Color-Tex1Color)*PrimAlpha+Tex1Color] * (PrimColor-EnvColor) + EnvColor
						// Alpha: [(Tex0Alpha-Tex1Alpha)*PrimAlpha+Tex1Alpha] * EnvAlpha
						BlendPasses.Count = 1;
						BlendPasses.Passes[0].Stages[0].ColorOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].ColorArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].ColorOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].ColorArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].ColorArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
						BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].Stages[1].AlphaOp = D3DTOP_BLENDFACTORALPHA;
						BlendPasses.Passes[0].Stages[1].AlphaArg1 = D3DTA_CURRENT;
						BlendPasses.Passes[0].Stages[1].AlphaArg2 = D3DTA_TEXTURE;
						BlendPasses.Passes[0].TFactor = State.PrimColor;
						BlendPasses.Passes[0].Stages[0].Texture = Prerender_ModulateAddTexColor((SubtractColor(State.PrimColor, State.EnvColor)&0x00FFFFFF)|(State.EnvColor&0xFF000000), State.EnvColor&0x00FFFFFF, Texture1);
						BlendPasses.Passes[0].Stages[0].AddressU = State.Address1U;
						BlendPasses.Passes[0].Stages[0].AddressV = State.Address1V;
						BlendPasses.Passes[0].Stages[1].Texture = Prerender_ModulateAddTexColor((SubtractColor(State.PrimColor, State.EnvColor)&0x00FFFFFF)|(State.EnvColor&0xFF000000), State.EnvColor&0x00FFFFFF, Texture2);
						BlendPasses.Passes[0].Stages[1].AddressU = State.Address2U;
						BlendPasses.Passes[0].Stages[1].AddressV = State.Address2V;
						break;
					case BuildCombineMode1(0, 0, 0, 0, 0, 0, 0, 0):
						// Tooie, dunno why
						BlendPasses.Count = 0;
						break;
					default:
						CombinerHandled = false;
						break;
					}
#if COMBINEDEBUGGER
					if (CombinerHandled){
						LoggedCombineModes.Modes[LogCombineModeIndex].EmulationMode = 0;
					}
#endif
				}
#endif
			}
		} else {
			CombinerHandled = true;
			HighlightCombiner(0xFFFF00FF);
			bHighlighted = true;
		}
		
		if (!CombinerHandled){
#if COMBINEDEBUGGER
			LoggedCombineModes.Modes[LogCombineModeIndex].EmulationMode = 2;
#endif
			FirstCombinerStage = true;
			CombinerStage = 0;
			TFactorUsed = false;
		
			switch (State.Othermode.fields.cycletype){
			case 0: // 1 Cycle type
				DoCycle(State.Combine.fields.a0, State.Combine.fields.b0, State.Combine.fields.c0, State.Combine.fields.d0);
				BlendPasses.Passes[0].Stages[CombinerStage].ColorOp = D3DTOP_DISABLE;
				if ((BlendPasses.Passes[0].Stages[0].ColorArg1 == D3DTA_TEXTURE) || (BlendPasses.Passes[0].Stages[0].ColorArg2 == D3DTA_TEXTURE)){
					BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
					BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
				} else {
					BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
				}
				break;
			case 1: // 2 Cycle type
				// do checks if this combiner is properly emulated
				bool CombinerOK;
				CombinerOK = false;
				if ((State.Combine.fields.a1==A_CombinedColor) && (State.Combine.fields.b1!=B_CombinedColor) && (State.Combine.fields.c1!=C_CombinedColor) && (State.Combine.fields.d1!=D_CombinedColor)){
					CombinerOK = true;
				}
				if ((State.Combine.fields.a1!=A_CombinedColor) && (State.Combine.fields.b1>=B_0_MIN)         && (State.Combine.fields.c1==C_CombinedColor) && (State.Combine.fields.d1!=D_CombinedColor)){
					CombinerOK = true;
				}
				if ((State.Combine.fields.a1>=A_0_MIN)         && (State.Combine.fields.b1>=B_0_MIN)         && (State.Combine.fields.c1>=C_0_MIN)         && (State.Combine.fields.d1==D_CombinedColor)){
					CombinerOK = true;
				}
				if ((State.Combine.fields.a1>=A_0_MIN)         && (State.Combine.fields.b1>=B_0_MIN)         && (State.Combine.fields.c1>=C_0_MIN)         && (State.Combine.fields.d1!=D_CombinedColor)){
					CombinerOK = true;
				}
				if (((State.Combine.fields.a1==A_PrimColor) || (State.Combine.fields.a1==A_EnvColor))
				 && ((State.Combine.fields.b1==B_PrimColor) || (State.Combine.fields.b1==B_EnvColor))       && (State.Combine.fields.c1==C_CombinedColor)  && (State.Combine.fields.d1!=D_CombinedColor)){
					CombinerOK = true;
				}
				bool SkipFirstCycle;
				SkipFirstCycle = false;
	
				if ((State.Combine.fields.a1!=A_CombinedColor) && (State.Combine.fields.b1!=B_CombinedColor) && (State.Combine.fields.c1!=C_CombinedColor) && (State.Combine.fields.d1!=D_CombinedColor)){
					SkipFirstCycle = true; // its not needed
					CombinerOK = true;
				}
				if (!CombinerOK){
					if (LogCombinerOutput){
						//Log("Cannot convert combiner properly to d3d\n");
					}
				}
				if (!SkipFirstCycle){
					DoCycle(State.Combine.fields.a0, State.Combine.fields.b0, State.Combine.fields.c0, State.Combine.fields.d0);
				}
				DoCycle(State.Combine.fields.a1, State.Combine.fields.b1, State.Combine.fields.c1, State.Combine.fields.d1);
				BlendPasses.Passes[0].Stages[CombinerStage].ColorOp = D3DTOP_DISABLE;
				BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
				if ((BlendPasses.Passes[0].Stages[0].ColorArg1 == D3DTA_TEXTURE) || (BlendPasses.Passes[0].Stages[0].ColorArg2 == D3DTA_TEXTURE)){
					BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_SELECTARG1;
					BlendPasses.Passes[0].Stages[0].AlphaArg1 = D3DTA_TEXTURE;
				} else {
					BlendPasses.Passes[0].Stages[0].AlphaOp = D3DTOP_DISABLE;
				}
				break;
			}
		}
		State.TextureChanged = false;
		State.CombineChanged = false;
		// check if alpha pipeline is longer than color pipeline. if yes, add SELECTARG1, CURRENT to color pipeline. Otherwise D3D doesn't like it
		for (j=0;j<BlendPasses.Count;j++){
			for (i=0;i<8;i++){
				if (BlendPasses.Passes[j].Stages[i].AlphaOp == D3DTOP_DISABLE){
					break;
				}
				if ((BlendPasses.Passes[j].Stages[i].ColorOp == D3DTOP_DISABLE) && (BlendPasses.Passes[j].Stages[i].AlphaOp != D3DTOP_DISABLE)){
					BlendPasses.Passes[j].Stages[i].ColorOp = D3DTOP_SELECTARG1;
					BlendPasses.Passes[j].Stages[i].ColorArg1 = D3DTA_CURRENT;
				}
			}
		}
		ChangeTextureCoordinates = false;
		for (j=0;j<_TEXTURES;j++){
			if (SecondTex[j]){
				ChangeTextureCoordinates = true;
			}
		}
		// this can in some cases eliminate a pass, if it just ADDs a black color
		for (j=0;j<BlendPasses.Count;j++){
			if ((BlendPasses.Passes[j].DstBlend == D3DBLEND_ONE) && (BlendPasses.Passes[j].Stages[0].ColorOp==D3DTOP_SELECTARG1) && ((BlendPasses.Passes[j].TFactor & BitM24) == 0x00000000) && (BlendPasses.Passes[j].Stages[0].ColorArg1==D3DTA_TFACTOR)){
				if (j==BlendPasses.Count-1){
					BlendPasses.Count--;
				} else {
					ShowError("Error: Trying to remove a pass in the middle of a BlendPassPipeline");
				}
			}
		}
		if (LogCombinerOutput){
			if (CombinerHandled){
				//Log("Combiner Hardcoded:\n");
			} else {
				//Log("Combiner unknown, trying universal combiner:\n");
			}
			for (j=0;j<BlendPasses.Count;j++){
				//Log("Pass %i: SrcBlend: %s, DstBlend: %s, TFactor: 0x%X\n", j, BlendNames[BlendPasses.Passes[j].SrcBlend], BlendNames[BlendPasses.Passes[j].DstBlend], BlendPasses.Passes[j].TFactor);
				for (i=0;i<8;i++){
					if (BlendPasses.Passes[j].Stages[i].ColorOp == D3DTOP_DISABLE){
						continue;
					}
					//Log("  Stage %i: ColorOp: %s, Arg1: %s, Arg2: %s, AlphaOp: %s, AlphaArg1: %s, AlphaArg2: %s\n", i,
					//			ColorOpNames[BlendPasses.Passes[j].Stages[i].ColorOp],
					//			ColorArgNames[BlendPasses.Passes[j].Stages[i].ColorArg1],
					//			(BlendPasses.Passes[j].Stages[i].ColorOp==D3DTOP_SELECTARG1?"n/a":ColorArgNames[BlendPasses.Passes[j].Stages[i].ColorArg2]),
					//			ColorOpNames[BlendPasses.Passes[j].Stages[i].AlphaOp],
					//			ColorArgNames[BlendPasses.Passes[j].Stages[i].AlphaArg1],
					//			(BlendPasses.Passes[j].Stages[i].AlphaOp==D3DTOP_SELECTARG1?"n/a":ColorArgNames[BlendPasses.Passes[j].Stages[i].AlphaArg2]));
				}
			}
			//Log("\n");
		}
	}
}
}