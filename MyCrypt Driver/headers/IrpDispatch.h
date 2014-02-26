#ifndef IRPDISPATCH_H
#define IRPDISPATCH_H

#include "common.h"

NTSTATUS FsFilterDispatchPassThrough(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
	);

NTSTATUS FsFilterDispatchCreate(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
	);

#endif
