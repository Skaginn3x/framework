# IPC slot value tinker

Each ipc slot offers a dbus api interface.
One for filters, get value and value tinkering.

## General information
The slot requests a name from dbus `<slot_name>._slot_`
storing its dbus connection, 
The implemented functionality inject their interfaces
into the slots established dbus connection.

### Interface names
the following interface names are used for the functionality.

- filters `<slot_name>/filters`
- tinker  `<slot_name>/tinker`
- value `<slot_name>/value`

## Specification of the tinker interface
property called `forced` of the type bool,
this property indicates weather the slots value is forced or not.

a method called `force` accepting a value of the type of the slot,
this method forces the value of the slot.

a method called `release` accepting no arguments.

## Specification of the value interface
property called `post_filter_value` of the type of the slot.
and a property called `raw_value` of the type of the slot,
raw value contains the value before the filters are applied.

### Monitoring value changes
Each of these properties transmit a properties changed event
and can be subscribed to.

### Effect of forcing a value
If the slot value is forced `post_filter_value` is set to the
forced value but `raw_value` still tracks the value changes
on the signal the slot is connected to.


## The filter interface 
This interface follows our convention of using json-schema for
configurations.