import React, { useEffect } from 'react';
import { Spinner, Title } from '@patternfly/react-core';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import FormGenerator from '../Form/Form';
import { useAlertContext } from '../Alert/AlertContext';
import {
  getData, handleNullValue, removeOrg, updateFormData,
} from '../Form/WidgetFunctions';

interface ModalType {
  slot: string;
}

export default function FilterModal({ slot }: ModalType) {
  const { addAlert } = useAlertContext();

  const [isLoading, setIsLoading] = React.useState(true); // Declare new state variable
  const [schemas, setSchemas] = React.useState<any>(undefined);
  const [formData, setFormData] = React.useState<any>(undefined);
  const [processName, setProcessName] = React.useState<string>('');
  const activeSlot = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/${slot.split('.').splice(2).join('/')}/Filter`;

  function loadData() {
    getData().then((allData) => {
      // eslint-disable-next-line @typescript-eslint/no-unused-vars
      allData.forEach(({ name, data }) => {
        setSchemas(() => {
          const filteredSchemas = Object.keys(data.schemas).reduce((acc: any, key: string) => {
            if (key.includes(activeSlot)) {
              // eslint-disable-next-line no-param-reassign
              acc = data.schemas[key];
            }
            return acc;
          }, {});
          return filteredSchemas;
        });

        setFormData(() => {
          const filteredData = Object.keys(data.data).reduce((acc: any, key: string) => {
            if (key.includes(activeSlot)) {
              // eslint-disable-next-line no-param-reassign
              acc = data.data[key];
            }
            return acc;
          }, {});
          return filteredData;
        });
        setProcessName(name);
        setIsLoading(false);
      });
    });
  }

  useEffect(() => {
    if (!slot) { return; }
    loadData();
  }, [slot]);

  return (
    <div style={{
      width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center', padding: '0.5rem 2rem',
    }}
    >
      {!isLoading && schemas && slot
        ? (
          <>
            <Title headingLevel="h2" size="lg" style={{ marginBottom: '1rem', padding: '0.5rem' }}>
              {removeOrg(slot) || 'Error - Unknown name'}
            </Title>
            <FormGenerator
              inputSchema={schemas}
              key={slot}
              intKey={slot}
              onSubmit={(data: any) => {
                const newdata = handleNullValue(data.values.config);
                updateFormData(
                  processName, // Process name
                  `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.Config`, // Interface name
                  activeSlot, // path
                  'Value', // property
                  newdata, // Data
                  setFormData,
                  addAlert,
                );
              }}
              values={formData}
            />
          </>
        ) : null }
      {isLoading && <Spinner /> }
    </div>
  );
}
