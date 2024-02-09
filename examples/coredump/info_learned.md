# Dump core from coredumpctl
```bash
coredumpctl dump <optional-pid> example_throw --output <file-name>
```
# Find source location in binary
```bash
objdump -g ./example_throw | grep example_throw.cpp
```