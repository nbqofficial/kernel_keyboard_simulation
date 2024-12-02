#include "keyboard.h"

PIRP keyboard::keyboard_class_dequeue_read(PDEVICE_EXTENSION device_extension)
{
	PIRP next_irp = 0;
	while (!next_irp && !IsListEmpty(&device_extension->ReadQueue))
	{
		PLIST_ENTRY list_entry = RemoveHeadList(&device_extension->ReadQueue);
		next_irp = CONTAINING_RECORD(list_entry, IRP, Tail.Overlay.ListEntry);
	}
	return next_irp;
}

VOID keyboard::keyboard_class_service_callback(PDEVICE_OBJECT device_object, PKEYBOARD_INPUT_DATA input_data_start, PKEYBOARD_INPUT_DATA input_data_end)
{
	PDEVICE_EXTENSION device_extension = (PDEVICE_EXTENSION)device_object->DeviceExtension;
	ULONG bytes_in_queue = (ULONG)((PCHAR)input_data_end - (PCHAR)input_data_start);

	KeAcquireSpinLockAtDpcLevel(&device_extension->SpinLock);

	PIRP irp = keyboard_class_dequeue_read(device_extension);
	if (irp)
	{
		PIO_STACK_LOCATION pio = IoGetCurrentIrpStackLocation(irp);
		ULONG bytes_to_move = pio->Parameters.Read.Length;
		ULONG move_size = bytes_in_queue < bytes_to_move ? bytes_in_queue : bytes_to_move;
		RtlMoveMemory(irp->AssociatedIrp.SystemBuffer, (PCHAR)input_data_start, move_size);
		irp->IoStatus.Status = STATUS_SUCCESS;
		irp->IoStatus.Information = move_size;
		pio->Parameters.Read.Length = move_size;
	}

	KeReleaseSpinLockFromDpcLevel(&device_extension->SpinLock);
	if (irp) { IoCompleteRequest(irp, IO_KEYBOARD_INCREMENT); }
}

NTSTATUS keyboard::init_keyboard(PKEYBOARD_OBJECT keyboard_obj)
{
	UNICODE_STRING class_string;
	RtlInitUnicodeString(&class_string, L"\\Driver\\KbdClass");

	PDRIVER_OBJECT class_driver_object = NULL;
	NTSTATUS status = ObReferenceObjectByName(&class_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&class_driver_object);
	if (!NT_SUCCESS(status)) { return status; }

	UNICODE_STRING hid_string;
	RtlInitUnicodeString(&hid_string, L"\\Driver\\KbdHID");

	PDRIVER_OBJECT hid_driver_object = NULL;
	status = ObReferenceObjectByName(&hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&hid_driver_object);
	if (!NT_SUCCESS(status))
	{
		if (class_driver_object) { ObDereferenceObject(class_driver_object); }
		return status;
	}

	if (!keyboard_obj->keyboard_device)
	{
		PDEVICE_OBJECT target_device_object = class_driver_object->DeviceObject;
		while (target_device_object)
		{
			if (!target_device_object->NextDevice)
			{
				keyboard_obj->keyboard_device = target_device_object;
				break;
			}
			target_device_object = target_device_object->NextDevice;
		}
	}

	ObDereferenceObject(class_driver_object);
	ObDereferenceObject(hid_driver_object);

	return STATUS_SUCCESS;
}

bool keyboard::keybd_event(KEYBOARD_OBJECT keyboard_obj, unsigned short code, unsigned short flags)
{
	if (!keyboard_obj.keyboard_device) { return false; }

	KEYBOARD_INPUT_DATA kid = { 0 };

	kid.UnitId = 1;
	kid.MakeCode = code;
	kid.Flags = flags;

	KIRQL irql;
	KeRaiseIrql(DISPATCH_LEVEL, &irql);
	keyboard::keyboard_class_service_callback(keyboard_obj.keyboard_device, &kid, (PKEYBOARD_INPUT_DATA)&kid + 1);
	KeLowerIrql(irql);

	return true;
}