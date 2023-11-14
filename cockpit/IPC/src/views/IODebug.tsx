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
  Spinner,
  Divider,
} from '@patternfly/react-core';
import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import './IODebug.css';
import ListItem from 'src/Components/IOList/ListItem';
import { DarkModeType } from 'src/App';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import DraggableModal from 'src/Components/DraggableModal/DraggableModal';
import { removeSlotOrg } from 'src/Components/Form/WidgetFunctions';
import { AngleDownIcon } from '@patternfly/react-icons';
import ToolBar, { FilterConfig } from 'src/Components/Table/Toolbar';
import MultiSelectAttribute from 'src/Components/Table/ToolbarItems/MultiSelectAttribute';
import TextboxAttribute from 'src/Components/Table/ToolbarItems/TextBoxAttribute';

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

  const handleChanged = (value: any, iFaceName:any) => {
    setDbusInterfaces((prevInterfaces) => {
      const index = prevInterfaces.findIndex((iface) => iface.interfaceName === iFaceName);
      if (index === -1) return prevInterfaces;

      const updatedInterfaces = [...prevInterfaces];
      updatedInterfaces[index].proxy.data.Value = value;
      if (openModals.includes(index) && !isMobile) {
        setHistory((prevHistory: any) => {
          const newHistory = { ...prevHistory };
          if (!newHistory[iFaceName]) {
            newHistory[iFaceName] = [];
          }
          if (newHistory[iFaceName].length > 200) { // Limit history to 200 entries per interface
            newHistory[iFaceName].shift();
          }
          newHistory[iFaceName].push({ value, timestamp: Date.now() });
          return newHistory;
        });
      }
      if (isMobile) {
        setUnsortedEvents((prevEvents: any) => {
          const newEvents = [...prevEvents];
          if (newEvents.length > 20) { // Limit history to 20 entries per interface
            newEvents.shift();
          }
          newEvents.push({ interface: iFaceName, value, timestamp: Date.now() });
          return newEvents;
        });
      }

      return updatedInterfaces;
    });
  };

  const subscribeToItem = async (name: string, process: string, path:string) => {
    const processDBUS = window.cockpit.dbus(process, { bus: 'system', superuser: 'try' });
    console.log('Subscribed to ', name);

    const match = {
      interface: 'org.freedesktop.DBus.Properties',
      path,
      member: 'PropertiesChanged', // Standard DBus signal for property changes
      arg0: name,
    };
    const subscription:any = processDBUS.subscribe(match, (pathz: any, iface: any, signal: any, args: any) => {
      // Check if the changed property is 'Value'
      if (args && Object.keys(args[1]).includes('Value') && args[1].Value.v !== undefined) {
        handleChanged(args[1].Value.v, name);
      }
    });
    eventHandlersRef.current.set(name, subscription);

    setDbusInterfaces((prevInterfaces) => {
      const index = prevInterfaces.findIndex((iface) => iface.interfaceName === name);
      if (index === -1) return prevInterfaces;

      const updatedInterfaces = [...prevInterfaces];
      updatedInterfaces[index].listener = subscription;
      return updatedInterfaces;
    });

    return subscription;
  };

  const unsubscribeFromItem = async (name: string) => {
    const subscription = dbusInterfaces.find((iface) => iface.interfaceName === name)?.listener;
    if (!subscription) return;

    subscription.remove();
    eventHandlersRef.current.delete(name);
    console.log('Unsubscribed from ', name);

    setDbusInterfaces((prevInterfaces) => {
      const index = prevInterfaces.findIndex((iface) => iface.interfaceName === name);
      if (index === -1) return prevInterfaces;

      const updatedInterfaces = [...prevInterfaces];
      updatedInterfaces[index].listener = false;
      return updatedInterfaces;
    });
  };

  const getInterfaceData = async (interfaces:any, processDBUS: any, path:string, direction:string, process:string) => {
    const processProxy = processDBUS.proxy('org.freedesktop.DBus.Introspectable', path);

    try {
      // eslint-disable-next-line no-await-in-loop, @typescript-eslint/no-loop-func
      await processProxy.call('Introspect').then(async (data: any) => {
        const interfacesData = parseXMLInterfaces(data);
        // eslint-disable-next-line no-restricted-syntax
        for (const interfaceData of interfacesData) {
          const proxy = processDBUS.proxy(interfaceData.name, path);
          // eslint-disable-next-line no-await-in-loop
          await proxy.wait().then(() => {
            interfaces.push({
              proxy,
              process,
              interfaceName: interfaceData.name,
              type: JSON.parse(proxy.Type).type[0] ?? interfaceData.valueType,
              direction,
              hidden: false,
              listener: false,
            });
          });
        }
      });
    } catch (e) {
      console.log(e);
    }
  };

  const getInterfaceDataForProcess = async (process: any) => {
    const interfaces: any[] = [];
    const processDBUS = window.cockpit.dbus(process, { bus: 'system', superuser: 'try' });
    try {
      await getInterfaceData(interfaces, processDBUS, slotPath, 'slot', process);
    } catch (e) {
      console.error(`Failed to get interface data for process ${process}:`, e);
    }
    try {
      await getInterfaceData(interfaces, processDBUS, signalPath, 'signal', process);
    } catch (e) {
      console.error(`Failed to get interface data for process ${process}:`, e);
    }
    return interfaces;
  };

  useEffect(() => {
    if (!processes) return;

    const fetchAndConnectInterfaces = async () => {
      const allInterfaces = await Promise.all(processes.map(getInterfaceDataForProcess));
      // Flatten the result since each item in allInterfaces is an array of interfaces
      const flatInterfaces = allInterfaces.flat();
      setDbusInterfaces(flatInterfaces);
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

      if (isNearBottom) {
        element.scrollTop = element.scrollHeight;
      }
    }
  }, [unsortedEvents]);

  // FILTER CODE
  const filterRefs: Record<string, React.RefObject<HTMLInputElement>> = {
    Name: useRef<HTMLInputElement | null>(null),
    Type: useRef<HTMLInputElement | null>(null),
    Process: useRef<HTMLInputElement | null>(null),
    Direction: useRef<HTMLInputElement | null>(null),
  };
  const [activeAttributeMenu, setActiveAttributeMenu] = React.useState<string>('Name');
  const [nameSelection, setNameSelection] = React.useState<string[]>([]);
  const [typeSelection, setTypeSelection] = React.useState<string[]>([]);
  const [directionSelection, setDirectionSelection] = React.useState<string[]>([]);
  const [processSelections, setProcessSelections] = React.useState<string[]>([]);
  const uniqueTypes = Array.from(new Set(dbusInterfaces.map((iface) => iface.type)));
  const uniqueProcesses = Array.from(new Set(dbusInterfaces.map((iface) => iface.process.split('.').slice(3).join('.'))));

  /**
   * Configuration file for the filters
   * Uses the Toolbar component's FilterConfig type
   */
  const Configs = [
    {
      key: 'Name',
      chips: nameSelection,
      categoryName: 'Name',
      setFiltered: setNameSelection,
      component:
  <TextboxAttribute
    selectedItems={nameSelection}
    setActiveItems={setNameSelection}
    attributeName="Name"
    activeAttributeMenu={activeAttributeMenu}
    innerRef={filterRefs.Name}
  />,
    },

    {
      key: 'Type',
      chips: typeSelection,
      categoryName: 'Type',
      setFiltered: setTypeSelection,
      component:
  <MultiSelectAttribute
    items={uniqueTypes}
    selectedItems={typeSelection}
    setActiveItems={setTypeSelection}
    attributeName="Type"
    activeAttributeMenu={activeAttributeMenu}
    innerRef={filterRefs.Type}
  />,
    },

    {
      key: 'Process',
      chips: processSelections,
      categoryName: 'Process',
      setFiltered: setProcessSelections,
      component:
  <MultiSelectAttribute
    items={uniqueProcesses}
    selectedItems={processSelections}
    setActiveItems={setProcessSelections}
    attributeName="Process"
    activeAttributeMenu={activeAttributeMenu}
    innerRef={filterRefs.Process}
  />,
    },

    {
      key: 'Direction',
      chips: directionSelection,
      categoryName: 'Direction',
      setFiltered: setDirectionSelection,
      component:
  <MultiSelectAttribute
    items={['signal', 'slot']}
    selectedItems={directionSelection}
    setActiveItems={setDirectionSelection}
    attributeName="Direction"
    activeAttributeMenu={activeAttributeMenu}
    innerRef={filterRefs.Direction}
  />,
    },
  ] as FilterConfig[];

  /**
   * When the the filters are updated, this function is called to determine if a signal should be displayed
   * @param signal The signal that is being checked
  */
  const onFilter = (dbusInterface: any) => {
    const createSafeRegex = (value: string) => {
      try {
        return new RegExp(value, 'i');
      } catch (err) {
        return new RegExp(value.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), 'i');
      }
    };
    let matchesSearchValue;
    if (nameSelection && nameSelection.length !== 0) {
      const searchRegexList = nameSelection.map((value) => createSafeRegex(value));
      matchesSearchValue = searchRegexList.some(
        (regex) => (dbusInterface.interfaceName).search(regex) >= 0,
      );
    } else {
      matchesSearchValue = true;
    }
    const matchesTypeSelection = typeSelection.length === 0 || typeSelection.includes(dbusInterface.type);

    const matchesDirection = directionSelection.length === 0 || directionSelection.includes(dbusInterface.direction);

    return (
      matchesSearchValue
      && matchesTypeSelection
      && (processSelections.length === 0 || processSelections.includes(dbusInterface.process.split('.').splice(3).join('.')))
      && matchesDirection
    );
  };

  useEffect(() => {
    const filteredInterfaces = dbusInterfaces.filter(onFilter);
    // set hidden to true for all except filteredInterfaces;
    setDbusInterfaces((prevInterfaces) => prevInterfaces.map((iface) => ({
      ...iface,
      hidden: !filteredInterfaces.includes(iface),
    })));
  }, [nameSelection, typeSelection, processSelections, directionSelection]);

  return (
    <div style={{
      minHeight: '100vh',
      fontFamily: '"RedHatText", helvetica, arial, sans-serif !important',
      width: '100%',
    }}
    >
      <div style={{
        minWidth: '300px',
        flex: 1,
        height: '100%',
        width: '100%',
        transition: 'width 0.2s ease-in-out',
      }}
      >
        <Title headingLevel="h1" size="2xl" style={{ marginBottom: '1rem', color: isDark ? '#EEE' : '#111' }}>
          IO Debugging Tool
        </Title>
        <ToolBar
          filteredItems={dbusInterfaces}
          filterConfigs={Configs}
          activeAttributeMenu={activeAttributeMenu}
          setActiveAttributeMenu={setActiveAttributeMenu}
          refs={filterRefs}
        />
        <div style={{
          width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center',
        }}
        >
          <Divider />
          <Title headingLevel="h2" size="xl" style={{ marginBottom: '2rem', color: isDark ? '#EEE' : '#111' }}>
            Slots
          </Title>
          <DataList aria-label="Checkbox and action data list example" key="slots">
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
                    onCheck={() => (dbusInterface.listener
                      ? unsubscribeFromItem(dbusInterface.interfaceName)
                      : subscribeToItem(dbusInterface.interfaceName, dbusInterface.process, slotPath))}
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
          <DataList aria-label="Checkbox and action data list example" key="signals">
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
                    onCheck={() => (dbusInterface.listener
                      ? unsubscribeFromItem(dbusInterface.name)
                      : subscribeToItem(dbusInterface.interfaceName, dbusInterface.process, slotPath))}
                  />
                );
              }
              return null;
            })
              : <Spinner />}
          </DataList>
          <div style={{ height: '10rem' }} />
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
                    zIndex: 2,
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
    </div>
  );
};

export default IODebug;
