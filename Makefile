
FILES := ed6fc_all.o 

export WINEDEBUG=-all

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
#CXXFLAGS += -isystem /Users/julian/.winedev/drive_c/Program\ Files/vc/compiler/include
#CXXFLAGS += -isystem /Users/julian/.winedev/drive_c/Program\ Files/Windows\ Kits/10/Include/10.0.17763.0/ucrt
CXXFLAGS += -isystem /Users/julian/.winedev/drive_c/Program\ Files/Microsoft\ Visual\ Studio\ 10.0/VC/include
CXXFLAGS += -isystem /Users/julian/.winedev/drive_c/Program\ Files/Microsoft\ SDKs/Windows/v7.1/Include
CXXFLAGS += -isystem /Users/julian/.winedev/drive_c/Program\ Files/Microsoft\ DirectX\ SDK\ \(August\ 2006\)/Include
CXXFLAGS += -isystem /Users/julian/.winedev/drive_c/WinDDK/7600.16385.1/inc/ddk

CXXFLAGS += -DUSE_NT_VER=1 -DNOT_USE_CUSTOM_MEMFUNC=1
CXXFLAGS += -DNDEBUG
CXXFLAGS += -O2
CXXFLAGS += -I.
CXXFLAGS += -I./ml
CXXFLAGS += -Wno-invalid-token-paste
CXXFLAGS += -Wno-address-of-temporary
CXXFLAGS += -Wno-c++11-narrowing

LDFLAGS += -fuse-ld=lld -target i686-pc-win32 -Wl,-machine:x86 -fmsc-version=1910
LDFLAGS += -L/Users/julian/.winedev/drive_c/Program\ Files/vc/compiler/lib/x86
#LDFLAGS += -L/Users/julian/.winedev/drive_c/Program\ Files/Windows\ Kits/10/Lib/10.0.17763.0/ucrt/x86
#LDFLAGS += -L/Users/julian/.winedev/drive_c/lib
LDFLAGS += -L/Users/julian/.winedev/drive_c/Program\ Files/Microsoft\ SDKs/Windows/v7.1/Lib
LDFLAGS += -L/Users/julian/.winedev/drive_c/Program\ Files/Microsoft\ DirectX\ SDK\ \(August\ 2006\)/Lib/x86
LDFLAGS += -L/Users/julian/.winedev/drive_c/WinDDK/7600.16385.1/lib/win7/i386
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

.PHONY: clean

DINPUT8.dll: $(FILES)
	link /LIBPATH:c:\lib delayimp.lib ntdll.lib kernel32.lib user32.lib gdiplus.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /OPT:REF /delayload:USER32.dll /nologo /dll /machine:I386 /ltcg /out:$@ $^
#$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)


%.o: %.cpp
	cl /nologo /Gr /MD /W4 /GR- /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D USE_NT_VER=1 /GS- /MP /arch:SSE /c /Fo$@ /Tp$< /I. /Iml /utf-8

#%.o: %.cpp
#	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(FILES) DINPUT8.dll DINPUT8.lib DINPUT8.exp
