# IPC slot value tinker

Each ipc slot offers a dbus api interface.
One for filters, get value and value tinkering.

## General information
The slot requests a name from dbus `<slot_name>._slot_`
storing its dbus connection, 
The implemented functionality inject their interfaces
into the slots established dbus connection.

### Interface names
The following interface names are used for the functionality.

- filters `<slot_name>/filters`
- tinker  `<slot_name>/tinker`
- value `<slot_name>/value`

## Specification of the tinker interface
A method called `set` accepting a value of the type of the slot,
this method sends a value to the slot.
Once a new value is posted on the signal
the slot is connected to the value is updated.

If you wish to completely control the value
you can disconnect the slot from the signal
and then set a value to the slot.

## Specification of the value interface
A property called `filtered_value` of the type of the slot.
A property called `raw_value` of the type of the slot,
raw value contains the value before the filters are applied.

### Monitoring value changes
Each of these properties transmit a properties changed event
and can be subscribed to.

### Effect of forcing a value
If the slot value is forced `filtered_value` is set to the
just set value but `raw_value` still tracks the value
on the signal the slot is connected to.

## The filter interface 
This interface follows our convention of using json-schema for
configurations.