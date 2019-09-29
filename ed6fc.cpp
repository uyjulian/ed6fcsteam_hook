// this file must be compiled under zh-CN locale

#pragma comment(linker, "/ENTRY:DllMain")
//#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text /MERGE:.text1=.text /SECTION:.idata,ERW")
//#pragma comment(linker, "/SECTION:.Asuna,ERW /MERGE:.text=.Asuna")

#include "ed6fc.h"
#include "ml.cpp"
#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>
#include <ftimage.h>
#include <ftbitmap.h>
#include <ftsynth.h>
#define FT_INT(_int, _float) ((_int << 6) | (_float))
FT_Library  FTLibrary;
FT_Face     Face;
#include <stdio.h>
#include <functional>

ML_OVERLOAD_NEW

BOOL SleepFix;
PED6_FC_FONT_RENDER GameFontRender;
API_POINTER(CreateFileA) StubCreateFileA;

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

inline void SearchAllPatterns(const ml::String& Pattern, PVOID Begin, LONG_PTR Length, std::function<void(const ml::GrowableArray<PVOID>& references)> callback)
{
    GrowableArray<SEARCH_PATTERN_DATA>  Patterns;
    GrowableArray<PVOID> references;
    GrowableArray<ml::String::ByteArray *>  BytesArray;
    ml::String::ByteArray*                  CurrentBytes;
    SEARCH_PATTERN_DATA                 CurrentPattern;
    ULONG_PTR                           GapBytes;

    SCOPE_EXIT
    {
        for (auto &p : BytesArray)
            delete p;
    }
    SCOPE_EXIT_END;

    CurrentBytes = nullptr;
    GapBytes = 0;
    ZeroMemory(&CurrentPattern, sizeof(CurrentPattern));

    for (ml::String &p : Pattern.Split(' '))
    {
        if (!p)
            continue;

        if (p.GetCount() != 2)
            return;

        if (p[0] == '?' && p[1] == '?')
        {
            ++GapBytes;
        }
        else
        {
            ULONG Hex;

            if (GapBytes != 0)
            {
                CurrentPattern.Pattern = CurrentBytes->GetData();
                CurrentPattern.Size = CurrentBytes->GetSize();
                CurrentPattern.HeadOffsetToNext = CurrentPattern.Size + GapBytes;
                Patterns.Add(CurrentPattern);

                ZeroMemory(&CurrentPattern, sizeof(CurrentPattern));

                GapBytes = 0;
                BytesArray.Add(CurrentBytes);
                CurrentBytes = nullptr;
            }

            if (CurrentBytes == nullptr)
                CurrentBytes = new ml::String::ByteArray;

            Hex = p.ToHex();
            CurrentBytes->Add(Hex);
        }
    }

    if (CurrentBytes != nullptr)
    {
        BytesArray.Add(CurrentBytes);

        CurrentPattern.Pattern = CurrentBytes->GetData();
        CurrentPattern.Size = CurrentBytes->GetSize();
        Patterns.Add(CurrentPattern);
    }

    for (PVOID reference = Begin; (ULONG_PTR)reference < ((ULONG_PTR)Begin + Length);)
    {
        reference = SearchPattern(Patterns.GetData(), Patterns.GetSize(), reference, ((ULONG_PTR)Begin - (ULONG_PTR)reference) + Length);
        if (!reference)
            break;
        references.Add(reference);
        reference = (PVOID)((ULONG_PTR)reference + CurrentPattern.Size);
    }

    callback(references);

}

VOID (NTAPI *StubGetGlyphsBitmap)(PCSTR Text, PVOID Buffer, ULONG Stride, ULONG ColorIndex);

NTSTATUS GetGlyphBitmap(LONG_PTR FontSize, WCHAR Chr, PVOID& Buffer, ULONG ColorIndex, ULONG Stride)
{
    PBYTE           Outline, Source;
    ULONG_PTR       Color;
    FT_Glyph        glyph;
    FT_BitmapGlyph  bitmap;

    ULONG strenth = FT_INT(1, 0);

    Color = FontColorTable[ColorIndex];

    FT_Load_Glyph(Face, FT_Get_Char_Index(Face, Chr), FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_RENDER);
    //FT_Bitmap_Embolden(FTLibrary, &Face->glyph->bitmap, 0, strenth);
    FT_Render_Glyph(Face->glyph, FT_RENDER_MODE_NORMAL);
    FT_Get_Glyph(Face->glyph, &glyph);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, TRUE);

    bitmap = (FT_BitmapGlyph)glyph;
    Source = (PBYTE)bitmap->bitmap.buffer;

    if (Source != nullptr)
    {
        PBYTE LocalOutline = new BYTE[(FontSize * FontSize) * 2];
        ZeroMemory(LocalOutline, (FontSize * FontSize) * 2);

        Outline = LocalOutline + bitmap->left + (FontSize - ML_MIN(FontSize, bitmap->top + 3)) * FontSize;

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

        delete LocalOutline;

        Buffer = PtrAdd(Buffer, (Face->glyph->advance.x >> 6) * sizeof(USHORT));//(bitmap->left + bitmap->bitmap.pitch + bitmap->left) * sizeof(USHORT));
        // Buffer = PtrAdd(Buffer, (Chr >= 0x80 ? FontSize : FontSize / 2) * sizeof(USHORT));
    }
    else
    {
        Buffer = PtrAdd(Buffer, (Face->glyph->advance.x >> 6) * sizeof(USHORT));
        // Buffer = PtrAdd(Buffer, Chr == ' ' ? FontSize : FontSize * 2);
    }

    FT_Done_Glyph(glyph);

    return STATUS_SUCCESS;

    //return Source != nullptr ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

BOOL TranslateChar(PCSTR Text, USHORT& translated)
{
    USHORT ch;

    ch = *(PUSHORT)Text;

#if 1

    switch (ch)
    {
        default:
            return FALSE;

        case 0xA181:
        case 0xF6A1:
            translated = L'■';
            break;

        case 0x4881:
            translated = L'？';
            break;

        case 0x9F81:
            translated = L'◆';
            break;

        case 0xAA84:
            translated = L'━';
            break;

        case 0x4081:
        case 0xA1A1:
            translated = L'　';
            break;

        case 0x9A81:
            translated = L'★';
            break;

        case 0x4C87:
            translated = L'⑬';
            break;

        case 0x4D87:
            translated = L'⑭';
            break;

        case 0x5C81:    // 手册
            // translated = 0x9F84;
            translated = L'―';
            break;

        case 0x5AA9:
            translated = L'♥';
            break;

        case 0xD1A1:
            translated = L'♪';
            break;

        case 0xADA1:
            translated = L'…';
            break;

        case 0xA4A1:
            translated = L'・';
            break;
    }

#else

    switch (ch)
    {
        default:
            return FALSE;

        case 0xA181:    // 仭
        case 0x9F81:    // 仧   菱形
        case 0xAA84:    // 劒   横杠
        case 0x4081:    // 丂   空格
        case 0x9A81:    // 仛   ★
        case 0x4C87:    // 圆圈13
        case 0x4D87:    // 圆圈14
            translated = ch;
            break;

        case 0xA1A1:    // 全角空格
            translated = 0x4081;
            break;

        case 0x5C81:    // 乗   横杠
            translated = 0x9F84;
            break;

        case 0x5AA9:    // ㈱ 心形
            translated = 0x8A87;    // TAG2('噴');
            break;

        case 0xD1A1:    // ⊙ 音符
            translated = 0xF481;  // TAG2('侓');
            break;

        case 0xF6A1:    // ■ 方块
            translated = 0xA181;
            break;

        case 0xADA1:    // … 中文省略号
            translated = 0x6381;
            break;

        case 0xA4A1:    // 中点
            translated = 0x4581;
            break;
    }

#endif

    return TRUE;
}

PVOID NTAPI GetGlyphsBitmap(PCSTR Text, PVOID Buffer, ULONG Stride, ULONG ColorIndex)
{
    ULONG_PTR       fontSize, fontIndex, color, width, runeWidth;
    ULONG_PTR Encoding = CP_SHIFTJIS;

    fontIndex   = GameFontRender->FontSizeIndex;
    fontSize    = FontSizeTable[fontIndex];
    color       = FontColorTable[ColorIndex];

    FT_Set_Pixel_Sizes(Face, fontSize, fontSize);

    int nLen = MultiByteToWideChar(Encoding, 0, Text, -1, NULL, NULL);
    LPWSTR wText = new WCHAR[nLen];
    MultiByteToWideChar(Encoding, 0, Text, -1, wText, nLen);

    for (WCHAR* chr = wText; *chr; ++chr)
    {
#if 0
        USHORT translated;
        CHAR ansi = Text[0];

        if (ansi == ' ')
        {
            width = fontSize / 2;
            ++Text;
        }
        else if (ansi > 0)
        {
            ansiRender->DrawRune(chr, color, Buffer, Stride, &runeWidth);
            width = fontSize / 2;
            ++Text;
        }
        else if (TranslateChar(Text, translated))
        {
            CHAR tmp[3] = { translated & 0xFF, translated >> 8 };
            //StubGetGlyphsBitmap(tmp, Buffer, Stride, ColorIndex);

            sjisRender->DrawRune(translated, color, Buffer, Stride, &runeWidth);
            width = fontSize;
            Text += 2;
        }
        else
        {
            mbcsRender->DrawRune(chr, color, Buffer, Stride, &runeWidth);
            width = fontSize;
            Text += 2;
        }

        Buffer = PtrAdd(Buffer, (LONG_PTR)width * 2);
#else
        if (NT_FAILED(GetGlyphBitmap(fontSize, *chr, Buffer, ColorIndex, Stride)))
        {
#if 0
            WCHAR wcs[] = { *chr, 0 };
            LPSTR nText = new CHAR[2];
            nText[1] = 0;
            WideCharToMultiByte(Encoding, 0, wcs, 2, nText, 2, NULL, NULL);
            StubGetGlyphsBitmap(nText, Buffer, Stride, ColorIndex);
            delete nText;
#endif
            Buffer = PtrAdd(Buffer, (LONG_PTR)fontSize * 2);
        }

#endif
    }

    delete wText;

    return Buffer;
}

PVOID FASTCALL DrawTalkText(PVOID thiz, PVOID, PVOID Buffer, ULONG Stride, PCSTR Text, ULONG ColorIndex)
{
    CHAR tmp[3] = { Text[0], Text[0] < 0 ? Text[1] : 0 };
    return GetGlyphsBitmap(tmp, Buffer, Stride * 2, ColorIndex);
}

NAKED PVOID NakedDrawDialogText(PVOID thiz, PVOID, PVOID Buffer, ULONG Stride, PCSTR Text, ULONG ColorIndex)
{
    INLINE_ASM
    {
        movzx   ebx, bl;
        push    ebx;
        lea     ebx, [esp];
        push    edx;                    // colorIndex
        mov     edx, [esp+10h];
        lea     edx, [edx*2];
        push    edx;                    // stride
        push    eax;                    // buffer
        push    ebx;                    // text
        call    GetGlyphsBitmap;
        pop     ebx;
        ret     8;
    }
}

/************************************************************************
  load file
************************************************************************/

BOOL CDECL LoadFileFromDat(PVOID buffer, ULONG datIndex, ULONG datOffset, ULONG fileSize)
{
    PED6_DIR_ENTRY entry;

    entry = DirCacheTable[datIndex];
    if (entry == nullptr)
        return FALSE;

    LOOP_FOREVER
    {
        if (entry->Offset == datOffset && entry->Size == fileSize)
            break;

        ++entry;
    }

    String path;
    NtFileDisk dat;

    GetModuleDirectory(path, nullptr);

    if (NT_SUCCESS(dat.Open(path + String::Format(L"DAT\\ED6_DT%02X\\%.*S", datIndex, sizeof(entry->FileName), entry->FileName))))
    {
        *(PULONG)PtrAdd(buffer, 0) = fileSize;
        *(PULONG)PtrAdd(buffer, 4) = RAW_FILE_MAGIC;
        *(PULONG)PtrAdd(buffer, 8) = dat.GetSize32();
        return NT_SUCCESS(dat.Read(PtrAdd(buffer, 12)));
    }

    if (NT_FAILED(dat.Open(path + String::Format(L"ED6_DT%02X.dat", datIndex))))
        return FALSE;

    dat.Seek(datOffset);
    return NT_SUCCESS(dat.Read(buffer, fileSize));
}

ULONG_PTR NTAPI DecompressData(PBYTE& compressed, PBYTE& uncompressed)
{
    if (*(PULONG)&compressed[4] != RAW_FILE_MAGIC)
        return ~0u;

    ULONG size = *(PULONG)(compressed + 8);
    CopyMemory(uncompressed, compressed + 12, size);

    compressed += size + 12;
    uncompressed += size;

    return size;
}

NAKED VOID CDECL NakedLoadFileFromDat()
{
    INLINE_ASM
    {
        push    [esp + 0Ch];
        push    [esp + 0Ch];
        push    [esp + 0Ch];
        push    edi;
        call    LoadFileFromDat;
        add     esp, 10h;
        ret;
    }
}

PVOID StubNakedDecompressData;

NAKED VOID NakedDecompressData()
{
    INLINE_ASM
    {
        push    ebx;
        push    edi;
        call    DecompressData;
        inc     eax;
        jnz     UNCOMPRESSED;
        jmp     [StubNakedDecompressData];

UNCOMPRESSED:
        dec     eax;
        ret;
    }
}

/************************************************************************
  cpu usage
************************************************************************/

WNDPROC OrigGameWindowProc;

LRESULT NTAPI GameWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ACTIVATEAPP:
            SleepFix = wParam == FALSE;
            break;
    }

    return OrigGameWindowProc(hwnd, message, wParam, lParam);
}

/************************************************************************
  init
************************************************************************/

PED6_FC_FONT_RENDER FindFontRender(PVOID BaseAddress)
{
    PVOID p;

     p = SearchPatternSafe(L"81 3D ?? ?? ?? ?? BC 02 00 00", BaseAddress, ImageNtHeaders(BaseAddress)->OptionalHeader.SizeOfImage);
     if (p == nullptr)
         return nullptr;

     return FIELD_BASE(*(PVOID *)PtrAdd(p, 2), ED6_FC_FONT_RENDER, FontWeight);
}

template<typename... ARGS>
PVOID FindAndAdvance(ULONG_PTR Advance, ARGS... args)
{
    PVOID p = SearchPatternSafe(args...);
    return p == nullptr ? IMAGE_INVALID_VA : PtrAdd(p, Advance);
}

BOOL UnInitialize(PVOID BaseAddress)
{
    return FALSE;
}

typedef struct
{
    PVOID       GetGlyphsBitmap;
    PVOID       DrawTalkText;
    PVOID       DrawDialogText;
    PVOID       LoadFileFromDAT;
    PVOID       DecompressData;

} ED6_FC_HOOK_FUNCTIONS, *PED6_FC_HOOK_FUNCTIONS;

NTSTATUS SearchFunctions(PED6_FC_HOOK_FUNCTIONS functions)
{
    PVOID       Address;
    PLDR_MODULE Exe;

    Exe = Ldr::FindLdrModuleByHandle(nullptr);

    FillMemory(functions, sizeof(*functions), 0xFF);

    struct
    {
        PVOID*      Address;
        ULONG_PTR   Type;
        ULONG_PTR   SearchLength;
        PCSTR       Pattern;
    } hooks[] =
    {
        { &functions->GetGlyphsBitmap,  0, 0x10, "8B 55 14 83 EC 34 83 FA 13" },
        { &functions->DrawTalkText,     0, 0x20, "8B E9 0F B6 08 C1 E1 08 0B CA 81 F9 40 81 00 00" },
        { &functions->DrawDialogText,   0, 0x20, "8B F8 80 FB 20" },
        { &functions->LoadFileFromDAT,  2, 0x30, (PCSTR)L"ED6_DT%02x.DAT" },
        { &functions->DecompressData,   0, 0x05, "83 EC 18 8B 03 89 44 24 04 8B 07" },
    };

    for (ULONG_PTR i = 0; i != countof(hooks); i++)
    {
        switch (hooks[i].Type)
        {
            case 0:
                Address = SearchPatternSafe(hooks[i].Pattern, Exe->DllBase, Exe->SizeOfImage);
                if (Address != nullptr)
                    *hooks[i].Address = ReverseSearchFunctionHeader(Address, hooks[i].SearchLength);

                break;

            case 2:
                Address = SearchStringReference(Exe, (PVOID)hooks[i].Pattern, StrLengthW((PCWSTR)hooks[i].Pattern) + 2);
                if (Address != nullptr)
                    *hooks[i].Address = ReverseSearchFunctionHeader(PtrSub(Address, 1), hooks[i].SearchLength);

                break;
        }
    }

    return STATUS_SUCCESS;
}

#ifdef __clang__
// Some fixes

HANDLE NTAPI CreateFileAFix(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    if (StrCompareA(lpFileName, "dll\\lang_jpn.dll") == 0)
    {
        lpFileName = "ed6_win.exe";
    }

    return StubCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

VOID NTAPI SleepFixFunc(ULONG ms)
{
    Ps::Sleep(SleepFix ? ms == 0 ? 1 : ms : ms);
}

BOOL NTAPI SetWindowPosFix(HWND Wnd, HWND InsertAfter, int X, int Y, int cx, int cy, UINT Flags)
{
    if (Flags == SWP_NOMOVE)
    {
        RECT WorkArea;

        SystemParametersInfoW(SPI_GETWORKAREA, 0, &WorkArea, 0);
        X = ((WorkArea.right - WorkArea.left) - cx) / 2;
        Y = ((WorkArea.bottom - WorkArea.top) - cy) / 2;

        CLEAR_FLAG(Flags, SWP_NOMOVE);

        OrigGameWindowProc = (WNDPROC)SetWindowLongPtrW(Wnd, GWLP_WNDPROC, (LONG_PTR)GameWindowProc);
    }

    return SetWindowPos(Wnd, InsertAfter, X, Y, cx, cy, Flags);
}
#endif

BOOL Initialize(PVOID BaseAddress)
{
    using namespace Mp;

    BOOL                    Success;
    ULONG_PTR               SizeOfImage;
    PVOID                   FaceBuffer;
    PLDR_MODULE             ExeModule;
    ED6_FC_HOOK_FUNCTIONS   Functions;

    LdrDisableThreadCalloutsForDll(BaseAddress);

    if (ImageNtHeaders(Ps::CurrentPeb()->ImageBaseAddress)->FileHeader.TimeDateStamp != 0x59A37AD3) {
        fprintf(stderr, "Incompatible timedatestamp 0x%x\n", ImageNtHeaders(Ps::CurrentPeb()->ImageBaseAddress)->FileHeader.TimeDateStamp);
        return TRUE;
    }

    ml::MlInitialize();

    BaseAddress = GetExeModuleHandle();

    //
    // FT_Load_Glyph
    // FT_Get_Glyph
    // FT_Render_Glyph
    // FT_Glyph_To_Bitmap
    //

    Rtl::SetExeDirectoryAsCurrent();

    Success = FALSE;
    FaceBuffer = nullptr;

    LOOP_ONCE
    {
        NtFileMemory file;

        if (FT_Init_FreeType(&FTLibrary) != FT_Err_Ok)
            break;

        if (NT_FAILED(file.Open(L"user.ttf")))
            break;

        FaceBuffer = AllocateMemoryP(file.GetSize32());
        if (FaceBuffer == nullptr)
            break;

        CopyMemory(FaceBuffer, file.GetBuffer(), file.GetSize32());

        if (FT_New_Memory_Face(FTLibrary, (PBYTE)FaceBuffer, file.GetSize32(), 0, &Face) != FT_Err_Ok)
            break;

        FT_Select_Charmap(Face, FT_ENCODING_SJIS);

        Success = TRUE;
    }

    if (Success == FALSE)
    {
        FreeMemoryP(FaceBuffer);
        // return TRUE;
    }

    PatchExeText(BaseAddress);

    if (Success)
    {
        GameFontRender = FindFontRender(BaseAddress);
        Success = GameFontRender != nullptr;
    }

    //FAIL_RETURN(SearchFunctions(&Functions));
    Functions.GetGlyphsBitmap = GET_GLYPHS_BITMAP_VA;
    Functions.DrawTalkText = DRAW_TALK_TEXT_VA;
    Functions.DrawDialogText = DRAW_DIALOG_TEXT_VA;
    Functions.LoadFileFromDAT = LOAD_FILE_FROM_DAT_VA;
    Functions.DecompressData = DECOMPRESS_DATA_VA;

    ExeModule = FindLdrModuleByHandle(nullptr);

    // char width
    SearchAllPatterns(
        L"76 ?? ?? 80 72 ?? ?? A0 72 ?? ?? E0 73 ??",
        ExeModule->DllBase,
        ExeModule->SizeOfImage,
        [](const ml::GrowableArray<PVOID>& references)
        {
            PVOID* addr;

            FOR_EACH_VEC(addr, references)
            {
                BYTE data = 0xC;
                Mm::WriteProtectMemory(CurrentProcess, PtrAdd(*addr, 1), &data, 1);
            }
        }
    );

    /*
        004667DA    .  80F9 80             cmp     cl, 0x80
        004667DD    .  72 31               jb      short 0x466810
        004667DF    .  80F9 A0             cmp     cl, 0xA0
        004667E2    .  72 05               jb      short 0x4667E9
        004667E4    .  80F9 E0             cmp     cl, 0xE0
        004667E7    .  72 4B               jb      short 0x466834
    */
    SearchAllPatterns(
        L"80 ?? 80 72 ?? 80 ?? A0 72 ?? 80 ?? E0",
        ExeModule->DllBase,
        ExeModule->SizeOfImage,
        [](const ml::GrowableArray<PVOID>& references)
        {
            PVOID* addr;

            FOR_EACH_VEC(addr, references)
            {
                BYTE data = 0xEB;
                Mm::WriteProtectMemory(CurrentProcess, PtrAdd(*addr, 8), &data, 1);
            }
        }
    );

    /*
        004837F0    .  3C 80               cmp     al, 0x80
        004837F2    .  72 0F               jb      short 0x483803
        004837F4    .  3C A0               cmp     al, 0xA0
        004837F6    .  72 04               jb      short 0x4837FC
        004837F8    .  3C E0               cmp     al, 0xE0
        004837FA    .  72 07               jb      short 0x483803
    */
    SearchAllPatterns(
        L"3C 80 72 ?? 3C A0 72 ?? 3C E0",
        ExeModule->DllBase,
        ExeModule->SizeOfImage,
        [](const ml::GrowableArray<PVOID>& references)
        {
            PVOID* addr;

            FOR_EACH_VEC(addr, references)
            {
                BYTE data = 0xEB;
                Mm::WriteProtectMemory(CurrentProcess, PtrAdd(*addr, 6), &data, 1);
            }
        }
    );
    
    /*
        00476198   |.  3C 80               |cmp     al, 0x80
        0047619A   |.  0F82 50010000       |jb      0x4762F0
        004761A0   |.  3C A0               |cmp     al, 0xA0
        004761A2       72 08               |jb      short 0x4761AC
        004761A4   |.  3C E0               |cmp     al, 0xE0
        004761A6   |.  0F82 44010000       |jb      0x4762F0
    */
    SearchAllPatterns(
        L"3C 80 0F ?? ?? ?? ?? ?? 3C A0 72 ?? 3C E0",
        ExeModule->DllBase,
        ExeModule->SizeOfImage,
        [](const ml::GrowableArray<PVOID>& references)
        {
            PVOID* addr;

            FOR_EACH_VEC(addr, references)
            {
                BYTE data = 0xEB;
                Mm::WriteProtectMemory(CurrentProcess, PtrAdd(*addr, 0xA), &data, 1);
            }
        }
    );

    /*
        00483950   |> /8801                /mov     byte ptr [ecx], al
        00483952   |. |3C 80               |cmp     al, 0x80
        00483954   |. |73 04               |jnb     short 0x48395A
        00483956   |. |33C0                |xor     eax, eax
        00483958   |. |EB 10               |jmp     short 0x48396A
        0048395A   |> |3C A0               |cmp     al, 0xA0
        0048395C   |. |73 07               |jnb     short 0x483965
        0048395E   |. |B8 01000000         |mov     eax, 0x1
        00483963   |. |EB 05               |jmp     short 0x48396A

        copy chr name
    */
    SearchAllPatterns(
        L"3C 80 73 ?? 33 C0 EB ?? 3C A0 73 ?? ?? 01 00 00 00 EB ??",
        ExeModule->DllBase,
        ExeModule->SizeOfImage,
        [](const ml::GrowableArray<PVOID>& references)
        {
            PVOID* addr;

            FOR_EACH_VEC(addr, references)
            {
                BYTE data = 0;
                Mm::WriteProtectMemory(CurrentProcess, PtrAdd(*addr, 0xB), &data, 1);
            }
        }
    );

    static FLOAT DefaultPlaceNameTextDeltaX = 0;

    PATCH_MEMORY_DATA p[] =
    {
#if 0
#ifdef __clang__
#if 0
        MemoryPatchVa(CreateFileAFix, sizeof(PVOID), LookupImportTable(BaseAddress, nullptr, KERNEL32_CreateFileA)),
#endif

        MemoryPatchVa((ULONG64)SleepFixFunc, sizeof(PVOID), LookupImportTable(BaseAddress, nullptr, KERNEL32_Sleep)),

        MemoryPatchVa((ULONG64)SetWindowPosFix, sizeof(PVOID), LookupImportTable(GetExeModuleHandle(), nullptr, USER32_SetWindowPos)),
#else
#if 0
        MemoryPatchVa(
            (ULONG64)(API_POINTER(::CreateFileA))[] (LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) -> HANDLE
            {
                if (StrCompareA(lpFileName, "dll\\lang_jpn.dll") == 0)
                {
                    lpFileName = "ed6_win.exe";
                }

                return StubCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
            },
            sizeof(PVOID), LookupImportTable(BaseAddress, nullptr, KERNEL32_CreateFileA)
        ),
#endif

        MemoryPatchVa(
            (ULONG64)(API_POINTER(::Sleep))[] (ULONG ms) -> VOID
            {
                Ps::Sleep(SleepFix ? ms == 0 ? 1 : ms : ms);
            },
            sizeof(PVOID), LookupImportTable(BaseAddress, nullptr, KERNEL32_Sleep)
        ),

        MemoryPatchVa(
            (ULONG64)(API_POINTER(SetWindowPos))[](HWND Wnd, HWND InsertAfter, int X, int Y, int cx, int cy, UINT Flags) -> BOOL
            {
                if (Flags == SWP_NOMOVE)
                {
                    RECT WorkArea;

                    SystemParametersInfoW(SPI_GETWORKAREA, 0, &WorkArea, 0);
                    X = ((WorkArea.right - WorkArea.left) - cx) / 2;
                    Y = ((WorkArea.bottom - WorkArea.top) - cy) / 2;

                    CLEAR_FLAG(Flags, SWP_NOMOVE);

                    OrigGameWindowProc = (WNDPROC)SetWindowLongPtrW(Wnd, GWLP_WNDPROC, (LONG_PTR)GameWindowProc);
                }

                return SetWindowPos(Wnd, InsertAfter, X, Y, cx, cy, Flags);
            },
            sizeof(PVOID),
            LookupImportTable(GetExeModuleHandle(), nullptr, USER32_SetWindowPos)
        ),
#endif
#endif

        // cmp r8, 80
        // 80 ?? 80 72 ?? 80 ?? A0 72 ?? 80 ?? E0 72 ??
        // 3C 80 72 ?? 3C A0 72 ?? 3C E0 72 ??
        // 3C 80 72 ?? 3C A0 72 ?? 3C E0 73 ??
#define X_FUNC_BASE 0x46A2E2
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x2B2),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x479B60
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0xC7),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x47B8A1
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0xB1),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x4853E0
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x2B),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x4858D0
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x71),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x4882C0
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x625),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x488A50
        //MemoryPatchVa(0x00ull, 1, X_FUNC_BASE + 0x1C + 0x1),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x488E37
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x247),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x4B8FC0
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x21),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x4B9030
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x14),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x4B9030
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x15),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x4B9140
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x1D),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x4B92D0
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x3C),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x4B9340
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x3C),
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0xEB),
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x4DB670
        //MemoryPatchVa(0xEBull, 1, X_FUNC_BASE + 0x5D),
#undef X_FUNC_BASE

        /************************************************************************
         calc ansi char width

        004B7926    .  8D51 E0             lea     edx, dword ptr [ecx-0x20]
        004B7929    .  83FA 5F             cmp     edx, 0x5F
        004B792C       76 40               jbe     short 0x4B796E                   <--
        004B792E    .  3C 80               cmp     al, 0x80
        004B7930    .  72 08               jb      short 0x4B793A
        004B7932    .  3C A0               cmp     al, 0xA0
        004B7934    .  EB 23               jmp     short 0x4B7959                   <==
        004B7936    .  3C E0               cmp     al, 0xE0
        004B7938    .  73 1F               jnb     short 0x4B7959

        76 ?? ?? 80 72 ?? ?? A0 72 ?? ?? E0 73 ??
        ************************************************************************/
#define X_FUNC_BASE 0x4B8FC0
        // MemoryPatchVa(0xCull, 1, X_FUNC_BASE + 0x19 + 0x1),
#undef X_FUNC_BASE
        // MemoryPatchVa(0xCull, 1, X_FUNC_BASE + 0x 0x4B792D), //nope
        // MemoryPatchVa(0xCull, 1, X_FUNC_BASE + 0x 0x4B79AE),
        // MemoryPatchVa(0xCull, 1, X_FUNC_BASE + 0x 0x4B7A46),
        // MemoryPatchVa(0xCull, 1, X_FUNC_BASE + 0x 0x4B7D14),

        // 物品已有个数窗口位置
#define X_FUNC_BASE 0x499280
        //CWindow::CWindow(104, 14, ...)         // 4743A0
        MemoryPatchVa(0x104ull, 4, X_FUNC_BASE + 0x10B + 0x4),  // x
        MemoryPatchVa(0x14ull,  4, X_FUNC_BASE + 0x11F + 0x4),  // width
#undef X_FUNC_BASE
#define X_FUNC_BASE 0x49C790
        MemoryPatchVa(0x104ull, 4, X_FUNC_BASE + 0x28 + 0x4),  // x
        MemoryPatchVa(0x14ull,  4, X_FUNC_BASE + 0x3C + 0x4),  // width
#undef X_FUNC_BASE

        // 战斗状态
#define X_FUNC_BASE 0x43AF10
        MemoryPatchVa(0xC98B0000003Eull,  6, X_FUNC_BASE + 0x128 + 0x1),  // hp fixed x
#undef X_FUNC_BASE

        // char type switch table
//referenced by 0x486DF0+19
        MemoryPatchVa(0x0404ull,    2, 0x486EBC),

        // jp font size limit
#define X_FUNC_BASE 0x4DD9B0
        MemoryPatchVa(0xEBull,      1, X_FUNC_BASE + 0x2A4),
#undef X_FUNC_BASE

        // HP EP font size
#define X_FUNC_BASE 0x4773A0
        MemoryPatchVa(0x02ull,      1, X_FUNC_BASE + 0x72A + 0x1),
#undef X_FUNC_BASE

        // place name text X delta
#define X_FUNC_BASE 0x4B7ED0
        MemoryPatchVa((ULONG64)&DefaultPlaceNameTextDeltaX,      4, X_FUNC_BASE + 0x3F + 0x2),
#undef X_FUNC_BASE

        FunctionJumpVa(Success ? Functions.GetGlyphsBitmap       : IMAGE_INVALID_VA, GetGlyphsBitmap, &StubGetGlyphsBitmap),
        FunctionJumpVa(Success ? Functions.DrawTalkText          : IMAGE_INVALID_VA, DrawTalkText),
        FunctionJumpVa(Success ? Functions.DrawDialogText        : IMAGE_INVALID_VA, NakedDrawDialogText),

        FunctionJumpVa(Functions.LoadFileFromDAT,    LoadFileFromDat),
        FunctionJumpVa(Functions.DecompressData,     NakedDecompressData, &StubNakedDecompressData),
    };

    PatchMemory(p, countof(p), BaseAddress);

    *(PVOID *)&StubCreateFileA = (PVOID)p[0].Memory.Backup;

    return TRUE;
}

BOOL WINAPI DllMain(PVOID BaseAddress, ULONG Reason, PVOID Reserved)
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
            return Initialize(BaseAddress) || UnInitialize(BaseAddress);

        case DLL_PROCESS_DETACH:
            UnInitialize(BaseAddress);
            break;
    }

    return TRUE;
}
