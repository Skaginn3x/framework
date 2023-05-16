# IPC manager

hold records of which slot is connected to which signal.

Maintain a list of these records along with metadata.

Make available the connections along with the ability to change these connections
to dbus api. 

Rough outline of the dbus api
```
register_signal (name, type_enum) -> Registers a signal for consumption ( no signal of same name can exist )
register_slot   (name, type_enum) -> Registers a slot for consumption ( no other slot of the same name can exist )
get_all -> Returns a list of slot, signals and connections
{
    signals: [
        {
            name: "some_signal_name"
            type: "int"
            created_by: "dbus-user"
            created_at: "ISO TIMESTAMP"
            last_registered: "ISO TIMESTAMP"
        }
    ]
    slots: [
        {
            name: "Some_slot_name"
            type: "int"
            created_by: "dbus-user"
            created_at: "ISO TIMESTAMP"
            last_registered: "ISO TIMESTAMP"
            last_modified: "ISO TIMESTAMP"
            modified_by: "dbus-user"
            connected_to: "slot_name"
        }
    ]
}
connect ( slot, signal ) -> Connect the slot to a signal
disconnect ( slot ) -> Disconnect the slot from its signal
```