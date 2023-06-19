# MQTT Broadcaster

## Description

This document provides a guide on using the MQTT broadcaster executable. The service forwards signal values
from the IPC manager to MQTT topics, providing a connection between IPC and MQTT.

## Command line arguments

The following command line arguments are necessary:

```bash
--mqtt_host <host> --mqtt_port <port>
```

The following command line arguments are optional:

```bash
--mqtt_username <username> --mqtt_password <password>
--stdout --log-level trace   # enables logging
```

## Testing

In order to see functionality simply run:

```bash
ipc-ruler --stdout --log-level trace
tfcctl --signal bool.test --stdout --log-level trace   # send values on this signal to see functionality
```

## Disregard signals

Often times a lot of signals are unnecessary to broadcast. In order to disregard signals, locate a file
under `/etc/tfc/mqtt-broadcaster/def/mqtt_broadcaster.json` and add the desired banned string under the `_banned_topics`
vector.

```json
{
  "_banned_topics": [
    "tub_tipper"
  ]
}
```

Example of banning all signals that contain the string tub_tipper.

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

The executable uses MQTT v5.

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

## Prerequisites for Local Testing

The following components must be available on your host machine for testing, development, and a better understanding of
the executable:

- MQTT broker (local Mosquitto broker or Docker)
- socat (For simulating offline state)
- IPC-ruler

## Setting Up MQTT Broker Locally

An MQTT broker can be set up locally in several ways. Here are two examples:

### Using Mosquitto and Running it as a Service

```bash
sudo pacman -S mosquitto
sudo systemctl start mosquitto.service
```

### Using Docker

```bash
docker run --rm -it --name mqtt -v $(pwd)/mosquitto.conf:/mosquitto/config/mosquitto.conf -v $(pwd)/mosquitto_data:/mosquitto/data eclipse-mosquitto
```

### Retrieving the IP Address of Docker Container

Run the following command to get the IP address of the Docker container:

```bash
docker inspect mqtt
```

## Configuring MQTT Broker

The placement of configuration files varies based on the startup method. With Docker, place it relative to where the
Docker container starts. If using a package manager, the default path is `/etc/lib/mosquitto`

### Minimal Configuration Example

Here's a non-secure config example (mosquitto.conf):

```bash
allow_anonymous true
listener 1883
listener 9001
persistence true
persistence_file /mosquitto/data/mosquitto.db
log_type all
autosave_on_changes true
```

## Simulating Offline State

You can simulate an offline state by forwarding the TCP traffic through socat. Here's an example of using socat with a
Docker-based Mosquitto server running at 172.18.0.2:

```bash
socat TCP4-LISTEN:1234,reuseaddr TCP4:172.18.0.2:1883
```

To simulate offline state, kill the socat process and restart it.

## Debugging the Database

To create a debugging database executable, follow these steps:

```bash
git clone https://github.com/eclipse/mosquitto/
cd mosquitto
make
./apps/db_dump/mosquitto_db_dump ~/mosquitto_data/mosquitto.db
```

Ensure to point to your instance of the database file.

## Wireshark

Install Wireshark with your package manager and add your user to the Wireshark group, for example:

```bash
sudo pacman -S wireshark-qt
sudo usermod -a -G wireshark $USER
# restart or log in again
```

If you are using Docker, the traffic will be limited to that interface (assuming you are not running other containers).

### Filter Example

Use the following filter example for Wireshark:

```bash
ip.src == 172.18.0.2 && ip.dst == 172.18.0.2 && tcp.port == 1883
```
