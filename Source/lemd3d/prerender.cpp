#include "stdafx.h"

U32 FindPrerenderedTexture(TPrerenderType Type, U32 Flag1, U32 Flag2, U32 Flag3, U32 TexUTI, U32 Width, U32 Height)
{
	TexUTI |= Bit32;
	U32 i;
	for (i=0;i<State.TextureCount;i++){
		if ((TexUTI==State.Textures[i].UTI) && (Type==State.Textures[i].PrerenderType) && (Flag1==State.Textures[i].Flag1) && (Flag2==State.Textures[i].Flag2) && (Flag3==State.Textures[i].Flag3) && (Width==State.Textures[i].Width) && (Height==State.Textures[i].Height)){
			State.Textures[i].Age=0;
			return i;
		}
	}
	return 0xFFFFFFFF;
}

U32 CreatePrerenderedTexture(TPrerenderType Type, U32 Flag1, U32 Flag2, U32 Flag3, U32 UTI, D3DFORMAT Format, U32 Width, U32 Height)
{
	UTI |= Bit32;
	U32 i, oldest;
	oldest = 0;
	if (State.TextureCount>=(MaxTextureCount-5)){
		for (i=1;i<State.TextureCount;i++){
			if ((State.Textures[i].Age>=State.Textures[oldest].Age) && (State.Textures[i].UTI != (UTI&BitM31))){
				oldest = i;
			}
		}
		FreeObject(State.Textures[oldest].D3DObject);
	} else {
		oldest = State.TextureCount;
		State.TextureCount++;
	}
	State.Textures[oldest].Age = 0;
	State.Textures[oldest].PrerenderType = Type;
	State.Textures[oldest].UTI = UTI;
	State.Textures[oldest].Flag1 = Flag1;
	State.Textures[oldest].Flag2 = Flag2;
	State.Textures[oldest].Flag3 = Flag3;
	State.Textures[oldest].Width = Width;
	State.Textures[oldest].Height= Height;
	SafeDX(g_pd3dDevice->CreateTexture(Width, Height, 1, 0, Format, D3DPOOL_MANAGED, &State.Textures[oldest].D3DObject, NULL));
	return oldest;
}

LPDIRECT3DTEXTURE9 Prerender_2ColTex(U32 Color1, U32 Color2, U32 TextureIndex)
{
	if ((Color1==0xFFFFFFFF) && (Color2==0x00000000)){
		return State.Textures[TextureIndex].D3DObject;
	}
	U32 Index, Width, Height;
	D3DSURFACE_DESC SourceDesc;
	SafeDX(State.Textures[TextureIndex].D3DObject->GetLevelDesc(0, &SourceDesc));
	Width = SourceDesc.Width;
	Height = SourceDesc.Height;
	Index = FindPrerenderedTexture(_2ColTex, Color1, Color2, 0, State.Textures[TextureIndex].UTI, Width, Height);
	if (Index==0xFFFFFFFF){
		LPDIRECT3DSURFACE9 TargetSurface, SourceSurface;
		D3DLOCKED_RECT TargetRect, SourceRect;

		

		Index = CreatePrerenderedTexture(_2ColTex, Color1, Color2, 0, State.Textures[TextureIndex].UTI, SourceDesc.Format, Width, Height);
		SafeDX(State.Textures[Index].D3DObject->GetSurfaceLevel(0, &TargetSurface));
		SafeDX(State.Textures[TextureIndex].D3DObject->GetSurfaceLevel(0, &SourceSurface));

		SafeDX(TargetSurface->LockRect(&TargetRect, 0, 0));
		SafeDX(SourceSurface->LockRect(&SourceRect, 0, D3DLOCK_READONLY));
		
		U32 x, y;
		union {
			TColor8888* Col8888;
			TColor4444* Col4444;
			TColor1555* Col1555;
			U8* p8;
			U16* p16;
			U32* p32;
			U64* p64;
		} sourceptr, targetptr;
		
		sourceptr.p8 = (U8*)SourceRect.pBits;
		targetptr.p8 = (U8*)TargetRect.pBits;
		switch (SourceDesc.Format){
		case D3DFMT_A8R8G8B8:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col8888[x].a = (((sourceptr.Col8888[x].a+1) * BITS(Color1, 24, 8))>>8) + (((255-sourceptr.Col8888[x].a+1) * BITS(Color2, 24,8))>>8);
					targetptr.Col8888[x].r = (((sourceptr.Col8888[x].r+1) * BITS(Color1, 16, 8))>>8) + (((255-sourceptr.Col8888[x].r+1) * BITS(Color2, 16,8))>>8);
					targetptr.Col8888[x].g = (((sourceptr.Col8888[x].g+1) * BITS(Color1,  8, 8))>>8) + (((255-sourceptr.Col8888[x].g+1) * BITS(Color2,  8,8))>>8);
					targetptr.Col8888[x].b = (((sourceptr.Col8888[x].b+1) * BITS(Color1,  0, 8))>>8) + (((255-sourceptr.Col8888[x].b+1) * BITS(Color2,  0,8))>>8);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A4R4G4B4:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col4444[x].a = (((sourceptr.Col4444[x].a+1) * BITS(Color1, 24, 8))>>8) + (((15-sourceptr.Col4444[x].a+1) * BITS(Color2, 24,8))>>8);
					targetptr.Col4444[x].r = (((sourceptr.Col4444[x].r+1) * BITS(Color1, 16, 8))>>8) + (((15-sourceptr.Col4444[x].r+1) * BITS(Color2, 16,8))>>8);
					targetptr.Col4444[x].g = (((sourceptr.Col4444[x].g+1) * BITS(Color1,  8, 8))>>8) + (((15-sourceptr.Col4444[x].g+1) * BITS(Color2,  8,8))>>8);
					targetptr.Col4444[x].b = (((sourceptr.Col4444[x].b+1) * BITS(Color1,  0, 8))>>8) + (((15-sourceptr.Col4444[x].b+1) * BITS(Color2,  0,8))>>8);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A1R5G5B5:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col1555[x].a = (((sourceptr.Col1555[x].a+1) * BITS(Color1, 24, 8))>>8) + ((( 1-sourceptr.Col1555[x].a+1) * BITS(Color2, 24,8))>>8);
					targetptr.Col1555[x].r = (((sourceptr.Col1555[x].r+1) * BITS(Color1, 16, 8))>>8) + (((31-sourceptr.Col1555[x].r+1) * BITS(Color2, 16,8))>>8);
					targetptr.Col1555[x].g = (((sourceptr.Col1555[x].g+1) * BITS(Color1,  8, 8))>>8) + (((31-sourceptr.Col1555[x].g+1) * BITS(Color2,  8,8))>>8);
					targetptr.Col1555[x].b = (((sourceptr.Col1555[x].b+1) * BITS(Color1,  0, 8))>>8) + (((31-sourceptr.Col1555[x].b+1) * BITS(Color2,  0,8))>>8);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		default:
			ShowError("Prerender_2ColTex: Unhandled texture format");
			break;
		}
		SafeDX(TargetSurface->UnlockRect());
		SafeDX(SourceSurface->UnlockRect());
		FreeObject(TargetSurface);
		FreeObject(SourceSurface);
	}
	return State.Textures[Index].D3DObject;
}

LPDIRECT3DTEXTURE9 Prerender_ModulateAddTexColor(U32 Color1, U32 Color2, U32 TextureIndex)
{
	if ((Color1==0xFFFFFFFF) && (Color2==0x00000000)){
		return State.Textures[TextureIndex].D3DObject;
	}
	U32 Index, Width, Height;
	D3DSURFACE_DESC SourceDesc;
	SafeDX(State.Textures[TextureIndex].D3DObject->GetLevelDesc(0, &SourceDesc));
	Width = SourceDesc.Width;
	Height = SourceDesc.Height;
	Index = FindPrerenderedTexture(_ModulateAddTexColor, Color1, Color2, 0, State.Textures[TextureIndex].UTI, Width, Height);
	if (Index==0xFFFFFFFF){
		LPDIRECT3DSURFACE9 TargetSurface, SourceSurface;
		D3DLOCKED_RECT TargetRect, SourceRect;

		Index = CreatePrerenderedTexture(_ModulateAddTexColor, Color1, Color2, 0, State.Textures[TextureIndex].UTI, SourceDesc.Format, Width, Height);
		SafeDX(State.Textures[Index].D3DObject->GetSurfaceLevel(0, &TargetSurface));
		SafeDX(State.Textures[TextureIndex].D3DObject->GetSurfaceLevel(0, &SourceSurface));

		SafeDX(TargetSurface->LockRect(&TargetRect, 0, 0));
		SafeDX(SourceSurface->LockRect(&SourceRect, 0, D3DLOCK_READONLY));
		
		U32 x, y;
		union {
			TColor8888* Col8888;
			TColor4444* Col4444;
			TColor1555* Col1555;
			U8* p8;
			U16* p16;
			U32* p32;
			U64* p64;
		} sourceptr, targetptr;
		
		sourceptr.p8 = (U8*)SourceRect.pBits;
		targetptr.p8 = (U8*)TargetRect.pBits;
		switch (SourceDesc.Format){
		case D3DFMT_A8R8G8B8:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.p32[x] = AddColor(MultColor(sourceptr.p32[x], Color1), Color2);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A1R5G5B5:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col1555[x].r = ((((sourceptr.Col1555[x].a+1)*((Color1>>24)&BitM8))>>8)) + (((Color2>>24)&BitM8)>>3);
					targetptr.Col1555[x].r = ((((sourceptr.Col1555[x].r+1)*((Color1>>16)&BitM8))>>8)) + (((Color2>>16)&BitM8)>>3);
					targetptr.Col1555[x].g = ((((sourceptr.Col1555[x].g+1)*((Color1>> 8)&BitM8))>>8)) + (((Color2>> 8)&BitM8)>>3);
					targetptr.Col1555[x].b = ((((sourceptr.Col1555[x].b+1)*((Color1>> 0)&BitM8))>>8)) + (((Color2>> 0)&BitM8)>>3);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A4R4G4B4:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col4444[x].a = ((((sourceptr.Col4444[x].a+1)*((Color1>>24)&BitM8))>>8)) + (((Color2>>24)&BitM8)>>4);
					targetptr.Col4444[x].r = ((((sourceptr.Col4444[x].r+1)*((Color1>>16)&BitM8))>>8)) + (((Color2>>16)&BitM8)>>4);
					targetptr.Col4444[x].g = ((((sourceptr.Col4444[x].g+1)*((Color1>> 8)&BitM8))>>8)) + (((Color2>> 8)&BitM8)>>4);
					targetptr.Col4444[x].b = ((((sourceptr.Col4444[x].b+1)*((Color1>> 0)&BitM8))>>8)) + (((Color2>> 0)&BitM8)>>4);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		default:
			ShowError("Prerender_ModulateAddTexColor: Unhandled texture format");
			break;
		}
		SafeDX(TargetSurface->UnlockRect());
		SafeDX(SourceSurface->UnlockRect());
		FreeObject(TargetSurface);
		FreeObject(SourceSurface);
	}
	return State.Textures[Index].D3DObject;
}

LPDIRECT3DTEXTURE9 Prerender_ModulateTexColor(U32 Color1, U32 TextureIndex)
{
	if (Color1==0xFFFFFFFF){
		return State.Textures[TextureIndex].D3DObject;
	}
	U32 Index, Width, Height;
	D3DSURFACE_DESC SourceDesc;
	SafeDX(State.Textures[TextureIndex].D3DObject->GetLevelDesc(0, &SourceDesc));
	Width = SourceDesc.Width;
	Height = SourceDesc.Height;
	Index = FindPrerenderedTexture(_ModulateTexColor, Color1, 0, 0, State.Textures[TextureIndex].UTI, Width, Height);
	if (Index==0xFFFFFFFF){
		LPDIRECT3DSURFACE9 TargetSurface, SourceSurface;
		D3DLOCKED_RECT TargetRect, SourceRect;

		if (SourceDesc.Format==D3DFMT_A1R5G5B5){
			// convert to 8888, otherwise the transparency wouldn't look right
			Index = CreatePrerenderedTexture(_ModulateTexColor, Color1, 0, 0, State.Textures[TextureIndex].UTI, D3DFMT_A8R8G8B8, Width, Height);
		} else {
			Index = CreatePrerenderedTexture(_ModulateTexColor, Color1, 0, 0, State.Textures[TextureIndex].UTI, SourceDesc.Format, Width, Height);
		}
		SafeDX(State.Textures[Index].D3DObject->GetSurfaceLevel(0, &TargetSurface));
		SafeDX(State.Textures[TextureIndex].D3DObject->GetSurfaceLevel(0, &SourceSurface));

		SafeDX(TargetSurface->LockRect(&TargetRect, 0, 0));
		SafeDX(SourceSurface->LockRect(&SourceRect, 0, D3DLOCK_READONLY));
		
		U32 x, y;
		union {
			TColor8888* Col8888;
			TColor4444* Col4444;
			TColor1555* Col1555;
			U8* p8;
			U16* p16;
			U32* p32;
			U64* p64;
		} sourceptr, targetptr;
		
		sourceptr.p8 = (U8*)SourceRect.pBits;
		targetptr.p8 = (U8*)TargetRect.pBits;
		switch (SourceDesc.Format){
		case D3DFMT_A8R8G8B8:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.p32[x] = MultColor(sourceptr.p32[x], Color1);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A1R5G5B5:
			// convert to 8888
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col8888[x].a = ((((sourceptr.Col1555[x].a  )*((Color1>>24)&BitM8))));
					targetptr.Col8888[x].r = ((((sourceptr.Col1555[x].r+1)*((Color1>>16)&BitM8))>>5));
					targetptr.Col8888[x].g = ((((sourceptr.Col1555[x].g+1)*((Color1>> 8)&BitM8))>>5));
					targetptr.Col8888[x].b = ((((sourceptr.Col1555[x].b+1)*((Color1>> 0)&BitM8))>>5));
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A4R4G4B4:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col4444[x].a = ((((sourceptr.Col4444[x].a+1)*((Color1>>24)&BitM8))>>8));
					targetptr.Col4444[x].r = ((((sourceptr.Col4444[x].r+1)*((Color1>>16)&BitM8))>>8));
					targetptr.Col4444[x].g = ((((sourceptr.Col4444[x].g+1)*((Color1>> 8)&BitM8))>>8));
					targetptr.Col4444[x].b = ((((sourceptr.Col4444[x].b+1)*((Color1>> 0)&BitM8))>>8));
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		default:
			ShowError("Prerender_ModulateTexColor: Unhandled texture format");
			break;
		}
		SafeDX(TargetSurface->UnlockRect());
		SafeDX(SourceSurface->UnlockRect());
		FreeObject(TargetSurface);
		FreeObject(SourceSurface);
	}
	return State.Textures[Index].D3DObject;
}

LPDIRECT3DTEXTURE9 Prerender_ModulateSubTexColor(U32 Color1, U32 Color2, U32 TextureIndex)
{
	if ((Color1==0xFFFFFFFF) && (Color2==0x00000000)){
		return State.Textures[TextureIndex].D3DObject;
	}
	U32 Index, Width, Height;
	D3DSURFACE_DESC SourceDesc;
	SafeDX(State.Textures[TextureIndex].D3DObject->GetLevelDesc(0, &SourceDesc));
	Width = SourceDesc.Width;
	Height = SourceDesc.Height;
	Index = FindPrerenderedTexture(_ModulateSubTexColor, Color1, Color2, 0, State.Textures[TextureIndex].UTI, Width, Height);
	if (Index==0xFFFFFFFF){
		LPDIRECT3DSURFACE9 TargetSurface, SourceSurface;
		D3DLOCKED_RECT TargetRect, SourceRect;

		Index = CreatePrerenderedTexture(_ModulateSubTexColor, Color1, Color2, 0, State.Textures[TextureIndex].UTI, SourceDesc.Format, Width, Height);
		SafeDX(State.Textures[Index].D3DObject->GetSurfaceLevel(0, &TargetSurface));
		SafeDX(State.Textures[TextureIndex].D3DObject->GetSurfaceLevel(0, &SourceSurface));

		SafeDX(TargetSurface->LockRect(&TargetRect, 0, 0));
		SafeDX(SourceSurface->LockRect(&SourceRect, 0, D3DLOCK_READONLY));
		
		U32 x, y;
		union {
			TColor8888* Col8888;
			TColor4444* Col4444;
			TColor1555* Col1555;
			U8* p8;
			U16* p16;
			U32* p32;
			U64* p64;
		} sourceptr, targetptr;
		
		sourceptr.p8 = (U8*)SourceRect.pBits;
		targetptr.p8 = (U8*)TargetRect.pBits;
		switch (SourceDesc.Format){
		case D3DFMT_A8R8G8B8:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.p32[x] = SubtractColor(MultColor(sourceptr.p32[x], Color1), Color2);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A1R5G5B5:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col1555[x].a = ((((sourceptr.Col1555[x].a+1)*((Color1>>24)&BitM8))>>8)) - (((Color2>>24)&BitM8)>>3);
					targetptr.Col1555[x].r = ((((sourceptr.Col1555[x].r+1)*((Color1>>16)&BitM8))>>8)) - (((Color2>>16)&BitM8)>>3);
					targetptr.Col1555[x].g = ((((sourceptr.Col1555[x].g+1)*((Color1>> 8)&BitM8))>>8)) - (((Color2>> 8)&BitM8)>>3);
					targetptr.Col1555[x].b = ((((sourceptr.Col1555[x].b+1)*((Color1>> 0)&BitM8))>>8)) - (((Color2>> 0)&BitM8)>>3);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A4R4G4B4:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col4444[x].a = ((((sourceptr.Col4444[x].a+1)*((Color1>>24)&BitM8))>>8)) - (((Color2>>24)&BitM8)>>4);
					targetptr.Col4444[x].r = ((((sourceptr.Col4444[x].r+1)*((Color1>>16)&BitM8))>>8)) - (((Color2>>16)&BitM8)>>4);
					targetptr.Col4444[x].g = ((((sourceptr.Col4444[x].g+1)*((Color1>> 8)&BitM8))>>8)) - (((Color2>> 8)&BitM8)>>4);
					targetptr.Col4444[x].b = ((((sourceptr.Col4444[x].b+1)*((Color1>> 0)&BitM8))>>8)) - (((Color2>> 0)&BitM8)>>4);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		default:
			ShowError("Prerender_ModulateSubTexColor: Unhandled texture format");
			break;
		}
		SafeDX(TargetSurface->UnlockRect());
		SafeDX(SourceSurface->UnlockRect());
		FreeObject(TargetSurface);
		FreeObject(SourceSurface);
	}
	return State.Textures[Index].D3DObject;
}

LPDIRECT3DTEXTURE9 Prerender_SubtractTexColor(U32 Color1, U32 TextureIndex)
{
	if (Color1==0x00000000){
		return State.Textures[TextureIndex].D3DObject;
	}
	U32 Index, Width, Height;
	D3DSURFACE_DESC SourceDesc;
	SafeDX(State.Textures[TextureIndex].D3DObject->GetLevelDesc(0, &SourceDesc));
	Width = SourceDesc.Width;
	Height = SourceDesc.Height;
	Index = FindPrerenderedTexture(_SubtractTexColor, Color1, 0, 0, State.Textures[TextureIndex].UTI, Width, Height);
	if (Index==0xFFFFFFFF){
		LPDIRECT3DSURFACE9 TargetSurface, SourceSurface;
		D3DLOCKED_RECT TargetRect, SourceRect;

		Index = CreatePrerenderedTexture(_SubtractTexColor, Color1, 0, 0, State.Textures[TextureIndex].UTI, SourceDesc.Format, Width, Height);
		SafeDX(State.Textures[Index].D3DObject->GetSurfaceLevel(0, &TargetSurface));
		SafeDX(State.Textures[TextureIndex].D3DObject->GetSurfaceLevel(0, &SourceSurface));

		SafeDX(TargetSurface->LockRect(&TargetRect, 0, 0));
		SafeDX(SourceSurface->LockRect(&SourceRect, 0, D3DLOCK_READONLY));
		
		U32 x, y;
		union {
			TColor8888* Col8888;
			TColor4444* Col4444;
			TColor1555* Col1555;
			U8* p8;
			U16* p16;
			U32* p32;
			U64* p64;
		} sourceptr, targetptr;
		
		sourceptr.p8 = (U8*)SourceRect.pBits;
		targetptr.p8 = (U8*)TargetRect.pBits;
		switch (SourceDesc.Format){
		case D3DFMT_A8R8G8B8:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.p32[x] = SubtractColor(sourceptr.p32[x], Color1);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A1R5G5B5:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col1555[x].a = sourceptr.Col1555[x].a - (((Color1>>24)&BitM8)>>7);
					targetptr.Col1555[x].r = sourceptr.Col1555[x].r - (((Color1>>16)&BitM8)>>3);
					targetptr.Col1555[x].g = sourceptr.Col1555[x].g - (((Color1>> 8)&BitM8)>>3);
					targetptr.Col1555[x].b = sourceptr.Col1555[x].b - (((Color1>> 0)&BitM8)>>3);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A4R4G4B4:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col4444[x].a = sourceptr.Col4444[x].a - (((Color1>>24)&BitM8)>>4);
					targetptr.Col4444[x].r = sourceptr.Col4444[x].r - (((Color1>>16)&BitM8)>>4);
					targetptr.Col4444[x].g = sourceptr.Col4444[x].g - (((Color1>> 8)&BitM8)>>4);
					targetptr.Col4444[x].b = sourceptr.Col4444[x].b - (((Color1>> 0)&BitM8)>>4);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		default:
			ShowError("Prerender_SubtactTexColor: Unhandled texture format");
			break;
		}
		SafeDX(TargetSurface->UnlockRect());
		SafeDX(SourceSurface->UnlockRect());
		FreeObject(TargetSurface);
		FreeObject(SourceSurface);
	}
	return State.Textures[Index].D3DObject;
}

LPDIRECT3DTEXTURE9 Prerender_ModulateColorReplaceAlpha(U32 Color1, U32 Alpha, U32 TextureIndex)
{
	U32 Index, Width, Height;
	D3DSURFACE_DESC SourceDesc;
	SafeDX(State.Textures[TextureIndex].D3DObject->GetLevelDesc(0, &SourceDesc));
	Width = SourceDesc.Width;
	Height = SourceDesc.Height;
	Index = FindPrerenderedTexture(_ModulateColorReplaceAlpha, Color1, Alpha, 0, State.Textures[TextureIndex].UTI, Width, Height);
	if (Index==0xFFFFFFFF){
		LPDIRECT3DSURFACE9 TargetSurface, SourceSurface;
		D3DLOCKED_RECT TargetRect, SourceRect;

		if (SourceDesc.Format==D3DFMT_A1R5G5B5){
			// convert to 8888, otherwise the transparency wouldn't look right
			Index = CreatePrerenderedTexture(_ModulateColorReplaceAlpha, Color1, Alpha, 0, State.Textures[TextureIndex].UTI, D3DFMT_A8R8G8B8, Width, Height);
		} else {
			Index = CreatePrerenderedTexture(_ModulateColorReplaceAlpha, Color1, Alpha, 0, State.Textures[TextureIndex].UTI, SourceDesc.Format, Width, Height);
		}
		SafeDX(State.Textures[Index].D3DObject->GetSurfaceLevel(0, &TargetSurface));
		SafeDX(State.Textures[TextureIndex].D3DObject->GetSurfaceLevel(0, &SourceSurface));

		SafeDX(TargetSurface->LockRect(&TargetRect, 0, 0));
		SafeDX(SourceSurface->LockRect(&SourceRect, 0, D3DLOCK_READONLY));
		
		U32 x, y;
		union {
			TColor8888* Col8888;
			TColor4444* Col4444;
			TColor1555* Col1555;
			U8* p8;
			U16* p16;
			U32* p32;
			U64* p64;
		} sourceptr, targetptr;
		
		sourceptr.p8 = (U8*)SourceRect.pBits;
		targetptr.p8 = (U8*)TargetRect.pBits;
		switch (SourceDesc.Format){
		case D3DFMT_A8R8G8B8:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.p32[x] = (MultColor(sourceptr.p32[x], Color1)&0x00FFFFFF)|(Alpha&0xFF000000);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A1R5G5B5:
			// convert to 8888
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col8888[x].a = Alpha>>24;
					targetptr.Col8888[x].r = ((((sourceptr.Col1555[x].r+1)*((Color1>>16)&BitM8))>>5));
					targetptr.Col8888[x].g = ((((sourceptr.Col1555[x].g+1)*((Color1>> 8)&BitM8))>>5));
					targetptr.Col8888[x].b = ((((sourceptr.Col1555[x].b+1)*((Color1>> 0)&BitM8))>>5));
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A4R4G4B4:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col4444[x].a = Alpha>>28;
					targetptr.Col4444[x].r = ((((sourceptr.Col4444[x].r+1)*((Color1>>16)&BitM8))>>8));
					targetptr.Col4444[x].g = ((((sourceptr.Col4444[x].g+1)*((Color1>> 8)&BitM8))>>8));
					targetptr.Col4444[x].b = ((((sourceptr.Col4444[x].b+1)*((Color1>> 0)&BitM8))>>8));
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		default:
			ShowError("Prerender_ModulateColorReplaceAlpha: Unhandled texture format");
			break;
		}
		SafeDX(TargetSurface->UnlockRect());
		SafeDX(SourceSurface->UnlockRect());
		FreeObject(TargetSurface);
		FreeObject(SourceSurface);
	}
	return State.Textures[Index].D3DObject;
}

LPDIRECT3DTEXTURE9 Prerender_ModulateAlphaReplaceColor(U32 Alpha, U32 Color1, U32 TextureIndex)
{
	U32 Index, Width, Height;
	D3DSURFACE_DESC SourceDesc;
	SafeDX(State.Textures[TextureIndex].D3DObject->GetLevelDesc(0, &SourceDesc));
	Width = SourceDesc.Width;
	Height = SourceDesc.Height;
	Index = FindPrerenderedTexture(_ModulateAlphaReplaceColor, Color1, Alpha, 0, State.Textures[TextureIndex].UTI, Width, Height);
	if (Index==0xFFFFFFFF){
		LPDIRECT3DSURFACE9 TargetSurface, SourceSurface;
		D3DLOCKED_RECT TargetRect, SourceRect;

		if (SourceDesc.Format==D3DFMT_A1R5G5B5){
			// convert to 8888, otherwise the transparency wouldn't look right
			Index = CreatePrerenderedTexture(_ModulateAlphaReplaceColor, Color1, Alpha, 0, State.Textures[TextureIndex].UTI, D3DFMT_A8R8G8B8, Width, Height);
		} else {
			Index = CreatePrerenderedTexture(_ModulateAlphaReplaceColor, Color1, Alpha, 0, State.Textures[TextureIndex].UTI, SourceDesc.Format, Width, Height);
		}
		SafeDX(State.Textures[Index].D3DObject->GetSurfaceLevel(0, &TargetSurface));
		SafeDX(State.Textures[TextureIndex].D3DObject->GetSurfaceLevel(0, &SourceSurface));

		SafeDX(TargetSurface->LockRect(&TargetRect, 0, 0));
		SafeDX(SourceSurface->LockRect(&SourceRect, 0, D3DLOCK_READONLY));
		
		U32 x, y;
		union {
			TColor8888* Col8888;
			TColor4444* Col4444;
			TColor1555* Col1555;
			U8* p8;
			U16* p16;
			U32* p32;
			U64* p64;
		} sourceptr, targetptr;
		
		sourceptr.p8 = (U8*)SourceRect.pBits;
		targetptr.p8 = (U8*)TargetRect.pBits;
		switch (SourceDesc.Format){
		case D3DFMT_A8R8G8B8:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col8888[x].a = ((sourceptr.Col8888[x].a+1)*(Alpha>>24))>>8;
					targetptr.Col8888[x].r = Color1>>16;
					targetptr.Col8888[x].g = Color1>>8;
					targetptr.Col8888[x].b = Color1;
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A1R5G5B5:
			// convert to 8888
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col8888[x].a = sourceptr.Col1555[x].a*(Alpha>>24);
					targetptr.Col8888[x].r = Color1>>16;
					targetptr.Col8888[x].g = Color1>>8;
					targetptr.Col8888[x].b = Color1;
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A4R4G4B4:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col4444[x].a = ((sourceptr.Col4444[x].a+1)*(Alpha>>24))>>8;
					targetptr.Col4444[x].r = Color1>>20;
					targetptr.Col4444[x].g = Color1>>12;
					targetptr.Col4444[x].b = Color1>>4;
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		default:
			ShowError("Prerender_ModulateColorReplaceAlpha: Unhandled texture format");
			break;
		}
		SafeDX(TargetSurface->UnlockRect());
		SafeDX(SourceSurface->UnlockRect());
		FreeObject(TargetSurface);
		FreeObject(SourceSurface);

	}
	return State.Textures[Index].D3DObject;
}

LPDIRECT3DTEXTURE9 Prerender_ModulateOneMinusTexColor(U32 Color1, U32 TextureIndex)
{
	U32 Index, Width, Height;
	D3DSURFACE_DESC SourceDesc;
	SafeDX(State.Textures[TextureIndex].D3DObject->GetLevelDesc(0, &SourceDesc));
	Width = SourceDesc.Width;
	Height = SourceDesc.Height;
	Index = FindPrerenderedTexture(_ModulateOneMinusTexColor, Color1, 0, 0, State.Textures[TextureIndex].UTI, Width, Height);
	if (Index==0xFFFFFFFF){
		LPDIRECT3DSURFACE9 TargetSurface, SourceSurface;
		D3DLOCKED_RECT TargetRect, SourceRect;

		if (SourceDesc.Format==D3DFMT_A1R5G5B5){
			// convert to 8888, otherwise the transparency wouldn't look right
			Index = CreatePrerenderedTexture(_ModulateOneMinusTexColor, Color1, 0, 0, State.Textures[TextureIndex].UTI, D3DFMT_A8R8G8B8, Width, Height);
		} else {
			Index = CreatePrerenderedTexture(_ModulateOneMinusTexColor, Color1, 0, 0, State.Textures[TextureIndex].UTI, SourceDesc.Format, Width, Height);
		}
		SafeDX(State.Textures[Index].D3DObject->GetSurfaceLevel(0, &TargetSurface));
		SafeDX(State.Textures[TextureIndex].D3DObject->GetSurfaceLevel(0, &SourceSurface));

		SafeDX(TargetSurface->LockRect(&TargetRect, 0, 0));
		SafeDX(SourceSurface->LockRect(&SourceRect, 0, D3DLOCK_READONLY));
		
		U32 x, y;
		union {
			TColor8888* Col8888;
			TColor4444* Col4444;
			TColor1555* Col1555;
			U8* p8;
			U16* p16;
			U32* p32;
			U64* p64;
		} sourceptr, targetptr;
		
		sourceptr.p8 = (U8*)SourceRect.pBits;
		targetptr.p8 = (U8*)TargetRect.pBits;
		switch (SourceDesc.Format){
		case D3DFMT_A8R8G8B8:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.p32[x] = MultColor(~sourceptr.p32[x], Color1);
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A1R5G5B5:
			// convert to 8888
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col8888[x].a = ((((~sourceptr.Col1555[x].a  )*((Color1>>24)&BitM8))));
					targetptr.Col8888[x].r = ((((~(sourceptr.Col1555[x].r+1))*((Color1>>16)&BitM8))>>5));
					targetptr.Col8888[x].g = ((((~(sourceptr.Col1555[x].g+1))*((Color1>> 8)&BitM8))>>5));
					targetptr.Col8888[x].b = ((((~(sourceptr.Col1555[x].b+1))*((Color1>> 0)&BitM8))>>5));
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		case D3DFMT_A4R4G4B4:
			for (y=0;y<Height;y++){
				for (x=0;x<Width;x++){
					targetptr.Col4444[x].a = ((((~(sourceptr.Col4444[x].a+1))*((Color1>>24)&BitM8))>>8));
					targetptr.Col4444[x].r = ((((~(sourceptr.Col4444[x].r+1))*((Color1>>16)&BitM8))>>8));
					targetptr.Col4444[x].g = ((((~(sourceptr.Col4444[x].g+1))*((Color1>> 8)&BitM8))>>8));
					targetptr.Col4444[x].b = ((((~(sourceptr.Col4444[x].b+1))*((Color1>> 0)&BitM8))>>8));
				}
				targetptr.p8 +=	TargetRect.Pitch;
				sourceptr.p8 +=	SourceRect.Pitch;
			}
			break;
		default:
			ShowError("Prerender_ModulateOneMinusTexColor: Unhandled texture format");
			break;
		}
		SafeDX(TargetSurface->UnlockRect());
		SafeDX(SourceSurface->UnlockRect());
		FreeObject(TargetSurface);
		FreeObject(SourceSurface);
	}
	return State.Textures[Index].D3DObject;
}