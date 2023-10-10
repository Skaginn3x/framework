/* eslint-disable react/function-component-definition */
import React, { useState } from 'react';
import {
  Nav,
  NavList,
  NavItem,
} from '@patternfly/react-core';
import './DynamicNavBar.css';
import { removeOrg } from '../Form/WidgetFunctions';

export const DynamicNavbar: React.FC<{
  names: string[];
  onItemSelect: (itemId:string) => void }> = ({
  names, onItemSelect,
}) => {
  const [activeItem, setActiveItem] = useState<string | null>(null);
  return (
    <div style={{
      minWidth: '15rem', backgroundColor: '#212427', height: '-webkit-fill-available',
    }}
    >
      {names.length > 0
        ? (
          <Nav
            onSelect={(e, { groupId }) => {
              setActiveItem(groupId as string);
              onItemSelect(groupId as string);
            }}
            onToggle={(e, { groupId }) => {
              setActiveItem(groupId as string);
              onItemSelect(groupId as string);
            }}
          >
            <NavList>
              {names.length > 0 && names.map((name:string) => (
                <NavItem
                  preventDefault
                  id={name}
                  key={name}
                  groupId={name}
                  className={name === activeItem ? 'Selected' : ''}
                >
                  {removeOrg(name)}
                </NavItem>
              ))}

            </NavList>
          </Nav>
        ) : null }
    </div>
  );
};

export default DynamicNavbar;
