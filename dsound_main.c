/*  			DirectSound
 *
 * Copyright 1998 Marcus Meissner
 * Copyright 1998 Rob Riggs
 * Copyright 2000-2002 TransGaming Technologies, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Most thread locking is complete. There may be a few race
 * conditions still lurking.
 *
 * TODO:
 *	Implement SetCooperativeLevel properly (need to address focus issues)
 *	Implement DirectSound3DBuffers (stubs in place)
 *	Use hardware 3D support if available
 *      Add critical section locking inside Release and AddRef methods
 *      Handle static buffers - put those in hardware, non-static not in hardware
 *      Hardware DuplicateSoundBuffer
 *      Proper volume calculation for 3d buffers
 *      Remove DS_HEL_FRAGS and use mixer fragment length for it
 */

#define CONST_VTABLE
#define INITGUID
#include <stdarg.h>
#include <string.h>

#include <windows.h>
#include "dsound_wrap.h"

#include "dsound_private.h"
#include "eax-presets.h"

#ifndef DECLSPEC_EXPORT
#ifdef _WIN32
#define DECLSPEC_EXPORT __declspec(dllexport)
#else
#define DECLSPEC_EXPORT
#endif
#endif


int LogLevel = 1;
FILE *LogFile;

const WCHAR aldriver_name[] = L"dsoal-aldrv.dll";
const WCHAR dsound_name[] = L"system32\\dsound.dll";


const EAXREVERBPROPERTIES EnvironmentDefaults[EAX_ENVIRONMENT_UNDEFINED] = {
    REVERB_PRESET_GENERIC,
    REVERB_PRESET_PADDEDCELL,
    REVERB_PRESET_ROOM,
    REVERB_PRESET_BATHROOM,
    REVERB_PRESET_LIVINGROOM,
    REVERB_PRESET_STONEROOM,
    REVERB_PRESET_AUDITORIUM,
    REVERB_PRESET_CONCERTHALL,
    REVERB_PRESET_CAVE,
    REVERB_PRESET_ARENA,
    REVERB_PRESET_HANGAR,
    REVERB_PRESET_CARPETEDHALLWAY,
    REVERB_PRESET_HALLWAY,
    REVERB_PRESET_STONECORRIDOR,
    REVERB_PRESET_ALLEY,
    REVERB_PRESET_FOREST,
    REVERB_PRESET_CITY,
    REVERB_PRESET_MOUNTAINS,
    REVERB_PRESET_QUARRY,
    REVERB_PRESET_PLAIN,
    REVERB_PRESET_PARKINGLOT,
    REVERB_PRESET_SEWERPIPE,
    REVERB_PRESET_UNDERWATER,
    REVERB_PRESET_DRUGGED,
    REVERB_PRESET_DIZZY,
    REVERB_PRESET_PSYCHOTIC
};

CRITICAL_SECTION openal_crst;

int libs_loaded = 0;

static HANDLE openal_handle = NULL;
LPALCCREATECONTEXT palcCreateContext = NULL;
LPALCMAKECONTEXTCURRENT palcMakeContextCurrent = NULL;
LPALCPROCESSCONTEXT palcProcessContext = NULL;
LPALCSUSPENDCONTEXT palcSuspendContext = NULL;
LPALCDESTROYCONTEXT palcDestroyContext = NULL;
LPALCGETCURRENTCONTEXT palcGetCurrentContext = NULL;
LPALCGETCONTEXTSDEVICE palcGetContextsDevice = NULL;
LPALCOPENDEVICE palcOpenDevice = NULL;
LPALCCLOSEDEVICE palcCloseDevice = NULL;
LPALCGETERROR palcGetError = NULL;
LPALCISEXTENSIONPRESENT palcIsExtensionPresent = NULL;
LPALCGETPROCADDRESS palcGetProcAddress = NULL;
LPALCGETENUMVALUE palcGetEnumValue = NULL;
LPALCGETSTRING palcGetString = NULL;
LPALCGETINTEGERV palcGetIntegerv = NULL;
LPALCCAPTUREOPENDEVICE palcCaptureOpenDevice = NULL;
LPALCCAPTURECLOSEDEVICE palcCaptureCloseDevice = NULL;
LPALCCAPTURESTART palcCaptureStart = NULL;
LPALCCAPTURESTOP palcCaptureStop = NULL;
LPALCCAPTURESAMPLES palcCaptureSamples = NULL;
LPALENABLE palEnable = NULL;
LPALDISABLE palDisable = NULL;
LPALISENABLED palIsEnabled = NULL;
LPALGETSTRING palGetString = NULL;
LPALGETBOOLEANV palGetBooleanv = NULL;
LPALGETINTEGERV palGetIntegerv = NULL;
LPALGETFLOATV palGetFloatv = NULL;
LPALGETDOUBLEV palGetDoublev = NULL;
LPALGETBOOLEAN palGetBoolean = NULL;
LPALGETINTEGER palGetInteger = NULL;
LPALGETFLOAT palGetFloat = NULL;
LPALGETDOUBLE palGetDouble = NULL;
LPALGETERROR palGetError = NULL;
LPALISEXTENSIONPRESENT palIsExtensionPresent = NULL;
LPALGETPROCADDRESS palGetProcAddress = NULL;
LPALGETENUMVALUE palGetEnumValue = NULL;
LPALLISTENERF palListenerf = NULL;
LPALLISTENER3F palListener3f = NULL;
LPALLISTENERFV palListenerfv = NULL;
LPALLISTENERI palListeneri = NULL;
LPALLISTENER3I palListener3i = NULL;
LPALLISTENERIV palListeneriv = NULL;
LPALGETLISTENERF palGetListenerf = NULL;
LPALGETLISTENER3F palGetListener3f = NULL;
LPALGETLISTENERFV palGetListenerfv = NULL;
LPALGETLISTENERI palGetListeneri = NULL;
LPALGETLISTENER3I palGetListener3i = NULL;
LPALGETLISTENERIV palGetListeneriv = NULL;
LPALGENSOURCES palGenSources = NULL;
LPALDELETESOURCES palDeleteSources = NULL;
LPALISSOURCE palIsSource = NULL;
LPALSOURCEF palSourcef = NULL;
LPALSOURCE3F palSource3f = NULL;
LPALSOURCEFV palSourcefv = NULL;
LPALSOURCEI palSourcei = NULL;
LPALSOURCE3I palSource3i = NULL;
LPALSOURCEIV palSourceiv = NULL;
LPALGETSOURCEF palGetSourcef = NULL;
LPALGETSOURCE3F palGetSource3f = NULL;
LPALGETSOURCEFV palGetSourcefv = NULL;
LPALGETSOURCEI palGetSourcei = NULL;
LPALGETSOURCE3I palGetSource3i = NULL;
LPALGETSOURCEIV palGetSourceiv = NULL;
LPALSOURCEPLAYV palSourcePlayv = NULL;
LPALSOURCESTOPV palSourceStopv = NULL;
LPALSOURCEREWINDV palSourceRewindv = NULL;
LPALSOURCEPAUSEV palSourcePausev = NULL;
LPALSOURCEPLAY palSourcePlay = NULL;
LPALSOURCESTOP palSourceStop = NULL;
LPALSOURCEREWIND palSourceRewind = NULL;
LPALSOURCEPAUSE palSourcePause = NULL;
LPALSOURCEQUEUEBUFFERS palSourceQueueBuffers = NULL;
LPALSOURCEUNQUEUEBUFFERS palSourceUnqueueBuffers = NULL;
LPALGENBUFFERS palGenBuffers = NULL;
LPALDELETEBUFFERS palDeleteBuffers = NULL;
LPALISBUFFER palIsBuffer = NULL;
LPALBUFFERF palBufferf = NULL;
LPALBUFFER3F palBuffer3f = NULL;
LPALBUFFERFV palBufferfv = NULL;
LPALBUFFERI palBufferi = NULL;
LPALBUFFER3I palBuffer3i = NULL;
LPALBUFFERIV palBufferiv = NULL;
LPALGETBUFFERF palGetBufferf = NULL;
LPALGETBUFFER3F palGetBuffer3f = NULL;
LPALGETBUFFERFV palGetBufferfv = NULL;
LPALGETBUFFERI palGetBufferi = NULL;
LPALGETBUFFER3I palGetBuffer3i = NULL;
LPALGETBUFFERIV palGetBufferiv = NULL;
LPALBUFFERDATA palBufferData = NULL;
LPALDOPPLERFACTOR palDopplerFactor = NULL;
LPALDOPPLERVELOCITY palDopplerVelocity = NULL;
LPALDISTANCEMODEL palDistanceModel = NULL;
LPALSPEEDOFSOUND palSpeedOfSound = NULL;

LPALGENFILTERS palGenFilters = NULL;
LPALDELETEFILTERS palDeleteFilters = NULL;
LPALFILTERI palFilteri = NULL;
LPALFILTERF palFilterf = NULL;
LPALGENEFFECTS palGenEffects = NULL;
LPALDELETEEFFECTS palDeleteEffects = NULL;
LPALEFFECTI palEffecti = NULL;
LPALEFFECTF palEffectf = NULL;
LPALEFFECTFV palEffectfv = NULL;
LPALGENAUXILIARYEFFECTSLOTS palGenAuxiliaryEffectSlots = NULL;
LPALDELETEAUXILIARYEFFECTSLOTS palDeleteAuxiliaryEffectSlots = NULL;
LPALAUXILIARYEFFECTSLOTI palAuxiliaryEffectSloti = NULL;
LPALAUXILIARYEFFECTSLOTF palAuxiliaryEffectSlotf = NULL;
LPALDEFERUPDATESSOFT palDeferUpdatesSOFT = NULL;
LPALPROCESSUPDATESSOFT palProcessUpdatesSOFT = NULL;
LPALBUFFERSTORAGESOFT palBufferStorageSOFT = NULL;
LPALMAPBUFFERSOFT palMapBufferSOFT = NULL;
LPALUNMAPBUFFERSOFT palUnmapBufferSOFT = NULL;
LPALFLUSHMAPPEDBUFFERSOFT palFlushMappedBufferSOFT = NULL;

static HANDLE dsound_handle = NULL;
LPDIRECTSOUNDCREATE             pDirectSoundCreate            = NULL;
LPDIRECTSOUNDENUMERATEA         pDirectSoundEnumerateA        = NULL;
LPDIRECTSOUNDENUMERATEW         pDirectSoundEnumerateW        = NULL;
LPDIRECTSOUNDCAPTURECREATE      pDirectSoundCaptureCreate     = NULL;
LPDIRECTSOUNDCAPTUREENUMERATEA  pDirectSoundCaptureEnumerateA = NULL;
LPDIRECTSOUNDCAPTUREENUMERATEW  pDirectSoundCaptureEnumerateW = NULL;
LPGETDEVICEID                   pGetDeviceID                  = NULL;
LPDIRECTSOUNDFULLDUPLEXCREATE   pDirectSoundFullDuplexCreate  = NULL;
LPDIRECTSOUNDCREATE8            pDirectSoundCreate8           = NULL;
LPDIRECTSOUNDCAPTURECREATE8     pDirectSoundCaptureCreate8    = NULL;

LPDLLCANUNLOADNOW               pDirectSoundDllCanUnloadNow   = NULL;
LPDLLGETCLASSOBJECT             pDirectSoundDllGetClassObject = NULL;





LPALCMAKECONTEXTCURRENT set_context;
LPALCGETCURRENTCONTEXT get_context;
BOOL local_contexts;


static void AL_APIENTRY wrap_DeferUpdates(void)
{ alcSuspendContext(alcGetCurrentContext()); }
static void AL_APIENTRY wrap_ProcessUpdates(void)
{ alcProcessContext(alcGetCurrentContext()); }

static void EnterALSectionTLS(ALCcontext *ctx);
static void LeaveALSectionTLS(void);
static void EnterALSectionGlob(ALCcontext *ctx);
static void LeaveALSectionGlob(void);

DWORD TlsThreadPtr;
void (*EnterALSection)(ALCcontext *ctx) = EnterALSectionGlob;
void (*LeaveALSection)(void) = LeaveALSectionGlob;


static BOOL hasAttemptedLoad = FALSE;
void lazyLoad(void)
{
    BOOL failed = FALSE;
    const char *str;
    const WCHAR *wstr;
    
    if (hasAttemptedLoad) return;
    EnterCriticalSection(&openal_crst);
    if (hasAttemptedLoad) return;
    
    str = getenv("DSOAL_LOGLEVEL");
    if(str && *str) LogLevel = atoi(str);
    LogFile = stderr;
    if((wstr=_wgetenv(L"DSOAL_LOGFILE")) != NULL && wstr[0] != 0)
    {
        FILE *f = _wfopen(wstr, L"wt");
        if(!f) ERR("Failed to open log file %ls\n", wstr);
        else LogFile = f;
    }

    openal_handle = LoadLibraryW(aldriver_name);
    if(!openal_handle)
    {
        ERR("Couldn't load openal: %lu\n", GetLastError());
        goto L_FAIL;
    }
    dsound_handle = LoadLibraryW(dsound_name);
    if(!dsound_handle)
    {
        ERR("Couldn't load dsound: %lu\n", GetLastError());
        goto L_FAIL;
    }

#define LOAD_FUNCPTR(f) do {                                     \
    union { void *ptr; FARPROC *proc; } func = { &p##f };        \
    if((*func.proc = GetProcAddress(LFPHANDLE, #f)) == NULL)     \
    {                                                            \
        ERR("Couldn't GetProcAddress %s\n", #f);                 \
        failed = TRUE;                                           \
    }                                                            \
} while(0)
    
#define LFPHANDLE openal_handle
    LOAD_FUNCPTR(alcCreateContext);
    LOAD_FUNCPTR(alcMakeContextCurrent);
    LOAD_FUNCPTR(alcProcessContext);
    LOAD_FUNCPTR(alcSuspendContext);
    LOAD_FUNCPTR(alcDestroyContext);
    LOAD_FUNCPTR(alcGetCurrentContext);
    LOAD_FUNCPTR(alcGetContextsDevice);
    LOAD_FUNCPTR(alcOpenDevice);
    LOAD_FUNCPTR(alcCloseDevice);
    LOAD_FUNCPTR(alcGetError);
    LOAD_FUNCPTR(alcIsExtensionPresent);
    LOAD_FUNCPTR(alcGetProcAddress);
    LOAD_FUNCPTR(alcGetEnumValue);
    LOAD_FUNCPTR(alcGetString);
    LOAD_FUNCPTR(alcGetIntegerv);
    LOAD_FUNCPTR(alcCaptureOpenDevice);
    LOAD_FUNCPTR(alcCaptureCloseDevice);
    LOAD_FUNCPTR(alcCaptureStart);
    LOAD_FUNCPTR(alcCaptureStop);
    LOAD_FUNCPTR(alcCaptureSamples);
    LOAD_FUNCPTR(alEnable);
    LOAD_FUNCPTR(alDisable);
    LOAD_FUNCPTR(alIsEnabled);
    LOAD_FUNCPTR(alGetString);
    LOAD_FUNCPTR(alGetBooleanv);
    LOAD_FUNCPTR(alGetIntegerv);
    LOAD_FUNCPTR(alGetFloatv);
    LOAD_FUNCPTR(alGetDoublev);
    LOAD_FUNCPTR(alGetBoolean);
    LOAD_FUNCPTR(alGetInteger);
    LOAD_FUNCPTR(alGetFloat);
    LOAD_FUNCPTR(alGetDouble);
    LOAD_FUNCPTR(alGetError);
    LOAD_FUNCPTR(alIsExtensionPresent);
    LOAD_FUNCPTR(alGetProcAddress);
    LOAD_FUNCPTR(alGetEnumValue);
    LOAD_FUNCPTR(alListenerf);
    LOAD_FUNCPTR(alListener3f);
    LOAD_FUNCPTR(alListenerfv);
    LOAD_FUNCPTR(alListeneri);
    LOAD_FUNCPTR(alListener3i);
    LOAD_FUNCPTR(alListeneriv);
    LOAD_FUNCPTR(alGetListenerf);
    LOAD_FUNCPTR(alGetListener3f);
    LOAD_FUNCPTR(alGetListenerfv);
    LOAD_FUNCPTR(alGetListeneri);
    LOAD_FUNCPTR(alGetListener3i);
    LOAD_FUNCPTR(alGetListeneriv);
    LOAD_FUNCPTR(alGenSources);
    LOAD_FUNCPTR(alDeleteSources);
    LOAD_FUNCPTR(alIsSource);
    LOAD_FUNCPTR(alSourcef);
    LOAD_FUNCPTR(alSource3f);
    LOAD_FUNCPTR(alSourcefv);
    LOAD_FUNCPTR(alSourcei);
    LOAD_FUNCPTR(alSource3i);
    LOAD_FUNCPTR(alSourceiv);
    LOAD_FUNCPTR(alGetSourcef);
    LOAD_FUNCPTR(alGetSource3f);
    LOAD_FUNCPTR(alGetSourcefv);
    LOAD_FUNCPTR(alGetSourcei);
    LOAD_FUNCPTR(alGetSource3i);
    LOAD_FUNCPTR(alGetSourceiv);
    LOAD_FUNCPTR(alSourcePlayv);
    LOAD_FUNCPTR(alSourceStopv);
    LOAD_FUNCPTR(alSourceRewindv);
    LOAD_FUNCPTR(alSourcePausev);
    LOAD_FUNCPTR(alSourcePlay);
    LOAD_FUNCPTR(alSourceStop);
    LOAD_FUNCPTR(alSourceRewind);
    LOAD_FUNCPTR(alSourcePause);
    LOAD_FUNCPTR(alSourceQueueBuffers);
    LOAD_FUNCPTR(alSourceUnqueueBuffers);
    LOAD_FUNCPTR(alGenBuffers);
    LOAD_FUNCPTR(alDeleteBuffers);
    LOAD_FUNCPTR(alIsBuffer);
    LOAD_FUNCPTR(alBufferf);
    LOAD_FUNCPTR(alBuffer3f);
    LOAD_FUNCPTR(alBufferfv);
    LOAD_FUNCPTR(alBufferi);
    LOAD_FUNCPTR(alBuffer3i);
    LOAD_FUNCPTR(alBufferiv);
    LOAD_FUNCPTR(alGetBufferf);
    LOAD_FUNCPTR(alGetBuffer3f);
    LOAD_FUNCPTR(alGetBufferfv);
    LOAD_FUNCPTR(alGetBufferi);
    LOAD_FUNCPTR(alGetBuffer3i);
    LOAD_FUNCPTR(alGetBufferiv);
    LOAD_FUNCPTR(alBufferData);
    LOAD_FUNCPTR(alDopplerFactor);
    LOAD_FUNCPTR(alDopplerVelocity);
    LOAD_FUNCPTR(alDistanceModel);
    LOAD_FUNCPTR(alSpeedOfSound);

#undef LFPHANDLE
#define LFPHANDLE dsound_handle
    LOAD_FUNCPTR(DirectSoundCreate);
    LOAD_FUNCPTR(DirectSoundEnumerateA);
    LOAD_FUNCPTR(DirectSoundEnumerateW);
    LOAD_FUNCPTR(DirectSoundCaptureCreate);
    LOAD_FUNCPTR(DirectSoundCaptureEnumerateA);
    LOAD_FUNCPTR(DirectSoundCaptureEnumerateW);
    LOAD_FUNCPTR(GetDeviceID);
    LOAD_FUNCPTR(DirectSoundFullDuplexCreate);
    LOAD_FUNCPTR(DirectSoundCreate8);
    LOAD_FUNCPTR(DirectSoundCaptureCreate8);

#undef LOAD_FUNCPTR 
#define LOAD_FUNCPTR(f) do {                                            \
    union { void *ptr; FARPROC *proc; } func = { &pDirectSound##f };    \
    if((*func.proc = GetProcAddress(LFPHANDLE, #f)) == NULL)            \
    {                                                                   \
        ERR("Couldn't GetProcAddress DS %s\n", #f);                     \
        failed = TRUE;                                                  \
    }                                                                   \
} while(0)
    LOAD_FUNCPTR(DllCanUnloadNow);
    LOAD_FUNCPTR(DllGetClassObject);
    
#undef LOAD_FUNCPTR
#undef LFPHANDLE
    if (failed) {
        WARN("Unloading libraries\n");
        goto L_FAIL;
    }

    TRACE("Loaded %ls\n", aldriver_name);

#define LOAD_FUNCPTR(f) p##f = alcGetProcAddress(NULL, #f)
    LOAD_FUNCPTR(alGenFilters);
    LOAD_FUNCPTR(alDeleteFilters);
    LOAD_FUNCPTR(alFilteri);
    LOAD_FUNCPTR(alFilterf);
    LOAD_FUNCPTR(alGenEffects);
    LOAD_FUNCPTR(alDeleteEffects);
    LOAD_FUNCPTR(alEffecti);
    LOAD_FUNCPTR(alEffectf);
    LOAD_FUNCPTR(alEffectfv);
    LOAD_FUNCPTR(alGenAuxiliaryEffectSlots);
    LOAD_FUNCPTR(alDeleteAuxiliaryEffectSlots);
    LOAD_FUNCPTR(alAuxiliaryEffectSloti);
    LOAD_FUNCPTR(alAuxiliaryEffectSlotf);
    LOAD_FUNCPTR(alDeferUpdatesSOFT);
    LOAD_FUNCPTR(alProcessUpdatesSOFT);
    LOAD_FUNCPTR(alBufferStorageSOFT);
    LOAD_FUNCPTR(alMapBufferSOFT);
    LOAD_FUNCPTR(alUnmapBufferSOFT);
    LOAD_FUNCPTR(alFlushMappedBufferSOFT);
#undef LOAD_FUNCPTR
    if(!palDeferUpdatesSOFT || !palProcessUpdatesSOFT)
    {
        palDeferUpdatesSOFT = wrap_DeferUpdates;
        palProcessUpdatesSOFT = wrap_ProcessUpdates;
    }
    
    local_contexts = 0;
    if ((GetVersion()&0xFF) > 0x05)
        local_contexts = alcIsExtensionPresent(NULL, "ALC_EXT_thread_local_context");
    else
        WARN("TLS not suitable on pre-NT6 OS, disabling\n");
    
    if(local_contexts)
    {
        TRACE("Found ALC_EXT_thread_local_context\n");

        set_context = alcGetProcAddress(NULL, "alcSetThreadContext");
        get_context = alcGetProcAddress(NULL, "alcGetThreadContext");
        if(!set_context || !get_context)
        {
            ERR("TLS advertised but functions not found, disabling thread local contexts\n");
            local_contexts = 0;
        }
    }
    
    if(!local_contexts)
    {
        set_context = alcMakeContextCurrent;
        get_context = alcGetCurrentContext;
    }
    else
    {
        EnterALSection = EnterALSectionTLS;
        LeaveALSection = LeaveALSectionTLS;
    }
    
    libs_loaded = 1;
    
    hasAttemptedLoad = TRUE;
    LeaveCriticalSection(&openal_crst);
    return;
    
    
    L_FAIL:
        if (openal_handle != NULL) FreeLibrary(openal_handle);
        if (dsound_handle != NULL) FreeLibrary(dsound_handle);
        openal_handle = NULL;
        dsound_handle = NULL;
        hasAttemptedLoad = TRUE;
        LeaveCriticalSection(&openal_crst);
}


static void EnterALSectionTLS(ALCcontext *ctx)
{
    if(LIKELY(ctx == TlsGetValue(TlsThreadPtr)))
        return;

    if(LIKELY(set_context(ctx) != ALC_FALSE))
        TlsSetValue(TlsThreadPtr, ctx);
    else
    {
        ERR("Couldn't set current context!!\n");
        checkALCError(alcGetContextsDevice(ctx));
    }
}
static void LeaveALSectionTLS(void)
{
}

static void EnterALSectionGlob(ALCcontext *ctx)
{
    EnterCriticalSection(&openal_crst);
    if(UNLIKELY(alcMakeContextCurrent(ctx) == ALC_FALSE))
    {
        ERR("Couldn't set current context!!\n");
        checkALCError(alcGetContextsDevice(ctx));
    }
}
static void LeaveALSectionGlob(void)
{
    LeaveCriticalSection(&openal_crst);
}



CHAR *strdupA(const CHAR *str)
{
    CHAR *ret;
    int l = lstrlenA(str);
    if(l < 0) return NULL;

    ret = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l+1)*sizeof(CHAR));
    if(!ret) return NULL;

    memcpy(ret, str, l*sizeof(CHAR));
    return ret;
}

WCHAR *strdupW(const WCHAR *str)
{
    WCHAR *ret;
    int l = lstrlenW(str);
    if(l < 0) return NULL;

    ret = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l+1)*sizeof(WCHAR));
    if(!ret) return NULL;

    memcpy(ret, str, l*sizeof(WCHAR));
    return ret;
}

void freedup(void *ptr)
{
    HeapFree(GetProcessHeap(), 0, ptr);
}

#define HACK_COMPARETOCALLER(COMPHAND) \
void* HCTC_CALLERADDRESS = __builtin_extract_return_addr(__builtin_return_address(0)); \
HMODULE HCTC_CALLERMODULE = NULL; \
BOOL HCTC_ISIDENTHANDLE = FALSE; \
if (COMPHAND && GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, HCTC_CALLERADDRESS, &HCTC_CALLERMODULE)) { \
    HCTC_ISIDENTHANDLE = COMPHAND == HCTC_CALLERMODULE; \
}

/*******************************************************************************
 *      DirectSoundCreate (DSOUND.1)
 *
 *  Creates and initializes a DirectSound interface.
 *
 *  PARAMS
 *     lpcGUID   [I] Address of the GUID that identifies the sound device.
 *     ppDS      [O] Address of a variable to receive the interface pointer.
 *     pUnkOuter [I] Must be NULL.
 *
 *  RETURNS
 *     Success: DS_OK
 *     Failure: DSERR_ALLOCATED, DSERR_INVALIDPARAM, DSERR_NOAGGREGATION,
 *              DSERR_NODRIVER, DSERR_OUTOFMEMORY
 */
HRESULT WINAPI
DSOAL_DirectSoundCreate(LPCGUID lpcGUID, IDirectSound **ppDS, IUnknown *pUnkOuter)
{
    HACK_COMPARETOCALLER(openal_handle);
    //lazyLoad();
    HRESULT hr;
    void *pDS;
    
    if (HCTC_ISIDENTHANDLE) return pDirectSoundCreate(lpcGUID, ppDS, pUnkOuter);
    TRACE("(%s, %p, %p)\n", debugstr_guid(lpcGUID), ppDS, pUnkOuter);

    if (ppDS == NULL) {
        WARN("invalid parameter: ppDS == NULL\n");
        return DSERR_INVALIDPARAM;
    }
    *ppDS = NULL;

    if (pUnkOuter != NULL) {
        WARN("invalid parameter: pUnkOuter != NULL\n");
        return DSERR_INVALIDPARAM;
    }

    hr = DSOUND_Create(&IID_IDirectSound, &pDS);
    if(SUCCEEDED(hr))
    {
        *ppDS = pDS;
        hr = IDirectSound_Initialize(*ppDS, lpcGUID);
        if(FAILED(hr))
        {
            IDirectSound_Release(*ppDS);
            *ppDS = NULL;
        }
    }

    return hr;
}

/*******************************************************************************
 *        DirectSoundCreate8 (DSOUND.11)
 *
 *  Creates and initializes a DirectSound8 interface.
 *
 *  PARAMS
 *     lpcGUID   [I] Address of the GUID that identifies the sound device.
 *     ppDS      [O] Address of a variable to receive the interface pointer.
 *     pUnkOuter [I] Must be NULL.
 *
 *  RETURNS
 *     Success: DS_OK
 *     Failure: DSERR_ALLOCATED, DSERR_INVALIDPARAM, DSERR_NOAGGREGATION,
 *              DSERR_NODRIVER, DSERR_OUTOFMEMORY
 */
HRESULT WINAPI
DSOAL_DirectSoundCreate8(LPCGUID lpcGUID, IDirectSound8 **ppDS, IUnknown *pUnkOuter)
{
    HACK_COMPARETOCALLER(openal_handle);
    //lazyLoad();
    HRESULT hr;
    void *pDS;

    if (HCTC_ISIDENTHANDLE) return pDirectSoundCreate8(lpcGUID, ppDS, pUnkOuter);
    TRACE("(%s, %p, %p)\n", debugstr_guid(lpcGUID), ppDS, pUnkOuter);

    if (ppDS == NULL) {
        WARN("invalid parameter: ppDS == NULL\n");
        return DSERR_INVALIDPARAM;
    }
    *ppDS = NULL;

    if (pUnkOuter != NULL) {
        WARN("invalid parameter: pUnkOuter != NULL\n");
        return DSERR_INVALIDPARAM;
    }

    hr = DSOUND_Create8(&IID_IDirectSound8, &pDS);
    if(SUCCEEDED(hr))
    {
        *ppDS = pDS;
        hr = IDirectSound8_Initialize(*ppDS, lpcGUID);
        if(FAILED(hr))
        {
            IDirectSound8_Release(*ppDS);
            *ppDS = NULL;
        }
    }

    return hr;
}

/*******************************************************************************
 * DirectSound ClassFactory
 */

typedef  HRESULT (*FnCreateInstance)(REFIID riid, LPVOID *ppobj);

typedef struct {
    IClassFactory IClassFactory_iface;
    LONG ref;
    REFCLSID rclsid;
    FnCreateInstance pfnCreateInstance;
} IClassFactoryImpl;

static inline IClassFactoryImpl *impl_from_IClassFactory(IClassFactory *iface)
{
    return CONTAINING_RECORD(iface, IClassFactoryImpl, IClassFactory_iface);
}

static HRESULT WINAPI DSCF_QueryInterface(LPCLASSFACTORY iface, REFIID riid, LPVOID *ppobj)
{
    IClassFactoryImpl *This = impl_from_IClassFactory(iface);
    TRACE("(%p, %s, %p)\n", This, debugstr_guid(riid), ppobj);
    if (ppobj == NULL)
        return E_POINTER;
    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IClassFactory))
    {
        *ppobj = iface;
        IUnknown_AddRef(iface);
        return S_OK;
    }
    *ppobj = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI DSCF_AddRef(LPCLASSFACTORY iface)
{
    IClassFactoryImpl *This = impl_from_IClassFactory(iface);
    ULONG ref = InterlockedIncrement(&(This->ref));
    TRACE("(%p) ref %lu\n", iface, ref);
    return ref;
}

static ULONG WINAPI DSCF_Release(LPCLASSFACTORY iface)
{
    IClassFactoryImpl *This = impl_from_IClassFactory(iface);
    ULONG ref = InterlockedDecrement(&(This->ref));
    TRACE("(%p) ref %lu\n", iface, ref);
    /* static class, won't be freed */
    return ref;
}

static HRESULT WINAPI DSCF_CreateInstance(
    LPCLASSFACTORY iface,
    LPUNKNOWN pOuter,
    REFIID riid,
    LPVOID *ppobj)
{
    IClassFactoryImpl *This = impl_from_IClassFactory(iface);
    TRACE("(%p, %p, %s, %p)\n", This, pOuter, debugstr_guid(riid), ppobj);

    if (pOuter)
        return CLASS_E_NOAGGREGATION;

    if (ppobj == NULL) {
        WARN("invalid parameter\n");
        return DSERR_INVALIDPARAM;
    }
    *ppobj = NULL;
    return This->pfnCreateInstance(riid, ppobj);
}

static HRESULT WINAPI DSCF_LockServer(LPCLASSFACTORY iface, BOOL dolock)
{
    IClassFactoryImpl *This = impl_from_IClassFactory(iface);
    FIXME("(%p, %d) stub!\n", This, dolock);
    return S_OK;
}

static const IClassFactoryVtbl DSCF_Vtbl = {
    DSCF_QueryInterface,
    DSCF_AddRef,
    DSCF_Release,
    DSCF_CreateInstance,
    DSCF_LockServer
};

static IClassFactoryImpl DSOUND_CF[] = {
    { {&DSCF_Vtbl}, 1, &CLSID_DirectSound, DSOUND_Create },
    { {&DSCF_Vtbl}, 1, &CLSID_DirectSound8, DSOUND_Create8 },
    { {&DSCF_Vtbl}, 1, &CLSID_DirectSoundCapture, DSOUND_CaptureCreate },
    { {&DSCF_Vtbl}, 1, &CLSID_DirectSoundCapture8, DSOUND_CaptureCreate8 },
    { {&DSCF_Vtbl}, 1, &CLSID_DirectSoundFullDuplex, DSOUND_FullDuplexCreate },
    { {&DSCF_Vtbl}, 1, &CLSID_DirectSoundPrivate, IKsPrivatePropertySetImpl_Create },
    { {NULL}, 0, NULL, NULL }
};

/*******************************************************************************
 * DllGetClassObject [DSOUND.@]
 * Retrieves class object from a DLL object
 *
 * NOTES
 *    Docs say returns STDAPI
 *
 * PARAMS
 *    rclsid [I] CLSID for the class object
 *    riid   [I] Reference to identifier of interface for class object
 *    ppv    [O] Address of variable to receive interface pointer for riid
 *
 * RETURNS
 *    Success: S_OK
 *    Failure: CLASS_E_CLASSNOTAVAILABLE, E_OUTOFMEMORY, E_INVALIDARG,
 *             E_UNEXPECTED
 */
HRESULT WINAPI DSOAL_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    HACK_COMPARETOCALLER(openal_handle);
    //lazyLoad();
    int i = 0;
    
    if (HCTC_ISIDENTHANDLE) return pDirectSoundDllGetClassObject(rclsid, riid, ppv);
    TRACE("(%s, %s, %p)\n", debugstr_guid(rclsid), debugstr_guid(riid), ppv);

    if (ppv == NULL) {
        WARN("invalid parameter\n");
        return E_INVALIDARG;
    }

    *ppv = NULL;

    if (!IsEqualIID(riid, &IID_IClassFactory) &&
        !IsEqualIID(riid, &IID_IUnknown)) {
        WARN("no interface for %s\n", debugstr_guid(riid));
        return E_NOINTERFACE;
    }

    while (NULL != DSOUND_CF[i].rclsid) {
        if (IsEqualGUID(rclsid, DSOUND_CF[i].rclsid)) {
            DSCF_AddRef(&DSOUND_CF[i].IClassFactory_iface);
            *ppv = &DSOUND_CF[i].IClassFactory_iface;
            return S_OK;
        }
        i++;
    }

    WARN("No class found for %s\n", debugstr_guid(rclsid));
    return CLASS_E_CLASSNOTAVAILABLE;
}


/*******************************************************************************
 * DllCanUnloadNow [DSOUND.4]
 * Determines whether the DLL is in use.
 *
 * RETURNS
 *    Success: S_OK
 *    Failure: S_FALSE
 */
HRESULT WINAPI DSOAL_DllCanUnloadNow(void)
{
    HACK_COMPARETOCALLER(openal_handle);
    if (HCTC_ISIDENTHANDLE) return pDirectSoundDllCanUnloadNow();
    //lazyLoad();
    FIXME("(void): stub\n");
    return S_FALSE;
}

/***********************************************************************
 *           DllMain (DSOUND.init)
 */
DECLSPEC_EXPORT BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    TRACE("(%p, %lu, %p)\n", hInstDLL, fdwReason, lpvReserved);

    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        InitializeCriticalSection(&openal_crst);
        TlsThreadPtr = TlsAlloc();
        lazyLoad();
        /* Increase refcount on dsound by 1 */
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)hInstDLL, &hInstDLL);
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        if(local_contexts)
            set_context(NULL);
        break;

    case DLL_PROCESS_DETACH:
        /* If process is exiting, do not risk cleanup.
           OS will do this better than us.             */
        if (!lpvReserved) {
            if(openal_handle) FreeLibrary(openal_handle);
            if(dsound_handle) FreeLibrary(dsound_handle);
            TlsFree(TlsThreadPtr);
            DeleteCriticalSection(&openal_crst);
            if(LogFile != stderr)
                fclose(LogFile);
            LogFile = stderr;
        }
        break;
    }
    return TRUE;
}

#ifdef __WINESRC__
/***********************************************************************
 *              DllRegisterServer (DSOUND.@)
 */
HRESULT WINAPI DllRegisterServer(void)
{
    return __wine_register_resources(instance);
}

/***********************************************************************
 *              DllUnregisterServer (DSOUND.@)
 */
HRESULT WINAPI DllUnregisterServer(void)
{
    return __wine_unregister_resources(instance);
}
#endif