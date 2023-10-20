/* eslint-disable react/no-unknown-property */
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
import DraggableModal from 'src/Components/DraggableModal/DraggableModal';
import { removeSlotOrg } from 'src/Components/Form/WidgetFunctions';
import { AngleDownIcon } from '@patternfly/react-icons';

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
  const [isShowingEvents, setIsShowingEvents] = useState<boolean>(false);
  const [activeDropdown, setActiveDropdown] = useState<number | null>(null);
  const [history, setHistory] = useState<any>({});
  const [openModals, setOpenModals] = useState<number[]>([]);
  // eslint-disable-next-line @typescript-eslint/comma-spacing
  const eventHandlersRef = useRef<Map<string,(e: any) => void>>(new Map()); // NOSONAR
  const isMobile = window.matchMedia('(max-width: 768px)').matches;
  const [unsortedEvents, setUnsortedEvents] = useState<any[]>([]);

  const slotPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Slots`;
  const signalPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Signals`;

  const getInterfaceData = async (interfaces:any, processDBUS: any, path:string, direction:string, process:string) => {
    const handleChanged = (value: any, name:any) => {
      setDbusInterfaces((prevInterfaces) => {
        const index = prevInterfaces.findIndex((iface) => iface.interfaceName === name);
        if (index === -1) return prevInterfaces;

        const updatedInterfaces = [...prevInterfaces];
        updatedInterfaces[index].proxy.data.Value = value;
        setHistory((prevHistory: any) => {
          const newHistory = { ...prevHistory };
          if (!newHistory[name]) {
            newHistory[name] = [];
          }
          if (newHistory[name].length > 200) { // Limit history to 200 entries per interface
            newHistory[name].shift();
          }
          newHistory[name].push({ value, timestamp: Date.now() });
          return newHistory;
        });

        setUnsortedEvents((prevEvents: any) => {
          const newEvents = [...prevEvents];
          if (newEvents.length > 20) { // Limit history to 20 entries per interface
            newEvents.shift();
          }
          newEvents.push({ interface: name, value, timestamp: Date.now() });
          return newEvents;
        });

        return updatedInterfaces;
      });
    };
    const processProxy = processDBUS.proxy('org.freedesktop.DBus.Introspectable', path);
    try {
      // eslint-disable-next-line no-await-in-loop, @typescript-eslint/no-loop-func
      await processProxy.call('Introspect').then((data: any) => {
        const interfacesData = parseXMLInterfaces(data);
        // eslint-disable-next-line no-restricted-syntax
        for (const interfaceData of interfacesData) {
          const proxy = processDBUS.proxy(interfaceData.name, path);
          proxy.wait().then(() => {
            const match = {
              interface: 'org.freedesktop.DBus.Properties',
              path,
              member: 'PropertiesChanged', // Standard DBus signal for property changes
              arg0: interfaceData.name,
            };
            const subscription:any = processDBUS.subscribe(match, (pathz: any, iface: any, signal: any, args: any) => {
              // Check if the changed property is 'Value'
              if (args && Object.keys(args[1]).includes('Value') && args[1].Value.v !== undefined) {
                handleChanged(args[1].Value.v, interfaceData.name);
              }
            });
            eventHandlersRef.current.set(interfaceData.name, subscription);
            interfaces.push({
              proxy,
              process,
              interfaceName: interfaceData.name,
              type: interfaceData.valueType,
              direction,
              hidden: true,
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
        // eslint-disable-next-line no-await-in-loop
        await getInterfaceData(interfaces, processDBUS, slotPath, 'slot', process);
        // eslint-disable-next-line no-await-in-loop
        await getInterfaceData(interfaces, processDBUS, signalPath, 'signal', process);
      }
      if (interfaces.length === 0) return;
      interfaces[0].hidden = false;
      const signalIndex = interfaces.findIndex((iface) => iface.direction === 'signal' && iface.process === interfaces[0].process);
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
    if (isMobile) {
      setIsDrawerExpanded(false);
    }
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

    document.addEventListener('click', handleClickOutside);
    return () => document.removeEventListener('click', handleClickOutside);
  }, []);

  const openModal = (index: number) => {
    setOpenModals((prevOpenModals) => [...prevOpenModals, index]);
  };
  const closeModal = (index: number) => {
    setOpenModals((prevOpenModals) => prevOpenModals.filter((i) => i !== index));
  };
  const isModalOpen = (index: number) => openModals.includes(index);

  // use effect to scroll to bottom of div on unsortedEvent change
  const scrollRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    if (scrollRef.current) {
      const element = scrollRef.current;
      const isNearBottom = element.scrollTop + element.clientHeight >= element.scrollHeight - 80;
      console.log(element.scrollTop, element.clientHeight, element.scrollHeight);

      if (isNearBottom) {
        element.scrollTop = element.scrollHeight;
      }
    }
  }, [unsortedEvents]);

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
              onClose={() => toggleDrawer()}
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
                IO Debugging Tool
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
                          setModalOpen={(i:number) => openModal(i)}
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
                          setModalOpen={(i:number) => openModal(i)}
                        />
                      );
                    }
                    return null;
                  })
                    : <Spinner />}
                </DataList>
              </div>
            </div>
            {!isMobile && dbusInterfaces.map((dbusInterface:any, index:number) => (
              <DraggableModal
                key={`${dbusInterface.interfaceName}-${dbusInterface.process}-Modal`}
                iface={dbusInterface}
                isOpen={isModalOpen(index)}
                visibilityIndex={openModals.indexOf(index)}
                onClose={() => closeModal(index)}
                datapoints={history[dbusInterface.interfaceName] ?? []}
              >
                {history[dbusInterface.interfaceName]?.map((datapoint: any) => (
                  <React.Fragment key={datapoint.timestamp}>
                    <div
                      style={{
                        display: 'flex', flexDirection: 'row', justifyContent: 'space-between', padding: '0px 2rem',
                      }}
                    >
                      <div style={{ color: '#EEE' }}>
                        {/* eslint-disable-next-line max-len */ }
                        {`${new Date(datapoint.timestamp).toLocaleTimeString('de-DE')}.${new Date(datapoint.timestamp).getMilliseconds()}`}
                      </div>
                      <div style={{ color: '#EEE' }}>{datapoint.value.toString()}</div>
                    </div>
                  </React.Fragment>
                ))}
              </DraggableModal>
            ))}
            {isMobile
                && (
                <AngleDownIcon
                  onClick={() => setIsShowingEvents(!isShowingEvents)}
                  style={{
                    position: 'fixed',
                    bottom: 'calc(6.8rem + 5px)',
                    left: '5px',
                    transform: isShowingEvents ? 'translateY(0)' : 'translateY(6.8rem) rotate(180deg)',
                  }}
                  className="TransitionUp ArrowIcons"
                />
                )}
            {isMobile
            && (
            <div
              ref={scrollRef}
              style={{
                position: 'fixed',
                bottom: '0px',
                height: '6.8rem',
                overflowY: 'scroll',
                transform: isShowingEvents ? 'translateY(0)' : 'translateY(7rem)',
                backgroundColor: '#111',
                width: '100vw',
              }}
              className="TransitionUp"
            >
              {unsortedEvents.map((event:any) => (
                <div
                  key={event.timestamp}
                  style={{
                    display: 'flex',
                    flexDirection: 'row',
                    justifyContent: 'space-between',
                    padding: '0px 0.2rem',
                    width: '100vw',
                    color: '#EEE',
                  }}
                >
                  <p>
                    {removeSlotOrg(event.interface)}
                  </p>
                  <p>
                    {`${new Date(event.timestamp).toLocaleTimeString('de-DE')}.${new Date(event.timestamp).getMilliseconds()}`}
                  </p>
                  <p>{event.value.toString()}</p>
                </div>
              ))}
            </div>
            )}
          </DrawerContentBody>
        </DrawerContent>
      </Drawer>
    </div>
  );
};

export default IODebug;
