/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable react/jsx-no-undef */
/* eslint-disable react/no-unstable-nested-components */
import React, { useEffect, useRef, useState } from 'react';
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
  Spinner,
} from '@patternfly/react-core';
import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import './IODebug.css';
import Circle from 'src/Components/Simple/Circle';
import DynamicNavbar from 'src/Components/NavBar/DynamicNavBar';
import Hamburger from 'hamburger-react';
import CustomMenuToggle from 'src/Components/Dropdown/CustomMenuToggle';
import { DarkModeType } from 'src/App';
import StringTinker from 'src/Components/Tinker/StringTinker';
import BoolTinker from 'src/Components/Tinker/BoolTinker';
import { removeSlotOrg } from 'src/Components/Form/WidgetFunctions';

declare global {
  interface Window { cockpit: any; }
}

// eslint-disable-next-line @typescript-eslint/no-unused-vars
const parseXMLInterfaces = (xml: string): { name: string, valueType: string }[] => {
  const parser = new DOMParser();
  const xmlDoc = parser.parseFromString(xml, 'text/xml');
  const interfaceElements = xmlDoc.querySelectorAll('interface[name^="com.skaginn3x."]');
  const interfaces: { name: string, valueType: string }[] = [];
  interfaceElements.forEach((element) => {
    const name = element.getAttribute('name');
    const valueType = element.querySelector('property[name="Value"]')?.getAttribute('type') || 'unknown';

    if (name) {
      interfaces.push({ name, valueType });
    }
  });

  return interfaces;
};

// eslint-disable-next-line react/function-component-definition
const IODebug:React.FC<DarkModeType> = ({ isDark }) => {
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);
  const [processes, setProcesses] = useState<string[]>();
  const [isDrawerExpanded, setIsDrawerExpanded] = useState<boolean>(true);
  const [activeDropdown, setActiveDropdown] = useState<number | null>(null);
  // eslint-disable-next-line @typescript-eslint/comma-spacing
  const eventHandlersRef = useRef<Map<string,(e:any) => void >>(new Map());
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
        const processProxy = processDBUS.proxy('org.freedesktop.DBus.Introspectable', '/com/skaginn3x/Slots');
        try {
          // eslint-disable-next-line no-await-in-loop, @typescript-eslint/no-loop-func
          await processProxy.call('Introspect').then((data:any) => {
            console.log(data);
            const interfacesData = parseXMLInterfaces(data);
            console.log('ifnames', interfacesData);
            // eslint-disable-next-line no-restricted-syntax
            for (const interfaceData of interfacesData) {
              const proxy = processDBUS.proxy(interfaceData.name, '/com/skaginn3x/Slots'); // Assuming all interfaces are at root
              proxy.wait().then(() => {
                const handler = handleChanged(interfaceData.name);
                console.log('adding event listeners');
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
      }
      console.log('ifaces', interfaces);
      setDbusInterfaces(interfaces);
    };

    fetchAndConnectInterfaces();
  }, [processes]);

  useEffect(() => {
    const callback = (allNames: string[]) => {
      console.log('Cherry Picked ', allNames.filter((name:string) => name.includes('com.skaginn3x.tfc.')));
      setProcesses(allNames.filter((name:string) => name.includes('com.skaginn3x.tfc.')));
    };
    loadExternalScript(callback);
  }, []);

  const onToggleClick = (index: number) => {
    if (activeDropdown === index) {
      setActiveDropdown(null);
    } else {
      setActiveDropdown(index);
    }
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
        content={`Value is ${data.Value ? 'true' : 'false'}`}
        enableFlip
        distance={5}
        entryDelay={1000}
      >
        <Circle size="1rem" color={data.Value ? 'green' : 'red'} />
      </Tooltip>
    );
  }
  function handleStringContent(data: any): JSX.Element {
    return (
      <p>{data.Value}</p>
    );
  }

  function toggleSelection(selected:string) {
    const updatedData = dbusInterfaces.map((dbusInterface) => ({
      ...dbusInterface,
      hidden: selected && dbusInterface.process !== selected,
    }));
    setDbusInterfaces(updatedData);
  }

  function getSecondaryContent(data: any): React.ReactElement | null {
    const internals = (interfacedata: any) => {
      switch (interfacedata.type) {
        case 'b':
          return handleBoolContent(interfacedata.proxy.data);

        case 's':
          return handleStringContent(interfacedata.proxy.data);

        case 'n': // INT16
        case 'q': // UINT16
        case 'i': // INT32
        case 'u': // UINT32
        case 'x': // INT64
        case 't': // UNIT64
        case 'd': // Double
        case 'y': // Byte
          return handleStringContent(interfacedata.proxy.data);

        default:
          return <>Type Error</>;
      }
    };

    return (
      <div style={{
        height: '100%',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'end',
      }}
      >
        {internals(data)}
      </div>
    );
  }

  function getTinkerInterface(data:any): React.ReactElement | null {
    const internals = (interfacedata: any) => {
      switch (interfacedata.type) {
        case 'b':
          return <BoolTinker data={interfacedata} />;

        case 's':
          return <StringTinker data={interfacedata} />;

        case 'n': // INT16
        case 'q': // UINT16
        case 'i': // INT32
        case 'u': // UINT32
        case 'x': // INT64
        case 't': // UNIT64
        case 'd': // Double
        case 'y': // Byte
          return <StringTinker data={interfacedata} />;

        default:
          return <>Type Error</>;
      }
    };
    return (
      <div style={{
        height: '100%',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'start',
      }}
      >
        {internals(data)}
      </div>
    );
  }

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
                  {dbusInterfaces.length > 0 ? dbusInterfaces.map((dbusInterface: any, index: number) => {
                    if (!dbusInterface.hidden) {
                      return (
                        <DataListItem aria-labelledby="check-action-item1" key={dbusInterface.proxy.iface + dbusInterface.process}>
                          <DataListItemRow size={10}>
                            <DataListItemCells
                              dataListCells={[
                                <DataListCell key="primary content" style={{ textAlign: 'left', maxWidth: '30rem' }}>
                                  <p style={{ minWidth: '30rem' }}>{removeSlotOrg(dbusInterface.proxy.iface)}</p>
                                </DataListCell>,
                                <DataListCell
                                  key="secondary content 1"
                                  style={{
                                    textAlign: 'right',
                                    height: '100%',
                                    display: 'flex',
                                    alignItems: 'center',
                                  }}
                                >
                                  {getSecondaryContent(dbusInterface)}
                                </DataListCell>,
                                <DataListCell
                                  key="secondary content 2"
                                  style={{
                                    textAlign: 'right',
                                    height: '100%',
                                    display: 'flex',
                                    alignItems: 'center',
                                  }}
                                >
                                  {getTinkerInterface(dbusInterface)}
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
                                className="DropdownItem"
                                onSelect={(e, val) => onSelect(e, val, index)}
                                key={`${dbusInterface.proxy.iface}${dbusInterface.process}`}
                                toggle={(toggleRef: React.Ref<MenuToggleElement>) => ( // NOSONAR
                                  <CustomMenuToggle
                                    toggleRef={(ref) => {
                                      if (typeof toggleRef === 'function') {
                                        toggleRef(ref);
                                      }
                                      dropdownRefs.current[index] = ref; // Store the ref for the dropdown
                                    }}
                                    onClick={() => onToggleClick(index)}
                                    isExpanded={dbusInterface.dropdown}
                                  />
                                )}
                                isOpen={activeDropdown === index}
                              >
                                <DropdownList>
                                  <DropdownItem key="tinker" style={{ textDecoration: 'none' }}> Tinker </DropdownItem>
                                  <DropdownItem key="history" style={{ textDecoration: 'none' }} isDisabled> View History </DropdownItem>
                                </DropdownList>
                              </Dropdown>
                            </DataListAction>
                          </DataListItemRow>
                        </DataListItem>
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
