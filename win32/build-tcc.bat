@rem ------------------------------------------------------
@rem batch file to build tcc using mingw gcc or tcc itself
@rem ------------------------------------------------------

@echo off

set CC=gcc -Os -s -fno-strict-aliasing
set T=32
if %PROCESSOR_ARCHITECTURE%_==AMD64_ set T=64
if %PROCESSOR_ARCHITEW6432%_==AMD64_ set T=64
set /p VERSION= < ..\VERSION
set INST=
set DOC=no
goto :a0

:usage
echo usage: build-tcc.bat [ options ... ]
echo options:
echo   -c prog              use prog (gcc or tcc) to compile tcc
echo   -c "prog options"    use prog with options to compile tcc
echo   -t 32/64             force 32/64 bit default target
echo   -v "version"         set tcc version
echo   -i dir               install tcc into dir
echo   -d                   create tcc-doc.html too (needs makeinfo)
exit /B 1

:a2
shift
:a1
shift
:a0
if not (%1)==(-c) goto :a3
set CC=%~2
goto :a2
:a3
if not (%1)==(-t) goto :a4
set T=%2
goto :a2
:a4
if not (%1)==(-v) goto :a5
set VERSION=%~2
goto :a2
:a5
if not (%1)==(-i) goto :a6
set INST=%2
goto :a2
:a6
if not (%1)==(-d) goto :a7
set DOC=yes
goto :a1
:a7
if not (%1)==() goto :usage

set D32=-DTCC_TARGET_PE -DTCC_TARGET_I386
set D64=-DTCC_TARGET_PE -DTCC_TARGET_X86_64
if %T%==64 goto :t64
set D=%D32%
set DX=%D64%
set TX=64
set PX=x86_64-win32
goto :t96
:t64
set D=%D64%
set DX=%D32%
set TX=32
set PX=i386-win32
:t96

@echo on

:config.h
echo>..\config.h #define TCC_VERSION "%VERSION%"
echo>> ..\config.h #ifdef TCC_TARGET_X86_64
echo>> ..\config.h #define CONFIG_TCC_LIBPATHS "{B}/lib/64;{B}/lib"
echo>> ..\config.h #else
echo>> ..\config.h #define CONFIG_TCC_LIBPATHS "{B}/lib/32;{B}/lib"
echo>> ..\config.h #endif

for %%f in (*tcc.exe tiny_*.exe *tcc.dll) do @del %%f

:compiler
%CC% -o libtcc.dll -shared ..\libtcc.c %D% -DONE_SOURCE -DLIBTCC_AS_DLL
@if errorlevel 1 goto :the_end
%CC% -o tcc.exe ..\tcc.c libtcc.dll %D%
%CC% -o %PX%-tcc.exe ..\tcc.c %DX% -DONE_SOURCE

:tools
%CC% -o tiny_impdef.exe tools\tiny_impdef.c %D%
%CC% -o tiny_libmaker.exe tools\tiny_libmaker.c %D%

@if (%TCC_FILES%)==(no) goto :files-done

if not exist libtcc mkdir libtcc
if not exist doc mkdir doc
if not exist lib\32 mkdir lib\32
if not exist lib\64 mkdir lib\64
copy>nul ..\include\*.h include
copy>nul ..\tcclib.h include
copy>nul ..\libtcc.h libtcc
tiny_impdef libtcc.dll -o libtcc\libtcc.def
copy>nul ..\tests\libtcc_test.c examples
copy>nul tcc-win32.txt doc

copy>nul tiny_libmaker.exe tiny_libmaker-m%T%.exe
%CC% -o tiny_libmaker-m%TX%.exe tools\tiny_libmaker.c %DX%

:libtcc1.a
@set O1=libtcc1.o crt1.o wincrt1.o dllcrt1.o dllmain.o chkstk.o bcheck.o
.\tcc -m32 %D32% -c ../lib/libtcc1.c
.\tcc -m32 %D32% -c lib/crt1.c
.\tcc -m32 %D32% -c lib/wincrt1.c
.\tcc -m32 %D32% -c lib/dllcrt1.c
.\tcc -m32 %D32% -c lib/dllmain.c
.\tcc -m32 %D32% -c lib/chkstk.S
.\tcc -m32 %D32% -w -c ../lib/bcheck.c
.\tcc -m32 %D32% -c ../lib/alloca86.S
.\tcc -m32 %D32% -c ../lib/alloca86-bt.S
tiny_libmaker-m32 lib/32/libtcc1.a %O1% alloca86.o alloca86-bt.o
@if errorlevel 1 goto :the_end
.\tcc -m64 %D64% -c ../lib/libtcc1.c
.\tcc -m64 %D64% -c lib/crt1.c
.\tcc -m64 %D64% -c lib/wincrt1.c
.\tcc -m64 %D64% -c lib/dllcrt1.c
.\tcc -m64 %D64% -c lib/dllmain.c
.\tcc -m64 %D64% -c lib/chkstk.S
.\tcc -m64 %D64% -w -c ../lib/bcheck.c
.\tcc -m64 %D64% -c ../lib/alloca86_64.S
.\tcc -m64 %D64% -c ../lib/alloca86_64-bt.S
tiny_libmaker-m64 lib/64/libtcc1.a %O1% alloca86_64.o alloca86_64-bt.o
@if errorlevel 1 goto :the_end

:tcc-doc.html
@if not (%DOC%)==(yes) goto :doc-done
echo>..\config.texi @set VERSION %VERSION%
cmd /c makeinfo --html --no-split ../tcc-doc.texi -o doc/tcc-doc.html
:doc-done

:files-done
for %%f in (*.o *.def *-m??.exe) do @del %%f

:copy-install
@if (%INST%)==() goto :the_end
if not exist %INST% mkdir %INST%
@for %%f in (*tcc.exe tiny_*.exe *tcc.dll) do copy>nul %%f %INST%
@for %%f in (include lib examples libtcc doc) do xcopy>nul /s/i/q/y %%f %INST%\%%f
del %INST%\lib\*.c %INST%\lib\*.S

:the_end
exit /B %ERRORLEVEL%
