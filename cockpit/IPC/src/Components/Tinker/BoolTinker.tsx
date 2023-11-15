/* eslint-disable react/function-component-definition */
import React from 'react';
import { AlertVariant, Switch } from '@patternfly/react-core';
import { useAlertContext } from '../Alert/AlertContext';

interface BoolTinkerIface {
  data: any;
  isChecked: boolean;
}

const BoolTinker: React.FC<BoolTinkerIface> = ({ data, isChecked }) => {
  const { addAlert } = useAlertContext();
  const [value, setValue] = React.useState(data.proxy.data.Value);

  const handleInputChange = (newValue: boolean) => {
    data.proxy.Tinker(newValue ? 1 : 0).then(() => {
      addAlert(`Value of ${data.interfaceName} has been set ${newValue ? 'true' : 'false'}`, AlertVariant.success);
      setValue(newValue);
    }).catch((e: any) => {
      addAlert(`Error setting value of ${data.interfaceName}`, AlertVariant.danger);
      console.log(e);
    });
  };

  return (
    <Switch
      aria-label={`tinker-{${data.iface}}-{${data.process}}}`}
      isChecked={value}
      onChange={(e, val) => handleInputChange(val)}
      isDisabled={!isChecked}
      key={`${data.iface}-${data.process}`}
    />
  );
};

export default BoolTinker;
