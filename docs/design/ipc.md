# Interprocess communication

Implementation of the
[Observer pattern](https://en.wikipedia.org/wiki/Observer_pattern)

Observers hereby known as slots and events known as signals.

## Responsibility

Take care of communication back and forth between user space components and fundamental components.

## Features
- string based naming of signals and slots
- One signal can indicate to many slots
- runtime capable of mapping

## IPC Slots and signals visulization
<iframe style="border: 1px solid rgba(0, 0, 0, 0.1);" width="450" height="450" src="https://www.figma.com/embed?embed_host=share&url=https%3A%2F%2Fwww.figma.com%2Ffile%2FKYcqq14K8HDq6KnuXw7YX4%2FUntitled%3Fnode-id%3D0%253A1%26t%3DguqGCyLBXZ3hVvOT-1" allowfullscreen></iframe>

## Example of information sources and consumers
consumer                  information source
(slot t.d. axes_count) - (signal t.d. servo drive)

## Example of a mapping between a signal and a slot
axes_count <-> servo.axes_count

## Example of mapping configuration
```json
connections : [
    {
        "signal": "servo.axes_count",
        "slot": "axes_count"
        "type": "uint16_t",
    },
    {
        "signal": "servo.axes_speed",
        "slot": "axes_speed"
        "type": "uint16_t",
    }
];
```
## Management
There is an api called ipc-ruler running on dbus that manages
ipc connections each signal registers with the service
and each slot listens asynchronously to new connection
information.

Both the services and the management of the services
is done over dbus. 


## Delay and real time considerations
Less than 1ms

## Sketch

```C++
#include <cstdint>
#include <functional>
#include <string_view>

template <typename T>
class Reader {
public:
    Reader(std::string_view name);
    void subscribe(std::function<void(T const&)>);
    T const& get() const noexcept;
    awaitable get() const noexcept;
private:
    void register_(std::string_view name);
    void deregister();
};

template <typename T>
concept Slottable = requires {
    std::same_as<std::uint64_t, T>,
            ...
}; 
template <Slottable T>
class Slot {
    // register into confman provide callback and get mapping for reader
    void connected(std::function<void(bool)>);
    
private:
    Reader<T> reader;    
    JsonSchema schema;
};

using DIOReader = Reader<bool>;



class motorcontroller {
public:
    DIOReader<"running"> foo;
    DIOReader running{"running"};
    
};
using DoubleReader = Reader<double>;
using IntReader = Reader<std::int64_t>;
//using JSONReader = Reader<nl::json>;

using status_code = std::int64_t;

template <typename T>
class Writer {
public:
    Writer(std::string_view name);
    void set(T const&);
    void publish(T const&, std::function<void(status_code)>);

};

```