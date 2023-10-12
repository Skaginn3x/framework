import React, { useEffect } from 'react';
import { AlertVariant, Spinner, Title } from '@patternfly/react-core';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import FormGenerator from '../Form/Form';
import { useAlertContext } from '../Alert/AlertContext';
import { loadExternalScript } from '../Interface/ScriptLoader';
import { fetchDataFromDBus, removeOrg, updateFormData } from '../Form/WidgetFunctions';

interface ModalType {
  slot: string | undefined;

}

export default function FilterModal({
  slot,
}: ModalType) {
  const { addAlert } = useAlertContext();

  const [isLoading, setIsLoading] = React.useState(true); // Declare new state variable
  const [error, setError] = React.useState<string[] | undefined>(undefined); // Declare new state variable
  const [dbusFilterName, setDbusFilterName] = React.useState<string | undefined>(undefined);
  const [schema, setSchema] = React.useState<any>(undefined);
  const [formData, setFormData] = React.useState<any>(undefined);

  useEffect(() => {
    if (!slot) { return; }
    loadExternalScript((allNames) => {
      // Your specific logic to filter and set the dbusFilterName
      const filteredName = allNames.filter(
        (name: string) => name.includes(`${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.tfc`),
      )[0];

      if (!filteredName || filteredName.length === 0) { // If not found, notify user
        addAlert(`DBUS-Name-Error: No BUS interface with name "${slot}"`, AlertVariant.danger);
        setError(['DBUS-Name-Error:', 'No BUS interface with name ', `"${slot}`]);
      }
      setDbusFilterName(filteredName);
    });
  }, [slot]);

  useEffect(() => {
    if (!slot) { return; }
    if (dbusFilterName) {
      fetchDataFromDBus(
        dbusFilterName, // Process name
        `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.${slot}`, // Interface name
        'Slots', // Path
        'Filter', // Property
      ).then(({ parsedData, parsedSchema }) => {
        setSchema(parsedSchema);
        setFormData(parsedData);
        setIsLoading(false);
      });
    }
  }, [dbusFilterName]);

  return (
    <div style={{
      minWidth: '350px', width: '40vw', maxWidth: '500px', display: 'flex', flexDirection: 'column', alignItems: 'center',
    }}
    >
      {!isLoading && schema && slot && !error
        ? (
          <>
            <Title headingLevel="h2" size="lg" style={{ marginBottom: '1rem', padding: '0.5rem' }}>
              {removeOrg(slot) || 'Error - Unknown name'}
            </Title>
            <FormGenerator
              inputSchema={schema}
              key={dbusFilterName}
              onSubmit={(data: any) => {
                updateFormData(
                  dbusFilterName,
                  `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.${slot}`,
                  'Slots',
                  'Filter',
                  data.values.config,
                  setFormData,
                  addAlert,
                );
              }}
              values={formData}
            />
          </>
        ) : null }
      {(error && isLoading)
        ? error.map((err) => <Title headingLevel="h3" key={err} size="md">{err}</Title>)
        : isLoading && <Spinner /> }
    </div>
  );
}
