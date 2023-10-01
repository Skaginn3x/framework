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
  Switch,
} from '@patternfly/react-core';

import 'bootstrap/dist/css/bootstrap.min.css';
import 'bootstrap-icons/font/bootstrap-icons.css';
import './Configurator.css';
import Hamburger from 'hamburger-react';
import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import { fetchDataFromDBus, removeOrg, updateFormData } from 'src/Components/Form/WidgetFunctions';
import { DarkModeType } from 'src/App';
import FormGenerator from '../Components/Form/Form';
import { useAlertContext } from '../Components/Alert/AlertContext';

declare global {
  interface Window { cockpit: any; }
}

// eslint-disable-next-line react/function-component-definition
const Configurator:React.FC<DarkModeType> = ({ isDark, setIsDark }) => {
  const { addAlert } = useAlertContext();
  const [names, setNames] = useState<string[]>([]);
  const [isDrawerExpanded, setIsDrawerExpanded] = useState(true);

  const toggleDrawer = () => {
    setIsDrawerExpanded(!isDrawerExpanded);
  };

  const [formData, setFormData] = useState<any>({});
  const [schemas, setSchemas] = useState<any>({});

  // Load cockpit.js and get dbus names
  useEffect(() => {
    loadExternalScript((allNames) => {
      const filteredNames = allNames.filter(
        (name: string) => name.includes('config')
        && !name.includes('ipc_ruler'),
        // && !name.includes('_filters_'),
      );
      setNames(filteredNames);
    });
  }, []);

  // Get data and schema for each name and store in states
  useEffect(() => {
    console.log('names: ', names);
    if (names.length > 0) {
      console.log('names > 0 ');
      names.forEach((name: string) => {
        fetchDataFromDBus(name).then(({ parsedData, parsedSchema }) => {
          setSchemas((prevState: any) => ({
            ...prevState,
            [name]: parsedSchema,
          }));
          setFormData((prevState: any) => ({
            ...prevState,
            [name]: parsedData,
          }));
        });
      });
    }
  }, [names]);

  function handleNullValue(data: any): any {
    if (Array.isArray(data)) {
      return data.filter((item) => item != null && item !== undefined).map(handleNullValue);
    }
    if (typeof data === 'object' && data !== null) {
      const newObj: any = {};
      // eslint-disable-next-line no-restricted-syntax, guard-for-in
      for (const key in data) {
        const childObj = handleNullValue(data[key]);

        if (key === 'internal_null_value_do_not_use' && childObj === null) {
          return null; // Set the entire object to null if this key is present and its value is null
        }

        if (childObj !== undefined) {
          newObj[key] = childObj;
        }
      }
      return newObj;
    }
    return data;
  }

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

  function handleSubmit(data:any) {
    // eslint-disable-next-line no-param-reassign
    data = handleNullValue(data);
    updateFormData(activeItem, data, setFormData, addAlert);
  }

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
      <div style={{ minWidth: '350px', width: '40vw', maxWidth: '500px' }}>
        <Title headingLevel="h2" size="lg" style={{ marginBottom: '1rem', padding: '0.5rem' }}>
          {removeOrg(activeItem) || 'Error - Unknown name'}
        </Title>
        <FormGenerator
          inputSchema={schemas[activeItem]}
          key={activeItem}
          onSubmit={(data: any) => handleSubmit(data)}
          values={formData[activeItem]}
        />
        <div style={{ marginBottom: '2rem' }} />
      </div>,
    );
  }, [activeItem]);

  const panelContent = (
    <DrawerPanelContent>
      <div style={{
        minWidth: '15rem', backgroundColor: '#212427',
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
      <div style={{
        width: '100%',
        backgroundColor: '#212427',
        display: 'flex',
        alignItems: 'flex-end',
        justifyContent: 'center',
        paddingBottom: '1rem',
        height: '-webkit-fill-available',
      }}
      >
        <div style={{ display: 'flex' }}>
          <Switch
            onChange={(_, state) => setIsDark(state)}
            isChecked={isDark}
          />
          <Title size="md" headingLevel="h5" color="#EEE" style={{ marginLeft: '1rem', color: '#EEE' }}>
            Dark Mode
          </Title>
        </div>
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
                Configurator - Time For Change
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
