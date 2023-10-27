/* eslint-disable react/function-component-definition */
import React from 'react';
import { AlertVariant, Switch } from '@patternfly/react-core';
import { useAlertContext } from '../Alert/AlertContext';

interface BoolTinkerIface {
  data: any
}

const BoolTinker: React.FC<BoolTinkerIface> = ({ data }) => {
  const { addAlert } = useAlertContext();
  const handleInputChange = (value: boolean) => {
    data.proxy.Tinker(value ? 1 : 0);
    addAlert(`Value of ${data.interfaceName} has been set ${value ? 'true' : 'false'}`, AlertVariant.success);
  };

  return (
    <Switch
      aria-label={`tinker-{${data.iface}}-{${data.process}}}`}
      isChecked={data.proxy.data.Value}
      onChange={(e, val) => handleInputChange(val)}
      isDisabled={false}
      key={`${data.iface}-${data.process}`}
    />
  );
};

export default BoolTinker;
