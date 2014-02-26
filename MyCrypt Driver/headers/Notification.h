#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "common.h"

VOID FsFilterNotificationCallback(
	__in PDEVICE_OBJECT DeviceObject,
	__in BOOLEAN        FsActive
	);

NTSTATUS FsFilterAttachToFileSystemDevice(
	__in PDEVICE_OBJECT DeviceObject
	);

VOID FsFilterDetachFromFileSystemDevice(
	__in PDEVICE_OBJECT DeviceObject
	);

NTSTATUS FsFilterEnumerateFileSystemVolumes(
	__in PDEVICE_OBJECT DeviceObject
	);

#endif