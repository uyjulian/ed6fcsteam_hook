
#pragma comment(linker, "/ENTRY:DllMain")

#include "ed6fc.h"
#include "ed6fc_info.h"
#include "ml.cpp"
#include <stdio.h>
#include <functional>

ML_OVERLOAD_NEW

ED6HOOK_HOOK_INFO* HookInfo;
BOOL SleepFix;
PED6_FC_FONT_RENDER GameFontRender;
API_POINTER(CreateFileA) StubCreateFileA;

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

PVOID NTAPI GetGlyphsBitmapJmp(PCSTR Text, PVOID Buffer, ULONG Stride, ULONG ColorIndex)
{
	PVOID (NTAPI *funcptr)(PCSTR, PVOID, ULONG, ULONG);
	*reinterpret_cast<void**>(&funcptr) = 0;
	
	if (HookInfo->GetGlyphsBitmap)
		*reinterpret_cast<void**>(&funcptr) = HookInfo->GetGlyphsBitmap;

	if (funcptr)
		return funcptr(Text, Buffer, Stride, ColorIndex);
	return NULL;
}

PVOID FASTCALL DrawTalkText(PVOID thiz, PVOID, PVOID Buffer, ULONG Stride, PCSTR Text, ULONG ColorIndex)
{
    CHAR tmp[3] = { Text[0], Text[0] < 0 ? Text[1] : 0 };
    return GetGlyphsBitmapJmp(tmp, Buffer, Stride * 2, ColorIndex);
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
        call    GetGlyphsBitmapJmp;
        pop     ebx;
        ret     8;
    }
}

/************************************************************************
  load file
************************************************************************/

BOOL CDECL LoadFileFromDatJmp(PVOID buffer, ULONG datIndex, ULONG datOffset, ULONG fileSize)
{
	BOOL (CDECL *funcptr)(PVOID, ULONG, ULONG, ULONG);
	*reinterpret_cast<void**>(&funcptr) = 0;
	
	if (HookInfo->LoadFileFromDat)
		*reinterpret_cast<void**>(&funcptr) = HookInfo->LoadFileFromDat;

	if (funcptr)
		return funcptr(buffer, datIndex, datOffset, fileSize);
	return FALSE;
}

ULONG_PTR NTAPI DecompressDataJmp(PBYTE compressed, PBYTE uncompressed)
{
	ULONG_PTR (NTAPI *funcptr)(PBYTE&, PBYTE&);
	*reinterpret_cast<void**>(&funcptr) = 0;
	
	if (HookInfo->DecompressData)
		*reinterpret_cast<void**>(&funcptr) = HookInfo->DecompressData;

	if (funcptr)
		return funcptr(compressed, uncompressed);
	return NULL;
}

NAKED VOID CDECL NakedLoadFileFromDat()
{
    INLINE_ASM
    {
        push    [esp + 0Ch];
        push    [esp + 0Ch];
        push    [esp + 0Ch];
        push    edi;
        call    LoadFileFromDatJmp;
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
        call    DecompressDataJmp;
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
    PLDR_MODULE             ExeModule;
    ED6_FC_HOOK_FUNCTIONS   Functions;
    ED6HOOK_INFO*           ED6HookInfo = NULL;
    ED6HOOK_EXE_INFO        ExeInfo;
    ExeInfo.PeTimeDateStamp = 0;

    LdrDisableThreadCalloutsForDll(BaseAddress);

    HMODULE LoaderModule = LoadLibraryA("ed6hook/ed6hook_core.dll");
    FARPROC LoaderModuleInfoFunc = NULL;
    if (LoaderModule)
    	LoaderModuleInfoFunc = GetProcAddress(LoaderModule, "ed6_hook_get_info");
    if (LoaderModuleInfoFunc)
    	ED6HookInfo = ((ED6HOOK_INFO* (*)(void))LoaderModuleInfoFunc)();

    DWORD CurrentExeTimeDateStamp = ImageNtHeaders(Ps::CurrentPeb()->ImageBaseAddress)->FileHeader.TimeDateStamp;
    HookInfo = NULL;
    if (ED6HookInfo) {
    	if (ED6HookInfo->HookInfo)
    		HookInfo = ED6HookInfo->HookInfo;

	    for (DWORD i = 0; i < ED6HookInfo->ExeInfoNum; i += 1) {
	    	if (ED6HookInfo->ExeInfo[i].PeTimeDateStamp == CurrentExeTimeDateStamp) {
	    		memcpy(&ExeInfo, ED6HookInfo->ExeInfo + i, sizeof(ED6HOOK_EXE_INFO));
	    		break;
	    	}
	    }
    }
    else
    {
    	fprintf(stderr, "Couldn't retrieve information from ed6hook_core\n");
    }

    if (CurrentExeTimeDateStamp != ExeInfo.PeTimeDateStamp) {
        fprintf(stderr, "Incompatible timedatestamp 0x%x\n", ImageNtHeaders(Ps::CurrentPeb()->ImageBaseAddress)->FileHeader.TimeDateStamp);
        return FALSE;
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

    Success = TRUE;

    PatchExeText(BaseAddress);

    if (Success)
    {
        GameFontRender = FindFontRender(BaseAddress);
        Success = GameFontRender != nullptr;
    }

	if (ED6HookInfo && ED6HookInfo->ExtraInfo) {
		if (GameFontRender != nullptr) {
			ED6HookInfo->ExtraInfo->GameFontRender = GameFontRender;
			fprintf(stderr, "GameFontRender is set\n");
		}

	}

    //FAIL_RETURN(SearchFunctions(&Functions));
    Functions.GetGlyphsBitmap = (PVOID)ExeInfo.GetGlyphsBitmapVa;
    Functions.DrawTalkText = (PVOID)ExeInfo.DrawTalkTextVa;
    Functions.DrawDialogText = (PVOID)ExeInfo.DrawDialogTextVa;
    Functions.LoadFileFromDAT = (PVOID)ExeInfo.LoadFileFromDatVa;
    Functions.DecompressData = (PVOID)ExeInfo.DecompressDataVa;

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

        // The item has a number of window positions
        //CWindow::CWindow(104, 14, ...)         // 4743A0
        MemoryPatchVa(0x104ull, 4, ExeInfo.WindowPosition1Addr + 0x10B + 0x4),  // x
        MemoryPatchVa(0x14ull,  4, ExeInfo.WindowPosition1Addr + 0x11F + 0x4),  // width
        MemoryPatchVa(0x104ull, 4, ExeInfo.WindowPosition2Addr + 0x28 + 0x4),  // x
        MemoryPatchVa(0x14ull,  4, ExeInfo.WindowPosition2Addr + 0x3C + 0x4),  // width

        // Combat state
        MemoryPatchVa(0xC98B0000003Eull,  6, ExeInfo.CombatStateAddr + 0x128 + 0x1),  // hp fixed x

        // char type switch table
//referenced by 0x486DF0+19
        MemoryPatchVa(0x0404ull,    2, 0x486EBC),

        // jp font size limit
        MemoryPatchVa(0xEBull,      1, ExeInfo.JpFontSizeLimitAddr + 0x2A4),

        // HP EP font size
        MemoryPatchVa(0x02ull,      1, ExeInfo.HpEpFontSizeAddr + 0x72A + 0x1),

        // place name text X delta
        MemoryPatchVa((ULONG64)&DefaultPlaceNameTextDeltaX,      4, ExeInfo.PlaceNameTextXDeltaAddr + 0x3F + 0x2),

        FunctionJumpVa(Functions.GetGlyphsBitmap ? Functions.GetGlyphsBitmap : IMAGE_INVALID_VA, GetGlyphsBitmapJmp),
        FunctionJumpVa(Functions.DrawTalkText    ? Functions.DrawTalkText    : IMAGE_INVALID_VA, DrawTalkText),
        FunctionJumpVa(Functions.DrawDialogText  ? Functions.DrawDialogText  : IMAGE_INVALID_VA, NakedDrawDialogText),

        FunctionJumpVa(Functions.LoadFileFromDAT ? Functions.LoadFileFromDAT : IMAGE_INVALID_VA, LoadFileFromDatJmp),
        FunctionJumpVa(Functions.DecompressData  ? Functions.DecompressData  : IMAGE_INVALID_VA, NakedDecompressData, &StubNakedDecompressData),
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
