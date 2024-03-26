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
import {
  getData, handleNullValue, sortItems, updateFormData,
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
  // const [names, setNames] = useState<Map<string, string>>(new Map());
  const [isDrawerExpanded, setIsDrawerExpanded] = useState(true);
  const [formSubmissionCount, setFormSubmissionCount] = useState(0);

  const toggleDrawer = () => {
    setIsDrawerExpanded(!isDrawerExpanded);
  };

  const [formData, setFormData] = useState<any>({});
  const [schemas, setSchemas] = useState<any>({});

  function loadData() {
    getData().then((allData) => {
      // data is an array of { name, data: { schemas, data } }
      console.log('Data:', allData);
      allData.forEach(({ name, data }) => {
        console.log('schemaz:', data.schemas);
        setSchemas((prevState: any) => {
          const newSchemas = { ...prevState };
          const filteredSchemas = Object.keys(data.schemas).reduce((acc: any, key: string) => {
            if (!key.includes('/Filter')) {
              acc[key] = data.schemas[key];
            }
            return acc;
          }, {});
          if (newSchemas[name]) {
            newSchemas[name] = { ...newSchemas[name], ...filteredSchemas };
          } else {
            newSchemas[name] = filteredSchemas;
          }
          return newSchemas;
        });

        setFormData((prevState: any) => {
          const newFormData = { ...prevState };
          const filteredData = Object.keys(data.data).reduce((acc: any, key: string) => {
            if (!key.includes('/Filter')) {
              acc[key] = data.data[key];
            }
            return acc;
          }, {});
          if (newFormData[name]) {
            newFormData[name] = { ...newFormData[name], ...filteredData };
          } else {
            newFormData[name] = filteredData;
          }
          return newFormData;
        });
      });
    });
  }

  useEffect(() => {
    loadData();
  }, []);

  const [activeItem, setActiveItem] = React.useState(Object.keys(schemas)[0]);
  const [activeItemProcess, setActiveItemProcess] = React.useState<string | undefined>(undefined);
  const onSelect = (selectedItem: {
    groupId: string | number;
    itemId: string | number;
  }) => {
    if (selectedItem.itemId) {
      setActiveItem(selectedItem.itemId as string);
      setIsDrawerExpanded(false);
    }
    if (selectedItem.groupId) {
      setActiveItemProcess(selectedItem.groupId as string);
    }
    if (!selectedItem.groupId && !selectedItem.itemId) {
      setActiveItemProcess(undefined);
    }
  };

  /**
   * Posts data to dbus
   * @param data Configuration values
   */
  function handleSubmit(data: any) {
    // eslint-disable-next-line no-param-reassign
    data = handleNullValue(data);
    // Find interface name from dbus name
    if (!activeItem) return;

    updateFormData(
      activeItemProcess, // Process name
      `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.Config`, // Interface name
      activeItem, // path
      'Value', // property
      data, // Data
      setFormData,
      addAlert,
    );
    setTimeout(() => {
      loadData();
    }, 100);
  }

  /**
   * Reads schemas, and gets the processes from the keys
   * Uses string splitting, which is not ideal, but no other option is available
   * @returns string[] of processes
   */
  function getProcesses(): string[] {
    const fullNameUnique = Object.keys(schemas).filter((value, index, self) => value && self.indexOf(value) === index);
    // 4. Sort the list alphabetically
    return sortItems(fullNameUnique);
  }

  function getPaths(process:string): string[] {
    if (Object.keys(schemas).includes(process)) {
      return sortItems(Object.keys(schemas[process] || []));
    }
    return [];
  }

  function prettify(name: string) {
    if (!name) return '';
    let newName = name.replaceAll('_', ' ');
    newName = newName.split(' ').map((word) => word.charAt(0).toUpperCase() + word.slice(1)).join(' ');
    return newName;
  }

  useEffect(() => {
    if (!schemas || !activeItem) return;
    setFormSubmissionCount((count) => count + 1);
  }, [activeItem, schemas]);

  const panelContent = (
    <DrawerPanelContent style={{ backgroundColor: '#212427' }}>
      <div style={{
        minWidth: '15rem', backgroundColor: '#212427', height: '-webkit-fill-available',
      }}
      >
        <Nav onSelect={(_, item) => onSelect(item)} aria-label="Grouped global">
          <NavGroup title="Processes">
            <NavItem
              preventDefault
              to="#all"
              key="all-navItem"
              groupId={undefined}
              isActive={activeItemProcess === undefined}
            >
              All
            </NavItem>
            {getProcesses().map((name) => (
              <NavItem
                preventDefault
                to={`#${name}`}
                key={`${name}-navItem`}
                groupId={name}
                isActive={activeItemProcess === name}
              >
                {name.split('.').slice(3).join('.')}
              </NavItem>
            ))}
          </NavGroup>
          <NavGroup title="Schemas">
            {activeItemProcess ? sortItems(getPaths(activeItemProcess))
              .map((name: string) => (
                <NavItem
                  preventDefault
                  to={`#${name}`}
                  key={`${name}-navItem`}
                  itemId={name}
                  isActive={activeItem === name}
                >
                  {name ? prettify(name.split('/').splice(-1)[0]) : name}
                </NavItem>
              ))
              : sortItems(Object.keys(schemas)).map((process) => (
                sortItems(Object.keys(schemas[process] || {})).map((path) => (
                  <NavItem
                    preventDefault
                    to={`#${path}`}
                    key={`${process}-${path}-navItem`}
                    groupId={process}
                    itemId={path}
                    isActive={activeItem === path && activeItemProcess === process}
                  >
                    {path ? `${process.split('.').slice(3).join('.')} ${prettify(path.split('/').splice(-1)[0])}` : path }
                  </NavItem>
                ))
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
                {activeItem && activeItemProcess
                  ? (
                    <div style={{ minWidth: '350px', width: '50vw', maxWidth: '600px' }} key={`${activeItem}div`}>
                      <Title headingLevel="h2" size="lg" style={{ marginBottom: '1rem', padding: '0.5rem' }}>
                        {`${activeItemProcess.split('.').slice(3).join('.')} ${prettify(activeItem.split('/').splice(-1)[0])}`}
                      </Title>
                      <FormGenerator
                        inputSchema={schemas[activeItemProcess][activeItem]}
                        key={`${activeItem}-form-${formSubmissionCount}`}
                        onSubmit={(data: any) => handleSubmit(data.values.config)}
                        values={formData[activeItemProcess][activeItem]}
                        intKey={formSubmissionCount}
                      />
                      <div style={{ marginBottom: '2rem' }} />
                    </div>
                  )
                  : <div style={{ minWidth: '350px', width: '50vw', maxWidth: '600px' }} key="emptyDiv" />}

              </div>
            </div>
          </DrawerContentBody>
        </DrawerContent>
      </Drawer>
    </div>
  );
};

export default Configurator;
