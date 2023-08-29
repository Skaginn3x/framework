/* eslint-disable no-continue */
import React, { useEffect, useState } from 'react';
import {
  AlertVariant,
  Tab, TabTitleText, Tabs, Title,
} from '@patternfly/react-core';
import 'bootstrap/dist/css/bootstrap.min.css';
import 'bootstrap-icons/font/bootstrap-icons.css';
import './Configurator.css';
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
  const [activeTabKey, setActiveTabKey] = React.useState<string | number>(0);

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
          // if name includes config, get interfaces (discard ipc_ruler and _filters_)
          setNames(
            Allnames[0].filter((name: string) => name.includes('config')
            && !name.includes('ipc_ruler')
            && !name.includes('_filters_')),
          );
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
          let parsedData = JSON.parse(data.config[0].replace('\\"', '"'));
          const parsedSchema = JSON.parse(data.config[1].replace('\\"', '"'));

          // TODO fix data coming from TFC to have config parent object like schema
          // if parseddata does not have key 'config'
          if (!Object.keys(parsedData).includes('config')) {
            parsedData = { config: parsedData };
          }
          // REMOVE after TODO is complete

          // eslint-disable-next-line arrow-body-style
          setSchemas((prevState: any) => {
            return {
              ...prevState,
              [name]: parsedSchema,
            };
          });
          setFormData((prevState: any) => ({
            ...prevState,
            [name]: parsedData,
          }));
        });
      });
    }
  }, [names]);

  function handleNullValue(data: any) {
    // go through data and find if there is a internal_null_value_do_not_use: { null }
    // if found, remove the value part and make it null.
    // Couldnt get UI-Schema to accept null as a vlaue
    Object.keys(data).forEach((key) => {
      if (!data[key] && typeof data[key] !== 'object') { return; }
      if (!Object.keys(data[key]).includes('internal_null_value_do_not_use')) {
        handleNullValue(data[key]);
      } else if (Object.keys(data[key]).includes('internal_null_value_do_not_use') && data[key].internal_null_value_do_not_use === null) {
        // eslint-disable-next-line no-param-reassign
        data[key] = null;
      }
    });
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
    // eslint-disable-next-line no-param-reassign
    handleNullValue(newData);
    setFormData((prevState: any) => ({
      ...prevState,
      [name]: newData,
    }));

    // set dbus property config to data
    console.log('stringdata: (ss) ', [JSON.stringify(newData), '']);
    const newdbus = window.cockpit.dbus(name, { superuser: 'try' });
    const propProxy = newdbus.proxy(name);

    propProxy.wait().then(() => {
      const stringdata = window.cockpit.variant('(ss)', [JSON.stringify(newData), '']);
      newdbus.call(`/${name.replaceAll('.', '/')}`, 'org.freedesktop.DBus.Properties', 'Set', [
        name, // The interface name
        'config', // The property name
        stringdata, // The new value
      ]).then(() => {
        addAlert('Property updated successfully', AlertVariant.success);
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
    <div style={{ minWidth: '400px', maxWidth: 'calc (100vw - 20rem)' }}>
      <Title headingLevel="h1" size="2xl" style={{ marginBottom: '1rem' }}> Configurator - Time For Change </Title>
      <Tabs
        activeKey={activeTabKey}
        onSelect={handleTabClick}
        aria-label="Tabs in the horizontal overflow example"
        role="region"
        isOverflowHorizontal
      >
        {Object.keys(schemas).length && Object.keys(schemas).map((name: string, index: number) => {
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
