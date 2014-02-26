#include "../headers/Main.h"
#include "../headers/FastIo.h"
#include "../headers/IrpDispatch.h"
#include "../headers/Notification.h"
#include "../headers/AttachDetach.h"

PDRIVER_OBJECT   g_fsFilterDriverObject = NULL;
FAST_IO_DISPATCH g_fastIoDispatch =
{
	sizeof(FAST_IO_DISPATCH),
	FsFilterFastIoCheckIfPossible,
	FsFilterFastIoRead,
	FsFilterFastIoWrite,
	FsFilterFastIoQueryBasicInfo,
	FsFilterFastIoQueryStandardInfo,
	FsFilterFastIoLock,
	FsFilterFastIoUnlockSingle,
	FsFilterFastIoUnlockAll,
	FsFilterFastIoUnlockAllByKey,
	FsFilterFastIoDeviceControl,
	NULL,
	NULL,
	FsFilterFastIoDetachDevice,
	FsFilterFastIoQueryNetworkOpenInfo,
	NULL,
	FsFilterFastIoMdlRead,
	FsFilterFastIoMdlReadComplete,
	FsFilterFastIoPrepareMdlWrite,
	FsFilterFastIoMdlWriteComplete,
	FsFilterFastIoReadCompressed,
	FsFilterFastIoWriteCompressed,
	FsFilterFastIoMdlReadCompleteCompressed,
	FsFilterFastIoMdlWriteCompleteCompressed,
	FsFilterFastIoQueryOpen,
	NULL,
	NULL,
	NULL,
};
PDEVICE_OBJECT             g_fdo;
PFSFILTER_DEVICE_EXTENSION g_fdo_ext;

NTSTATUS DriverEntry(
	__inout PDRIVER_OBJECT  DriverObject,
	__in    PUNICODE_STRING RegistryPath
	)
{
	DBG_PRINT(DBG_LEVEL_INFO, "Started driver loading routine.\n");
	//__debugbreak();

	NTSTATUS status = STATUS_SUCCESS;
	ULONG    i = 0;

	UNICODE_STRING             devName;
	g_fsFilterDriverObject = DriverObject;

	RtlInitUnicodeString(&devName, DEVNAME);

	DBG_PRINT(DBG_LEVEL_INFO, "Creating control FDO...\n");
	status = IoCreateDevice(
		DriverObject,
		sizeof(FSFILTER_DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&g_fdo);
	if (!NT_SUCCESS(status))
	{
		DBG_PRINT(DBG_LEVEL_WARN, "Could not create control FDO!\n");
		return status;
	}

	g_fdo_ext = (PFSFILTER_DEVICE_EXTENSION)g_fdo->DeviceExtension;
	g_fdo_ext->AttachedToDeviceObject = NULL;

	RtlInitUnicodeString(&g_fdo_ext->ustrSymLinkName, SYM_LINK_NAME);
	DBG_PRINT(DBG_LEVEL_INFO, "Creating symlink %wZ...\n", &g_fdo_ext->ustrSymLinkName);
	status = IoCreateSymbolicLink(&g_fdo_ext->ustrSymLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		DBG_PRINT(DBG_LEVEL_WARN, "Could not create symlink %wZ!\n", &g_fdo_ext->ustrSymLinkName);
		DBG_PRINT(DBG_LEVEL_INFO, "Deleting control FDO...\n");
		IoDeleteDevice(g_fdo);
		return status;
	}

	//
	// Store our driver object.
	//

	g_fsFilterDriverObject = DriverObject;

	//
	//  Initialize the driver object dispatch table.
	//

	DBG_PRINT(DBG_LEVEL_INFO, "Populating %i IRP Major handlers with FsFilterDispatchPassThrough...\n", IRP_MJ_MAXIMUM_FUNCTION);
	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; ++i)
	{
		DriverObject->MajorFunction[i] = FsFilterDispatchPassThrough;
	}

	DBG_PRINT(DBG_LEVEL_INFO, "Populating IRP_MJ_CREATE handler with FsFilterDispatchCreate...\n");
	DriverObject->MajorFunction[IRP_MJ_CREATE] = FsFilterDispatchCreate;

	//
	// Set fast-io dispatch table.
	//

	DBG_PRINT(DBG_LEVEL_INFO, "Populating Fast-IO dispatch table...\n");
	DriverObject->FastIoDispatch = &g_fastIoDispatch;

	//
	//  Registered callback routine for file system changes.
	//

	DBG_PRINT(DBG_LEVEL_INFO, "Registering FsFilterNotificationCallback for file system changes...\n");
	status = IoRegisterFsRegistrationChange(DriverObject, FsFilterNotificationCallback);
	if (!NT_SUCCESS(status))
	{
		DBG_PRINT(DBG_LEVEL_WARN, "Could not register callback routine for file system changes!\n");
		DBG_PRINT(DBG_LEVEL_INFO, "Deleting control FDO...\n");
		IoDeleteDevice(g_fdo);
		return status;
	}

	//
	// Set driver unload routine (debug purpose only).
	//

	//DriverObject->DriverUnload = FsFilterUnload;

	DBG_PRINT(DBG_LEVEL_INFO, "Driver loaded.\n");
	return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// Unload routine

/*
VOID FsFilterUnload(
	__in PDRIVER_OBJECT DriverObject
	)
{
	PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];
	ULONG numDevices = 0;
	ULONG i = 0;
	LARGE_INTEGER interval;

	//
	//  Unregistered callback routine for file system changes.
	//

	IoUnregisterFsRegistrationChange(DriverObject, FsFilterNotificationCallback);

	for (;;)
	{
		IoEnumerateDeviceObjectList(
			DriverObject,
			devList,
			sizeof(devList),
			&numDevices);

		if (0 == numDevices)
		{
			break;
		}

		numDevices = min(numDevices, RTL_NUMBER_OF(devList));

		__debugbreak();
		DbgPrint("Deleting symlink %wZ...\n", &g_fdo_ext->ustrSymLinkName);
		IoDeleteSymbolicLink(&g_fdo_ext->ustrSymLinkName);

		for (i = 0; i < numDevices; ++i)
		{
			OBJECT_NAME_INFORMATION name;
			WCHAR buf[1024];
			name.Name.Buffer = buf;
			ULONG resultLength;
			ObQueryNameString(devList[i], &name, 1024, &resultLength);
			DbgPrint("%wZ\n", &name.Name);
			FsFilterDetachFromDevice(devList[i]);
			if (devList[i] != g_fdo)
				ObDereferenceObject(devList[i]);
			else
				DbgPrint("Control FDO met, no dereference\n");
		}

		interval.QuadPart = 5 * DELAY_ONE_SECOND;
		KeDelayExecutionThread(KernelMode, FALSE, &interval);

		DbgPrint("Driver unloaded.\n");
		__debugbreak();
	}
}
*/

//////////////////////////////////////////////////////////////////////////
// Misc

BOOLEAN FsFilterIsMyDeviceObject(
	__in PDEVICE_OBJECT DeviceObject
	)
{
	return DeviceObject->DriverObject == g_fsFilterDriverObject;
}
