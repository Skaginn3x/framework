/* eslint-disable no-param-reassign */
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
import { Graphviz } from '@hpcc-js/wasm';
import { Spinner } from '@patternfly/react-core';
import parse from 'html-react-parser';
import { DarkModeType } from 'src/App';

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
const StateMachine: React.FC<DarkModeType> = ({ isDark }) => {
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);
  const [processes, setProcesses] = useState<string[]>();
  const [svg, setSVG] = useState<string>();

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

  function toggleDarkMode(svgString: string) {
    if (isDark) {
      svgString = svgString.replace(/fill="white"/g, 'fill="#1B1D21"');
      svgString = svgString.replace(/fill="black"/g, 'fill="#EEE"');
      svgString = svgString.replace(/stroke="black"/g, 'stroke="#EEE"');
      svgString = svgString.replace(/stroke="white"/g, 'stroke="#1B1D21"');
      svgString = svgString.replace(/<text /g, '<text fill="#EEE" ');
    } else {
      svgString = svgString.replace(/fill="#1B1D21"/g, 'fill="white"');
      svgString = svgString.replace(/fill="#EEE"/g, 'fill="black"');
      svgString = svgString.replace(/stroke="#EEE"/g, 'stroke="black"');
      svgString = svgString.replace(/stroke="#1B1D21"/g, 'stroke="white"');
      svgString = svgString.replace(/<text /g, '<text fill="black" ');
    }
    return svgString;
  }

  async function generateGraphviz() {
    if (dbusInterfaces.length === 0) return;
    const graphviz = await Graphviz.load();
    let svgString = graphviz.dot(dbusInterfaces[0].proxy.data.StateMachine);
    svgString = svgString.replace(/width="\d+\.?\d*pt"/g, 'width="100%"');
    // same with height
    svgString = svgString.replace(/height="\d+\.?\d*pt"/g, 'height="100%"');
    svgString = svgString.replace('<title>G</title>', '');
    svgString = toggleDarkMode(svgString);
    setSVG(svgString);
  }

  useEffect(() => {
    if (!svg) return;
    let newSVG = svg;
    newSVG = toggleDarkMode(newSVG);
    setSVG(newSVG);
  }, [isDark]);

  useEffect(() => {
    generateGraphviz();
  }, [dbusInterfaces]);

  useEffect(() => {
    const interval = setInterval(
      () => {
        if (!svg) return;
        let newSVG = svg;
        if (newSVG.includes('stroke="green"')) {
          newSVG = newSVG.replaceAll(/stroke="green"/g, 'stroke="red"');
          newSVG = newSVG.replaceAll(/fill="green"/g, 'fill="red"');
        } else {
          newSVG = newSVG.replaceAll(/stroke="red"/g, 'stroke="green"');
          newSVG = newSVG.replaceAll(/fill="red"/g, 'fill="green"');
        }
        setSVG(newSVG);
      },
      500,
    );
    return () => clearInterval(interval);
  }, [svg]);

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

  return (
    <div style={{
      maxHeight: '95vh',
      fontFamily: '"RedHatText", helvetica, arial, sans-serif !important',
      width: '95vw',
    }}
    >
      {parse(svg ?? '') ?? <div style={{ height: '100vh', width: '100vw' }}><Spinner size="xl" /></div>}

      {/* {dbusInterfaces.length > 0 && dbusInterfaces[0].proxy.data.StateMachine
        ? <Graphviz dot={dbusInterfaces[0].proxy.data.StateMachine} />
        : <div style={{ height: '100vh', width: '100vw' }}><Spinner size="xl" /></div>} */}
    </div>
  );
};

export default StateMachine;
