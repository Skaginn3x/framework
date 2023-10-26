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

  const testString = `digraph {

  [*] --> init
  init -[#gold]-> stopped #limegreen : set_stopped / lambda
  stopped : entry / lambda
  stopped : exit / lambda
  stopped -[#green]-> starting : set_starting / lambda
  stopped -[#green]-> starting : run_button / lambda
  starting : entry / lambda
  starting : exit / lambda
  starting --> running : starting_timeout / lambda
  starting --> running : starting_finished / lambda
  running : entry / lambda
  running : exit / lambda
  running --> stopping : run_button / lambda
  running --> stopping : set_stopped / lambda
  stopping : entry / lambda
  stopping : exit / lambda
  stopping --> stopped : stopping_timeout / lambda
  stopping --> stopped : stopping_finished / lambda
  stopped -[#green]-> cleaning : cleaning_button / lambda
  stopped -[#green]-> cleaning : set_cleaning / lambda
  cleaning : entry / lambda
  cleaning : exit / lambda
  cleaning --> stopped : cleaning_button / lambda
  cleaning --> stopped : set_stopped / lambda
  stopped -[#green]-> emergency : set_emergency / lambda
  stopping --> emergency : set_emergency / lambda
  starting --> emergency : set_emergency / lambda
  running --> emergency : set_emergency / lambda
  cleaning --> emergency : set_emergency / lambda
  fault --> emergency : set_emergency / lambda
  maintenance --> emergency : set_emergency / lambda
  stopped -[#green]-> emergency : emergency_on / lambda
  stopping --> emergency : emergency_on / lambda
  starting --> emergency : emergency_on / lambda
  running --> emergency : emergency_on / lambda
  cleaning --> emergency : emergency_on / lambda
  fault --> emergency : emergency_on / lambda
  maintenance --> emergency : emergency_on / lambda
  emergency : entry / lambda
  emergency : exit / lambda
  emergency --> stopped : emergency_off / lambda
  stopped -[#green]-> fault : fault_on / lambda
  stopped -[#green]-> fault : set_fault / lambda
  running --> fault : fault_on / lambda
  running --> fault : set_fault / lambda
  fault : entry / lambda
  fault : exit / lambda
  fault --> stopped : fault_off / lambda
  fault --> stopped : set_stopped / lambda
  stopped -[#green]-> maintenance : maintenance_button / lambda
  stopped -[#green]-> maintenance : set_maintenance / lambda
  maintenance : entry / lambda
  maintenance : exit / lambda
  maintenance --> stopped : maintenance_button / lambda
  maintenance --> stopped : set_stopped / lambda
  
  }`;

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
