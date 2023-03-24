# Overview

## Introduction
TFC is a software framework designed to develop scalable and maintainable software for controlling mechanics in fish and poultry factories.
The framework provides the necessary tools to create software that can easily be reused, and includes generic components for control.
TFC is designed to run on an industrial PC with a touch screen and a user-friendly human-machine interface (HMI).
The framework is capable of connecting to frequency converters and input/output units through an EtherCAT master, which enables users to runtime connect inputs and outputs to running services.

## Proposed Work
The TFC framework includes the following components:

### Interprocess Communication Mechanisms
TFC includes two interprocess communication mechanisms, one with centralized mapping from a web interface and the second using a publish-subscribe pattern with predefined topics.

#### Type specific IPC
The first mechanism is centralized, meaning connections can be changed during runtime without restarting.
This IPC interface includes static types, such as boolean, integer, double, string, and JSON, and the mapping of connections is in a one-to-many format, similar to signals and slots.
Signals are stored as read-only parameters in the configuration, and slots have read-write in the configuration to specify which signal to listen to.
This is also brokerless.

#### Message based IPC
The second mechanism is using a publish-subscribe pattern with predefined topics.

### Config Management
The framework includes a config management component that allows users to change configuration parameters for all running services, and the service receives a callback for the given configuration change. The config manager, named confman, is a daemon that relays changes from the web interface to the running services. The service stores the configuration in JSON and provides the web interface with the service's JSON schema. The service also exposes a UI JSON schema to easily expose the information necessary to configure the service.

### Reporting Mechanism
TFC includes a reporting mechanism to report noticeable events and alarms exposed in the HMI.

Export metadata from Inventor to target and being able to order spare parts on the target based on lifetime or faults.
Store System Manual on target.

### Inter Process Communication Relay
The framework includes a relay for interprocess communication to SCADA using OPC UA or MQTT, and a relay from the two interprocess communication mechanisms from the current PC to other industrial PCs running this framework.

### Proactive Service
The idea is to offer proactive service where a running controller (industrial PC) can send log dump and/or stack trace into a Baader server.

## Serviceability
The serviceability of the TFC framework is accomplished through the web interface, which is most likely using the Red Hat Cockpit project with in-house-made plugins. The web interface provides the following items:

- Start and stop running systemd services that are using this framework
- View logs and download logs from a running system
- Configuration management of this framework's services
- Connection of IPC signals and slots
- Terminal
- Offline documentation for the framework and its running services in Readthedocs
- The controller will host a repository for the framework packages and be able to update and revert packages
- Map of EtherCAT devices, states of the devices, and the ability to navigate to a device to get greater information

## Deployment
The deployment of systems such as Fluid Ice System (FIS) is made with a UI JSON schema, which exposes high-level configuration parameters for electricians to configure systems when they have been connected electrically.

