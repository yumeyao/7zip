LIBS = $(LIBS) oleaut32.lib ole32.lib

!IFDEF CPU
!IFNDEF NO_BUFFEROVERFLOWU
LIBS = $(LIBS) bufferoverflowU.lib
!ENDIF
!ENDIF


!IFNDEF O
!IFDEF CPU
O=$(CPU)
!ELSE
O=O
!ENDIF
!ENDIF

!IF "$(CPU)" == "AMD64"
MY_ML = ml64 -Dx64
!ELSEIF "$(CPU)" == "ARM"
MY_ML = armasm
!ELSE
MY_ML = ml
!ENDIF


!IFDEF UNDER_CE
RFLAGS = $(RFLAGS) -dUNDER_CE
!IFDEF MY_CONSOLE
LFLAGS = $(LFLAGS) /ENTRY:mainACRTStartup
!ENDIF
!ELSE
!IFNDEF NEW_COMPILER
!IF "$(CPU)" != "AMD64"
LFLAGS = $(LFLAGS) -OPT:NOWIN98
!ENDIF
!ENDIF
CFLAGS = $(CFLAGS) -Gr
LIBS = $(LIBS) user32.lib advapi32.lib shell32.lib
!ENDIF

!IF "$(CPU)" == "ARM"
COMPL_ASM = $(MY_ML) $** $O/$(*B).obj
!ELSE
COMPL_ASM = $(MY_ML) -c -Fo$O/ $**
!ENDIF

CFLAGS = $(CFLAGS) -nologo -c -Fo$O/ -WX -EHsc -Gy -GR- -GS-

!IFDEF MY_STATIC_LINK
!IFNDEF MY_SINGLE_THREAD
CFLAGS = $(CFLAGS) -MT
!ENDIF
!ELSE
CFLAGS = $(CFLAGS) -MD
!ENDIF

!IFDEF NEW_COMPILER
CFLAGS = $(CFLAGS) -W4 -GS- -Zc:forScope
!ELSE
CFLAGS = $(CFLAGS) -W3
!ENDIF

#!IF "$(CPU)" == "AMD64"
#CFLAGS = $(CFLAGS) -GL
#LFLAGS = $(LFLAGS) -LTCG
#!ENDIF

CFLAGS_O1 = $(CFLAGS) -O1 -Oi
CFLAGS_O2 = $(CFLAGS) -O2

LFLAGS = $(LFLAGS) -nologo -OPT:REF -OPT:ICF

!IFNDEF UNDER_CE
LFLAGS = $(LFLAGS) /LARGEADDRESSAWARE
!ENDIF

!IFDEF DEF_FILE
LFLAGS = $(LFLAGS) -DLL -DEF:$(DEF_FILE)
!ENDIF

PROGPATH = $O\$(PROG)

COMPL_O1   = $(CC) $(CFLAGS_O1) $**
COMPL_O2   = $(CC) $(CFLAGS_O2) $**
COMPL_PCH  = $(CC) $(CFLAGS_O1) -Yc"StdAfx.h" -Fp$O/a.pch $**
COMPL      = $(CC) $(CFLAGS_O1) -Yu"StdAfx.h" -Fp$O/a.pch $**

all: $(PROGPATH) copy-target


!IF "$(PROGIS7ZFMFULL)" == "Yes"
COPYTARGET = $(COPYPATH)\7zFMFull.exe
!ELSE
COPYTARGET = $(COPYPATH)\$(PROG)
!ENDIF

copy-target: $(PROGPATH)
	if not "$(COPYPATH)"=="" copy /y "$(PROGPATH)" "$(COPYTARGET)"

clean:
	-del /Q $(PROGPATH) $O\*.exe $O\*.dll $O\*.obj $O\*.lib $O\*.exp $O\*.res $O\*.pch

$O:
	if not exist "$O" mkdir "$O"

$(PROGPATH): $O $(OBJS) $(DEF_FILE)
	@cmd /q /c "for /f %%i in ('dir /b $O\*.obj') do ((replace -AH $O\%%i 22059319 20059319)&(replace $O\%%i __CxxFrameHandler3 __CxxFrameHandler))"
	link $(LFLAGS) -out:$(PROGPATH) $(OBJS) $(LIBS)

!IFNDEF NO_DEFAULT_RES
$O\resource.res: $(*B).rc
	rc $(RFLAGS) -fo$@ $**
!ENDIF
$O\StdAfx.obj: $(*B).cpp
	$(COMPL_PCH)
