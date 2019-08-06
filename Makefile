
export WINEDEBUG=-all

FILES := dinput8.o DWriteRender.o ed6fc.o steam_api.o TextPatcher.o

.PHONY: clean

DINPUT8.dll: $(FILES)
	link /LIBPATH:c:\lib delayimp.lib ntdll.lib kernel32.lib user32.lib gdiplus.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib freetype.lib /OPT:REF /delayload:USER32.dll /nologo /dll /machine:I386 /ltcg /out:$@ $^

%.o: %.cpp
	cl /nologo /Gr /MD /W4 /GR- /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D USE_NT_VER=1 /GS- /MP /arch:SSE /GL /c /Fo$@ /Tp$< /I. /Iml /Ifreetype /Isteam /utf-8

clean:
	rm -f $(FILES) DINPUT8.dll DINPUT8.lib DINPUT8.exp
