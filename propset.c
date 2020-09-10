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
 */

#define CONST_VTABLE
#include <stdarg.h>
#include <string.h>

#include <windows.h>
#include "dsound_wrap.h"
#include <dsconf.h>

#include "dsound_private.h"

static WCHAR *strdupW(const WCHAR *str)
{
    WCHAR *ret;
    int l = lstrlenW(str);
    if(l < 0) return NULL;

    ret = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l+1)*sizeof(WCHAR));
    if(!ret) return NULL;

    memcpy(ret, str, l*sizeof(WCHAR));
    return ret;
}


typedef struct IKsPrivatePropertySetImpl
{
    IKsPropertySet IKsPropertySet_iface;
    LONG ref;
} IKsPrivatePropertySetImpl;

static IKsPrivatePropertySetImpl *impl_from_IKsPropertySet(IKsPropertySet *iface)
{
    return CONTAINING_RECORD(iface, IKsPrivatePropertySetImpl, IKsPropertySet_iface);
}

/*******************************************************************************
 *              IKsPrivatePropertySet
 */

/* IUnknown methods */
static HRESULT WINAPI IKsPrivatePropertySetImpl_QueryInterface(
        IKsPropertySet *iface, REFIID riid, void **ppobj)
{
    IKsPrivatePropertySetImpl *This = impl_from_IKsPropertySet(iface);
    TRACE("(%p,%s,%p)\n",This,debugstr_guid(riid),ppobj);

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IKsPropertySet)) {
        *ppobj = iface;
        IKsPropertySet_AddRef(iface);
        return S_OK;
    }
    *ppobj = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI IKsPrivatePropertySetImpl_AddRef(LPKSPROPERTYSET iface)
{
    IKsPrivatePropertySetImpl *This = impl_from_IKsPropertySet(iface);
    ULONG ref = InterlockedIncrement(&(This->ref));
    TRACE("(%p) ref %lu\n", iface, ref);
    return ref;
}

static ULONG WINAPI IKsPrivatePropertySetImpl_Release(LPKSPROPERTYSET iface)
{
    IKsPrivatePropertySetImpl *This = impl_from_IKsPropertySet(iface);
    ULONG ref = InterlockedDecrement(&(This->ref));
    TRACE("(%p) ref %lu\n", iface, ref);

    if(!ref)
    {
        HeapFree(GetProcessHeap(), 0, This);
        TRACE("(%p) released\n", This);
    }
    return ref;
}

typedef struct {
    BOOL isFound;
    LPGUID foundGuid;
    const WCHAR* wantedName;
} UserDatSearchByNameW;

static BOOL CALLBACK cbSearchByName(LPGUID lpGuid, LPCWSTR lpcstrDescription, LPCWSTR lpcstrModule, LPVOID lpContext)
{
    UserDatSearchByNameW* user = lpContext;
    (void)lpcstrModule;
    
    user->isFound = FALSE;
    if(lstrcmpW(lpcstrDescription, user->wantedName) != 0)
        return TRUE;
    
    user->isFound = TRUE;
    *user->foundGuid = *lpGuid;
    return FALSE;
}

static HRESULT DSPROPERTY_WaveDeviceMappingW(
    LPVOID pPropData,
    ULONG cbPropData,
    PULONG pcbReturned )
{
    HRESULT hr;
    PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA ppd = pPropData;
    UserDatSearchByNameW user = {0};

    TRACE("(pPropData=%p,cbPropData=%ld,pcbReturned=%p)\n",
          pPropData,cbPropData,pcbReturned);

    if (!ppd)
    {
        WARN("invalid parameter: pPropData\n");
        return DSERR_INVALIDPARAM;
    }

    user.wantedName = ppd->DeviceName;
    user.foundGuid  = &ppd->DeviceId;

    if (ppd->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_RENDER)
        hr = pDirectSoundEnumerateW(&cbSearchByName, &user);
    else if (ppd->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE)
        hr = pDirectSoundCaptureEnumerateW(&cbSearchByName, &user);
    else
        return DSERR_INVALIDPARAM;

    if(FAILED(hr) || !user.isFound)
        /* device was not found */
        return DSERR_INVALIDPARAM;

    if (pcbReturned)
        *pcbReturned = cbPropData;

    return DS_OK;
}

static HRESULT DSPROPERTY_WaveDeviceMappingA(
    LPVOID pPropData,
    ULONG cbPropData,
    PULONG pcbReturned )
{
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA *ppd = pPropData;
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA data;
    DWORD len;
    HRESULT hr;

    TRACE("(pPropData=%p,cbPropData=%ld,pcbReturned=%p)\n",
      pPropData,cbPropData,pcbReturned);

    if (!ppd || !ppd->DeviceName) {
        WARN("invalid parameter: ppd=%p\n", ppd);
        return DSERR_INVALIDPARAM;
    }

    data.DataFlow = ppd->DataFlow;
    len = MultiByteToWideChar(CP_ACP, 0, ppd->DeviceName, -1, NULL, 0);
    data.DeviceName = HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
    if (!data.DeviceName)
        return E_OUTOFMEMORY;
    MultiByteToWideChar(CP_ACP, 0, ppd->DeviceName, -1, data.DeviceName, len);

    hr = DSPROPERTY_WaveDeviceMappingW(&data, cbPropData, pcbReturned);
    HeapFree(GetProcessHeap(), 0, data.DeviceName);
    ppd->DeviceId = data.DeviceId;

    if (pcbReturned)
        *pcbReturned = cbPropData;

    return hr;
}

typedef struct {
    BOOL isFound;
    LPGUID wantedGuid;
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA ppd;
} UserDatSearchByGuidW;

static BOOL CALLBACK cbSearchByGUID(LPGUID lpGuid, LPCWSTR lpcstrDescription, LPCWSTR lpcstrModule, LPVOID lpContext)
{
    UserDatSearchByGuidW* user = lpContext;
    (void)lpcstrModule;
    
    user->isFound = FALSE;
    if (IsEqualGUID(user->wantedGuid, lpGuid)) {
        user->isFound = TRUE;
        user->ppd->Description = strdupW(lpcstrDescription);
        return FALSE;
    }
    
    return TRUE;
}

static HRESULT DSPROPERTY_DescriptionW(
    LPVOID pPropData,
    ULONG cbPropData,
    PULONG pcbReturned )
{
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA ppd = pPropData;
    GUID dev_guid;
    UserDatSearchByGuidW user;
    HRESULT hr;

    TRACE("pPropData=%p,cbPropData=%ld,pcbReturned=%p)\n",
          pPropData,cbPropData,pcbReturned);

    TRACE("DeviceId=%s\n",debugstr_guid(&ppd->DeviceId));
    if ( IsEqualGUID( &ppd->DeviceId , &GUID_NULL) ) {
        /* default device of type specified by ppd->DataFlow */
        if (ppd->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE) {
            TRACE("DataFlow=DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE\n");
            ppd->DeviceId = DSDEVID_DefaultCapture;
        } else if (ppd->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_RENDER) {
            TRACE("DataFlow=DIRECTSOUNDDEVICE_DATAFLOW_RENDER\n");
            ppd->DeviceId = DSDEVID_DefaultPlayback;
        } else {
            WARN("DataFlow=Unknown(%d)\n", ppd->DataFlow);
            return E_PROP_ID_UNSUPPORTED;
        }
    }

    pGetDeviceID(&ppd->DeviceId, &dev_guid);
    
    user.wantedGuid = &dev_guid;
    hr = pDirectSoundEnumerateW(&cbSearchByGUID, &user);
    if (FAILED(hr) || !user.isFound) {
        hr = pDirectSoundCaptureEnumerateW(&cbSearchByGUID, &user);
        if (FAILED(hr) || !user.isFound) {
            return DSERR_INVALIDPARAM;
        }
    }
    
    ppd->Module = strdupW(aldriver_name);
    ppd->Interface = strdupW(L"Interface");
    ppd->Type = DIRECTSOUNDDEVICE_TYPE_WDM;

    if (pcbReturned)
    {
        *pcbReturned = sizeof(*ppd);
        TRACE("*pcbReturned=%ld\n", *pcbReturned);
    }

    return S_OK;
}

typedef struct {
    LPGUID wantedGuid;
    PDSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W_DATA ppd;
    DIRECTSOUNDDEVICE_DATAFLOW DataFlow;
} UserDatEnumerateW;

static BOOL CALLBACK cbEnumerate(LPGUID lpGuid, LPCWSTR lpcstrDescription, LPCWSTR lpcstrModule, LPVOID lpContext)
{
    UserDatEnumerateW* user = lpContext;
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA data;
    DWORD len;
    BOOL ret;
    
    TRACE("%s %ls %ls %p\n", debugstr_guid(lpGuid), lpcstrDescription, lpcstrModule, user->ppd);
    
    if (!lpGuid) return TRUE;
    
    data.Type = DIRECTSOUNDDEVICE_TYPE_WDM;
    data.DataFlow = user->DataFlow;
    data.DeviceId = *lpGuid;
    
    len = lstrlenW(lpcstrModule) + 1;
    data.Module = HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
    memcpy(data.Module, lpcstrModule, len * sizeof(WCHAR));

    len = lstrlenW(lpcstrDescription) + 1;
    data.Description = HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
    memcpy(data.Description, lpcstrDescription, len * sizeof(WCHAR));
    
    data.Interface = L"Interface";
    
    ret = user->ppd->Callback(&data, user->ppd->Context);

    HeapFree(GetProcessHeap(), 0, data.Module);
    HeapFree(GetProcessHeap(), 0, data.Description);

    return ret;
}

static HRESULT DSPROPERTY_EnumerateW(
    LPVOID pPropData,
    ULONG cbPropData,
    PULONG pcbReturned )
{
    PDSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W_DATA ppd = pPropData;
    UserDatEnumerateW user = {0};
    HRESULT hr;

    TRACE("(pPropData=%p,cbPropData=%ld,pcbReturned=%p)\n",
          pPropData,cbPropData,pcbReturned);

    if (pcbReturned)
        *pcbReturned = 0;

    if (!ppd || !ppd->Callback)
    {
        WARN("Invalid ppd %p\n", ppd);
        return E_PROP_ID_UNSUPPORTED;
    }
    
    user.ppd = ppd;
    user.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
    hr = pDirectSoundEnumerateW(&cbEnumerate, &user);
    if(hr == S_OK) {        
        user.DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
        hr = pDirectSoundCaptureEnumerateW(&cbEnumerate, &user);
    }
    
    return SUCCEEDED(hr) ? DS_OK : hr;
}

static BOOL DSPROPERTY_descWtoA(const DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA *dataW,
                                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA *dataA)
{
    static char Interface[] = "Interface";
    DWORD modlen, desclen;

    modlen = WideCharToMultiByte(CP_ACP, 0, dataW->Module, -1, NULL, 0, NULL, NULL);
    desclen = WideCharToMultiByte(CP_ACP, 0, dataW->Description, -1, NULL, 0, NULL, NULL);
    dataA->Type = dataW->Type;
    dataA->DataFlow = dataW->DataFlow;
    dataA->DeviceId = dataW->DeviceId;
    dataA->WaveDeviceId = dataW->WaveDeviceId;
    dataA->Interface = Interface;
    dataA->Module = HeapAlloc(GetProcessHeap(), 0, modlen);
    dataA->Description = HeapAlloc(GetProcessHeap(), 0, desclen);
    if (!dataA->Module || !dataA->Description)
    {
        HeapFree(GetProcessHeap(), 0, dataA->Module);
        HeapFree(GetProcessHeap(), 0, dataA->Description);
        dataA->Module = dataA->Description = NULL;
        return FALSE;
    }

    WideCharToMultiByte(CP_ACP, 0, dataW->Module, -1, dataA->Module, modlen, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, dataW->Description, -1, dataA->Description, desclen, NULL, NULL);
    return TRUE;
}

static void DSPROPERTY_descWto1(const DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA *dataW,
                                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA *data1)
{
    data1->DeviceId = dataW->DeviceId;
    lstrcpynW(data1->ModuleW, dataW->Module, sizeof(data1->ModuleW)/sizeof(*data1->ModuleW));
    lstrcpynW(data1->DescriptionW, dataW->Description, sizeof(data1->DescriptionW)/sizeof(*data1->DescriptionW));
    WideCharToMultiByte(CP_ACP, 0, data1->DescriptionW, -1, data1->DescriptionA, sizeof(data1->DescriptionA)-1, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, data1->ModuleW, -1, data1->ModuleA, sizeof(data1->ModuleA)-1, NULL, NULL);
    data1->DescriptionA[sizeof(data1->DescriptionA)-1] = 0;
    data1->ModuleA[sizeof(data1->ModuleA)-1] = 0;
    data1->Type = dataW->Type;
    data1->DataFlow = dataW->DataFlow;
    data1->WaveDeviceId = data1->Devnode = dataW->WaveDeviceId;
}

static BOOL CALLBACK DSPROPERTY_enumWtoA(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA *descW, void *data)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA descA;
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A_DATA *ppd = data;
    BOOL ret;

    ret = DSPROPERTY_descWtoA(descW, &descA);
    if (!ret)
        return FALSE;
    ret = ppd->Callback(&descA, ppd->Context);
    HeapFree(GetProcessHeap(), 0, descA.Module);
    HeapFree(GetProcessHeap(), 0, descA.Description);
    return ret;
}

static HRESULT DSPROPERTY_EnumerateA(
    LPVOID pPropData,
    ULONG cbPropData,
    PULONG pcbReturned)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A_DATA *ppd = pPropData;
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W_DATA data;

    if (!ppd || !ppd->Callback)
    {
        WARN("Invalid ppd %p\n", ppd);
        return E_PROP_ID_UNSUPPORTED;
    }

    data.Callback = DSPROPERTY_enumWtoA;
    data.Context = ppd;

    return DSPROPERTY_EnumerateW(&data, cbPropData, pcbReturned);
}

static BOOL CALLBACK DSPROPERTY_enumWto1(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA *descW, void *data)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA desc1;
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1_DATA *ppd = data;
    BOOL ret;

    DSPROPERTY_descWto1(descW, &desc1);
    ret = ppd->Callback(&desc1, ppd->Context);
    return ret;
}

static HRESULT DSPROPERTY_Enumerate1(
    LPVOID pPropData,
    ULONG cbPropData,
    PULONG pcbReturned)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1_DATA *ppd = pPropData;
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W_DATA data;

    if (!ppd || !ppd->Callback)
    {
        WARN("Invalid ppd %p\n", ppd);
        return E_PROP_ID_UNSUPPORTED;
    }

    data.Callback = DSPROPERTY_enumWto1;
    data.Context = ppd;

    return DSPROPERTY_EnumerateW(&data, cbPropData, pcbReturned);
}

static HRESULT DSPROPERTY_DescriptionA(
    LPVOID pPropData,
    ULONG cbPropData,
    PULONG pcbReturned)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA data;
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA *ppd = pPropData;
    HRESULT hr;

    if(cbPropData < sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA))
        return E_INVALIDARG;
    if (pcbReturned)
        *pcbReturned = sizeof(*ppd);
    if (!pPropData)
        return S_OK;

    data.DeviceId = ppd->DeviceId;
    data.DataFlow = ppd->DataFlow;
    hr = DSPROPERTY_DescriptionW(&data, sizeof(data), NULL);
    if (FAILED(hr))
        return hr;
    if (!DSPROPERTY_descWtoA(&data, ppd))
        hr = E_OUTOFMEMORY;
    HeapFree(GetProcessHeap(), 0, data.Description);
    HeapFree(GetProcessHeap(), 0, data.Module);
    HeapFree(GetProcessHeap(), 0, data.Interface);
    return hr;
}

static HRESULT DSPROPERTY_Description1(
    LPVOID pPropData,
    ULONG cbPropData,
    PULONG pcbReturned)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA data;
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA *ppd = pPropData;
    HRESULT hr;

    if(cbPropData < sizeof(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA))
        return E_INVALIDARG;
    if (pcbReturned)
        *pcbReturned = sizeof(*ppd);
    if (!pPropData)
        return S_OK;

    data.DeviceId = ppd->DeviceId;
    data.DataFlow = ppd->DataFlow;
    hr = DSPROPERTY_DescriptionW(&data, sizeof(data), NULL);
    if (FAILED(hr))
        return hr;
    DSPROPERTY_descWto1(&data, ppd);
    HeapFree(GetProcessHeap(), 0, data.Description);
    HeapFree(GetProcessHeap(), 0, data.Module);
    HeapFree(GetProcessHeap(), 0, data.Interface);
    return hr;
}

static HRESULT WINAPI IKsPrivatePropertySetImpl_Get(
    LPKSPROPERTYSET iface,
    REFGUID guidPropSet,
    ULONG dwPropID,
    LPVOID pInstanceData,
    ULONG cbInstanceData,
    LPVOID pPropData,
    ULONG cbPropData,
    PULONG pcbReturned )
{
    IKsPrivatePropertySetImpl *This = impl_from_IKsPropertySet(iface);
    TRACE("(iface=%p,guidPropSet=%s,dwPropID=%ld,pInstanceData=%p,cbInstanceData=%ld,pPropData=%p,cbPropData=%ld,pcbReturned=%p)\n",
          This,debugstr_guid(guidPropSet),dwPropID,pInstanceData,cbInstanceData,pPropData,cbPropData,pcbReturned);

    if ( IsEqualGUID( &DSPROPSETID_DirectSoundDevice, guidPropSet) ) {
        switch (dwPropID) {
        case DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A:
            return DSPROPERTY_WaveDeviceMappingA(pPropData,cbPropData,pcbReturned);
        case DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1:
            return DSPROPERTY_Description1(pPropData,cbPropData,pcbReturned);
        case DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1:
            return DSPROPERTY_Enumerate1(pPropData,cbPropData,pcbReturned);
        case DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W:
            return DSPROPERTY_WaveDeviceMappingW(pPropData,cbPropData,pcbReturned);
        case DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A:
            return DSPROPERTY_DescriptionA(pPropData,cbPropData,pcbReturned);
        case DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W:
            return DSPROPERTY_DescriptionW(pPropData,cbPropData,pcbReturned);
        case DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A:
            return DSPROPERTY_EnumerateA(pPropData,cbPropData,pcbReturned);
        case DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W:
            return DSPROPERTY_EnumerateW(pPropData,cbPropData,pcbReturned);
        default:
            FIXME("unsupported ID: %ld\n",dwPropID);
            break;
        }
    } else {
        FIXME("unsupported property: %s\n",debugstr_guid(guidPropSet));
    }

    if (pcbReturned) {
        *pcbReturned = 0;
        FIXME("*pcbReturned=%ld\n", *pcbReturned);
    }

    return E_PROP_ID_UNSUPPORTED;
}

static HRESULT WINAPI IKsPrivatePropertySetImpl_Set(
    LPKSPROPERTYSET iface,
    REFGUID guidPropSet,
    ULONG dwPropID,
    LPVOID pInstanceData,
    ULONG cbInstanceData,
    LPVOID pPropData,
    ULONG cbPropData )
{
    IKsPrivatePropertySetImpl *This = impl_from_IKsPropertySet(iface);

    FIXME("(%p,%s,%ld,%p,%ld,%p,%ld), stub!\n",This,debugstr_guid(guidPropSet),dwPropID,pInstanceData,cbInstanceData,pPropData,cbPropData);
    return E_PROP_ID_UNSUPPORTED;
}

static HRESULT WINAPI IKsPrivatePropertySetImpl_QuerySupport(
    LPKSPROPERTYSET iface,
    REFGUID guidPropSet,
    ULONG dwPropID,
    PULONG pTypeSupport )
{
    IKsPrivatePropertySetImpl *This = impl_from_IKsPropertySet(iface);
    TRACE("(%p,%s,%ld,%p)\n",This,debugstr_guid(guidPropSet),dwPropID,pTypeSupport);

    if ( IsEqualGUID( &DSPROPSETID_DirectSoundDevice, guidPropSet) ) {
        switch (dwPropID) {
        case DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A:
            *pTypeSupport = KSPROPERTY_SUPPORT_GET;
            return S_OK;
        case DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1:
            *pTypeSupport = KSPROPERTY_SUPPORT_GET;
            return S_OK;
        case DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1:
            *pTypeSupport = KSPROPERTY_SUPPORT_GET;
            return S_OK;
        case DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W:
            *pTypeSupport = KSPROPERTY_SUPPORT_GET;
            return S_OK;
        case DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A:
            *pTypeSupport = KSPROPERTY_SUPPORT_GET;
            return S_OK;
        case DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W:
            *pTypeSupport = KSPROPERTY_SUPPORT_GET;
            return S_OK;
        case DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A:
            *pTypeSupport = KSPROPERTY_SUPPORT_GET;
            return S_OK;
        case DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W:
            *pTypeSupport = KSPROPERTY_SUPPORT_GET;
            return S_OK;
        default:
            FIXME("unsupported ID: %ld\n",dwPropID);
            break;
        }
    } else {
        FIXME("unsupported property: %s (propid: %lu)\n",debugstr_guid(guidPropSet),dwPropID);
    }

    return E_PROP_ID_UNSUPPORTED;
}

static IKsPropertySetVtbl ikspvt = {
    IKsPrivatePropertySetImpl_QueryInterface,
    IKsPrivatePropertySetImpl_AddRef,
    IKsPrivatePropertySetImpl_Release,
    IKsPrivatePropertySetImpl_Get,
    IKsPrivatePropertySetImpl_Set,
    IKsPrivatePropertySetImpl_QuerySupport
};

HRESULT IKsPrivatePropertySetImpl_Create(REFIID riid, void **ppv)
{
    IKsPrivatePropertySetImpl *iks;
    HRESULT hr;

    TRACE("(%s, %p)\n", debugstr_guid(riid), ppv);

    iks = HeapAlloc(GetProcessHeap(), 0, sizeof(*iks));
    if (!iks) {
        WARN("out of memory\n");
        return DSERR_OUTOFMEMORY;
    }

    iks->ref = 1;
    iks->IKsPropertySet_iface.lpVtbl = &ikspvt;

    hr = IKsPropertySet_QueryInterface(&iks->IKsPropertySet_iface, riid, ppv);
    IKsPropertySet_Release(&iks->IKsPropertySet_iface);

    return hr;
}
