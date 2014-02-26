#ifndef MAIN_H
#define MAIN_H

#include "common.h"

NTSTATUS DriverEntry(
	__inout PDRIVER_OBJECT  DriverObject,
	__in    PUNICODE_STRING RegistryPath
	);
VOID FsFilterUnload(
	__in PDRIVER_OBJECT DriverObject
	);

#endif
