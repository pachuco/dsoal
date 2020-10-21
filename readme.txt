About
=====

This project is for a DirectSound DLL replacement. It implements the
DirectSound interfaces by translating the calls to OpenAL, and fools
applications into thinking there is a hardware accelerated sound device. EAX is
also implemented (up to version 4) by using OpenAL's EFX extension, allowing
for environmental reverb with sound obstruction and occlusion effects.

Ultimately, this enables DirectSound applications to enable their DirectSound3D
acceleration path, and turn on EAX. The actual processing is being done by
OpenAL with no hardware acceleration requirement, allowing it to work on
systems where audio acceleration is not otherwise available.

Or more succinctly: it enables DirectSound3D surround sound and EAX for systems
without the requisite hardware.

This specific fork is designed to run in windows XP.

Source Code
===========

To build the source, you need mingw+gcc. Define appropriate path in the batch file
and run it.

Once successfully built, it should have created dsound.dll.


Usage
=====

Copy the provided dsound.dll and dsoal-aldrv.dll into game's folder.
Some applications may need to be configured to use DirectSound3D acceleration and EAX.
You may also want to tune SoftOAL by providing a config file named "alsoft.ini",
either in C:\Documents and Settings\USERNAME\Application Data\alsoft.ini
or in game's folder.
Details on how to configure here:
https://github.com/kcat/openal-soft/blob/master/alsoftrc.sample

IMPORTANT:
If you provide your own OpenAL implementation DLL in place of included dsoal-aldrv.dll,
special care must be taken.
Either have OpenAL use backend different from DirectSound, or have it explicitly load
"system32\dsound.dll" instead of just "dsound.dll".
Failure to do so will result in application locking up on startup.

Official source releases, the Git repository, and Windows binaries for OpenAL Soft are
available at its homepage: http://kcat.strangesoft.net/openal.html
Instructions are also provided there.
