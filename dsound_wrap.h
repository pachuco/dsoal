#ifndef __DSOUND_WRAP_INCLUDED__
#define __DSOUND_WRAP_INCLUDED__

#ifdef DS_STATIC_PROXY
#define DirectSoundCreate            fnDirectSoundCreate
#define DirectSoundEnumerateA        fnDirectSoundEnumerateA
#define DirectSoundEnumerateW        fnDirectSoundEnumerateW
#define DirectSoundCaptureCreate     fnDirectSoundCaptureCreate
#define DirectSoundCaptureEnumerateA fnDirectSoundCaptureEnumerateA
#define DirectSoundCaptureEnumerateW fnDirectSoundCaptureEnumerateW
#define GetDeviceID                  fnGetDeviceID
#define DirectSoundFullDuplexCreate  fnDirectSoundFullDuplexCreate
#define DirectSoundCreate8           fnDirectSoundCreate8
#define DirectSoundCaptureCreate8    fnDirectSoundCaptureCreate8
HRESULT WINAPI fnDirectSoundDllCanUnloadNow(void);
HRESULT WINAPI fnDirectSoundDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
#endif

#include <dsound.h>


typedef HRESULT (WINAPI *LPDIRECTSOUNDCREATE)           (LPCGUID lpcGUID, IDirectSound **ppDS, IUnknown *pUnkOuter);
typedef HRESULT (WINAPI *LPDIRECTSOUNDENUMERATEA)       (LPDSENUMCALLBACKA lpDSEnumCallback, LPVOID lpContext);
typedef HRESULT (WINAPI *LPDIRECTSOUNDENUMERATEW)       (LPDSENUMCALLBACKW lpDSEnumCallback, LPVOID lpContext);
typedef HRESULT (WINAPI *LPDIRECTSOUNDCAPTURECREATE)    (LPCGUID lpcGUID, IDirectSoundCapture **ppDSC, IUnknown *pUnkOuter);
typedef HRESULT (WINAPI *LPDIRECTSOUNDCAPTUREENUMERATEA)(LPDSENUMCALLBACKA lpDSEnumCallback, LPVOID lpContext);
typedef HRESULT (WINAPI *LPDIRECTSOUNDCAPTUREENUMERATEW)(LPDSENUMCALLBACKW lpDSEnumCallback, LPVOID lpContext);
typedef HRESULT (WINAPI *LPGETDEVICEID)                 (LPCGUID pGuidSrc, LPGUID pGuidDest);
typedef HRESULT (WINAPI *LPDIRECTSOUNDFULLDUPLEXCREATE) (LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice, LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd, DWORD dwLevel, IDirectSoundFullDuplex **ppDSFD, IDirectSoundCaptureBuffer8 **ppDSCBuffer8, IDirectSoundBuffer8 **ppDSBuffer8, IUnknown *pUnkOuter);
typedef HRESULT (WINAPI *LPDIRECTSOUNDCREATE8)          (LPCGUID lpcGUID, IDirectSound8 **ppDS, IUnknown *pUnkOuter);
typedef HRESULT (WINAPI *LPDIRECTSOUNDCAPTURECREATE8)   (LPCGUID lpcGUID, IDirectSoundCapture8 **ppDSC8, IUnknown *pUnkOuter);


typedef HRESULT (WINAPI *LPDLLCANUNLOADNOW)             (void);
typedef HRESULT (WINAPI *LPDLLGETCLASSOBJECT)           (REFCLSID rclsid, REFIID riid, LPVOID *ppv);

#endif