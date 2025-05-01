// UVCXUApp.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <vidcap.h>
#ifndef DSHOW_ONLY
#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#endif
#include <vector>
#include <string>
#include <initguid.h>
#include <uuids.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "UVCXUApp.h"
#include "getopt.h"

#ifdef DSHOW_ONLY
#define MaxVideoDeviceNum 20
#include <atlconv.h>
#include <algorithm>
#pragma comment(lib,"strmbase.lib")

IBaseFilter *pVideoSource = NULL;
LPTSTR FriendlyName[MaxVideoDeviceNum];

void lptstr2str(LPTSTR tch, char* &pch)
{
#ifndef UNICODE
    std::memcpy(pch, tch, strlen(tch) + 1);
#else
    size_t n =
        sizeof(TCHAR) / sizeof(char)* wcsnlen(tch, std::string::npos);
    pch = new char[n + 1];
    std::memcpy(pch, tch, n + 1);
    int len = n - std::count(pch, pch + n, NULL);
    std::remove(pch, pch + n, NULL);
    pch[len] = NULL;
#endif
}
#endif

#ifndef DSHOW_ONLY
//Media foundation and DSHOW specific structures, class and variables
IMFMediaSource *pVideoSource = NULL;
IMFAttributes *pVideoConfig = NULL;
IMFActivate **ppVideoDevices = NULL;
IMFSourceReader *pVideoReader = NULL;
WCHAR *szFriendlyName = NULL;
#endif

//Other variables
UINT32 noOfVideoDevices = 0;
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

#define CAMERA_NAME "TinyUSB UVC"

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

static const char *shortopts = "d";

static struct option long_options[] = {
    { "help",   no_argument,       NULL, '?' },
    { "g1",   required_argument, NULL, '1' },
    { "g2", required_argument, NULL, '2' },
    { "g3", required_argument, NULL, '3' },
    { "g4",  required_argument, NULL, '4' },
    { "g5",  required_argument, NULL, '5' },
    { "deviceName",  required_argument, NULL, 'd' },
    { "wdataValue",  required_argument, NULL, 'w' },
    { "xferBytes",  required_argument, NULL, 'x' },
    { "controlSelectors",  required_argument, NULL, 'c' },
    { 0 ,0, 0, 0 }
};
void usage_long_options() {
    printf("Usage: UVCXUApp\n");
    printf("Options:\n");
    printf(" --deviceName STR             specify device name (e.g., USB Camera)\n");
    printf(" --g1 HEX                     XU GUID g1 32bit Hax value\n");
    printf(" --g2 HEX                     XU GUID g2 16bit Hax value\n");
    printf(" --g3 HEX                     XU GUID g3 16bit Hax value\n");
    printf(" --g4 HEX                     XU GUID g4 16bit Hax value\n");
    printf(" --g5 HEX                     XU GUID g5 48bit Hax value\n");
    printf(" --wdataValue HEX             Write 64bit data to wdataByesArray\n");
    printf(" --xferBytes Decimal          xfer Bytes (1 ~ 64)\n");
    printf(" --controlSelectors Decimal   control Selectors (1 ~ 32)\n");
    printf(" --help                       help message\n");
    printf("\n");
    printf("Example USB Camera XU GUID {01234567-89AB-CDEF-1122-334455667788}:\n");
    printf(" Write 4 Bytes\n");
    printf("  UVCXUApp --deviceName=\"USB Camera\"\n"
           "   --controlSelectors=1\n"
           "   --xferBytes=4\n"
           "   --wdataValue=0x55AA7788\n"
           "   --g1=0x01234567\n"
           "   --g2=0x89AB\n"
           "   --g3=0xCDEF\n"
           "   --g4=0x1122\n"
           "   --g5=0x334455667788\n");
    printf(" Read 4 Bytes\n");
    printf("  UVCXUApp --deviceName=\"USB Camera\"\n"
           "   --controlSelectors=1\n"
           "   --xferBytes=4\n"
           "   --g1=0x01234567\n"
           "   --g2=0x89AB\n"
           "   --g3=0xCDEF\n"
           "   --g4=0x1122\n"
           "   --g5=0x334455667788\n");
}

int main(int argc, char **argv)
{
    static char *deviceName = CAMERA_NAME;
    static uint32_t g1 = 0x00000000;
    static uint16_t g2 = 0x0000;
    static uint16_t g3 = 0x0000;
    static uint16_t g4 = 0x0000;
    static uint64_t g5 = 0x000000000000;
    static uint64_t wdataValue = 0;
    static bool toWrite = false;
    static uint8_t xferBytes = 0;
    static uint8_t controlSelectors = 1;
    int opt;
    UINT32 ExtensionNode = 0xFFFFFFFF;
    UINT32 selectedVal = 0xFFFFFFFF;

    while ((opt = getopt_long(argc, argv, "", long_options, NULL)) != EOF)
    {
        //printf("proces index:%d\n", optind);
        //printf("option arg:%s\n", optarg);
        //printf("opt:%s\n", opt);

        switch (opt)
        {
        case 'd':
            deviceName = optarg;
            break;
        case '1':
            g1 = strtoul(optarg, NULL, 16);
            break;
        case '2':
            g2 = strtoul(optarg, NULL, 16);
            break;
        case '3':
            g3 = strtoul(optarg, NULL, 16);
            break;
        case '4':
            g4 = strtoul(optarg, NULL, 16);
            break;
        case '5':
            g5 = strtoull(optarg, NULL, 16);
            break;
        case 'w':
            wdataValue = strtoull(optarg, NULL, 16);
            toWrite = true;
            break;
        case 'x':
            xferBytes = strtol(optarg, NULL, 10);
            break;
        case 'c':
            controlSelectors = strtol(optarg, NULL, 10);
            break;
        case '?':
        default:
            usage_long_options();
            /* NOTREACHED */
            return 0;
            //break;
        }
    }

    static const GUID xuAppGuid = {
        g1,
        g2,
        g3,
        { 
            (uint8_t)(g4 >>  8), (uint8_t)(g4 >>  0),
            (uint8_t)(g5 >> 40), (uint8_t)(g5 >> 32),
            (uint8_t)(g5 >> 24), (uint8_t)(g5 >> 16),
            (uint8_t)(g5 >>  8), (uint8_t)(g5 >>  0),
        } 
    };

	CHAR enteredStr[MAX_PATH], videoDevName[20][MAX_PATH];
	ULONG flags, bytesReturned;
	size_t returnValue;
    BYTE rdataByesArray[64] = { 0 };
    BYTE wdataByesArray[64] = {
        (uint8_t)(wdataValue >> 0),
        (uint8_t)(wdataValue >> 8),
        (uint8_t)(wdataValue >> 16),
        (uint8_t)(wdataValue >> 24),
        (uint8_t)(wdataValue >> 32),
        (uint8_t)(wdataValue >> 40),
        (uint8_t)(wdataValue >> 48),
        (uint8_t)(wdataValue >> 56),
    };

    if (xferBytes > sizeof(wdataByesArray)) { 
        xferBytes = sizeof(wdataByesArray);
    }
    if (xferBytes < 0) {
        xferBytes = 1;
    }
    if (controlSelectors < 1) {
        controlSelectors = 1;
    }
    if (controlSelectors > 32) {
        controlSelectors = 32;
    }

    printf("deviceName %s\n", deviceName);
    printf("GUID {%08X-%04X-%04X-%04X-%012llX}\n", g1, g2, g3, g4, g5);
    if (toWrite) {
        printf("wdataValue 0x%llX\n", wdataValue);
    }
    printf("xferBytes %d\n", xferBytes);
    printf("controlSelectors %d\n", controlSelectors);

    //Get all video devices
    GetVideoDevices();
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    printf("Video Devices connected:\n");
    for (UINT32 i = 0; i < noOfVideoDevices; i++)
    {
        //Get the device names
#ifdef DSHOW_ONLY
        char* strFriendlyName;
        lptstr2str(FriendlyName[i], strFriendlyName);
        printf("%d: %s\n", i, strFriendlyName);

        if (!(strcmp(strFriendlyName, deviceName)))
            selectedVal = i;
#else
        GetVideoDeviceFriendlyNames(i);
        wcstombs_s(&returnValue, videoDevName[i], MAX_PATH, szFriendlyName, MAX_PATH);
        printf("%d: %s\n", i, videoDevName[i]);

        if (!(strcmp(videoDevName[i], deviceName)))
            selectedVal = i;
#endif
    }

    //Find to UVC extension unit
    if (selectedVal != 0xFFFFFFFF)
    {
        printf("\nFound %s device\n", deviceName);

        //Initialize the selected device
        InitVideoDevice(selectedVal);

        if (toWrite)
            flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
        else
            flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;

        printf("\nTrying to invoke UVC extension unit...\n");

        //Find Extension Node
        DWORD numNodes = 0;
        GUID guidNodeType;
        ULONG bytesReturned = 0;
        IKsTopologyInfo *pKsTopologyInfo;
        KSP_NODE kspNode;
        IKsControl *ks_control;

        HRESULT hr = pVideoSource->QueryInterface(__uuidof(IKsTopologyInfo), (void **)&pKsTopologyInfo);

        pKsTopologyInfo->get_NumNodes(&numNodes);
        printf("get_NumNodes %d\n", numNodes);

        for (UINT32 ulNode = 0; ulNode< numNodes; ulNode++)
        {
            pKsTopologyInfo->get_NodeType(ulNode, &guidNodeType);

            if (IsEqualGUID(guidNodeType, KSNODETYPE_DEV_SPECIFIC))
            {
                ExtensionNode= ulNode;

                if ((hr = pKsTopologyInfo->CreateNodeInstance(ulNode, IID_IKsControl, (void **)&ks_control)) == S_OK)
                {
                    kspNode.Property.Set = xuAppGuid;
                    kspNode.Property.Id = KSPROPERTY_EXTENSION_UNIT_INFO; //0x00
                    kspNode.Property.Flags = KSPROPERTY_TYPE_SETSUPPORT | KSPROPERTY_TYPE_TOPOLOGY;
                    kspNode.NodeId = ulNode;
                    kspNode.Reserved = 0;

                    if ((hr = ((IKsControl *)ks_control)->KsProperty((PKSPROPERTY)&kspNode, sizeof(kspNode), NULL, 0, &bytesReturned)) != S_OK)
                    {
                        ks_control->Release();
                        continue;
                    }

                    SAFE_RELEASE(ks_control);
                    break;
                }
            }
        }

        SAFE_RELEASE(pKsTopologyInfo);

        if (ExtensionNode != 0xFFFFFFFF) {
            printf("Find GUID ExtensionNode = %d\n", ExtensionNode);
        }
        else {
            ExtensionNode = 0;
            printf("Did not find GUID ExtensionNode. Try ExtensionNode = %d\n", ExtensionNode);
        }

        if (toWrite) {
            printf("Set Control (%d)\n", controlSelectors);

            HRESULT hr = SetGetExtensionUnit(xuAppGuid, ExtensionNode, controlSelectors, flags, (void*)wdataByesArray, xferBytes, &bytesReturned);
            if (hr == S_OK) {
                printf("xferBytes %d bytesReturned %d\n", xferBytes, bytesReturned);

                for (int i = 0; i < xferBytes; i++)
                    printf("w[%d] 0x%02X\n", i, wdataByesArray[i]);
            }
            printf("hr = 0x%08X\n", hr);
        }
        else
        {
            printf("Get Control (%d)\n", controlSelectors);
            HRESULT hr = SetGetExtensionUnit(xuAppGuid, ExtensionNode, controlSelectors, flags, (void*)rdataByesArray, xferBytes, &bytesReturned);
            if (hr == S_OK) {
                printf("xferBytes %d bytesReturned %d\n", xferBytes, bytesReturned);

                for (int i = 0; i < bytesReturned; i++)
                    printf("r[%d] 0x%02X\n", i, rdataByesArray[i]);
            }
            printf("hr = 0x%08X\n", hr);
        }
    }
    else {
        printf("\nDid not find %s device\n\n", deviceName);
    }

    //Release all the video device resources
#ifdef DSHOW_ONLY
    SAFE_RELEASE(pVideoSource);
#else
    for (UINT32 i = 0; i < noOfVideoDevices; i++)
    {
       SafeRelease(&ppVideoDevices[i]);
    }

    CoTaskMemFree(ppVideoDevices);
    SafeRelease(&pVideoConfig);
    SafeRelease(&pVideoSource);
    CoTaskMemFree(szFriendlyName);
#endif

    CoUninitialize();
    printf("\nExiting App in 10 msec...\n");
    Sleep(10);

    return 0;
}

//Function to get UVC video devices
#ifdef DSHOW_ONLY
HRESULT GetVideoDevices(void)
{
    HRESULT hr;
    IBaseFilter *pSrc = NULL;
    IMoniker *pMoniker = NULL;
    ULONG cFetched;
    noOfVideoDevices = 0;
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    // Create the system device enumerator
    ICreateDevEnum *pDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void**)&pDevEnum);
    if (FAILED(hr))
    {
        printf("Couldn't create system enumerator!  hr=0x%x", hr);
        return hr;
    }

    // Create an enumerator for the video capture devices
    IEnumMoniker *pClassEnum = NULL;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr))
    {
        pDevEnum->Release();
        printf("Couldn't create class enumerator!  hr=0x%x", hr);
        return hr;
    }

    // If there are no enumerators for the requested type, then 
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    if (pClassEnum == NULL)
    {
        printf("No video capture device was detected.");
        return E_FAIL;
    }

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.3
    while ((pClassEnum->Next(1, &pMoniker, &cFetched)) == S_OK)
    {
        IPropertyBag *pPropBag;
        HRESULT hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);

        if (SUCCEEDED(hr))
        {
            VARIANT varName;
            LPOLESTR strName = NULL;
            VariantInit(&varName);
            hr = pPropBag->Read(L"FriendlyName", &varName, 0);
            if (SUCCEEDED(hr))
            {
                // Display the name in your UI somehow.
                hr = pMoniker->GetDisplayName(NULL, NULL, &strName);

                USES_CONVERSION;
                //LPTSTR DevicePath = OLE2T(strName);
                //LPTSTR FriendlyName = W2T(varName.bstrVal);
                if (noOfVideoDevices < MaxVideoDeviceNum) {
                    FriendlyName[noOfVideoDevices] = W2T(varName.bstrVal);
                    noOfVideoDevices++;
                }
            }
            VariantClear(&varName);
            pPropBag->Release();
        }
        pMoniker->Release();
    }
    pClassEnum->Release();
    pDevEnum->Release();
    return hr;
}
#else
HRESULT GetVideoDevices()
{
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    MFStartup(MF_VERSION);

    // Create an attribute store to specify the enumeration parameters.
    HRESULT hr = MFCreateAttributes(&pVideoConfig, 1);
    CHECK_HR_RESULT(hr, "Create attribute store");

    // Source type: video capture devices
    hr = pVideoConfig->SetGUID(
      MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
      MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    );
    CHECK_HR_RESULT(hr, "Video capture device SetGUID");

    // Enumerate devices.
    hr = MFEnumDeviceSources(pVideoConfig, &ppVideoDevices, &noOfVideoDevices);
    CHECK_HR_RESULT(hr, "Device enumeration");

done:
	return hr;
}

//Function to get device friendly name
HRESULT GetVideoDeviceFriendlyNames(int deviceIndex)
{
    // Get the the device friendly name.
    UINT32 cchName;

    HRESULT hr = ppVideoDevices[deviceIndex]->GetAllocatedString(
        MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
        &szFriendlyName, &cchName);
    CHECK_HR_RESULT(hr, "Get video device friendly name");

done:
    return hr;
}
#endif

//Function to initialize video device
#ifdef DSHOW_ONLY
HRESULT InitVideoDevice(int deviceIndex)
{
    HRESULT hr;
    ICreateDevEnum *pDevEnum = NULL;
    IEnumMoniker *pClassEnum = NULL;
    IMoniker *pMoniker = NULL;

    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void**)&pDevEnum);
    CHECK_HR_RESULT(hr, "CoCreateInstance");

    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    CHECK_HR_RESULT(hr, "CreateClassEnumerator");

    ULONG cFetched;

    for (int i = 0; i <= deviceIndex; i++)
    {
        hr = pClassEnum->Next(1, &pMoniker, &cFetched);
        CHECK_HR_RESULT(hr, "pClassEnum->Next");
    }

    hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pVideoSource);
    pMoniker->Release();
    CHECK_HR_RESULT(hr, "pMoniker->BindToObject");

done:
    return hr;
}
#else
HRESULT InitVideoDevice(int deviceIndex)
{
    HRESULT hr = ppVideoDevices[deviceIndex]->ActivateObject(IID_PPV_ARGS(&pVideoSource));
    CHECK_HR_RESULT(hr, "Activating video device");

    // Create a source reader.
    hr = MFCreateSourceReaderFromMediaSource(pVideoSource, pVideoConfig, &pVideoReader);
    CHECK_HR_RESULT(hr, "Creating video source reader");

done:
    return hr;
}
#endif

//Function to set/get parameters of UVC extension unit
HRESULT SetGetExtensionUnit(GUID xuGuid, DWORD dwExtensionNode, ULONG xuPropertyId, ULONG flags, void *data, int len, ULONG *bytesReturned)
{
    KSP_NODE kspNode;
#if 0
    GUID pNodeType;
    IUnknown *unKnown;
    IKsControl * ks_control = NULL;
    IKsTopologyInfo * pKsTopologyInfo = NULL;

    HRESULT hr = pVideoSource->QueryInterface(__uuidof(IKsTopologyInfo), (void **)&pKsTopologyInfo);
    CHECK_HR_RESULT(hr, "IMFMediaSource::QueryInterface(IKsTopologyInfo)");

    hr = pKsTopologyInfo->get_NodeType(dwExtensionNode, &pNodeType);
    CHECK_HR_RESULT(hr, "IKsTopologyInfo->get_NodeType(...)");

    hr = pKsTopologyInfo->CreateNodeInstance(dwExtensionNode, IID_IUnknown, (LPVOID *)&unKnown);
    CHECK_HR_RESULT(hr, "ks_topology_info->CreateNodeInstance(...)");

    hr = unKnown->QueryInterface(__uuidof(IKsControl), (void **)&ks_control);
    CHECK_HR_RESULT(hr, "ks_topology_info->QueryInterface(...)");
#else
    IKsControl * ks_control = NULL;

    HRESULT hr = pVideoSource->QueryInterface(__uuidof(IKsControl), (void **)&ks_control);
    CHECK_HR_RESULT(hr, "pVideoSource->QueryInterface(ks_control)");
#endif

    kspNode.Property.Set = xuGuid;             // XU GUID
    kspNode.NodeId = (ULONG)dwExtensionNode;   // XU Node ID
    kspNode.Property.Id = xuPropertyId;        // XU control ID
    kspNode.Property.Flags = flags;            // Set/Get request

    hr = ks_control->KsProperty((PKSPROPERTY)&kspNode, sizeof(kspNode), (PVOID)data, len, bytesReturned);
    CHECK_HR_RESULT(hr, "ks_control->KsProperty(...)");

done:
    SafeRelease(&ks_control);
    return hr;
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
        CoUninitialize();
        return TRUE;

    default:
        return FALSE;
    }
}