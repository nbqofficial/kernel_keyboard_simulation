#pragma once

#include "../defs/definitions.h"

extern "C" POBJECT_TYPE* IoDriverObjectType;

typedef struct _DEVICE_EXTENSION
{
    PDEVICE_OBJECT  Self;
    PDEVICE_OBJECT  TrueClassDevice;
    PDEVICE_OBJECT  TopPort;
    PDEVICE_OBJECT  PDO;
    IO_REMOVE_LOCK  RemoveLock;
    BOOLEAN         PnP;
    BOOLEAN         Started;
    BOOLEAN OkayToLogOverflow;
    KSPIN_LOCK WaitWakeSpinLock;
    ULONG TrustedSubsystemCount;
    ULONG InputCount;
    UNICODE_STRING  SymbolicLinkName;
    PKEYBOARD_INPUT_DATA InputData;
    PKEYBOARD_INPUT_DATA DataIn;
    PKEYBOARD_INPUT_DATA DataOut;
    KEYBOARD_ATTRIBUTES  MouseAttributes;
    KSPIN_LOCK SpinLock;
    LIST_ENTRY ReadQueue;
    ULONG SequenceNumber;
    DEVICE_POWER_STATE DeviceState;
    SYSTEM_POWER_STATE SystemState;
    ULONG UnitId;
    WMILIB_CONTEXT WmiLibInfo;
    DEVICE_POWER_STATE SystemToDeviceState[PowerSystemHibernate];
    DEVICE_POWER_STATE MinDeviceWakeState;
    SYSTEM_POWER_STATE MinSystemWakeState;
    PIRP WaitWakeIrp;
    PIRP ExtraWaitWakeIrp;
    PVOID TargetNotifyHandle;
    LIST_ENTRY Link;
    PFILE_OBJECT File;
    BOOLEAN Enabled;
    BOOLEAN WaitWakeEnabled;
    BOOLEAN SurpriseRemoved;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

typedef VOID
(*KeyboardClassServiceCallback)(
    PDEVICE_OBJECT DeviceObject,
    PKEYBOARD_INPUT_DATA InputDataStart,
    PKEYBOARD_INPUT_DATA InputDataEnd,
    PULONG InputDataConsumed
    );

typedef struct _KEYBOARD_OBJECT
{
    PDEVICE_OBJECT keyboard_device;
} KEYBOARD_OBJECT, * PKEYBOARD_OBJECT;

namespace keyboard
{
    PIRP keyboard_class_dequeue_read(PDEVICE_EXTENSION device_extension);

    VOID keyboard_class_service_callback(PDEVICE_OBJECT device_object, PKEYBOARD_INPUT_DATA input_data_start, PKEYBOARD_INPUT_DATA input_data_end);

    NTSTATUS init_keyboard(PKEYBOARD_OBJECT keyboard_obj);

    bool keybd_event(KEYBOARD_OBJECT keyboard_obj, unsigned short code, unsigned short flags);
}