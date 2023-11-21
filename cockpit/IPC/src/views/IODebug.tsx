/* eslint-disable react/no-unknown-property */
/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable react/jsx-no-undef */
/* eslint-disable react/no-unstable-nested-components */
import React, {
  useEffect, useRef, useState,
} from 'react';
import {
  Title,
  Divider,
  DataList,
  Spinner,
  AlertVariant,
} from '@patternfly/react-core';
import './IODebug.css';
import { DarkModeType } from 'src/App';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import loadExternalScript from 'src/Components/Interface/ScriptLoader';
import ListItem from 'src/Components/IOList/ListItem';
import { useAlertContext } from 'src/Components/Alert/AlertContext';
import TextboxAttribute from 'src/Components/Table/ToolbarItems/TextBoxAttribute';
import MultiSelectAttribute from 'src/Components/Table/ToolbarItems/MultiSelectAttribute';
import ToolBar, { FilterConfig } from 'src/Components/Table/Toolbar';

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
  const [processDBUS, setProcessDBUS] = useState<any>();
  const [processes, setProcesses] = useState<string[]>();
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);
  const [loading, setLoading] = useState<boolean>(true);

  const slotPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Slots`;
  const signalPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Signals`;

  const { addAlert } = useAlertContext();

  const dbusInterfaceRef = useRef<any[]>([]);

  useEffect(() => {
    loadExternalScript((allNames: string[]) => {
      setProcesses(
        allNames.filter((name: string) => name.includes(`${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.tfc.`)),
      );
      setProcessDBUS(window.cockpit.dbus(null, { bus: 'system', superuser: 'try' }));
    });
  }, []);

  const notifyHandler = (data: any) => {
    const removePath = data.detail[Object.keys(data.detail)[0]];
    if (!removePath) return;
    const ifaceName = Object.keys(removePath)[0];
    const changedIndex = dbusInterfaceRef.current.findIndex((iface) => iface.interfaceName === ifaceName);
    if (changedIndex !== -1 && dbusInterfaceRef.current[changedIndex].listener) {
      setDbusInterfaces((prevState) => {
        const newState = [...prevState];
        const index = newState.findIndex((iface) => iface.interfaceName === ifaceName);
        if (index !== -1) {
          newState[index].data = removePath[ifaceName].Value;
        }
        return newState;
      });
    }
  };

  useEffect(() => {
    if (!processDBUS || !loading) return;

    const match = {
      path_namespace: `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}`,
    };
    processDBUS.watch(match);

    // Ensure only one listener is active at a time
    processDBUS.removeEventListener('notify', notifyHandler);
    console.log('Adding Event listener');
    processDBUS.addEventListener('notify', notifyHandler);

    setLoading(false);

    // eslint-disable-next-line consistent-return
    return () => {
      console.log('Removing Event listener');
      processDBUS.removeEventListener('notify', notifyHandler);
    };
  }, [processDBUS]);

  const subscribe = async (interfaceName: string) => {
    const interfaceIndex = dbusInterfaces.findIndex((iface) => iface.interfaceName === interfaceName);
    if (interfaceIndex === -1) return;
    const client = window.cockpit.dbus(dbusInterfaces[interfaceIndex].process, { bus: 'system', superuser: 'try' });
    const proxy = client.proxy(interfaceName, dbusInterfaces[interfaceIndex].direction === 'slot' ? slotPath : signalPath);
    await proxy.wait().then(() => {
      setDbusInterfaces((prevState) => {
        const newState = [...prevState];
        const index = newState.findIndex((iface) => iface.interfaceName === interfaceName);
        if (index !== -1) {
          newState[index].listener = true;
          newState[index].data = proxy.data.Value;
        }
        return newState;
      });
    }).catch((error: any) => {
      addAlert(`Failed to subscribe to ${interfaceName}`, AlertVariant.danger);
      console.error('Failed to subscribe to interface:', error);
    });
  };

  const unsubscribe = (interfaceName: string) => {
    setDbusInterfaces((prevState) => {
      const newState = [...prevState];
      const index = newState.findIndex((iface) => iface.interfaceName === interfaceName);
      if (index !== -1) {
        newState[index].listener = false;
      }
      if (index === -1) {
        addAlert(`Failed to unsubscribe from ${interfaceName}`, AlertVariant.danger);
      }
      return newState;
    });
  };

  useEffect(() => {
    dbusInterfaceRef.current = dbusInterfaces;
  }, [dbusInterfaces]);

  const getInterfaceData = async (interfaces:any, tempDBUS: any, path:string, direction:string, process:string) => {
    const processProxy = tempDBUS.proxy('org.freedesktop.DBus.Introspectable', path);

    try {
      const data = await processProxy.call('Introspect');
      const interfacesData = parseXMLInterfaces(data);

      // Create an array of promises for each proxy creation
      const proxyPromises = interfacesData.map(async (interfaceData) => {
        const proxy = tempDBUS.proxy(interfaceData.name, path);
        await proxy.wait(); // Wait for the proxy to be ready

        return {
          data: proxy.data.Value,
          process,
          interfaceName: interfaceData.name,
          typeJson: JSON.parse(proxy.Type) ?? interfaceData.valueType,
          direction,
          hidden: false,
          listener: false,
        };
      });

      // Wait for all proxies to be created
      const newInterfaces = await Promise.all(proxyPromises);
      interfaces.push(...newInterfaces);
    } catch (e) {
      console.error('Error in getInterfaceData:', e);
    }
  };

  const getInterfaceDataForProcess = async (process: any) => {
    const interfaces: any[] = [];
    const tempDBUS = window.cockpit.dbus(process, { bus: 'system', superuser: 'try' });
    try {
      await getInterfaceData(interfaces, tempDBUS, slotPath, 'slot', process);
    } catch (e) {
      console.error(`Failed to get interface data for process ${process}:`, e);
    }
    try {
      await getInterfaceData(interfaces, tempDBUS, signalPath, 'signal', process);
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
            {dbusInterfaces.length > 0 ? dbusInterfaces.map((dbusInterface: any) => {
              if (!dbusInterface.hidden && dbusInterface.direction === 'slot') {
                return (
                  <ListItem
                    dbusInterface={dbusInterface}
                    key={`${dbusInterface.interfaceName}-${dbusInterface.process}-List-Slot`}
                    isChecked={dbusInterface.listener}
                    data={dbusInterface.data}
                    onCheck={() => (dbusInterface.listener
                      ? unsubscribe(dbusInterface.interfaceName)
                      : subscribe(dbusInterface.interfaceName)
                    )}
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
            {dbusInterfaces.length > 0 ? dbusInterfaces.map((dbusInterface: any) => {
              if (!dbusInterface.hidden && dbusInterface.direction === 'signal') {
                return (
                  <ListItem
                    dbusInterface={dbusInterface}
                    key={`${dbusInterface.interfaceName}-${dbusInterface.process}-List-Signal`}
                    isChecked={dbusInterface.listener}
                    data={dbusInterface.data}
                    onCheck={() => (dbusInterface.listener
                      ? unsubscribe(dbusInterface.interfaceName)
                      : subscribe(dbusInterface.interfaceName)
                    )}
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
    </div>
  );
};

export default IODebug;
