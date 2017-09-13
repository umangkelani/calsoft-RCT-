/****************************** Module Header ******************************\
Module Name:  CppVhdAPI.cpp
Project:      CppVhdAPI (VHD API demo)
Copyright (c) Microsoft Corporation.

Demonstrates various VHD API usage, such as VHD creation, attaching, 
detaching and getting and setting disk information.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include <windows.h>
#include <stdio.h>
#define DEFIND_GUID
#include <initguid.h>
#include <virtdisk.h>
#pragma comment(lib, "VirtDisk.lib")


#define PHYS_PATH_LEN 1024+1
GUID GUID_TEST = {12345678-1234-5678-1234-000000000000};
GUID zGuid = GUID_NULL;


void PrintErrorMessage(ULONG ErrorId)
{
    PVOID Message = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        ErrorId,
        0,
        (LPWSTR)&Message,
        16,
        NULL);

    wprintf(L"%s\n", Message);
    LocalFree(Message);
}

/*void usage()
{
    printf("CppVhdAPI.exe -[cxaomdgpe] -f <vhdfile> -s <size>\n");
    printf("-c CreateVirtualDisk............input: -f <vhd file name>, -s <size in MB>\n");
    printf("-a AttachVirtualDisk............input: -f <vhd file name>\n");
    printf("-d DetachVirtualDisk............input: -f <vhd file name>\n");
    printf("-g GetVirtualDiskInformation....input: -f <vhd file name>\n");
    printf("-p GetVirtualDiskPhysicalPath...input: -f <vhd file name> -- note: must be attached\n");
    printf("-e SetVirtualDiskInformation....input: -f <vhd file name>, -u <new GUID>\n");
    printf("Examples:\n");
    printf("  Create a 3.6 Gb VHD named 'mytest.vhd'\n");
    printf("CppVhdAPI.exe -c -f c:\\testdir\\mytest.vhd -s 3600\n\n");
    printf("  Attach a VHD named 'mytest.vhd'\n");
    printf("CppVhdAPI.exe -a -f c:\\testdir\\mytest.vhd\n\n");
    printf("  Set VHD GUID 'mytest.vhd'\n");
    printf("CppVhdAPI.exe -e -f c:\\testdir\\mytest.vhd -u {12345678-1234-5678-1234-000000000000}\n");
}
*/
BOOL ValidateActionAndParameters(wchar_t action, PCWSTR pszFilename, ULONG sizeInMb, PCWSTR pszGuid)
{
    HRESULT hr;

    switch (action)
    {
    case L'c':  // CreateVirtualDisk
    case L'x':  // ExpandVirtualDisk
        {
            // Validate file name and size
            return (wcslen(pszFilename) && (sizeInMb));
        }

    case L'e':  // SetVirtualDiskInformation    
        if (wcslen(pszFilename) && wcslen(pszGuid))
        {
            // Validate filename and size
            hr = CLSIDFromString(pszGuid, &zGuid);
            if (SUCCEEDED(hr))
            {
                printf("failed to convert %ws to GUID, err %d\n", pszGuid, GetLastError());
                return FALSE;
            }
            return TRUE;
        } 
        else
        {
            return FALSE;
        }
        break;

    case L'a':  // AttachVirtualDisk            
    case L'o':  // CompactVirtualDisk           
    case L'd':  // DetachVirtualDisk            
    case L'g':  // GetVirtualDiskInformation    
    case L'p':  // GetVirtualDiskPhysicalPath   
    case L'm':  // MergeVirtualDisk             
        { 
            // Validate filename only
            return (wcslen(pszFilename));
        }

    default:
        return FALSE;
    }
    return FALSE;
}

int OpenAndGetVHDInfo(PCWSTR pszVhdPath, PCWSTR pszGuid)
{
    BOOL bRet = FALSE;
    DWORD ret;
    HANDLE hVhd;
    GET_VIRTUAL_DISK_INFO Info;
    ULONG InfoSize;
    ULONG SizeUsed;
    VIRTUAL_STORAGE_TYPE            vst =
    {
        VIRTUAL_STORAGE_TYPE_DEVICE_VHDX,
        VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT
    };

    wprintf(L"OpenAndGetVHDInfo %s\n", pszVhdPath);

    ret = OpenVirtualDisk(&vst, pszVhdPath, 
        VIRTUAL_DISK_ACCESS_ALL, OPEN_VIRTUAL_DISK_FLAG_NONE, 
        NULL, &hVhd);

    if (ERROR_SUCCESS == ret)
    {
        printf("success opening vdisk...\n");
        InfoSize = (ULONG)sizeof(GET_VIRTUAL_DISK_INFO);
        Info.Version = GET_VIRTUAL_DISK_INFO_SIZE;
		//Info.Version = GET_VIRTUAL_DISK_INFO_PARENT_LOCATION;
		//Info.Version = GET_VIRTUAL_DISK_INFO_PHYSICAL_DISK;

        ret = GetVirtualDiskInformation(hVhd,
            &InfoSize,
            &Info,
            &SizeUsed);

		if (ret == ERROR_SUCCESS)
		{
			printf("success getting virtual disk size info\n");
			printf("Info.Size.VirtualSize----------------------->(bytes) = %I64d (dec)\n", Info.Size.VirtualSize);
			printf("Info.Size.PhysicalSize---------------------->(bytes) = %I64d (dec)\n", Info.Size.PhysicalSize);
			printf("Info.Size.BlockSize------------------------->(bytes) = %d (dec)\n", Info.Size.BlockSize);
			printf("Info.Size.SectorSize------------------------>(bytes) = %d (dec)\n", Info.Size.SectorSize);
			bRet = TRUE;

			printf("**************************************************************************************************\n");
		}
		else
		{
			printf("\n1.failed to get virtual disk size info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}
			//Info.Identifier= GET_VIRTUAL_DISK_INFO_IDENTIFIER;
		Info.Version = GET_VIRTUAL_DISK_INFO_PARENT_LOCATION;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n2.success getting virtual disk ParentLocation info\n");
			printf("Info.ParentLocation.ParentLocationBuffer--->(wchar) = %s (dec)\n", Info.ParentLocation.ParentLocationBuffer);
			printf("Info.ParentLocation.ParentResolved--------->(bool ) = %d (dec)\n", Info.ParentLocation.ParentResolved);
		}
		else
		{
			printf("\n2.failed to get virtual disk parent location info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}


		Info.Version = GET_VIRTUAL_DISK_INFO_PHYSICAL_DISK;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n3.success getting virtual disk PhysicalDisk info\n");

			printf("Info.PhysicalDisk.IsRemote----------------->(bool ) = %d (dec)\n", Info.PhysicalDisk.IsRemote);
			printf("Info.PhysicalDisk.LogicalSectorSize-------->(bytes) = %d (dec)\n", Info.PhysicalDisk.LogicalSectorSize);
			printf("Info.PhysicalDisk.PhysicalSectorSize------->(bytes) = %d (dec)\n", Info.PhysicalDisk.PhysicalSectorSize);
		}
		else
		{
			printf("\n3.failed to get virtual disk phyical disk info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}


		Info.Version = GET_VIRTUAL_DISK_INFO_CHANGE_TRACKING_STATE;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n4.success getting virtual disk ChangeTrackingState info\n");
			printf("Info.ChangeTrackingState.Enabled----------->(bool ) = %d (dec)\n", Info.ChangeTrackingState.Enabled);
			printf("Info.ChangeTrackingState.MostRecentId------>(wchar) = %s (dec)\n", Info.ChangeTrackingState.MostRecentId);
			printf("Info.ChangeTrackingState.NewerChanges------>(bool ) = %d (dec)\n", Info.ChangeTrackingState.NewerChanges);
		}
		else
		{
			printf("\n4.failed to get virtual disk change tracking info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}
		//printf("Info.Identifier---------------------------->(char ) = %d (dec)\n", Info.Identifier);
			//printf("Info.ParentIdentifier						(char ) = %d (dec)\n", Info.ParentIdentifier);
		Info.Version = GET_VIRTUAL_DISK_INFO_PARENT_TIMESTAMP;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n5.success getting virtual disk ParentTimestamp info\n");
			printf("Info.ParentTimestamp----------------------->(bytes) = %d (dec)\n", Info.ParentTimestamp);
		}
		else
		{
			printf("\n5.failed to get virtual disk parent timestamp info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}
		Info.Version = GET_VIRTUAL_DISK_INFO_PROVIDER_SUBTYPE;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n6.success getting virtual disk ProviderSubtype info\n");
			printf("Info.ProviderSubtype)---------------------->(bytes) = %d (dec)\n", Info.ProviderSubtype);
		}
		 else
		{
			printf("\n6.failed to get virtual disk provider subtype info %d\n", ret);
			PrintErrorMessage(GetLastError());
		}

		Info.Version = GET_VIRTUAL_DISK_INFO_IS_4K_ALIGNED;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n7.success getting virtual disk Is4kAligned info\n");
			printf("Info.Is4kAligned--------------------------->(bool ) = %d (dec)\n", Info.Is4kAligned);
		}
		else
		{
			printf("\n7.failed to get virtual disk 4k aligned info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}
			
		Info.Version = GET_VIRTUAL_DISK_INFO_IS_LOADED;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n8.success getting virtual disk IsLoaded info\n");
			printf("Info.IsLoaded------------------------------>(bool ) = %d (dec)\n", Info.IsLoaded);
		}
		else
		{
			printf("\n8.failed to get virtual disk isloaded info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}
		Info.Version = GET_VIRTUAL_DISK_INFO_VHD_PHYSICAL_SECTOR_SIZE;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n9.success getting virtual disk VhdPhysicalSectorSize info\n");
			printf("Info.VhdPhysicalSectorSize)---------------->(bytes) = %d (dec)\n", Info.VhdPhysicalSectorSize);
		}
		else
		{
			printf("\n9.failed to get virtual disk vhdphysicalsectosize info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}
		Info.Version = GET_VIRTUAL_DISK_INFO_SMALLEST_SAFE_VIRTUAL_SIZE;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n10.success getting virtual disk SmallestSafeVirtualSize info\n");
			printf("Info.SmallestSafeVirtualSize--------------->(bytes) = %d (dec)\n", Info.SmallestSafeVirtualSize);
		}
		else
		{
			printf("\n10.failed to get virtual disk SmallestSafeVirtualSize info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}
		Info.Version = GET_VIRTUAL_DISK_INFO_FRAGMENTATION;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n11.success getting virtual disk FragmentationPercentage info\n");
			printf("Info.FragmentationPercentage--------------->(%    ) = %d (dec)\n", Info.FragmentationPercentage);
		}
		else
		{
			printf("\n11.failed to get virtual disk FragmentationPercentage info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}
		Info.Version = GET_VIRTUAL_DISK_INFO_VIRTUAL_DISK_ID;
		ret = GetVirtualDiskInformation(hVhd,
			&InfoSize,
			&Info,
			&SizeUsed);
		if (ret == ERROR_SUCCESS)
		{
			printf("\n12.success getting virtual disk VirtualDiskId info\n");
			printf("Info.VirtualDiskId------------------------->(num  ) = %d (dec)\n", Info.VirtualDiskId);
		}
		else
		{
			printf("\n12.failed to get virtual disk VirtualDiskId info %d\n", ret);
			//PrintErrorMessage(GetLastError());
		}
		printf("**************************************************************************************************");





			//**************************************************************************************************
            bRet = TRUE;

       // else
       // {
        //    printf("failed to get virtual disk size info %d\n", ret);
       //     PrintErrorMessage(GetLastError());
       // }

        InfoSize = (ULONG)sizeof(GET_VIRTUAL_DISK_INFO);
        Info.Version = GET_VIRTUAL_DISK_INFO_IDENTIFIER;
        ret = GetVirtualDiskInformation(hVhd,
            &InfoSize,
            &Info,
            &SizeUsed);

        if (ret == ERROR_SUCCESS) 
        {
            StringFromCLSID(Info.Identifier, (LPOLESTR *) &pszGuid);
            printf("success getting virtual disk ID info\n");
            wprintf(L"Info.Identifier  (GUID) = %s\n", pszGuid);
            bRet = TRUE;
        }
        else
        {
            printf("failed to get virtual disk ID info %d\n", ret);
            PrintErrorMessage(GetLastError());
        }




		




    } 
    else
    {
        printf("failed to open vdisk...err %d\n", ret);
        PrintErrorMessage(GetLastError());
    }

    if (INVALID_HANDLE_VALUE != hVhd)
    {
        CloseHandle(hVhd);
    }

    return bRet;
}


BOOL OpenAndGetPhysVHD(PCWSTR pszVhdPath, PWSTR pszPhysicalDiskPath)
{    
    BOOL bRet = FALSE;
    HANDLE hVhd = INVALID_HANDLE_VALUE;
    DWORD ret;
    OPEN_VIRTUAL_DISK_PARAMETERS oparams;
    ATTACH_VIRTUAL_DISK_PARAMETERS iparams;
    VIRTUAL_STORAGE_TYPE            vst =
    {
        VIRTUAL_STORAGE_TYPE_DEVICE_VHD,
        VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT
    };

    wprintf(L"OpenAndGetPhysVHD %s\n", pszVhdPath);

    oparams.Version = OPEN_VIRTUAL_DISK_VERSION_1;
    oparams.Version1.RWDepth = OPEN_VIRTUAL_DISK_RW_DEPTH_DEFAULT;

    iparams.Version = ATTACH_VIRTUAL_DISK_VERSION_1;

    ret = OpenVirtualDisk(&vst, pszVhdPath,
        VIRTUAL_DISK_ACCESS_ATTACH_RW | VIRTUAL_DISK_ACCESS_GET_INFO | VIRTUAL_DISK_ACCESS_DETACH,
        OPEN_VIRTUAL_DISK_FLAG_NONE, &oparams, &hVhd);

    if (ERROR_SUCCESS == ret) 
    {
        ULONG sizePhysicalDisk;
        printf("success opening vdisk...\n");
        memset(pszPhysicalDiskPath, 0, sizeof (wchar_t) * PHYS_PATH_LEN);
        sizePhysicalDisk = (PHYS_PATH_LEN * sizeof(wchar_t)) * 256;
        ret = GetVirtualDiskPhysicalPath(hVhd, &sizePhysicalDisk, pszPhysicalDiskPath);
        if (ERROR_SUCCESS == ret)
        {
            wprintf(L"success getting physical path %s vhdname\n", pszPhysicalDiskPath);
            bRet = TRUE;
        }
        else
        {
            printf("failed to get vhd physical info %d\n", ret);
            PrintErrorMessage(GetLastError());
        }
    } 
    else 
    {
        printf("failed to open vdisk...err 0x%x\n", ret);
        PrintErrorMessage(GetLastError());
    }

    if (INVALID_HANDLE_VALUE != hVhd) 
    {
        CloseHandle(hVhd);
    }

    return bRet;
}

int wmain(int argc, wchar_t *argv[])
{
    wchar_t cmd = 0;
    wchar_t action = 0;
    ULONG sizeInMb = 0;
    wchar_t szFilename[132] = {0};
    wchar_t szGuid[132] = {0};
    wchar_t szPhysicalDiskPath[PHYS_PATH_LEN * 256];

    for (int i = 1; i < argc; i++)
    {
        cmd = towlower(argv[i][1]);
       wprintf(L"cmd = %c\n", cmd);
        switch (cmd) 
        {
        case L'g': // GetVirtualDiskInformation    - GetVHDInfo(HANDLE VhdHandle) - input=path
            action = cmd;
			wprintf(L"action 1 %c   ", cmd);
            break;
        case L'f':
            //wprintf(L"f: %s\n", &argv[i][3]);
            wcscpy_s(szFilename, _countof(szFilename), &argv[i][3]);
            break;
        case L's':
            //wprintf(L"s: %s\n", &argv[i][3]);
            swscanf_s(&argv[i][3], L"%d", &sizeInMb);
            break;
        case L'u':
            //wprintf(L"u: %s\n", &argv[i][3]);
            wcscpy_s(szGuid, _countof(szGuid), &argv[i][3]);
            break;
        case L'h':
        case L'?':
            //usage();
            goto _test_exit;
            break;
        default:
            break;
        }
    }

    if (!ValidateActionAndParameters(action, szFilename, sizeInMb, szGuid)) 
    {
        printf("invalid command %c\n", action);
        //usage();
        return -1;
    }
	wprintf(L"action %c   ", cmd);
    wprintf(L"Command = %c, Filename = %s, SizeMB = %d, Guid = %s\n", action, szFilename, sizeInMb, szGuid);
    switch (action)
    {
    case L'g':
        printf("GetVirtualDiskInformation\n");
        OpenAndGetVHDInfo(szFilename, szGuid);
		break;
    default:
        wprintf(L"Unknown command %c\n", action);
        break;
    }

_test_exit:

    return 0;
}