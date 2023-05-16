# Logger
### Introduction
spdlog is utilized to log messages. There are two "sinks" used. 
One outputs to the journald socket if available ```/run/systemd/journal/socket```.
The systemd output is done directly to the socket instead of utilizing libsystemd so that
the project can be built statically inside of alpine where it is hard to have systemd as a dependency.

See [Native journal format](https://systemd.io/JOURNAL_NATIVE_PROTOCOL/) for the format of the messages and
[Systemd journal fields](https://www.freedesktop.org/software/systemd/man/systemd.journal-fields.html) for reference field assignments.

If there is no journal socket found then the output is placed on the cli
and when ```--stdout``` is passed to program executables their output is also displayed in the cli.

### Deciding the log level
Passing ```--log-level <level>```
where level options are
- trace
- debug
- info
- warn
- error
- critical
- off

> **_Note:_**  Setting the log level only affects the verboseness of the cli output

### TFC Specific metadata
To enrich the logging provided to the journal TFC outputs specific
metadata fields. They are
- TFC_KEY a string passed to the logger at construction f.e. ipc_send
- TFC_EXE the executable name
- TFC_ID parameter passed to the process at startup using ```--id```

