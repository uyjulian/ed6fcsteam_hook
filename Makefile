
FILES := ed6fc_all.o 

export WINEPREFIX := $(HOME)/.winedev
PROGRAMFILES := c:/Program Files
WSDK := $(PROGRAMFILES)/Microsoft Visual Studio 10.0
WSDK2 := $(PROGRAMFILES)/vc/compiler
WPSDK := $(PROGRAMFILES)/Microsoft SDKs/Windows/v7.1
WDXSDK := $(PROGRAMFILES)/Microsoft DirectX SDK (August 2006)
WDDK := c:/WinDDK/7600.16385.1
export WINEPATH := c:/windows;c:/windows/system32;$(WSDK2)/bin/Hostx86/x86;$(WSDK)/Common7/IDE;$(WSDK)/VC/bin
export INCLUDE := $(WSDK)/VC/include;$(WPSDK)/Include;$(WDXSDK)/Include;$(WDDK)/inc/ddk
export LIB := c:/lib;$(WDDK)/lib/win7/i386;$(WSDK)/VC/lib;$(WPSDK)/Lib;$(WDXSDK)/Lib/x86
export WINEDEBUG := -all

OUTFILE := DINPUT8.dll

LINK = wine link.exe
CL = wine cl.exe
CXX = /usr/local/opt/llvm/bin/clang++

CXXFLAGS += -target i686-pc-win32
CXXFLAGS += -fms-compatibility-version=19
CXXFLAGS += -fms-extensions
CXXFLAGS += -fdelayed-template-parsing
CXXFLAGS += -fexceptions
CXXFLAGS += -mthread-model posix
CXXFLAGS += -fno-threadsafe-statics
CXXFLAGS += -Wno-msvc-not-found
CXXFLAGS += -DWIN32
CXXFLAGS += -D_WIN32
CXXFLAGS += -D_MT
CXXFLAGS += -D_DLL
CXXFLAGS += -Xclang -disable-llvm-verifier
CXXFLAGS += -Xclang '--dependent-lib=msvcrt'
# CXXFLAGS += -Xclang '--dependent-lib=libcmt'
CXXFLAGS += -Xclang '--dependent-lib=oldnames'
# CXXFLAGS += -Xclang '--dependent-lib=vcruntime'
CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS
CXXFLAGS += -D_CRT_NONSTDC_NO_DEPRECATE
CXXFLAGS += -U__GNUC__
CXXFLAGS += -U__gnu_linux__
CXXFLAGS += -U__GNUC_MINOR__
CXXFLAGS += -U__GNUC_PATCHLEVEL__
CXXFLAGS += -U__GNUC_STDC_INLINE__
CXXFLAGS += -isystem /usr/local/opt/llvm/include
#CXXFLAGS += -isystem $(WINEPREFIX)/drive_c/Program\ Files/vc/compiler/include
#CXXFLAGS += -isystem $(WINEPREFIX)/drive_c/Program\ Files/Windows\ Kits/10/Include/10.0.17763.0/ucrt
CXXFLAGS += -isystem $(WINEPREFIX)/drive_c/Program\ Files/Microsoft\ Visual\ Studio\ 10.0/VC/include
CXXFLAGS += -isystem $(WINEPREFIX)/drive_c/Program\ Files/Microsoft\ SDKs/Windows/v7.1/Include
CXXFLAGS += -isystem $(WINEPREFIX)/drive_c/Program\ Files/Microsoft\ DirectX\ SDK\ \(August\ 2006\)/Include
CXXFLAGS += -isystem $(WINEPREFIX)/drive_c/WinDDK/7600.16385.1/inc/ddk

CXXFLAGS += -DUSE_NT_VER=1 -DNOT_USE_CUSTOM_MEMFUNC=1
CXXFLAGS += -DNDEBUG
CXXFLAGS += -O2
CXXFLAGS += -I.
CXXFLAGS += -I./ml
CXXFLAGS += -Wno-invalid-token-paste
CXXFLAGS += -Wno-address-of-temporary
CXXFLAGS += -Wno-c++11-narrowing

LDFLAGS += -fuse-ld=lld -target i686-pc-win32 -Wl,-machine:x86 -fmsc-version=1910
LDFLAGS += -L$(WINEPREFIX)/drive_c/Program\ Files/vc/compiler/lib/x86
#LDFLAGS += -L$(WINEPREFIX)/drive_c/Program\ Files/Windows\ Kits/10/Lib/10.0.17763.0/ucrt/x86
#LDFLAGS += -L$(WINEPREFIX)/drive_c/lib
LDFLAGS += -L$(WINEPREFIX)/drive_c/Program\ Files/Microsoft\ SDKs/Windows/v7.1/Lib
LDFLAGS += -L$(WINEPREFIX)/drive_c/Program\ Files/Microsoft\ DirectX\ SDK\ \(August\ 2006\)/Lib/x86
LDFLAGS += -L$(WINEPREFIX)/drive_c/WinDDK/7600.16385.1/lib/win7/i386
LDLIBS += -nostdlib -lmsvcrt -Wno-msvc-not-found 

LDLIBS += -ldelayimp
LDLIBS += -lntdll
LDLIBS += -lkernel32
LDLIBS += -luser32
LDLIBS += -lgdiplus
LDLIBS += -lgdi32
LDLIBS += -lwinspool
LDLIBS += -lcomdlg32
LDLIBS += -ladvapi32
LDLIBS += -lshell32
LDLIBS += -lole32
LDLIBS += -loleaut32
LDLIBS += -luuid
LDLIBS += -lodbc32
LDLIBS += -lodbccp32
LDLIBS += -shared

CLFLAGS += /nologo
CLFLAGS += /Gr
CLFLAGS += /MD
CLFLAGS += /GR-
CLFLAGS += /utf-8
CLFLAGS += /Ob1
CLFLAGS += /D "WIN32"
CLFLAGS += /GS-
CLFLAGS += /MP
CLFLAGS += /arch:SSE

CLFLAGS += /D USE_NT_VER=1
CLFLAGS += /D "NDEBUG"
CLFLAGS += /O2
CLFLAGS += /I.
CLFLAGS += /Iml
CLFLAGS += /W4

LINKFLAGS += /nologo
LINKFLAGS += /OPT:REF
LINKFLAGS += /dll
LINKFLAGS += /machine:I386

LINKLIBS += advapi32.lib
LINKLIBS += comdlg32.lib
LINKLIBS += delayimp.lib
LINKLIBS += gdi32.lib
LINKLIBS += gdiplus.lib
LINKLIBS += kernel32.lib
LINKLIBS += ntdll.lib
LINKLIBS += odbc32.lib
LINKLIBS += odbccp32.lib
LINKLIBS += ole32.lib
LINKLIBS += oleaut32.lib
LINKLIBS += shell32.lib
LINKLIBS += user32.lib
LINKLIBS += uuid.lib
LINKLIBS += winspool.lib

.PHONY: clean

$(OUTFILE): $(FILES)
	$(LINK) $(LINKFLAGS) $(LINKLIBS) /out:$@ $^
#	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)


%.o: %.cpp
	$(CL) /c $(CLFLAGS) /Fo$@ /Tp$<
#	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(FILES) $(OUTFILE) DINPUT8.lib DINPUT8.exp
