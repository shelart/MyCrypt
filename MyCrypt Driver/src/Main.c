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

NTSTATUS DriverEntry(
	__inout PDRIVER_OBJECT  DriverObject,
	__in    PUNICODE_STRING RegistryPath
	)
{
	DbgPrint("My Filter started.\n");

	NTSTATUS status = STATUS_SUCCESS;
	ULONG    i = 0;

	PDEVICE_OBJECT             fdo;
	UNICODE_STRING             devName;
	PFSFILTER_DEVICE_EXTENSION fdo_ext;
	g_fsFilterDriverObject = DriverObject;

	RtlInitUnicodeString(&devName, DEVNAME);

	status = IoCreateDevice(
		DriverObject,
		sizeof(FSFILTER_DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&fdo);
	if (!NT_SUCCESS(status))
		return status;

	fdo_ext = (PFSFILTER_DEVICE_EXTENSION)fdo->DeviceExtension;
	fdo_ext->AttachedToDeviceObject = NULL;

	RtlInitUnicodeString(&fdo_ext->ustrSymLinkName, SYM_LINK_NAME);
	status = IoCreateSymbolicLink(&fdo_ext->ustrSymLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(fdo);
		return status;
	}

	//
	// Store our driver object.
	//

	g_fsFilterDriverObject = DriverObject;

	//
	//  Initialize the driver object dispatch table.
	//

	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; ++i)
	{
		DriverObject->MajorFunction[i] = FsFilterDispatchPassThrough;
	}

	DriverObject->MajorFunction[IRP_MJ_CREATE] = FsFilterDispatchCreate;

	//
	// Set fast-io dispatch table.
	//

	DriverObject->FastIoDispatch = &g_fastIoDispatch;

	//
	//  Registered callback routine for file system changes.
	//

	status = IoRegisterFsRegistrationChange(DriverObject, FsFilterNotificationCallback);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	//
	// Set driver unload routine (debug purpose only).
	//

	DriverObject->DriverUnload = FsFilterUnload;

	return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// Unload routine

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

		for (i = 0; i < numDevices; ++i)
		{
			FsFilterDetachFromDevice(devList[i]);
			ObDereferenceObject(devList[i]);
		}

		interval.QuadPart = 5 * DELAY_ONE_SECOND;
		KeDelayExecutionThread(KernelMode, FALSE, &interval);
	}
}

//////////////////////////////////////////////////////////////////////////
// Misc

BOOLEAN FsFilterIsMyDeviceObject(
	__in PDEVICE_OBJECT DeviceObject
	)
{
	return DeviceObject->DriverObject == g_fsFilterDriverObject;
}
