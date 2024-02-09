# Dump core from coredumpctl
```bash
coredumpctl dump <optional-pid> example_throw --output <file-name>
```
# Find source location in binary
```bash
objdump -g ./example_throw | grep example_throw.cpp
```
# Show debug file name
```bash
readelf -p .gnu_debuglink example_throw

String dump of section '.gnu_debuglink':
  [     0]  example_throw.sym
  [    14]  C?X
```
