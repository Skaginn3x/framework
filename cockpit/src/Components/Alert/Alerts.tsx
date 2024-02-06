import React from 'react';
import { Alert, AlertActionCloseButton, AlertGroup } from '@patternfly/react-core';
import { useAlertContext } from './AlertContext';
import './Alerts.css';

function Alerts() {
  const { alerts, removeAlert } = useAlertContext();

  return (
    <AlertGroup className="alert-box" isLiveRegion>
      {alerts.map(({ title, variant, key }) => (
        <Alert
          key={key}
          variant={variant}
          title={title}
          actionClose={<AlertActionCloseButton onClose={() => removeAlert(key)} />}
        />
      ))}
    </AlertGroup>
  );
}

export default Alerts;
