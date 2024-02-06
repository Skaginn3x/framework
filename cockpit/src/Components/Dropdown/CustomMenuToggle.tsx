import { MenuToggle, MenuToggleElement } from '@patternfly/react-core';
import { EllipsisVIcon } from '@patternfly/react-icons';
import React from 'react';

type CustomMenuToggleProps = {
  toggleRef: React.Ref<MenuToggleElement>;
  onClick: () => void;
  isExpanded: boolean;
};

// eslint-disable-next-line react/function-component-definition
const CustomMenuToggle: React.FC<CustomMenuToggleProps> = ({ toggleRef, onClick, isExpanded }) => (
  <MenuToggle
    ref={toggleRef}
    onClick={onClick}
    variant="plain"
    isExpanded={isExpanded}
  >
    <EllipsisVIcon />
  </MenuToggle>
);

export default CustomMenuToggle;
