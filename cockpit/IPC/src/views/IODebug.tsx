/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable react/jsx-no-undef */
/* eslint-disable react/no-unstable-nested-components */
import React, { useEffect, useState } from 'react';
import {
  DataList,
  DataListItem,
  DataListItemCells,
  DataListItemRow,
  DataListCell,
  DataListAction,
  Dropdown,
  DropdownItem,
  Title,
  Tooltip,
  DrawerContent,
  Drawer,
  DrawerContentBody,
  DrawerPanelContent,
  MenuToggleElement,
  DropdownList,
} from '@patternfly/react-core';
import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import './IODebug.css';
import Circle from 'src/Components/Simple/Circle';
import DynamicNavbar from 'src/Components/NavBar/DynamicNavBar';
import Hamburger from 'hamburger-react';
import CustomMenuToggle from 'src/Components/Dropdown/CustomMenuToggle';
import { DarkModeType } from 'src/App';
// import { getIOData } from './demoData';

declare global {
  interface Window { cockpit: any; }
}

// eslint-disable-next-line @typescript-eslint/no-unused-vars
const connectToDBusNames = (names: string[], dbus: any) => {
  const proxies: any[] = [];
  names.forEach((name) => {
    const proxy = dbus.proxy(name);
    proxies.push(proxy);
  });
  return proxies;
};

// eslint-disable-next-line @typescript-eslint/no-unused-vars
const parseXMLInterfaces = (xml: string): string[] => {
  const parser = new DOMParser();
  const xmlDoc = parser.parseFromString(xml, 'text/xml');
  const interfaceElements = xmlDoc.querySelectorAll('interface[name^="com.skaginn3x."]');
  const interfaceNames: string[] = [];

  interfaceElements.forEach((element) => {
    const name = element.getAttribute('name');
    if (name) {
      interfaceNames.push(name);
    }
  });

  return interfaceNames;
};

// eslint-disable-next-line react/function-component-definition
const IODebug:React.FC<DarkModeType> = ({ isDark }) => {
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);
  const [processes, setProcesses] = useState<string[]>();
  const [isDrawerExpanded, setIsDrawerExpanded] = useState<boolean>(false);

  useEffect(() => {
    if (!processes) return;

    const fetchAndConnectInterfaces = async () => {
      const interfaces: any[] = [];

      // eslint-disable-next-line no-restricted-syntax
      for (const process of processes) {
        const processDBUS = window.cockpit.dbus(process, { bus: 'system', superuser: 'try' });
        console.log('DBUS', processDBUS);
        const processProxy = processDBUS.proxy('org.freedesktop.DBus.Introspectable', '/com/skaginn3x/Slot');
        // Awaiting should be ok in this loop, as it waits for each process
        console.log('PROXY', processProxy);
        // eslint-disable-next-line no-await-in-loop

        try {
          // eslint-disable-next-line no-await-in-loop, @typescript-eslint/no-loop-func
          await processProxy.call('Introspect').then((data:any) => {
            console.log(data);
            const interfaceNames = parseXMLInterfaces(data);
            console.log('ifnames', interfaceNames);
            // eslint-disable-next-line no-restricted-syntax
            for (const interfaceName of interfaceNames) {
              const proxy = processDBUS.proxy(interfaceName); // Assuming all interfaces are at root
              interfaces.push({
                proxy,
                process,
                interfaceName,
                dropdown: false,
                forcestate: null,
                hidden: false,
              });
            }
          });
        } catch (e) {
          console.log(e);
        }
      }
      console.log('ifaces', interfaces);
      setDbusInterfaces(interfaces);
    };

    fetchAndConnectInterfaces();
  }, [processes]);

  useEffect(() => {
    const callback = (allNames: string[]) => {
      console.log('all', allNames);
      console.log('cherry ', allNames.filter((name:string) => name.includes('com.skaginn3x.')));
      setProcesses(allNames.filter((name:string) => name.includes('com.skaginn3x.')));
    };
    loadExternalScript(callback);
  }, []);

  const onToggleClick = (index: number) => {
    const updatedInterfaces = [...dbusInterfaces];
    updatedInterfaces[index].dropdown = !dbusInterfaces[index].dropdown;
    setDbusInterfaces(updatedInterfaces);
  };
  const onSelect = (_event: React.MouseEvent<Element, MouseEvent> | undefined, value: string | number | undefined, index:number) => {
    // eslint-disable-next-line no-console
    console.log('selected', value);
    const updatedInterfaces = [...dbusInterfaces];
    updatedInterfaces[index].dropdown = false;
    setDbusInterfaces(updatedInterfaces);
  };

  function handleBoolContent(data: any): JSX.Element {
    return (
      <Tooltip
        content={`Value is ${data.value ? 'true' : 'false'}`}
        enableFlip
        distance={5}
        entryDelay={1000}
      >
        <Circle size="1rem" color={data.value ? 'green' : 'red'} />
      </Tooltip>
    );
  }
  function handleStringContent(data: any): JSX.Element {
    return (
      <p>{data.value}</p>
    );
  }

  function toggleSelection(selected:string) {
    const updatedData = dbusInterfaces.map((dbusInterface) => ({
      ...dbusInterface,
      hidden: selected && dbusInterface.process !== selected,
    }));
    console.log('updated: ', updatedData);
    console.log('selected: ', selected);
    setDbusInterfaces(updatedData);
  }

  function getSecondaryContent(data: any): JSX.Element {
    const internals = (interfacedata: any) => {
      switch (interfacedata.type) {
        case 'bool':
          return handleBoolContent(data);
        case 'string':
          return handleStringContent(data);
        case 'double':
          return handleStringContent(data);
        default:
          return <>Type Error</>;
      }
    };

    return (
      <div style={{
        width: '10rem',
        height: '100%',
        marginLeft: 'auto',
        display: 'flex',
        alignItems: 'center',
      }}
      >
        {internals(data)}
      </div>
    );
  }

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
        <DrawerContent panelContent={
          // eslint-disable-next-line react/jsx-wrap-multilines
          <DrawerPanelContent style={{ backgroundColor: '#212427' }}>
            <DynamicNavbar
              names={processes ?? []}
              onItemSelect={(it: string) => toggleSelection(it)}
            />
          </DrawerPanelContent>
        }
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
              <Title headingLevel="h1" size="2xl" style={{ marginBottom: '2rem', color: isDark ? '#EEE' : '#111' }}>
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
                <DataList aria-label="Checkbox and action data list example">
                  {dbusInterfaces.length > 0 && dbusInterfaces.map((dbusInterface: any, index: number) => {
                    if (!dbusInterface.hidden) {
                      return (
                        <DataListItem aria-labelledby="check-action-item1" key={dbusInterface.proxy.iface + dbusInterface.process}>
                          <DataListItemRow size={10}>
                            <DataListItemCells
                              dataListCells={[
                                <DataListCell key="primary content" style={{ textAlign: 'right' }}>
                                  <span id="check-action-item1">{dbusInterface.proxy.iface}</span>
                                </DataListCell>,
                                <DataListCell key="secondary content 1" style={{ textAlign: 'right' }}>
                                  {getSecondaryContent(dbusInterface.proxy)}
                                </DataListCell>,
                              ]}
                            />
                            <DataListAction
                              aria-labelledby="check-action-item1 check-action-action1"
                              id="check-action-action1"
                              aria-label="Actions"
                              isPlainButtonAction
                            >
                              <Dropdown
                                onSelect={(e, val) => onSelect(e, val, index)}
                                key={`${dbusInterface.proxy.iface}${dbusInterface.process}`}
                                toggle={(toggleRef: React.Ref<MenuToggleElement>) => (
                                  <CustomMenuToggle
                                    toggleRef={toggleRef}
                                    onClick={() => onToggleClick(index)}
                                    isExpanded={dbusInterface.dropdown}
                                  />
                                )}
                                isOpen={dbusInterface.dropdown ?? false}
                              >
                                <DropdownList>
                                  <DropdownItem key="action" style={{ textDecoration: 'none' }}>Action</DropdownItem>
                                  <DropdownItem key="disabled action" style={{ textDecoration: 'none' }} isDisabled>
                                    Disabled Action
                                  </DropdownItem>
                                </DropdownList>
                              </Dropdown>
                            </DataListAction>
                          </DataListItemRow>
                        </DataListItem>
                      );
                    }
                    return null;
                  })}

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
