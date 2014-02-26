#include <ntddk.h>

VOID DriverUnload(
	__in PDRIVER_OBJECT
	);

NTSTATUS DriverEntry(
	__inout PDRIVER_OBJECT  DriverObject,
	__in    PUNICODE_STRING	RegistryPath
	)
{
	DriverObject->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}

VOID DriverUnload(
	__in PDRIVER_OBJECT DriverObject
	)
{
	return;
}
