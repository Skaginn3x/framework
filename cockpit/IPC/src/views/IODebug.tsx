/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable react/jsx-no-undef */
/* eslint-disable react/no-unstable-nested-components */
import React, {
  useEffect, useRef, useState,
} from 'react';
import {
  DataList,
  Title,
  DrawerContent,
  Drawer,
  DrawerContentBody,
  DrawerPanelContent,
  Spinner,
  Divider,
} from '@patternfly/react-core';
import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import './IODebug.css';
import DynamicNavbar from 'src/Components/NavBar/DynamicNavBar';
import ListItem from 'src/Components/IOList/ListItem';
import Hamburger from 'hamburger-react';
import { DarkModeType } from 'src/App';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';

declare global {
  interface Window { cockpit: any; }
}

/**
 * Parses DBUS XML strings to extract interfaces
 * @param xml XML string
 * @returns Array of interfaces with name and value type { name: string, valueType: string}
 */
const parseXMLInterfaces = (xml: string): { name: string, valueType: string }[] => {
  const parser = new DOMParser();
  const xmlDoc = parser.parseFromString(xml, 'text/xml');
  const interfaceElements = xmlDoc.querySelectorAll(`interface[name^="${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}."]`);
  const interfaces: { name: string, valueType: string }[] = [];
  interfaceElements.forEach((element) => {
    const name = element.getAttribute('name');
    const valueType = element.querySelector('property[name="Value"]')?.getAttribute('type') ?? 'unknown';

    if (name) {
      interfaces.push({ name, valueType });
    }
  });

  return interfaces;
};

// eslint-disable-next-line react/function-component-definition
const IODebug: React.FC<DarkModeType> = ({ isDark }) => {
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);
  const [processes, setProcesses] = useState<string[]>();
  const [isDrawerExpanded, setIsDrawerExpanded] = useState<boolean>(true);
  const [activeDropdown, setActiveDropdown] = useState<number | null>(null);
  // eslint-disable-next-line @typescript-eslint/comma-spacing
  const eventHandlersRef = useRef<Map<string,(e: any) => void>>(new Map()); // NOSONAR

  const slotPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Slots`;
  const signalPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Signals`;

  useEffect(() => {
    if (!processes) return;

    const fetchAndConnectInterfaces = async () => {
      const interfaces: any[] = [];

      const handleChanged = (name: string) => (event: any) => {
        setDbusInterfaces((prevInterfaces) => {
          const index = prevInterfaces.findIndex((iface) => iface.interfaceName === name);
          if (index === -1) return prevInterfaces;

          const updatedInterfaces = [...prevInterfaces];
          updatedInterfaces[index].proxy.data.Value = event.detail.Value;
          return updatedInterfaces;
        });
      };

      // eslint-disable-next-line no-restricted-syntax
      for (const process of processes) {
        const processDBUS = window.cockpit.dbus(process, { bus: 'system', superuser: 'try' });
        const slotProcessProxy = processDBUS.proxy('org.freedesktop.DBus.Introspectable', slotPath);
        try {
          // eslint-disable-next-line no-await-in-loop, @typescript-eslint/no-loop-func
          await slotProcessProxy.call('Introspect').then((data: any) => {
            const interfacesData = parseXMLInterfaces(data);
            // eslint-disable-next-line no-restricted-syntax
            for (const interfaceData of interfacesData) {
              const proxy = processDBUS.proxy(interfaceData.name, slotPath);

              proxy.wait().then(() => {
                const handler = handleChanged(interfaceData.name);
                proxy.addEventListener('changed', handler);
                eventHandlersRef.current.set(interfaceData.name, handler);
                interfaces.push({
                  proxy,
                  process,
                  interfaceName: interfaceData.name,
                  type: interfaceData.valueType,
                  forcestate: null,
                  hidden: false,
                });
              });
            }
          });
        } catch (e) {
          console.log(e);
        }

        const signalProcessProxy = processDBUS.proxy('org.freedesktop.DBus.Introspectable', signalPath);
        try {
          // eslint-disable-next-line no-await-in-loop, @typescript-eslint/no-loop-func
          await signalProcessProxy.call('Introspect').then((data: any) => {
            const interfacesData = parseXMLInterfaces(data);
            // eslint-disable-next-line no-restricted-syntax
            for (const interfaceData of interfacesData) {
              const proxy = processDBUS.proxy(interfaceData.name, signalPath);
              proxy.wait().then(() => {
                const handler = handleChanged(interfaceData.name);
                proxy.addEventListener('changed', handler);
                eventHandlersRef.current.set(interfaceData.name, handler);
                interfaces.push({
                  proxy,
                  process,
                  interfaceName: interfaceData.name,
                  type: interfaceData.valueType,
                  direction: 'slot',
                  hidden: true,
                });
              });
            }
          });
        } catch (e) {
          console.log(e);
        }

        const signalProcessProxy = processDBUS.proxy('org.freedesktop.DBus.Introspectable', signalPath);
        try {
          // eslint-disable-next-line no-await-in-loop, @typescript-eslint/no-loop-func
          await signalProcessProxy.call('Introspect').then((data: any) => {
            const interfacesData = parseXMLInterfaces(data);
            // eslint-disable-next-line no-restricted-syntax
            for (const interfaceData of interfacesData) {
              const proxy = processDBUS.proxy(interfaceData.name, signalPath);
              proxy.wait().then(() => {
                const handler = handleChanged(interfaceData.name);
                proxy.addEventListener('changed', handler);
                eventHandlersRef.current.set(interfaceData.name, handler);
                interfaces.push({
                  proxy,
                  process,
                  interfaceName: interfaceData.name,
                  type: interfaceData.valueType,
                  direction: 'signal',
                  hidden: true,
                });
              });
            }
          });
        } catch (e) {
          console.log(e);
        }
      }
      interfaces[0].hidden = false;
      const signalIndex = interfaces.findIndex((iface) => iface.direction === 'signal' && iface.iface === interfaces[0]);
      interfaces[signalIndex >= 0 ? signalIndex : 0].hidden = false;

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

  /**
   * Toggles the dropdown for the given index
   * @param index Index of the dropdown
   */
  const onToggleClick = (index: number) => {
    if (activeDropdown === index) {
      setActiveDropdown(null);
    } else {
      setActiveDropdown(index);
    }
  };

  /**
   * Toggles the visibility of IO for the given process
   * @param selected The selected process
   */
  function toggleSelection(selected: string) {
    const updatedData = dbusInterfaces.map((dbusInterface) => ({
      ...dbusInterface,
      hidden: selected && dbusInterface.process !== selected,
    }));
    setDbusInterfaces(updatedData);
  }

  /**
   * Toggles the drawer
   */
  const toggleDrawer = () => {
    setIsDrawerExpanded(!isDrawerExpanded);
  };

  const dropdownRefs = useRef<any[]>([]); // Create a ref array to hold dropdown references

  useEffect(() => {
    const handleClickOutside = (event: MouseEvent) => {
      if (dropdownRefs.current.some((ref) => ref?.contains(event.target))) {
        return;
      }
      setActiveDropdown(null); // Close all dropdowns
    };

    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  return (
    <div style={{
      height: '100vh',
      fontFamily: '"RedHatText", helvetica, arial, sans-serif !important',
      width: '100%',
    }}
    >
      <Drawer isExpanded={isDrawerExpanded} position="right">
        <DrawerContent panelContent={(
          <DrawerPanelContent style={{ backgroundColor: '#212427' }}>
            <DynamicNavbar
              names={processes ?? []}
              onItemSelect={(it: string) => toggleSelection(it)}
            />
          </DrawerPanelContent>
        )}
        >
          <DrawerContentBody>
            <div style={{
              minWidth: '300px',
              flex: 1,
              height: '100%',
              width: isDrawerExpanded ? 'calc(100% - 28rem)' : '100%',
              transition: 'width 0.2s ease-in-out',
            }}
            >
              <Title headingLevel="h1" size="2xl" style={{ marginBottom: '1rem', color: isDark ? '#EEE' : '#111' }}>
                DBUS IO Debugging Tool
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
                width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center',
              }}
              >
                <Divider />
                <Title headingLevel="h2" size="xl" style={{ marginBottom: '2rem', color: isDark ? '#EEE' : '#111' }}>
                  Slots
                </Title>
                <DataList aria-label="Checkbox and action data list example">
                  {dbusInterfaces.length > 0 ? dbusInterfaces.map((dbusInterface: any, index: number) => {
                    if (!dbusInterface.hidden && dbusInterface.direction === 'slot') {
                      return (
                        <ListItem
                          dbusInterface={dbusInterface}
                          index={index}
                          key={`${dbusInterface.interfaceName}-${dbusInterface.process}-List-Slot`}
                          activeDropdown={activeDropdown}
                          dropdownRefs={dropdownRefs}
                          onToggleClick={onToggleClick}
                        />
                      );
                    }
                    return null;
                  })
                    : <Spinner />}
                </DataList>
                <Divider />
                <Title headingLevel="h2" size="xl" style={{ marginBottom: '2rem', color: isDark ? '#EEE' : '#111' }}>
                  Signals
                </Title>
                <DataList aria-label="Checkbox and action data list example">
                  {dbusInterfaces.length > 0 ? dbusInterfaces.map((dbusInterface: any, index: number) => {
                    if (!dbusInterface.hidden && dbusInterface.direction === 'signal') {
                      return (
                        <ListItem
                          dbusInterface={dbusInterface}
                          index={index}
                          key={`${dbusInterface.interfaceName}-${dbusInterface.process}-List-Signal`}
                          activeDropdown={activeDropdown}
                          dropdownRefs={dropdownRefs}
                          onToggleClick={onToggleClick}
                        />
                      );
                    }
                    return null;
                  })
                    : <Spinner />}
                </DataList>
              </div>
            </div>
          </DrawerContentBody>
        </DrawerContent>
      </Drawer>
    </div>
  );
};

export default IODebug;
