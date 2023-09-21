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
import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import { fetchDataFromDBus, removeOrg, updateFormData } from 'src/Components/Form/WidgetFunctions';
import {
  VariantData,
  VariantSchema,
  ATVDemoData,
  ATVDemoSchema,
  newDemoData1, newDemoSchema1, ArrayTestSchema,
} from './demoData';
import FormGenerator from '../Components/Form/Form';
import { useAlertContext } from '../Components/Alert/AlertContext';

declare global {
  interface Window { cockpit: any; }
}

// TODO: Remove demo data and schemas when done.
export default function Configurator() {
  const { addAlert } = useAlertContext();
  const [names, setNames] = useState<string[]>([]);
  const [isDrawerExpanded, setIsDrawerExpanded] = useState(false);

  const toggleDrawer = () => {
    setIsDrawerExpanded(!isDrawerExpanded);
  };

  const [formData, setFormData] = useState<any>(
    { // Demo Data, remove when done
      brandNew: newDemoData1(),
      arrayTest: {
        config: [{ amper: 256, a_int: 128 }, { amper: 512, a_int: 256 }, { amper: 1024, a_int: 512 }],
      },
      ATVDemo: ATVDemoData(),
      variant: VariantData(),
    },
  );
  const [schemas, setSchemas] = useState<any>(
    { // Demo Schemas, remove when done
      brandNew: newDemoSchema1(),
      arrayTest: ArrayTestSchema(),
      ATVDemo: ATVDemoSchema(),
      variant: VariantSchema(),
    },
  );

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
  const [form, setForm] = React.useState<React.JSX.Element>(<div />);

  const onSelect = (selectedItem: {
    groupId: string | number;
    itemId: string | number;
    to: string;
    event: React.FormEvent<HTMLInputElement>;
  }) => {
    console.log(selectedItem.itemId);
    setActiveItem(selectedItem.itemId as string);
    setIsDrawerExpanded(false);
  };

  function handleSubmit(data:any) {
    // eslint-disable-next-line no-param-reassign
    data = handleNullValue(data);
    console.log('data: ', data);
    updateFormData(activeItem, data, setFormData, addAlert);
  }

  useEffect(() => {
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
        minWidth: '15rem', backgroundColor: '#212427', height: '100%',
      }}
      >
        <Nav onSelect={onSelect} aria-label="Grouped global">
          {/* Remove this group to get rid of demo data */}
          <NavGroup title="Demo Schemas">
            {Object.keys(schemas).slice(0, 4).map((name: string) => (
              <NavItem
                preventDefault
                to={`#${name}`}
                key={`${name}-navItem`}
                itemId={name}
                isActive={activeItem === name}
              >
                {getTitle(name)}
              </NavItem>
            ))}
          </NavGroup>
          {/* End Remove */}
          <NavGroup title="Schemas">
            {/* Might want to remove this slice too, if demoData is removed from state */}
            {Object.keys(schemas).slice(4).map((name: string) => (
              <NavItem
                preventDefault
                to={`#${name}`}
                key={`${name}-navItem`}
                itemId={name}
                isActive={activeItem === name}
              >
                {getTitle(name)}
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
              width: isDrawerExpanded ? 'calc(100vw - 28rem)' : '100vw',
              transition: 'width 0.2s ease-in-out',
            }}
            >
              <Title
                headingLevel="h1"
                size="2xl"
                style={{ marginBottom: '1rem' }}
                className="title"
              >
                Configurator - Time For Change
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
                {form}
              </div>
            </div>
          </DrawerContentBody>
        </DrawerContent>
      </Drawer>
    </div>
  );
}
