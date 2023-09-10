import React, { useEffect } from 'react';
import { AlertVariant, Spinner, Title } from '@patternfly/react-core';
import { SignalType, SlotType } from '../../Types';
import FormGenerator from '../Form/Form';
import { useAlertContext } from '../Alert/AlertContext';
import { loadExternalScript } from '../Interface/ScriptLoader';
import { removeOrg } from '../Form/WidgetFunctions';

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
    loadExternalScript((allNames) => {
      // Your specific logic to filter and set the dbusFilterName
      const filteredName = allNames.filter(
        (name: string) => name.includes(signal.name) && name.includes('_filters_'),
      )[0];
      setDbusFilterName(filteredName);
    });
  }, [signal]);

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
