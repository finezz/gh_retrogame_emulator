// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//-----------------------------------------------------------------------------
/// \file
/// \brief load and initialise the 3D driver DLL

#include "../doomdef.h"
#ifdef HWRENDER
#include "../hardware/hw_drv.h"        // get the standard 3D Driver DLL exports prototypes
#endif

#ifdef HW3SOUND
#include "../hardware/hw3dsdrv.h"      // get the 3D sound driver DLL export prototypes
#endif

#ifdef _WINDOWS

#include "win_dll.h"
#include "win_main.h"       // I_ShowLastError()

#if defined(HWRENDER) || defined(HW3SOUND)
typedef struct loadfunc_s {
	LPCSTR fnName;
	LPVOID fnPointer;
} loadfunc_t;

// --------------------------------------------------------------------------
// Load a DLL, returns the HMODULE handle or NULL
// --------------------------------------------------------------------------
static HMODULE LoadDLL (LPCSTR dllName, loadfunc_t *funcTable)
{
	LPVOID      funcPtr;
	loadfunc_t *loadfunc;
	HMODULE     hModule;

	if ((hModule = LoadLibraryA(dllName)) != NULL)
	{
		// get function pointers for all functions we use
		for (loadfunc = funcTable; loadfunc->fnName != NULL; loadfunc++)
		{
			funcPtr = GetProcAddress(hModule, loadfunc->fnName);
			if (!funcPtr) {
				I_ShowLastError(FALSE);
				MessageBoxA(NULL, va("The '%s' haven't the good specification (function %s missing)\n\n"
				           "You must use dll from the same zip of this exe\n", dllName, loadfunc->fnName),
				           "Error", MB_OK|MB_ICONINFORMATION);
				return FALSE;
			}
			// store function address
			*((LPVOID*)loadfunc->fnPointer) = funcPtr;
		}
	}
	else
	{
		I_ShowLastError(FALSE);
		MessageBoxA(NULL, va("LoadLibrary() FAILED : couldn't load '%s'\r\n", dllName), "Warning", MB_OK|MB_ICONINFORMATION);
	}

	return hModule;
}


// --------------------------------------------------------------------------
// Unload the DLL
// --------------------------------------------------------------------------
static VOID UnloadDLL (HMODULE* pModule)
{
	if (FreeLibrary(*pModule))
		*pModule = NULL;
	else
		I_ShowLastError(TRUE);
}
#endif

// ==========================================================================
// STANDARD 3D DRIVER DLL FOR DOOM LEGACY
// ==========================================================================

// note : the 3D driver loading should be put somewhere else..

#ifdef HWRENDER
static HMODULE hwdModule = NULL;

static loadfunc_t hwdFuncTable[] = {
#ifdef _X86_
	{"_Init@4",              &hwdriver.pfnInit},
	{"_Shutdown@0",          &hwdriver.pfnShutdown},
	{"_GetModeList@8",       &hwdriver.pfnGetModeList},
	{"_SetPalette@8",        &hwdriver.pfnSetPalette},
	{"_FinishUpdate@4",      &hwdriver.pfnFinishUpdate},
	{"_Draw2DLine@12",       &hwdriver.pfnDraw2DLine},
	{"_DrawPolygon@16",      &hwdriver.pfnDrawPolygon},
	{"_SetBlend@4",          &hwdriver.pfnSetBlend},
	{"_ClearBuffer@12",      &hwdriver.pfnClearBuffer},
	{"_SetTexture@4",        &hwdriver.pfnSetTexture},
	{"_ReadRect@24",         &hwdriver.pfnReadRect},
	{"_GClipRect@20",        &hwdriver.pfnGClipRect},
	{"_ClearMipMapCache@0",  &hwdriver.pfnClearMipMapCache},
	{"_SetSpecialState@8",   &hwdriver.pfnSetSpecialState},
	{"_DrawMD2@16",          &hwdriver.pfnDrawMD2},
	{"_DrawMD2i@36",         &hwdriver.pfnDrawMD2i},
	{"_SetTransform@4",      &hwdriver.pfnSetTransform},
	{"_GetTextureUsed@0",    &hwdriver.pfnGetTextureUsed},
	{"_GetRenderVersion@0",  &hwdriver.pfnGetRenderVersion},
#ifdef SHUFFLE
	{"_PostImgRedraw@4",     &hwdriver.pfnPostImgRedraw},
	{"_StartScreenWipe@0",   &hwdriver.pfnStartScreenWipe},
	{"_EndScreenWipe@0",     &hwdriver.pfnEndScreenWipe},
	{"_DoScreenWipe@4",      &hwdriver.pfnDoScreenWipe},
	{"_DrawIntermissionBG@0",&hwdriver.pfnDrawIntermissionBG},
	{"_MakeScreenTexture@0", &hwdriver.pfnMakeScreenTexture},
#endif
#else
	{"Init",                &hwdriver.pfnInit},
	{"Shutdown",            &hwdriver.pfnShutdown},
	{"GetModeList",         &hwdriver.pfnGetModeList},
	{"SetPalette",          &hwdriver.pfnSetPalette},
	{"FinishUpdate",        &hwdriver.pfnFinishUpdate},
	{"Draw2DLine",          &hwdriver.pfnDraw2DLine},
	{"DrawPolygon",         &hwdriver.pfnDrawPolygon},
	{"SetBlend",            &hwdriver.pfnSetBlend},
	{"ClearBuffer",         &hwdriver.pfnClearBuffer},
	{"SetTexture",          &hwdriver.pfnSetTexture},
	{"ReadRect",            &hwdriver.pfnReadRect},
	{"GClipRect",           &hwdriver.pfnGClipRect},
	{"ClearMipMapCache",    &hwdriver.pfnClearMipMapCache},
	{"SetSpecialState",     &hwdriver.pfnSetSpecialState},
	{"DrawMD2",             &hwdriver.pfnDrawMD2},
	{"DrawMD2i",            &hwdriver.pfnDrawMD2i},
	{"SetTransform",        &hwdriver.pfnSetTransform},
	{"GetTextureUsed",      &hwdriver.pfnGetTextureUsed},
	{"GetRenderVersion",    &hwdriver.pfnGetRenderVersion},
#ifdef SHUFFLE
	{"PostImgRedraw",       &hwdriver.pfnPostImgRedraw},
	{"StartScreenWipe",     &hwdriver.pfnStartScreenWipe},
	{"EndScreenWipe",       &hwdriver.pfnEndScreenWipe},
	{"DoScreenWipe",        &hwdriver.pfnDoScreenWipe},
	{"DrawIntermissionBG",  &hwdriver.pfnDrawIntermissionBG},
	{"MakeScreenTexture",   &hwdriver.pfnMakeScreenTexture},
#endif
#endif
	{NULL,NULL}
};

BOOL Init3DDriver (LPCSTR dllName)
{
	hwdModule = LoadDLL(dllName, hwdFuncTable);
	return (hwdModule != NULL);
}

VOID Shutdown3DDriver (VOID)
{
	UnloadDLL(&hwdModule);
}
#endif

#ifdef HW3SOUND
static HMODULE hwsModule = NULL;

static loadfunc_t hwsFuncTable[] = {
#ifdef _X86_
	{"_Startup@8",              &hw3ds_driver.pfnStartup},
	{"_Shutdown@0",             &hw3ds_driver.pfnShutdown},
	{"_AddSfx@4",               &hw3ds_driver.pfnAddSfx},
	{"_AddSource@8",            &hw3ds_driver.pfnAddSource},
	{"_StartSource@4",          &hw3ds_driver.pfnStartSource},
	{"_StopSource@4",           &hw3ds_driver.pfnStopSource},
	{"_GetHW3DSVersion@0",      &hw3ds_driver.pfnGetHW3DSVersion},
	{"_BeginFrameUpdate@0",     &hw3ds_driver.pfnBeginFrameUpdate},
	{"_EndFrameUpdate@0",       &hw3ds_driver.pfnEndFrameUpdate},
	{"_IsPlaying@4",            &hw3ds_driver.pfnIsPlaying},
	{"_UpdateListener@8",       &hw3ds_driver.pfnUpdateListener},
	{"_UpdateSourceParms@12",   &hw3ds_driver.pfnUpdateSourceParms},
	{"_SetCone@8",              &hw3ds_driver.pfnSetCone},
	{"_SetGlobalSfxVolume@4",   &hw3ds_driver.pfnSetGlobalSfxVolume},
	{"_Update3DSource@8",       &hw3ds_driver.pfnUpdate3DSource},
	{"_ReloadSource@8",         &hw3ds_driver.pfnReloadSource},
	{"_KillSource@4",           &hw3ds_driver.pfnKillSource},
	{"_KillSfx@4",              &hw3ds_driver.pfnKillSfx},
	{"_GetHW3DSTitle@8",        &hw3ds_driver.pfnGetHW3DSTitle},
#else
	{"Startup",                &hw3ds_driver.pfnStartup},
	{"Shutdown",               &hw3ds_driver.pfnShutdown},
	{"AddSfx",                 &hw3ds_driver.pfnAddSfx},
	{"AddSource",              &hw3ds_driver.pfnAddSource},
	{"StartSource",            &hw3ds_driver.pfnStartSource},
	{"StopSource",             &hw3ds_driver.pfnStopSource},
	{"GetHW3DSVersion",        &hw3ds_driver.pfnGetHW3DSVersion},
	{"BeginFrameUpdate",       &hw3ds_driver.pfnBeginFrameUpdate},
	{"EndFrameUpdate",         &hw3ds_driver.pfnEndFrameUpdate},
	{"IsPlaying",              &hw3ds_driver.pfnIsPlaying},
	{"UpdateListener",         &hw3ds_driver.pfnUpdateListener},
	{"UpdateSourceParms",      &hw3ds_driver.pfnUpdateSourceParms},
	{"SetCone",                &hw3ds_driver.pfnSetCone},
	{"SetGlobalSfxVolume",     &hw3ds_driver.pfnSetGlobalSfxVolume},
	{"Update3DSource",         &hw3ds_driver.pfnUpdate3DSource},
	{"ReloadSource",           &hw3ds_driver.pfnReloadSource},
	{"KillSource",             &hw3ds_driver.pfnKillSource},
	{"KillSfx",                &hw3ds_driver.pfnKillSfx},
	{"GetHW3DSTitle",          &hw3ds_driver.pfnGetHW3DSTitle},
#endif
	{NULL, NULL}
};

BOOL Init3DSDriver(LPCSTR dllName)
{
	hwsModule = LoadDLL(dllName, hwsFuncTable);
	return (hwsModule != NULL);
}

VOID Shutdown3DSDriver (VOID)
{
	UnloadDLL(&hwsModule);
}
#endif
#endif //_WINDOWS
