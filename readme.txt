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

Once built, copy dsound.dll to the same location as the desired application's
executable. You must also provide an OpenAL DLL in the same location, named as
dsoal-aldrv.dll, or else the DLL will fail to work. Some applications may need
to be configured to use DirectSound3D acceleration and EAX, but it otherwise
goes to work the next time the application is run.


Source releases, the Git repository, and Windows binaries for OpenAL Soft are
available at its homepage <http://kcat.strangesoft.net/openal.html>.
Instructions are also provided there.
