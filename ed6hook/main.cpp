
#include <Windows.h>
#include "ed6fc_info.h"

#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>
#include <ftimage.h>
#include <ftbitmap.h>
#include <ftsynth.h>
#define FT_INT(_int, _float) ((_int << 6) | (_float))
FT_Library  FTLibrary;
FT_Face     Face;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

ED6HOOK_INFO*           ED6HookInfo = NULL;

enum // ECodePage
{
    CP_SHIFTJIS = 932,
    CP_GBK      = 936,
    CP_GB2312   = CP_GBK,
    CP_BIG5     = 950,
    CP_UTF16_LE = 1200,
    CP_UTF16_BE = 1201,
};


typedef struct
{
    CHAR  FileName[0xC];
    ULONG Unknown;
    ULONG Size;
    ULONG Unknown2[3];
    ULONG Offset;

} ED6_DIR_ENTRY, *PED6_DIR_ENTRY;


static BYTE FontSizeTable[] =
{
    0x08, 0x0c, 0x10, 0x14,
    0x18, 0x20, 0x12, 0x1a,
    0x1e, 0x24, 0x28, 0x2c,
    0x30, 0x32, 0x36, 0x3c,
    0x40, 0x48, 0x50, 0x60,
    0x80, 0x90, 0xa0, 0xc0,
};

static USHORT FontColorTable[] =
{
    0x0fff, 0x0fc7, 0x0f52, 0x08cf, 0x0fb4, 0x08fa, 0x0888, 0x0fee, 0x0853, 0x0333,
    0x0ca8, 0x0fdb, 0x0ace, 0x0cff, 0x056b, 0x0632, 0x0135, 0x0357, 0x0bbb,
};

static BYTE FontLumaTable[] =
{
    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0E, 0x0E,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
};



BOOL GetGlyphBitmap(LONG_PTR FontSize, WCHAR Chr, PVOID& Buffer, ULONG ColorIndex, ULONG Stride)
{
    PBYTE           Outline, Source;
    ULONG_PTR       Color;
    FT_Glyph        glyph;
    FT_BitmapGlyph  bitmap;

    ULONG strenth = FT_INT(1, 0);

    Color = FontColorTable[ColorIndex];

    FT_Load_Glyph(Face, FT_Get_Char_Index(Face, Chr), FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_RENDER);
    FT_Bitmap_Embolden(FTLibrary, &Face->glyph->bitmap, 6, 0);
    FT_Render_Glyph(Face->glyph, FT_RENDER_MODE_NORMAL);
    FT_Get_Glyph(Face->glyph, &glyph);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, TRUE);

    bitmap = (FT_BitmapGlyph)glyph;
    Source = (PBYTE)bitmap->bitmap.buffer;

    if (Source != nullptr)
    {
        PBYTE LocalOutline = (PBYTE) malloc((FontSize * FontSize) * 2);
        memset(LocalOutline, 0, (FontSize * FontSize) * 2);

        Outline = LocalOutline + bitmap->left + (FontSize - MIN(FontSize, bitmap->top + 8)) * FontSize;

        for (ULONG_PTR Height = bitmap->bitmap.rows; Height; --Height)
        {
            PBYTE out = Outline;

            for (ULONG_PTR Width = bitmap->bitmap.width; Width; --Width)
            {
                *out++ = FontLumaTable[*Source++];
            }

            Outline += FontSize;

            if (out > Outline)
                break;
        }

        PBYTE Surface = (PBYTE)Buffer;

        Source = LocalOutline;

        for (ULONG_PTR Height = FontSize; Height; --Height)
        {
            PUSHORT out = (PUSHORT)Surface;

            for (ULONG_PTR Width = FontSize; Width; --Width)
            {
                if (*Source != 0)
                    *out++ = ((*Source << 0xC) | Color);
                else
                    out++;
                ++Source;
            }

            Surface += Stride;
            if ((PBYTE)out > Surface)
                break;
        }

        free(LocalOutline);

        Buffer += (Face->glyph->advance.x >> 6) * sizeof(USHORT);
    }
    else
    {
        Buffer += (Face->glyph->advance.x >> 6) * sizeof(USHORT);
    }

    FT_Done_Glyph(glyph);

    return TRUE;
}


PVOID NTAPI GetGlyphsBitmap(PCSTR Text, PVOID Buffer, ULONG Stride, ULONG ColorIndex)
{
	if (!ED6HookInfo->ExtraInfo->GameFontRender)
		return NULL;
    ULONG_PTR       fontSize, fontIndex, color, width, runeWidth;
    ULONG_PTR Encoding = CP_SHIFTJIS;

    fontIndex   = ED6HookInfo->ExtraInfo->GameFontRender->FontSizeIndex;
    fontSize    = FontSizeTable[fontIndex];
    color       = FontColorTable[ColorIndex];

    FT_Size_RequestRec request;
    request.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
    request.width = (fontSize * 64) * 15 / 20;
    request.height = fontSize * 64;
    request.horiResolution = 0;
    request.vertResolution = 0;

    FT_Request_Size(Face, &request);
    // FT_Set_Pixel_Sizes(Face, (fontSize * 15 / 20) - 2, fontSize - 2);

    int nLen = MultiByteToWideChar(Encoding, 0, Text, -1, NULL, NULL);
    LPWSTR wText = (LPWSTR) malloc(nLen * sizeof(WCHAR));
    MultiByteToWideChar(Encoding, 0, Text, -1, wText, nLen);

    for (WCHAR* chr = wText; *chr; ++chr)
    {
        if (!GetGlyphBitmap(fontSize, *chr, Buffer, ColorIndex, Stride))
        {
            Buffer += fontSize * 2 * 4;
        }
    }

    free(wText);

    return Buffer;
}

static const PED6_DIR_ENTRY *DirCacheTable = (PED6_DIR_ENTRY*)0x5E1968;

BOOL CDECL LoadFileFromDat(PVOID buffer, ULONG datIndex, ULONG datOffset, ULONG fileSize)
{
    PED6_DIR_ENTRY entry;

    entry = DirCacheTable[datIndex];
    if (entry == nullptr)
        return FALSE;

    while (true)
    {
        if (entry->Offset == datOffset && entry->Size == fileSize)
            break;

        ++entry;
    }

    FILE* dat;
    char file_name[13];
    memcpy(file_name, entry->FileName, 12);
    file_name[12] = 0;
    char nice_path[256];
    snprintf(nice_path, 256, "DAT\\ED6_DT%02X\\%s", datIndex, file_name);
    long total_sz = 0;
    dat = fopen(nice_path, "rb");
    if (dat) {
    	fseek(dat, 0, SEEK_END);
        total_sz = ftell(dat);
        fseek(dat, 0, SEEK_SET);
    }

    // GetModuleDirectory(path, nullptr);

    if (dat)
    {
        *(PULONG)(buffer + 0) = fileSize;
        *(PULONG)(buffer + 4) = 0x43464445;
        *(PULONG)(buffer + 8) = total_sz;

	    size_t read_countX = fread(buffer + 12, fileSize, 1, dat);

	    fclose(dat);

        return read_countX == total_sz;
    }

    snprintf(nice_path, 256, "ED6_DT%02X.dat", datIndex);

    dat = fopen(nice_path, "rb");

    if (!dat)
        return FALSE;

    fseek(dat, datOffset, SEEK_SET);

    size_t read_count = fread(buffer, fileSize, 1, dat);

    fclose(dat);

    return read_count == fileSize;
}


ULONG_PTR NTAPI DecompressData(PBYTE& compressed, PBYTE& uncompressed)
{
    if (*(PULONG)&compressed[4] != 0x43464445)
        return ~0u;

    ULONG size = *(PULONG)(compressed + 8);
    memcpy(uncompressed, compressed + 12, size);

    compressed += size + 12;
    uncompressed += size;

    return size;
}

extern "C" __declspec(dllexport) ED6HOOK_INFO* ed6_hook_get_info(void) {
	return ED6HookInfo;
}



BOOL Initialize(PVOID /*BaseAddress*/) {

	BOOL Success = FALSE;
	void* FaceBuffer = nullptr;

    for (BOOL b = TRUE; b; b = !b)
    {
        if (FT_Init_FreeType(&FTLibrary) != FT_Err_Ok)
            break;

        FILE * fontfile;
        fontfile = fopen("user.ttf", "rb");
        if (!fontfile)
        	break;

        fseek(fontfile, 0, SEEK_END);

        long fontfile_sz = ftell(fontfile);

        FaceBuffer = malloc(fontfile_sz);
        if (FaceBuffer == nullptr)
            break;

        fseek(fontfile, 0, SEEK_SET);

        fread(FaceBuffer, fontfile_sz, 1, fontfile);

        fclose(fontfile);

        if (FT_New_Memory_Face(FTLibrary, (PBYTE)FaceBuffer, fontfile_sz, 0, &Face) != FT_Err_Ok)
            break;

        FT_Select_Charmap(Face, FT_ENCODING_SJIS);

        Success = TRUE;
    }

    if (Success == FALSE)
    {
    	if (FaceBuffer)
    		free(FaceBuffer);
    }

    
    ED6HookInfo = (ED6HOOK_INFO*) malloc(sizeof(ED6HOOK_INFO));
    ED6HookInfo->ExeInfoNum = 1;
    ED6HookInfo->ExeInfo = (ED6HOOK_EXE_INFO*) malloc(sizeof(ED6HOOK_EXE_INFO));
	ED6HookInfo->ExeInfo->PeTimeDateStamp         = 0x59A37AD3;
	ED6HookInfo->ExeInfo->GetGlyphsBitmapVa       =   0x4b8060;
	ED6HookInfo->ExeInfo->DrawTalkTextVa          =   0x4868a0;
	ED6HookInfo->ExeInfo->DrawDialogTextVa        =   0x4868f0;
	ED6HookInfo->ExeInfo->LoadFileFromDatVa       =   0x4629e0;
	ED6HookInfo->ExeInfo->DecompressDataVa        =   0x46a6b0;
	ED6HookInfo->ExeInfo->WindowPosition1Addr     =   0x499280;
	ED6HookInfo->ExeInfo->WindowPosition2Addr     =   0x49C790;
	ED6HookInfo->ExeInfo->CombatStateAddr         =   0x43AF10;
	ED6HookInfo->ExeInfo->JpFontSizeLimitAddr     =   0x4DD9B0;
	ED6HookInfo->ExeInfo->HpEpFontSizeAddr        =   0x4773A0;
	ED6HookInfo->ExeInfo->PlaceNameTextXDeltaAddr =   0x4B7ED0;

	ED6HookInfo->HookInfo = (ED6HOOK_HOOK_INFO*) malloc(sizeof(ED6HOOK_HOOK_INFO));
	ED6HookInfo->HookInfo->GetGlyphsBitmap = GetGlyphsBitmap;
	ED6HookInfo->HookInfo->DecompressData = DecompressData;
	ED6HookInfo->HookInfo->LoadFileFromDat = LoadFileFromDat;

	ED6HookInfo->ExtraInfo = (ED6HOOK_EXTRA_INFO*) malloc(sizeof(ED6HOOK_EXTRA_INFO));
	ED6HookInfo->ExtraInfo->GameFontRender = NULL;

	return TRUE;
}

BOOL Uninitialize(PVOID /*BaseAddress*/) {
	return TRUE;
}

BOOL WINAPI DllMain(PVOID BaseAddress, ULONG Reason, PVOID /*Reserved*/)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		return Initialize(BaseAddress);

	case DLL_PROCESS_DETACH:
		return Uninitialize(BaseAddress);
	}

	return TRUE;
}
