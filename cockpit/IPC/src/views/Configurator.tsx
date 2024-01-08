import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
/* eslint-disable no-continue */
import React, { useEffect, useState } from 'react';
import {
  Title,
  Nav,
  NavGroup,
  NavItem,
  Drawer,
  DrawerPanelContent,
  DrawerContent,
  DrawerContentBody,
} from '@patternfly/react-core';
import 'bootstrap/dist/css/bootstrap.min.css';
import 'bootstrap-icons/font/bootstrap-icons.css';
import './Configurator.css';
import Hamburger from 'hamburger-react';
import { loadExternalScript, parseXMLInterfaces } from 'src/Components/Interface/ScriptLoader';
import {
  fetchDataFromDBus, handleNullValue, removeOrg, updateFormData,
} from 'src/Components/Form/WidgetFunctions';
import { useDarkMode } from 'src/Components/Simple/DarkModeContext';
import FormGenerator from '../Components/Form/Form';
import { useAlertContext } from '../Components/Alert/AlertContext';

declare global {
  interface Window { cockpit: any; }
}

// eslint-disable-next-line react/function-component-definition
const Configurator: React.FC = () => {
  const { addAlert } = useAlertContext();
  const { isDark } = useDarkMode();
  const [names, setNames] = useState<Map<string, string>>(new Map());
  const [isDrawerExpanded, setIsDrawerExpanded] = useState(true);

  const toggleDrawer = () => {
    setIsDrawerExpanded(!isDrawerExpanded);
  };

  const [formData, setFormData] = useState<any>({});
  const [schemas, setSchemas] = useState<any>({});

  // Load cockpit.js and get dbus names
  useEffect(() => {
    loadExternalScript(async (allNames) => {
      const filteredNames = allNames.filter(
        (name: string) => (
          name.includes(`${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.config`)
            || name.includes(`${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.tfc`))
            && !name.includes('ipc_ruler'),
      );

      // eslint-disable-next-line no-restricted-syntax
      for (const name of filteredNames) {
        const dbus = window.cockpit.dbus(name);
        const path = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/Config`;
        const processProxy = dbus.proxy('org.freedesktop.DBus.Introspectable', path);

        try {
          // eslint-disable-next-line no-await-in-loop
          const data = await processProxy.call('Introspect');
          const interfacesData = parseXMLInterfaces(data);
          setNames((prevNames) => new Map(prevNames).set(name, interfacesData[0].name));
        } catch (e) {
          console.error('Error in getInterfaceData:', e);
        }
      }
    });
  }, []);

  useEffect(() => {
    if (names.size > 0) {
      names.forEach((interfaceName, processName) => {
        fetchDataFromDBus(
          processName,
          interfaceName,
          'Config', // path
          'config', // property
        ).then(({ parsedData, parsedSchema }) => {
          setSchemas((prevState: any) => ({
            ...prevState,
            [processName]: parsedSchema,
          }));
          setFormData((prevState: any) => ({
            ...prevState,
            [processName]: parsedData,
          }));
        });
      });
    }
  }, [names]);

  /**
   * Get title from dbus name
   * @param name  string
   * @returns string
   */
  function getTitle(name: string) {
    // com.skaginn3x.config.etc.tfc.operation_mode.def.state_machine
    // return operation_mode.def.state_machine if there are more than 5 dots
    if (name.split('.').length > 5) {
      return name.split('.').slice(5).join('.');
    }
    return name;
  }

  const [activeItem, setActiveItem] = React.useState(Object.keys(schemas)[0]);
  const [activeItemFilter, setActiveItemFilter] = React.useState<string | undefined>(undefined);
  const [form, setForm] = React.useState<React.JSX.Element>(<div />);

  const onSelect = (selectedItem: {
    groupId: string | number;
    itemId: string | number;
  }) => {
    if (selectedItem.itemId) {
      setActiveItem(selectedItem.itemId as string);
      setIsDrawerExpanded(false);
    } else if (selectedItem.groupId) {
      setActiveItemFilter(selectedItem.groupId as string);
    } else {
      setActiveItemFilter(undefined);
    }
  };

  /**
   * Posts data to dbus
   * @param data Configuration values
   */
  function handleSubmit(data: any) {
    // eslint-disable-next-line no-param-reassign
    data = handleNullValue(data);
    updateFormData(
      activeItem, // Process name
      activeItem, // Interface name
      'Config', // Path
      'config', // property
      data, // Data
      setFormData,
      addAlert,
    );
  }

  /**
   * Reads schemas, and gets the processes from the keys
   * Uses string splitting, which is not ideal, but no other option is available
   * @returns string[] of processes
   */
  function getProcesses(): string[] {
    // 1. Extract processes from schema keys
    const extractedProcesses = Object.keys(schemas).map((schema) => {
      const parts = schema.split('.');
      return parts.slice(3, 5).join('.');
    });
    // 2. & 3. Filter out unique and non-undefined values
    const uniqueProcesses = extractedProcesses.filter((value, index, self) => value && self.indexOf(value) === index);
    // 4. Sort the list alphabetically
    return uniqueProcesses.sort((a, b) => {
      if (a < b) {
        return -1;
      }
      if (a > b) {
        return 1;
      }
      return 0;
    });
  }

  useEffect(() => {
    if (!schemas || !activeItem) return;
    setForm(
      <div style={{ minWidth: '350px', width: '50vw', maxWidth: '600px' }}>
        <Title headingLevel="h2" size="lg" style={{ marginBottom: '1rem', padding: '0.5rem' }}>
          {removeOrg(activeItem) || 'Error - Unknown name'}
        </Title>
        <FormGenerator
          inputSchema={schemas[activeItem]}
          key={activeItem}
          onSubmit={(data: any) => handleSubmit(data.values.config)}
          values={formData[activeItem]}
        />
        <div style={{ marginBottom: '2rem' }} />
      </div>,
    );
  }, [activeItem]);

  const panelContent = (
    <DrawerPanelContent style={{ backgroundColor: '#212427' }}>
      <div style={{
        minWidth: '15rem', backgroundColor: '#212427', height: '-webkit-fill-available',
      }}
      >
        <Nav onSelect={(_, item) => onSelect(item)} aria-label="Grouped global">
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
            {getProcesses().map((name: string) => (
              <NavItem
                preventDefault
                to={`#${name}`}
                key={`${name}-navItem`}
                groupId={name}
                isActive={activeItemFilter === name}
              >
                {name}
              </NavItem>
            ))}
          </NavGroup>
          {/* End Remove */}
          <NavGroup title="Schemas">
            {/* Might want to remove this slice too, if demoData is removed from state */}
            {Object.keys(schemas)
              .filter((name) => !activeItemFilter || name.includes(activeItemFilter))
              // .slice(4)
              .map((name: string) => (
                <NavItem
                  preventDefault
                  to={`#${name}`}
                  key={`${name}-navItem`}
                  itemId={name}
                  isActive={activeItem === name}
                >
                  {activeItemFilter ? getTitle(name) : removeOrg(name)}
                </NavItem>
              ))}
          </NavGroup>
        </Nav>
      </div>
    </DrawerPanelContent>
  );

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
                Configurator
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
                width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center', color: isDark ? '#EEE' : '#111',
              }}
              >
                {form}
              </div>
            </div>
          </DrawerContentBody>
        </DrawerContent>
      </Drawer>
    </div>
  );
};

export default Configurator;
