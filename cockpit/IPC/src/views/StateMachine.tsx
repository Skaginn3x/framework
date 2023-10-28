/* eslint-disable @typescript-eslint/no-unused-vars */
/* eslint-disable react/no-unknown-property */
/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable react/jsx-no-undef */
/* eslint-disable react/no-unstable-nested-components */
import React, {
  useEffect, useRef, useState,
} from 'react';

import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import './IODebug.css';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import { Graphviz } from 'graphviz-react';
import { Spinner } from '@patternfly/react-core';

declare global {
  interface Window { cockpit: any; }
}

/**
 * Parses DBUS XML strings to extract interfaces
 * @param xml XML string
 * @returns Array of interfaces with name and value type { name: string, valueType: string}
 */
const parseXMLInterfaces = (xml: string): string[] => {
  const parser = new DOMParser();
  const xmlDoc = parser.parseFromString(xml, 'text/xml');
  const interfaceElements = xmlDoc.querySelectorAll(`interface[name^="${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}."]`);
  const interfaces: string[] = [];
  interfaceElements.forEach((element) => {
    const name = element.getAttribute('name');

    if (name) {
      interfaces.push(name);
    }
  });

  return interfaces;
};

// eslint-disable-next-line react/function-component-definition
const StateMachine: React.FC = () => {
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);
  const [processes, setProcesses] = useState<string[]>();
  // eslint-disable-next-line @typescript-eslint/comma-spacing
  const eventHandlersRef = useRef<Map<string,(e: any) => void>>(new Map()); // NOSONAR

  const stateMachinePath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/StateMachines`;

  const getInterfaceData = async (interfaces:any, processDBUS: any, path:string, process:string) => {
    const handleChanged = (value: any, name:any) => {
      setDbusInterfaces((prevInterfaces) => {
        const index = prevInterfaces.findIndex((iface) => iface.interfaceName === name);
        if (index === -1) return prevInterfaces;

        const updatedInterfaces = [...prevInterfaces];
        updatedInterfaces[index].proxy.data.StateMachine = value;

        return updatedInterfaces;
      });
    };
    const processProxy = processDBUS.proxy('org.freedesktop.DBus.Introspectable', path);
    try {
      // eslint-disable-next-line no-await-in-loop, @typescript-eslint/no-loop-func
      await processProxy.call('Introspect').then(async (data: any) => {
        const interfacesData = parseXMLInterfaces(data);
        console.log('data', interfacesData);
        // eslint-disable-next-line no-restricted-syntax
        for (const interfaceData of interfacesData) {
          const proxy = processDBUS.proxy(interfaceData, path);
          console.log('proxy', proxy);
          // eslint-disable-next-line no-await-in-loop
          await proxy.wait().then(() => {
            const match = {
              interface: 'org.freedesktop.DBus.Properties',
              path,
              member: 'PropertiesChanged', // Standard DBus signal for property changes
              arg0: interfaceData,
            };
            const subscription:any = processDBUS.subscribe(match, (pathz: any, iface: any, signal: any, args: any) => {
              // Check if the changed property is 'Value'
              if (args && Object.keys(args[1]).includes('StateMachine') && args[1].StateMachine.v !== undefined) {
                handleChanged(args[1].StateMachine.v, interfaceData);
              }
            });
            eventHandlersRef.current.set(interfaceData, subscription);
            interfaces.push({
              proxy,
              process,
              interfaceName: interfaceData,
              hidden: false,
            });
          });
        }
      });
    } catch (e) {
      console.log(e);
    }
  };

  useEffect(() => {
    if (!processes) return;

    const fetchAndConnectInterfaces = async () => {
      const interfaces: any[] = [];

      // eslint-disable-next-line no-restricted-syntax
      for (const process of processes) {
        const processDBUS = window.cockpit.dbus(process, { bus: 'system', superuser: 'try' });
        console.log('process', process);
        // eslint-disable-next-line no-await-in-loop
        await getInterfaceData(interfaces, processDBUS, stateMachinePath, process);
      }
      console.log('ifaces', interfaces);
      setDbusInterfaces(interfaces);
    };

    fetchAndConnectInterfaces();
  }, [processes]);

  useEffect(() => {
    const callback = (allNames: string[]) => {
      setProcesses(
        allNames.filter((name: string) => name.includes(`${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.tfc.`)),
      );
    };
    loadExternalScript(callback);
  }, []);

  const [testString, setTestString] = useState<string>(`digraph G {

    // Node definitions
    init [shape=ellipse, color=black];
    stopped [shape=ellipse, color=limegreen, label="stopped\nentry / lambda\nexit / lambda"];
    starting [shape=ellipse, label="starting\nentry / lambda\nexit / lambda"];
    running [shape=ellipse, label="running\nentry / lambda\nexit / lambda"];
    stopping [shape=ellipse, label="stopping\nentry / lambda\nexit / lambda"];
    cleaning [shape=ellipse, label="cleaning\nentry / lambda\nexit / lambda"];
    emergency [shape=ellipse, label="emergency\nentry / lambda\nexit / lambda"];
    fault [shape=ellipse, label="fault\nentry / lambda\nexit / lambda"];
    maintenance [shape=ellipse, label="maintenance\nentry / lambda\nexit / lambda"];
  
    // Transitions
    init -> stopped [color=gold, label="set_stopped / lambda"];
    stopped -> starting [color=green, label="set_starting / lambda"];
    stopped -> starting [color=green, label="run_button / lambda"];
    starting -> running [label="starting_timeout / lambda"];
    starting -> running [label="starting_finished / lambda"];
    running -> stopping [label="run_button / lambda"];
    running -> stopping [label="set_stopped / lambda"];
    stopping -> stopped [label="stopping_timeout / lambda"];
    stopping -> stopped [label="stopping_finished / lambda"];
    stopped -> cleaning [color=green, label="cleaning_button / lambda"];
    stopped -> cleaning [color=green, label="set_cleaning / lambda"];
    cleaning -> stopped [label="cleaning_button / lambda"];
    cleaning -> stopped [label="set_stopped / lambda"];
    stopped -> emergency [color=green, label="set_emergency / lambda"];
    stopping -> emergency [label="set_emergency / lambda"];
    starting -> emergency [label="set_emergency / lambda"];
    running -> emergency [label="set_emergency / lambda"];
    cleaning -> emergency [label="set_emergency / lambda"];
    fault -> emergency [label="set_emergency / lambda"];
    maintenance -> emergency [label="set_emergency / lambda"];
    stopped -> emergency [color=green, label="emergency_on / lambda"];
    stopping -> emergency [label="emergency_on / lambda"];
    starting -> emergency [label="emergency_on / lambda"];
    running -> emergency [label="emergency_on / lambda"];
    cleaning -> emergency [label="emergency_on / lambda"];
    fault -> emergency [label="emergency_on / lambda"];
    maintenance -> emergency [label="emergency_on / lambda"];
    emergency -> stopped [label="emergency_off / lambda"];
    stopped -> fault [color=green, label="fault_on / lambda"];
    stopped -> fault [color=green, label="set_fault / lambda"];
    running -> fault [label="fault_on / lambda"];
    running -> fault [label="set_fault / lambda"];
    fault -> stopped [label="fault_off / lambda"];
    fault -> stopped [label="set_stopped / lambda"];
    stopped -> maintenance [color=green, label="maintenance_button / lambda"];
    stopped -> maintenance [color=green, label="set_maintenance / lambda"];
    maintenance -> stopped [label="maintenance_button / lambda"];
    maintenance -> stopped [label="set_stopped / lambda"];
  }`);

  // Increment label on testString every second
  // useEffect(() => {
  //   const interval = setInterval(() => {
  //     setTestString((prevString) => prevString.replace(/color=green/, 'color=blue'));
  //   }, 1000);
  //   return () => clearInterval(interval);
  // }, []);
  useEffect(() => {
    const interval = setInterval(() => {
      // find svg element inside div with id graphviz0
      const svg = document.getElementById('graphviz0')?.getElementsByTagName('svg')[0];
      // toggle green edges, red&green
      if (svg) {
        const greenEdges = svg.querySelectorAll('g.edge path[fill="none"][stroke="#00ff00"]');
        const redEdges = svg.querySelectorAll('g.edge path[fill="none"][stroke="#ff0000"]');
        greenEdges.forEach((edge) => {
          edge.setAttribute('stroke', '#ff0000');
        });
        redEdges.forEach((edge) => {
          edge.setAttribute('stroke', '#00ff00');
        });
      }
    }, 1000);
    return () => clearInterval(interval);
  }, []);

  return (
    <div style={{
      height: '100vh',
      fontFamily: '"RedHatText", helvetica, arial, sans-serif !important',
      width: '100%',
    }}
    >
      <Graphviz dot={testString} />
      {/* {dbusInterfaces.length > 0 && dbusInterfaces[0].proxy.data.StateMachine
        ? <Graphviz dot={dbusInterfaces[0].proxy.data.StateMachine} />
        : <div style={{ height: '100vh', width: '100vw' }}><Spinner size="xl" /></div>} */}
    </div>
  );
};

export default StateMachine;
