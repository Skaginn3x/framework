/* eslint-disable react/function-component-definition */
import React, {
  useState,
} from 'react';
import { AlertVariant, Switch } from '@patternfly/react-core';
import { useAlertContext } from '../Alert/AlertContext';
import {DBusEndpoint} from "../../Types";

interface BoolTinkerIface {
  endpoint: DBusEndpoint;
  data: boolean;
}

const BoolTinker: React.FC<BoolTinkerIface> = ({ endpoint, data }) => {
  const { addAlert } = useAlertContext();

  const handleInputChange = async (newValue: boolean) => {
    endpoint.call('Tinker', newValue ? 1 : 0).then(() => {
      // addAlert(`Value of ${endpoint.interface} has been set ${newValue ? 'true' : 'false'}`, AlertVariant.success);
    }).catch((err: any) => {
      addAlert(`Error ${err} setting value of ${endpoint.interface}`, AlertVariant.danger);
    });
  };

  return (
    <Switch
      aria-label={`tinker-{${endpoint.interface}}-{${endpoint.service}}}`}
      onChange={(e, val) => handleInputChange(val)}
      key={`${endpoint.interface}-${endpoint.service}`}
      isChecked={data}
    />
  );
};

export default BoolTinker;
