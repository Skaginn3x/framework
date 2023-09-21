/* eslint-disable react/function-component-definition */
import { MenuToggle, MenuToggleElement } from '@patternfly/react-core';
import { EllipsisVIcon } from '@patternfly/react-icons';
import React from 'react';

interface ToggleProps {
  toggleRef: React.Ref<MenuToggleElement>;
  isExpanded: boolean;
  onClick: () => void;
  index: number;
}

const DropdownToggle: React.FC<ToggleProps> = ({
  toggleRef, isExpanded, onClick, index,
}) => (
  <MenuToggle
    ref={toggleRef}
    isExpanded={isExpanded}
    onClick={onClick}
    variant="plain"
    aria-label={`Data list kebab toggle ${index}`}
  >
    <EllipsisVIcon aria-hidden="true" />
  </MenuToggle>
);

export default DropdownToggle;
