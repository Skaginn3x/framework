import React, { useEffect } from 'react';
import { AlertVariant, Spinner, Title } from '@patternfly/react-core';
import { SignalType, SlotType } from '../../Types';
import FormGenerator from '../Form/Form';
import { useAlertContext } from '../Alert/AlertContext';

interface ModalType {
  slots: SlotType[];
  signal: SignalType | undefined;
}

export default function FilterModal({
  signal, slots,
}: ModalType) {
  const { addAlert } = useAlertContext();

  useEffect(() => {
    console.log('signal: ', signal);
    console.log('slots:', slots);
  }, [signal, slots]);

  const [isLoading, setIsLoading] = React.useState(true); // Declare new state variable
  const [dbusFilterName, setDbusFilterName] = React.useState<string | undefined>(undefined);
  const [schema, setSchema] = React.useState<any>(undefined);
  const [formData, setFormData] = React.useState<any>(undefined);

  useEffect(() => {
    if (!signal) { return; }
    const externalScript = '../base1/cockpit.js';
    let script = document.querySelector(`script[src="${externalScript}"]`) as HTMLScriptElement;

    const handleScriptLoad = () => {
      const dbus = window.cockpit.dbus('org.freedesktop.DBus');
      const proxy = dbus.proxy();
      proxy.wait().then(() => {
        proxy.call('ListNames').then((Allnames: any[]) => {
          // Find correct filter dbus
          setDbusFilterName(
            Allnames[0].filter((name: string) => name.includes(signal.name)
            && name.includes('_filters_'))[0],
          );
        });
      });
    };

    if (!script) {
      console.log('3');
      script = document.createElement('script');
      script.src = externalScript;
      script.async = true;
      script.addEventListener('load', handleScriptLoad);
      script.addEventListener('error', (e) => { console.error('Error loading script', e); });
      document.body.appendChild(script);
    } else {
      handleScriptLoad();
      script.addEventListener('error', (e) => { console.error('Error loading script', e); });
    }
  }, []);

  // Get data and schema for each name and store in states
  useEffect(() => {
    if (dbusFilterName) {
      const dbus = window.cockpit.dbus(dbusFilterName);
      console.log('dbus:', dbusFilterName);
      const OBJproxy = dbus.proxy(dbusFilterName);
      OBJproxy.wait().then(() => {
        const { data } = OBJproxy;
        let parsedData = JSON.parse(data.config[0].replace('\\"', '"'));
        const parsedSchema = JSON.parse(data.config[1].replace('\\"', '"'));
        console.log('schema: ', parsedSchema);
        // TODO fix data coming from TFC to have config parent object like schema
        // if parseddata does not have key 'config'
        if (!Object.keys(parsedData).includes('config')) {
          parsedData = { config: parsedData };
        }
        // REMOVE after TODO is complete

        // eslint-disable-next-line arrow-body-style
        setSchema(parsedSchema);
        setFormData(parsedData);
        setIsLoading(false); // Set isLoading to false when schema is loaded
      });
    }
  }, [dbusFilterName]);

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

  /**
   * Update form data and dbus property config
   * @param name string
   * @param newData Object
   * @returns void
   */
  const updateFormData = (name: string | undefined, newData: any) => {
    if (!newData || !name) return;

    // Unwrap config object if it exists (for nested schemas)
    if (newData.config) {
      // eslint-disable-next-line no-param-reassign
      newData = newData.config;
    }

    // eslint-disable-next-line no-param-reassign
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

  return (
    <div style={{
      minWidth: '350px', width: '40vw', maxWidth: '500px', display: 'flex', flexDirection: 'column', alignItems: 'center',
    }}
    >
      {!isLoading && schema && signal
        ? (
          <>
            <Title headingLevel="h2" size="lg" style={{ marginBottom: '1rem', padding: '0.5rem' }}>
              {removeOrg(signal.name) || 'Error - Unknown name'}
            </Title>
            <FormGenerator
              inputSchema={schema}
              key={dbusFilterName}
              onSubmit={(data: any) => updateFormData(dbusFilterName, data)}
              values={formData}
            />
          </>
        ) : <Spinner />}
    </div>
  );
}
