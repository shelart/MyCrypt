#ifndef ATTACHDETACH_H
#define ATTACHDETACH_H

#include "common.h"

NTSTATUS FsFilterAttachToDevice(
	__in PDEVICE_OBJECT        DeviceObject,
	__out_opt PDEVICE_OBJECT * pFilterDeviceObject
	);

void FsFilterDetachFromDevice(
	__in PDEVICE_OBJECT DeviceObject
	);

BOOLEAN FsFilterIsAttachedToDevice(
	__in PDEVICE_OBJECT DeviceObject
	);

#endif