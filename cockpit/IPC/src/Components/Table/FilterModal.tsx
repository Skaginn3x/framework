import React, { useEffect } from 'react';
import { AlertVariant, Spinner, Title } from '@patternfly/react-core';
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
      let filteredName = allNames.filter(
        (name: string) => name.includes(`${slot}._filters_`),
      )[0];

      // ============================= REMOVE THIS AFTER TESTING ==============================
      // Replaces dashes with underscores, which is required as sdbus does not allow dashes.
      if (!filteredName || filteredName.length === 0) {
        // eslint-disable-next-line prefer-destructuring
        filteredName = allNames.filter(
          (name: string) => name.includes(`${slot.replaceAll('-', '_')}._filters_`),
        )[0];
      }
      // ===================================== END REMOVE =====================================

      if (!filteredName || filteredName.length === 0) { // If not found, notify user
        addAlert(`DBUS-Name-Error: No BUS interface with name "${slot}._filters_"`, AlertVariant.danger);
        setError(['DBUS-Name-Error:', 'No BUS interface with name ', `"${slot}._filters_"`]);
      }
      setDbusFilterName(filteredName);
    });
  }, [slot]);

  useEffect(() => {
    if (dbusFilterName) {
      fetchDataFromDBus(dbusFilterName).then(({ parsedData, parsedSchema }) => {
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
              onSubmit={(data: any) => updateFormData(dbusFilterName, data, setFormData, addAlert)}
              values={formData}
            />
          </>
        ) : null }
      {error && isLoading
        ? error.map((err) => <Title headingLevel="h3" key={err} size="md">{err}</Title>)
        : <Spinner /> }
    </div>
  );
}
