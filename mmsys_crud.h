#include <windows.h>

#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <devpropdef.h>

DEFINE_DEVPROPKEY(DEVPKEY_Device_FriendlyName, 0xa45c254e,0xdf1c,0x4efd,0x80,0x20,0x67,0xd1,0x46,0xa8,0x50,0xe0, 14);

typedef BOOL (CALLBACK *PRVTENUMCALLBACK)(EDataFlow flow, LPGUID guid, LPCWSTR descW, LPCWSTR modW, LPVOID data);
HRESULT enumerate_mmdevices(EDataFlow flow, PRVTENUMCALLBACK cb, void *user);