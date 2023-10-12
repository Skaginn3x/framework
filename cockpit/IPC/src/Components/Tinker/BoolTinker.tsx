/* eslint-disable react/function-component-definition */
import React from 'react';
import { Switch } from '@patternfly/react-core';

interface BoolTinkerIface {
  data: any
}

const BoolTinker: React.FC<BoolTinkerIface> = ({ data }) => {
  const handleInputChange = (value: boolean) => {
    data.proxy.Tinker(value ? 1 : 0);
  };

  return (
    <Switch
      isChecked={data.proxy.data.Value}
      onChange={(e, val) => handleInputChange(val)}
      isDisabled={false}
      key={`${data.iface}-${data.process}`}
    />
  );
};

export default BoolTinker;
