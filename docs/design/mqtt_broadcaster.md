# MQTT Broadcaster

## Description

This document provides a guide on using the MQTT broadcaster executable. The service forwards signal values
from the IPC manager to MQTT topics, providing a connection between IPC and Aveva. It can also be used for other
purposes.

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
under `/etc/tfc/mqtt-broadcaster/def/mqtt_broadcaster.json` and add the desired banned string under the `_banned_topics` vector.

```json
{
   "_banned_topics": ["tub_tipper"]
}
```

Example of banning all signals that contain the string tub_tipper.

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
# restart or relogin
```

If you are using Docker, the traffic will be limited to that interface (assuming you are not running other containers).

### Filter Example

Use the following filter example for Wireshark:

```bash
ip.src == 172.18.0.2 && ip.dst == 172.18.0.2 && tcp.port == 1883
```
