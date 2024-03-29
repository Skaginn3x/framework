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
import {
  Drawer,
  DrawerContent,
  DrawerContentBody,
  DrawerPanelContent, Nav, NavGroup, NavItem, Spinner, Title, Tooltip,
} from '@patternfly/react-core';
import parse from 'html-react-parser';
import Hamburger from 'hamburger-react';
import { removeOrg } from 'src/Components/Form/WidgetFunctions';
import { useDarkMode } from 'src/Components/Simple/DarkModeContext';

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
  const { isDark } = useDarkMode();
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);
  const [processes, setProcesses] = useState<string[]>();
  const [svg, setSVG] = useState<string>();

  const [activeItem, setActiveItem] = React.useState('');
  const [activeItemFilter, setActiveItemFilter] = React.useState<string | undefined>(undefined);
  const [isDrawerExpanded, setIsDrawerExpanded] = useState(true);

  // eslint-disable-next-line @typescript-eslint/comma-spacing
  const eventHandlersRef = useRef<Map<string,(e: any) => void>>(new Map()); // NOSONAR

  const stateMachinePath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/StateMachines`;

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
    if (!activeItem) return;
    const graphviz = await Graphviz.load();
    let svgString = graphviz.dot(dbusInterfaces.find((iface) => iface.interfaceName === activeItem)?.proxy.data.StateMachine ?? '');
    svgString = svgString.replace(/width="\d+\.?\d*pt"/g, 'width="100%"');
    // same with height
    svgString = svgString.replace(/height="\d+\.?\d*pt"/g, 'height="100%"');
    svgString = svgString.replace('<title>G</title>', '');
    svgString = toggleDarkMode(svgString);
    setSVG(svgString);
  }

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
        // eslint-disable-next-line no-restricted-syntax
        for (const interfaceData of interfacesData) {
          const proxy = processDBUS.proxy(interfaceData, path);
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
    if (!svg) return;
    let newSVG = svg;
    newSVG = toggleDarkMode(newSVG);
    setSVG(newSVG);
  }, [isDark]);

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

  const [subtitle, setSubtile] = useState<string>('');

  const onSelect = (selectedItem: {
    groupId: string;
    itemId: string;
  }) => {
    if (selectedItem.itemId) {
      setActiveItem(selectedItem.itemId.split('=')[0]);
      const process = selectedItem.itemId.split('=')[1];
      setSubtile(`${process ?? ''} - ${removeOrg(selectedItem.itemId.split('=')[0])}`);
      setIsDrawerExpanded(false);
    } else if (selectedItem.groupId) {
      setActiveItemFilter(selectedItem.groupId);
      setActiveItem('');
    } else {
      setActiveItemFilter(undefined);
    }
  };

  useEffect(() => {
    if (!dbusInterfaces) return;
    if (activeItem) {
      generateGraphviz();
    }
  }, [dbusInterfaces, activeItem]);

  const panelContent = (
    <DrawerPanelContent style={{ backgroundColor: '#212427' }}>
      <div style={{
        minWidth: '15rem', backgroundColor: isDark ? '#212427' : '#EEEEEE', height: '-webkit-fill-available',
      }}
      >
        {processes && dbusInterfaces.length > 0
          ? (
            <Nav onSelect={(_, item) => onSelect(item as { groupId: string, itemId: string })} aria-label="Grouped global">
              {/* Remove this group to get rid of demo data */}
              <NavGroup title="Processes">
                <NavItem
                  preventDefault
                  to="#all"
                  key="all-navItem"
                  groupId={undefined}
                  isActive={activeItemFilter === undefined}
                >
                  All
                </NavItem>
                {processes.map((name: string) => {
                  if (dbusInterfaces.find((iface) => iface.process === name) === undefined) return null;
                  return (
                    <NavItem
                      preventDefault
                      to={`#${name}`}
                      key={`${name}-navItem`}
                      groupId={name}
                      isActive={activeItemFilter === name}
                    >
                      {name}
                    </NavItem>
                  );
                })}
              </NavGroup>
              {/* End Remove */}
              <NavGroup title="Interfaces">
                {/* Might want to remove this slice too, if demoData is removed from state */}
                {dbusInterfaces.filter((iface) => !activeItemFilter || iface.process === activeItemFilter).map((iface: any) => (
                  <Tooltip
                    content={iface.process}
                    key={`${iface.interfaceName}${iface.process}-Tooltip`}
                  >
                    <NavItem
                      preventDefault
                      to={`#${iface.interfaceName}`}
                      key={`${iface.interfaceName}${iface.process}-navItem`}
                      itemId={`${iface.interfaceName}=${iface.process}`}
                      isActive={activeItem === iface.interfaceName && activeItemFilter === iface.process}
                    >
                      {removeOrg(iface.interfaceName)}
                    </NavItem>
                  </Tooltip>
                ))}
              </NavGroup>
            </Nav>
          )
          : (
            <div style={{
              display: 'flex',
              flexDirection: 'column',
              justifyContent: 'center',
              alignItems: 'center',
              width: '100%',
              height: '100vh',
              color: isDark ? '#EEE' : '#111',
            }}
            >
              <p> No processes Running</p>
            </div>
          )}
      </div>
    </DrawerPanelContent>
  );
  const toggleDrawer = () => {
    setIsDrawerExpanded(!isDrawerExpanded);
  };

  return (
    <div style={{
      height: '100vh',
      fontFamily: '"RedHatText", helvetica, arial, sans-serif !important',
      width: '100%',
    }}
    >
      <Drawer isExpanded={isDrawerExpanded} position="right">
        <DrawerContent panelContent={panelContent}>
          <DrawerContentBody>
            <div style={{
              minWidth: '300px',
              flex: 1,
              height: '100%',
              width: isDrawerExpanded ? 'calc(100% - 28rem)' : '100%',
              transition: 'width 0.2s ease-in-out',
            }}
            >
              <Title
                headingLevel="h1"
                size="2xl"
                style={{ marginBottom: '2rem', color: isDark ? '#EEE' : '#111' }}
              >
                State Machines
              </Title>
              <Title
                headingLevel="h1"
                size="md"
                style={{ marginBottom: '2rem', color: isDark ? '#EEE' : '#111' }}
              >
                {subtitle}
              </Title>
              <div style={{
                position: 'fixed',
                right: isDrawerExpanded ? '29.5rem' : '1.5rem',
                transition: 'right 0.2s ease-in-out',
                top: '0rem',
                zIndex: '10000',
              }}
              >
                <Hamburger
                  toggled={isDrawerExpanded}
                  toggle={toggleDrawer}
                  size={30}
                  color={isDark ? '#EEE' : undefined}
                />
              </div>
              <div style={{
                width: '100%',
                display: 'flex',
                flexDirection: 'column',
                alignItems: 'center',
                color: isDark ? '#EEE' : '#111',
                maxHeight: 'calc(100vh - 4rem)',
              }}
              >
                {parse(svg ?? '') ?? <div style={{ height: '100vh', width: '100vw' }}><Spinner size="xl" /></div>}
              </div>
            </div>
          </DrawerContentBody>
        </DrawerContent>
      </Drawer>
    </div>
  );
};

export default StateMachine;
