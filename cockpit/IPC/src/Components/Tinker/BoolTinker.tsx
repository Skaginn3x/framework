import React from 'react';
import { Switch } from '@patternfly/react-core';

interface BoolTinkerIface {
  data: any
}

// eslint-disable-next-line react/function-component-definition
const BoolTinker: React.FC<BoolTinkerIface> = ({ data }) => {
  const handleInputChange = (value: boolean) => {
    data.proxy.Tinker(value ? 1 : 0);
  };

  return (
    <Switch
      isChecked={data.proxy.data.Value}
      onChange={(e, val) => handleInputChange(val)}
      isDisabled={false}
    />
  );
};

export default BoolTinker;
