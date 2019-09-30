
#pragma once

typedef struct {
	DWORD PeTimeDateStamp;
	DWORD GetGlyphsBitmapVa;
	DWORD DrawTalkTextVa;
	DWORD DrawDialogTextVa;
	DWORD LoadFileFromDatVa;
	DWORD DecompressDataVa;
	DWORD WindowPosition1Addr;
	DWORD WindowPosition2Addr;
	DWORD CombatStateAddr;
	DWORD JpFontSizeLimitAddr;
	DWORD HpEpFontSizeAddr;
	DWORD PlaceNameTextXDeltaAddr;
} ED6HOOK_EXE_INFO;

typedef struct {
	PVOID GetGlyphsBitmap;
	PVOID DecompressData;
	PVOID LoadFileFromDat;
} ED6HOOK_HOOK_INFO;

typedef struct {
	DWORD ExeInfoNum;
	ED6HOOK_EXE_INFO * ExeInfo;
	ED6HOOK_HOOK_INFO * HookInfo;
} ED6HOOK_INFO;
