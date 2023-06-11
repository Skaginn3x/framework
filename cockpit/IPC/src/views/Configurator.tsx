/* eslint-disable no-continue */
/* eslint-disable @typescript-eslint/no-use-before-define */
import React, { useEffect, useState } from 'react';
import {
  AlertVariant,
  Tab, TabTitleText, Tabs, Title,
} from '@patternfly/react-core';
import 'bootstrap/dist/css/bootstrap.min.css';
import 'bootstrap-icons/font/bootstrap-icons.css';
import './Configurator.css';
import {
  demoUiData3,
  demoUiSchema, demoUiSchema3,
} from './demoData';
import FormGenerator from '../Components/Form/Form';
import { useAlertContext } from '../Components/Alert/AlertContext';

declare global {
  interface Window { cockpit: any; }
}

export default function Configurator() {
  const [names, setNames] = useState<string[]>([]);
  const [formData, setFormData] = useState<any>({ variantDemo: {}, atvDemo: { config: demoUiData3() } });
  const [schemas, setSchemas] = useState<any>({ variantDemo: demoUiSchema(), atvDemo: addUnits(demoUiSchema3()) });
  const [activeTabKey, setActiveTabKey] = React.useState<string | number>(0);

  const { addAlert } = useAlertContext();

  // Handle Tabs for different DBus Interfaces
  const handleTabClick = (_event: any, tabIndex: string | number) => {
    setActiveTabKey(tabIndex);
  };

  // Load cockpit.js and get dbus names
  useEffect(() => {
    const externalScript = '../base1/cockpit.js';
    let script = document.querySelector(`script[src="${externalScript}"]`) as HTMLScriptElement;

    const handleScriptLoad = () => {
      const dbus = window.cockpit.dbus('org.freedesktop.DBus');
      const proxy = dbus.proxy();
      proxy.wait().then(() => {
        proxy.call('ListNames').then((Allnames: any[]) => {
          // if name includes skaginn3x, get interfaces
          console.log('names: ', Allnames[0].filter((name: string) => name.includes('config') && !name.includes('ipc_ruler')));
          setNames(Allnames[0].filter((name: string) => name.includes('config') && !name.includes('ipc_ruler')));
        });
      });
    };

    if (!script) {
      script = document.createElement('script');
      script.src = externalScript;
      script.async = true;
      script.addEventListener('load', handleScriptLoad);
      script.addEventListener('error', (e) => { console.error('Error loading script', e); });
      document.body.appendChild(script);
    } else {
      script.addEventListener('load', handleScriptLoad);
      script.addEventListener('error', (e) => { console.error('Error loading script', e); });
    }
  }, []);

  // Get data and schema for each name and store in states
  useEffect(() => {
    if (names.length > 0) {
      names.forEach((name: string) => {
        const dbus = window.cockpit.dbus(name);
        const OBJproxy = dbus.proxy(name);
        OBJproxy.wait().then(() => {
          const { data } = OBJproxy;
          // const parsedData = valueAsNumber(JSON.parse(data.config[0].replace('\\"', '"')));
          const parsedData = JSON.parse(data.config[0].replace('\\"', '"'));
          const parsedSchema = addUnits(JSON.parse(data.config[1].replace('\\"', '"')));
          // eslint-disable-next-line arrow-body-style
          setSchemas((prevState: any) => {
            return {
              ...prevState,
              [name]: parsedSchema,
            };
          });
          setFormData((prevState: any) => ({
            ...prevState,
            [name]: { config: parsedData },
          }));
        });
      });
    }
  }, [names]);

  function shouldHaveUnits(key: string) {
    const units = [
      'std::chrono::duration',
      'units::quantity',
      'int',
    ];
    // if units is found in key, return true
    return units.some((unit) => key.includes(unit));
  }

  /**
   * Recursively add unit widget to schema if key includes units
   */
  function addUnits(inputSchema: any) {
    // Deep clone the schema
    const schema = JSON.parse(JSON.stringify(inputSchema));

    function addUnitToObject(obj: any) {
      Object.keys(obj).forEach((key) => {
        if (typeof obj[key] === 'object') {
          addUnitToObject(obj[key]);
        }

        if (key === '$ref' && shouldHaveUnits(obj[key])) {
          // eslint-disable-next-line no-param-reassign
          obj.widget = 'Units';
        }
      });
    }

    addUnitToObject(schema);
    return schema;
  }

  /**
   * Update form data and dbus property config
   * @param name string
   * @param newData Object
   * @returns void
   */
  const updateFormData = (name: string, newData: any) => {
    if (!newData) return;
    if (newData.config) { // Unwrap config object if it exists (for nested schemas)
      // eslint-disable-next-line no-param-reassign
      newData = newData.config;
    }
    setFormData((prevState: any) => ({
      ...prevState,
      [name]: newData,
    }));

    // set dbus property config to data
    console.log('stringdata: (ss) ', [JSON.stringify(newData), '']);
    const newdbus = window.cockpit.dbus(name);
    const propProxy = newdbus.proxy(name);

    propProxy.wait().then(() => {
      const stringdata = window.cockpit.variant('(ss)', [JSON.stringify(newData), '']);
      newdbus.call(`/${name.replaceAll('.', '/')}`, 'org.freedesktop.DBus.Properties', 'Set', [
        name, // The interface name
        'config', // The property name
        stringdata, // The new value
      ]).then(() => {
        addAlert('Property updated successfully', AlertVariant.success);
        console.log('Property updated successfully');
      }).catch((error: any) => {
        addAlert('Failed to update property', AlertVariant.danger);
        console.error('Failed to update property:', error);
      });
    }).catch((error: any) => {
      addAlert('Failed to get DBUS proxy', AlertVariant.danger);
      console.error('Failed to get DBUS proxy:', error);
    });
  };

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

  /**
   * Remove org from dbus name
   * @param name  string
   * @returns string
   */
  function removeOrg(name: string) {
    // com.skaginn3x.config.etc.tfc.operation_mode.def.state_machine
    // return etc.tfc.operation_mode.def.state_machine
    if (name.split('.').length > 4) {
      return name.split('.').slice(3).join('.');
    }
    return name;
  }

  return (
    <div style={{ minWidth: '400px', maxWidth: '90vw' }}>
      <Title headingLevel="h1" size="2xl" style={{ marginBottom: '1rem' }}> Configurator - Time For Change </Title>
      <Tabs
        activeKey={activeTabKey}
        onSelect={handleTabClick}
        aria-label="Tabs in the horizontal overflow example"
        role="region"
        isOverflowHorizontal={false}
      >
        {names.length && Object.keys(schemas).length && Object.keys(schemas).map((name: string, index: number) => {
          if (schemas[name] && formData[name]) {
            return (
              <Tab
                eventKey={index}
                title={<TabTitleText>{getTitle(name) || 'Unknown name'}</TabTitleText>}
              >
                <div style={{
                  width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center',
                }}
                >
                  <div style={{ minWidth: '250px', width: '40vw', maxWidth: '500px' }}>
                    <Title headingLevel="h2" size="lg" style={{ marginBottom: '1rem', padding: '0.5rem' }}>
                      {removeOrg(name) || 'Error - Unknown name'}
                    </Title>
                    <FormGenerator
                      inputSchema={schemas[name]}
                      onSubmit={(data: any) => updateFormData(name, data)}
                      values={formData[name]}
                    />
                    <div style={{ marginBottom: '2rem' }} />
                  </div>
                </div>
              </Tab>
            );
          }
          return null;
        })}
      </Tabs>
      <div style={{ height: '2rem' }} />
    </div>
  );
}
