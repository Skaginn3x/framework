/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable react/jsx-no-undef */
/* eslint-disable react/no-unstable-nested-components */
import React, {
  ReactElement, useEffect, useRef, useState,
} from 'react';
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
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import NumberTinker from 'src/Components/Tinker/NumberTinker';

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
   * Handles the content of the secondary column for booleans
   * @param data The data to be displayed
   * @returns ReactElement to be displayed
   */
  function handleBoolContent(data: any): ReactElement {
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

  /**
   * Handles the content of the secondary column for strings
   * @param data The data to be displayed
   * @returns ReactElement to be displayed
   */
  function handleStringContent(data: any): ReactElement {
    return (
      <p style={{ marginBottom: '0px' }}>{data.Value}</p>
    );
  }

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
   * Handles the content of the secondary column
   * @param data Interface data
   * @returns ReactElement to be displayed
   */
  function getSecondaryContent(data: any): ReactElement | null {
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

  /**
   * Handles the content of the third column to enable tinkering
   * @param data Interface data
   * @returns ReactElement to be displayed
   */
  function getTinkerInterface(data: any): React.ReactElement | null {
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
          return <NumberTinker data={interfacedata} />;

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
