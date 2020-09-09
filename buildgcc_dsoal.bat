@echo off
set gccbase=G:\p_files\rtdk\i686-8.1.0-win32-dwarf-rt_v6-rev0\mingw32\bin

set opts=-std=c99 -mconsole -Wall -Wextra -Os -s
set opts=%opts% -shared -Wl,--enable-stdcall-fixup -static-libgcc -DCOBJMACROS -DDS_STATIC_PROXY

set linkinc=-Iinclude/AL -lole32
set files=buffer.c bypass.c chorus.c debug.c dsound8.c dsound_main.c duplex.c eax.c eax4.c primary.c propset.c reverb.c
set files=%files% dsound.def build/versionres.o

set PATH=%PATH%;%gccbase%

::pushd ..
del build\dsound.dll
windres -i version.rc build/versionres.o
gcc -o build/dsound.dll %files% %opts% %linkinc% 2> build/dsoal_err.log
::popd
IF %ERRORLEVEL% NEQ 0 (
    echo oops!
    pause
)