/*
 * These functions have been forwarded to real DirectSound
 */
 
#define CONST_VTABLE
#include <stdarg.h>

#include <windows.h>
#include "dsound_wrap.h"

#include "dsound_private.h"

HRESULT DSOUND_CaptureCreate8(REFIID riid, void **cap)
{
    IClassFactory* pFact;
    HRESULT ret = pDirectSoundDllGetClassObject(&IID_IClassFactory, riid, (void**)&pFact);
    
    *cap = NULL;
    if (ret == S_OK) {
        ret = IClassFactory_CreateInstance(pFact, NULL, &IID_IDirectSoundCapture8, cap);
        
        IClassFactory_Release(pFact);
    }
    
    return ret;
}

HRESULT DSOUND_CaptureCreate(REFIID riid, void **cap)
{
    IClassFactory* pFact;
    HRESULT ret = pDirectSoundDllGetClassObject(&IID_IClassFactory, riid, (void**)&pFact);
    
    *cap = NULL;
    if (ret == S_OK) {
        ret = IClassFactory_CreateInstance(pFact, NULL, &IID_IDirectSoundCapture, cap);
        
        IClassFactory_Release(pFact);
    }
    
    return ret;
}

/***************************************************************************
 * GetDeviceID	[DSOUND.9]
 *
 * Retrieves unique identifier of default device specified
 *
 * PARAMS
 *    pGuidSrc  [I] Address of device GUID.
 *    pGuidDest [O] Address to receive unique device GUID.
 *
 * RETURNS
 *    Success: DS_OK
 *    Failure: DSERR_INVALIDPARAM
 *
 * NOTES
 *    pGuidSrc is a valid device GUID or DSDEVID_DefaultPlayback,
 *    DSDEVID_DefaultCapture, DSDEVID_DefaultVoicePlayback, or
 *    DSDEVID_DefaultVoiceCapture.
 *    Returns pGuidSrc if pGuidSrc is a valid device or the device
 *    GUID for the specified constants.
 */
HRESULT WINAPI DSOAL_GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest)
{
    return pGetDeviceID(pGuidSrc, pGuidDest);
}

/***************************************************************************
 * DirectSoundEnumerateA [DSOUND.2]
 *
 * Enumerate all DirectSound drivers installed in the system
 *
 * PARAMS
 *    lpDSEnumCallback  [I] Address of callback function.
 *    lpContext         [I] Address of user defined context passed to callback function.
 *
 * RETURNS
 *    Success: DS_OK
 *    Failure: DSERR_INVALIDPARAM
 */
HRESULT WINAPI DSOAL_DirectSoundEnumerateA(
    LPDSENUMCALLBACKA lpDSEnumCallback,
    LPVOID lpContext)
{
    return pDirectSoundEnumerateA(lpDSEnumCallback, lpContext);
}

/***************************************************************************
 * DirectSoundEnumerateW [DSOUND.3]
 *
 * Enumerate all DirectSound drivers installed in the system
 *
 * PARAMS
 *    lpDSEnumCallback  [I] Address of callback function.
 *    lpContext         [I] Address of user defined context passed to callback function.
 *
 * RETURNS
 *    Success: DS_OK
 *    Failure: DSERR_INVALIDPARAM
 */
HRESULT WINAPI DSOAL_DirectSoundEnumerateW(
    LPDSENUMCALLBACKW lpDSEnumCallback,
    LPVOID lpContext )
{
    return pDirectSoundEnumerateW(lpDSEnumCallback, lpContext);
}

/***************************************************************************
 * DirectSoundCaptureEnumerateA [DSOUND.7]
 *
 * Enumerate all DirectSound drivers installed in the system.
 *
 * PARAMS
 *    lpDSEnumCallback  [I] Address of callback function.
 *    lpContext         [I] Address of user defined context passed to callback function.
 *
 * RETURNS
 *    Success: DS_OK
 *    Failure: DSERR_INVALIDPARAM
 */
HRESULT WINAPI DSOAL_DirectSoundCaptureEnumerateA(
    LPDSENUMCALLBACKA lpDSEnumCallback,
    LPVOID lpContext)
{
    return pDirectSoundCaptureEnumerateA(lpDSEnumCallback, lpContext);
}

/***************************************************************************
 * DirectSoundCaptureEnumerateW [DSOUND.8]
 *
 * Enumerate all DirectSound drivers installed in the system.
 *
 * PARAMS
 *    lpDSEnumCallback  [I] Address of callback function.
 *    lpContext         [I] Address of user defined context passed to callback function.
 *
 * RETURNS
 *    Success: DS_OK
 *    Failure: DSERR_INVALIDPARAM
 */
HRESULT WINAPI DSOAL_DirectSoundCaptureEnumerateW(
    LPDSENUMCALLBACKW lpDSEnumCallback,
    LPVOID lpContext)
{
    return pDirectSoundCaptureEnumerateW(lpDSEnumCallback, lpContext);
}

/***************************************************************************
 * DirectSoundCaptureCreate [DSOUND.6]
 *
 * Create and initialize a DirectSoundCapture interface.
 *
 * PARAMS
 *    lpcGUID   [I] Address of the GUID that identifies the sound capture device.
 *    lplpDSC   [O] Address of a variable to receive the interface pointer.
 *    pUnkOuter [I] Must be NULL.
 *
 * RETURNS
 *    Success: DS_OK
 *    Failure: DSERR_NOAGGREGATION, DSERR_ALLOCATED, DSERR_INVALIDPARAM,
 *             DSERR_OUTOFMEMORY
 *
 * NOTES
 *    lpcGUID must be one of the values returned from DirectSoundCaptureEnumerate
 *    or NULL for the default device or DSDEVID_DefaultCapture or
 *    DSDEVID_DefaultVoiceCapture.
 *
 *    DSERR_ALLOCATED is returned for sound devices that do not support full duplex.
 */
HRESULT WINAPI
DSOAL_DirectSoundCaptureCreate(LPCGUID lpcGUID, IDirectSoundCapture **ppDSC, IUnknown *pUnkOuter)
{
    return pDirectSoundCaptureCreate(lpcGUID, ppDSC, pUnkOuter);
}

/***************************************************************************
 * DirectSoundCaptureCreate8 [DSOUND.12]
 *
 * Create and initialize a DirectSoundCapture interface.
 *
 * PARAMS
 *    lpcGUID   [I] Address of the GUID that identifies the sound capture device.
 *    lplpDSC   [O] Address of a variable to receive the interface pointer.
 *    pUnkOuter [I] Must be NULL.
 *
 * RETURNS
 *    Success: DS_OK
 *    Failure: DSERR_NOAGGREGATION, DSERR_ALLOCATED, DSERR_INVALIDPARAM,
 *             DSERR_OUTOFMEMORY
 *
 * NOTES
 *    lpcGUID must be one of the values returned from DirectSoundCaptureEnumerate
 *    or NULL for the default device or DSDEVID_DefaultCapture or
 *    DSDEVID_DefaultVoiceCapture.
 *
 *    DSERR_ALLOCATED is returned for sound devices that do not support full duplex.
 */
HRESULT WINAPI
DSOAL_DirectSoundCaptureCreate8(LPCGUID lpcGUID, IDirectSoundCapture8 **ppDSC8, IUnknown *pUnkOuter)
{
    return pDirectSoundCaptureCreate8(lpcGUID, ppDSC8, pUnkOuter);
}