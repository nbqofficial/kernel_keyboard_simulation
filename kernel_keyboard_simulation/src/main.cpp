#include "keyboard/keyboard.h"

NTSTATUS sleep(ULONG ms)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -(LONGLONG)ms * 10000;
	return KeDelayExecutionThread(KernelMode, FALSE, &interval);
}

void unload(PDRIVER_OBJECT driver_object)
{
	DbgPrintEx(0, 0, "KEYBOARD TEST UNLOAD\n");
}

extern "C" NTSTATUS driver_entry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
	UNREFERENCED_PARAMETER(registry_path);

	driver_object->DriverUnload = unload;

	KEYBOARD_OBJECT keyboard_obj = { 0 };
	NTSTATUS status = keyboard::init_keyboard(&keyboard_obj);
	DbgPrintEx(0, 0, "%lx\n", status);

	if (NT_SUCCESS(status))
	{
		sleep(3000);
		for (int i = 0; i < 50; i++)
		{
			keyboard::keybd_event(keyboard_obj, MAKE_CODE_A, KEY_MAKE);
			sleep(100);
			keyboard::keybd_event(keyboard_obj, MAKE_CODE_A, KEY_BREAK);
			sleep(100);
		}
	}

	DbgPrintEx(0, 0, "KEYBOARD TEST LOADED\n");
	return STATUS_SUCCESS;
}