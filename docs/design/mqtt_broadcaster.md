# MQTT Broadcaster

## Description

This document provides a guide on using the MQTT broadcaster executable. The service forwards signal values
from the IPC manager to MQTT topics, providing a connection between IPC and MQTT.

## Configuration

In order to configure the executable, locate a file under `/etc/tfc/mqtt-broadcaster/def/mqtt_broadcaster.json` and
set the following variables:

```bash
{
   "_mqtt_broker_address": "",
   "_mqtt_broker_port": "8883",
   "_mqtt_broker_username": "",
   "_mqtt_broker_password": "",
   "_sparkplug_b_edge_node_id": "",
   "_sparkplug_b_group_id": "",
   "_client_id": ""
}
```

The following command line arguments are optional:

```bash
--stdout --log-level trace   # enables logging
```

## Testing

In order to see functionality run:

```bash
ipc-ruler --stdout --log-level trace
tfcctl --signal bool.test --stdout --log-level trace   # send values on this signal to see functionality
```

## Key elements of coroutines

The executable uses a coroutine which it posts to in order to perform tasks. This is an effective way to have the
benefits of multithreading and single-threading combined. There are a number of elements of this feature that need to be
understood when developing.

### Detached and Awaitable

```cpp
asio::co_spawn(mqtt_client_->strand(), tfc::base::exit_signals(ctx_), asio::detached);
co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);
```

If a coroutine is spawned with `asio::detached` the coroutine will run in the background and will not be waited on, also
known as "fire-and-forget". These are most typically tasks that are not critical to the operation of the executable and
can run alongside other tasks.

If a coroutine is spawned with `asio::use_awaitable` the coroutine will be waited on and will block the thread until it
has completed. An example of this can be when the program is connecting to the broker. Other parts of the program cannot
function until the connection has been established and therefore the coroutine must be waited on.

### Context and Strand

```cpp
co_await asio::co_spawn(mqtt_client_->strand(), mqtt_client_->close(asio::use_awaitable), asio::use_awaitable);
co_await asio::co_spawn(ctx_, mqtt_client_->close(asio::use_awaitable), asio::use_awaitable);
```

If a coroutine is spawned in a strand, it will run in the strand. This means that all routines will run synchronously
and the risk of concurrency is avoided. This is useful when you want to ensure that a routine is not interrupted by any
other task than the one running. This executable uses a strand in order to guarantee memory safety.

If a coroutine is spawned in a context this consecutive execution is not guaranteed. Multiple jobs can run at the same
time and memory safety is not guaranteed.

## Key elements of the MQTT protocol

The executable uses MQTT v5 and the Sparkplug B specification.

Assumption: The reader has a basic understanding of the MQTT protocol.

The executable utilizes 2 very important flags:

- session expiry interval
- clean session / clean start

When a client connects to the broker it starts a session. This session is kept alive by PING messages. If the client
disconnects the broker stores information about the client for a certain amount of time. This is the session expiry
interval. If the client doesn't set a session expiry interval then it is set to 0 and the broker immediately deletes the
session as soon as it
detects a disconnect.

The clean session flag determines weather the client should use a previous session or start a new one. If the clean
session flag is set to true then the broker will always start a new session upon every new connection. Otherwise, it will
try to use the previous session if there is one available. This executable sets the clean session flag to false.

## SparkPlug B

The SparkPlug B specification is a specification for MQTT payloads. It is a specification that is used by the MQTT
broker to determine how to handle the payload, topic structure and other things.

### NBIRTH

In order to create a new session, the client must send an NBIRTH message. This message contains information about the
available devices and their signal values.

### NDEATH

When a client disconnects it sends an NDEATH message, this is handled through MQTT Will messages.

### NDATA

When a client wants to update a signal value it sends an NDATA message. This message contains information about the
signal value and the device it belongs to. It should only be used for updates, not for repeated values.

## Prerequisites for Local Testing

The following components must be available on your host machine for testing, development, and a better understanding of
the executable:

- MQTT broker that supports SparkPlugB (Ignition ... etc)
- socat (For simulating offline state)
- IPC-ruler

## Simulating Offline State

You can simulate an offline state by forwarding the TCP traffic through socat. 

```bash
socat TCP4-LISTEN:1234,reuseaddr TCP4:<ip>:8883
```

To simulate offline state, kill the socat process and restart it.
