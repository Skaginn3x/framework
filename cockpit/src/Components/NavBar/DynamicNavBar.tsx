/* eslint-disable react/function-component-definition */
import React, { useState } from 'react';
import {
  Nav,
  NavList,
  NavItem,
} from '@patternfly/react-core';
import './DynamicNavBar.css';
import { TimesIcon } from '@patternfly/react-icons';
import { removeOrg } from '../Form/WidgetFunctions';

export const DynamicNavbar: React.FC<{
  names: string[];
  onClose: () => void;
  onItemSelect: (itemId:string) => void }> = ({
  names, onItemSelect, onClose,
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
            <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'flex-end' }}>
              <TimesIcon onClick={onClose} color="#EEE" style={{ padding: '0.3rem', width: '2rem', height: '2rem' }} />
            </div>
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
