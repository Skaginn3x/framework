# Alarm system ( Themis )

## Introduction
Handle the registration, activation level and notification of alarms.

## Alarm template
Alarm messages have the requirement of being translatable, dynamic sections
of the message shall be marked with a fmt formattable string.
Example of message strings 
```txt
"Temperture out of bounds it is greater then {}"
"Actuation time out of bounds {}ms expected took {}ms"
"Frequency drive {} not reaching speed {} current {}"
```
Please note that for the short and long text associated with an alarm the
dynamic variable count must be the same.

## DBUS API
The alarm system is exposed through the `com.skaginn3x.Alarm` interface.

### Methods
    RegisterAlarm (s: tfc_id, i: alarm_level, b: latching) -> i: alarm_id Note errors can only be latched
    # ListAlarms returns boolean true in_locale if translation exists otherwise english
    ListAlarms () -> s -> json of: std::vector<struct { string description; string details; bool latching; enum alarm_level; std::map<locale, struct translations{ string description; string details}> ; }>
    # Note for state = -1 for all, alarm_level = -1 for all a max size of 100 alarms will be sent at a time
    ListActivations (s: locale, i: start_count, i: count, i: alarm_level, i: state, x: startunixTimestamp, x: endUnixTimestamp) -> s -> json of: struct { string description; string details; bool latching; enum alarm_level; state_e state; std::uint64_t millisec_from_epoch; };
    SetAlarm(i: alarm_id, as: variables)
    ResetAlarm(i: alarm_id)
    TryReset(i: alarm_id) # Transmits a signal to the alarm to reset itself
    TryResetAll() # Transmits a signal to all alarms to reset themselfs
### Signals
    AlarmActivationChanged(i: alarm_id, n: level, n: state)
    TryReset(i: alarm_id)
    TryResetAll()
### Properties
    As of now there are no properties
## Database schema
```
Alarms
|alarm_id|exe.id.component|sha1sum|alarm_level|template_short_en|template_en|alarm_latching|

SHA1SUM is the SHA1SUM of both alarm_template_short_en and alarm_template_en
Combined primary key of exe.id.component and SHA1SUM
this combination shall be unique

Alarm translation
|sha1sum|locale|alarm_template_short|alarm_template

Alarm activations
|activation_id|alarm_id|timestamp|activation_level|

Alarm acks
|activation_id|ack_timestamp|

Alarm variables
|activation_id|variable_index|variable_string|
```


## Policy
Executables must register their alarms on construction. Unregistered alarms cannot be notified.

Once an alarm is registered the Alarm with Themis he will monitor the NameOwnerChanged signal for the
name that registered the alarm. If the name is lost the alarm will be deactivated.

If an alarm is registered having changed the level or latching attributes the alarm shall be
updated in the database
