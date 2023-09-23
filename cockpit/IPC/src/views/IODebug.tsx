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
} from '@patternfly/react-core';
import EllipsisVIcon from '@patternfly/react-icons/dist/esm/icons/ellipsis-v-icon';
import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import './IODebug.css';
import Circle from 'src/Components/Simple/Circle';
import DynamicNavbar from 'src/Components/NavBar/DynamicNavBar';
import Hamburger from 'hamburger-react';
import { getIOData } from './demoData';

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

export default function IODebug() {
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);
  const [isDrawerExpanded, setIsDrawerExpanded] = useState<boolean>(false);

  const simulateData = () => {
    if (dbusInterfaces.length === 0) { return; }
    const simulatedData = dbusInterfaces.map((dbusInterface) => {
      // Generate random data based on the type.
      let newValue;
      switch (dbusInterface.proxy.type) {
        case 'bool':
          newValue = Math.random() > 0.5;
          break;
        case 'string':
          newValue = Math.random() > 0.5 ? 'Yes, finished' : 'Stopp Finish';
          break;
        case 'double':
          newValue = (Math.random() * 100).toFixed(2);
          break;
        default:
          newValue = dbusInterface.proxy.value; // Default to current value if no simulation rule exists.
      }

      return {
        ...dbusInterface,
        proxy: {
          ...dbusInterface.proxy,
          value: newValue,
        },
      };
    });
    setDbusInterfaces(simulatedData);
  };

  useEffect(() => {
    const interval = setInterval(() => {
      simulateData();
    }, 500);

    // Clear the interval when the component is unmounted or when dbusInterfaces changes.
    return () => clearInterval(interval);
  }, [dbusInterfaces]);

  useEffect(() => {
    const callback = (allNames: string[]) => {
      // const dbus = window.cockpit.dbus('org.freedesktop.DBus');
      // const filteredNames = allNames.filter((name: string) => name.includes('config')); // && name.includes('_slot_')
      // const proxies = connectToDBusNames(filteredNames, dbus);
      console.log(allNames);
      const proxies = getIOData();
      const interfaces = proxies.map((proxy: any) => ({
        proxy,
        dropdown: false,
        forcestate: null,
        hidden: false,
      }));
      setDbusInterfaces(interfaces);
    };
    loadExternalScript(callback);
  }, []);

  const setDropdownAtIndex = (index: number, bool:boolean) => {
    const updatedInterfaces = [...dbusInterfaces];
    updatedInterfaces[index].dropdown = bool;
    setDbusInterfaces(updatedInterfaces);
  };

  const onFocus = (index:number) => {
    setDropdownAtIndex(index, false);
    const element = document.getElementById('toggle-initial-selection');
    element?.focus();
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
      hidden: selected && !dbusInterface.proxy.iface.includes(selected),
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
          <DrawerPanelContent>
            <DynamicNavbar
              names={dbusInterfaces.map((iface) => iface.proxy.iface)}
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
              width: isDrawerExpanded ? 'calc(100vw - 28rem)' : '100vw',
              transition: 'width 0.2s ease-in-out',
            }}
            >
              <Title headingLevel="h1" size="2xl" style={{ marginBottom: '2rem' }}>
                DBUS IO Debugging Tool
              </Title>
              <div style={{
                position: 'fixed',
                right: isDrawerExpanded ? '29rem' : '0.5rem',
                transition: 'right 0.2s ease-in-out',
                top: '0rem',
                zIndex: '10000',
              }}
              >
                <Hamburger
                  toggled={isDrawerExpanded}
                  toggle={toggleDrawer}
                  size={30}
                />
              </div>
              <div style={{
                width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center',
              }}
              >
                <DataList aria-label="Checkbox and action data list example">
                  {dbusInterfaces.map((dbusInterface: any, index: number) => {
                    if (!dbusInterface.hidden) {
                      return (
                        <DataListItem aria-labelledby="check-action-item1" key={dbusInterface.proxy.iface}>
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
                                onSelect={() => onFocus(index)}
                                onBlur={() => setDropdownAtIndex(index, false)}
                                key={`${dbusInterface.proxy.iface}-DD`}
                                toggle={(
                                  <button
                                    type="button"
                                    onClick={() => setDropdownAtIndex(index, !dbusInterface.dropdown)}
                                    className="Hovr"
                                  >
                                    <EllipsisVIcon aria-hidden="true" />
                                  </button>
                            )}
                                isOpen={dbusInterface.dropdown}
                            // onChange={() => toggleDropdownAtIndex(index)}
                                dropdownItems={[
                                  <DropdownItem key="action" style={{ textDecoration: 'none' }}>Action</DropdownItem>,
                                  <DropdownItem key="disabled action" style={{ textDecoration: 'none' }} isDisabled>
                                    Disabled Action
                                  </DropdownItem>,
                                ]}
                              />
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
}
