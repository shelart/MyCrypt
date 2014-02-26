#ifndef COMMON_H
#define COMMON_H

#define WINVER        _WIN32_WINNT_WIN8
#define _WIN32_WINNT  _WIN32_WINNT_WIN8

#ifndef NTDDI_VERSION
#define NTDDI_VERSION   NTDDI_WIN8
#endif

#pragma warning(push)
#pragma warning(disable:4013)
#pragma warning(disable:4005)
#pragma warning(disable:4163)
#include <ntifs.h>
#include <stdlib.h>
#include <ntddk.h>
#pragma warning(pop)

#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND * 1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND * 1000)

#define DEVOBJ_LIST_SIZE        64

#define FILE_NAME_LENGTH        100

//name and symbolic link for FDO
#define DEVNAME         L"\\Device\\FSFILTER"
#define SYM_LINK_NAME   L"\\DosDevices\\FsFilter"

//////////////////////////////////////////////////////////////////////////
// Structures
/*Device extension. ustrSymLinkName uses for FDO, AttachedToDeviceObject uses for FiDO*/
typedef struct _FSFILTER_DEVICE_EXTENSION
{
	UNICODE_STRING ustrSymLinkName;
	PDEVICE_OBJECT AttachedToDeviceObject;
} FSFILTER_DEVICE_EXTENSION, *PFSFILTER_DEVICE_EXTENSION;

/*Item of linked list, that contained filename for hiding*/
typedef struct _RULE_ENTRY
{
	WCHAR           name[FILE_NAME_LENGTH];
	USHORT          length;
	struct _RULE_ENTRY * pNext;
} RULE_ENTRY, *PRULE_ENTRY;

//////////////////////////////////////////////////////////////////////////
// Global data declaration (defined in Main.c)
extern PDRIVER_OBJECT g_fsFilterDriverObject;

NTSTATUS DriverEntry(
	__inout PDRIVER_OBJECT  DriverObject,
	__in    PUNICODE_STRING RegistryPath
	);

VOID FsFilterUnload(
	__in PDRIVER_OBJECT DriverObject
	);

BOOLEAN FsFilterIsMyDeviceObject(
	__in PDEVICE_OBJECT DeviceObject
	);

#endif
