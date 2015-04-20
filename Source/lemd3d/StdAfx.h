// stdafx.h : Include-Datei für Standard-System-Include-Dateien,
//  oder projektspezifische Include-Dateien, die häufig benutzt, aber
//      in unregelmäßigen Abständen geändert werden.
//

#if !defined(AFX_STDAFX_H__0A3D67C0_3359_11D5_8D7F_444553540000__INCLUDED_)
#define AFX_STDAFX_H__0A3D67C0_3359_11D5_8D7F_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DIRECT3D_VERSION 0x0900

// Fügen Sie hier Ihre Header-Dateien ein
#define WIN32_LEAN_AND_MEAN		// Selten benutzte Teile der Windows-Header nicht einbinden

#include <windows.h>
#if DIRECT3D_VERSION==0x0900
#include <d3d9.h>
#include <d3dx9math.h>
#else
#include <d3d.h>
#include <d3dxmath.h>
#endif
#include <dxerr.h>
#include <commctrl.h>
#include <mmsystem.h>

// ZU ERLEDIGEN: Verweisen Sie hier auf zusätzliche Header-Dateien, die Ihr Programm benötigt

#include "globals.h"
#include "resource.h"
#include "DListParser.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt zusätzliche Deklarationen unmittelbar vor der vorherigen Zeile ein.

#endif // !defined(AFX_STDAFX_H__0A3D67C0_3359_11D5_8D7F_444553540000__INCLUDED_)
