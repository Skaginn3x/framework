/* eslint-disable react/function-component-definition */
import React, { useState } from 'react';
import {
  Nav,
  NavList,
  NavItem,
  NavExpandable,
} from '@patternfly/react-core';
import './DynamicNavBar.css';
import { removeOrg } from '../Form/WidgetFunctions';

// eslint-disable-next-line @typescript-eslint/no-unused-vars
const createTree = (names: string[]) => {
  const tree: any = {};

  names.forEach((name: string) => {
    const parts = name.split('.');
    let currentLevel = tree;
    parts.slice(3, -2).forEach((part: string) => {
      if (!currentLevel[part]) {
        currentLevel[part] = {};
      }
      currentLevel = currentLevel[part];
    });
  });

  return tree;
};

// eslint-disable-next-line @typescript-eslint/no-unused-vars
const renderTree = (tree: any, parentFullName = '', parentRelevantName = '', activeItem:string | null = '') => {
  const rendered: any[] = [];

  // eslint-disable-next-line no-restricted-syntax
  for (const [key, value] of Object.entries(tree)) {
    const currentFullName = parentFullName ? `${parentFullName}.${key}` : key; // Full name for key
    const currentRelevantName = parentRelevantName ? `${parentRelevantName}.${key}` : key; // For display

    const hasValidKey = typeof value === 'object' && Object.keys(value as object).some((k) => k.trim() !== '');
    if (hasValidKey) {
      rendered.push(
        <NavExpandable
          title={currentRelevantName}
          key={currentFullName}
          groupId={currentFullName}
          className={currentFullName === activeItem ? 'Selected' : ''}
        >
          {renderTree(value, currentFullName, currentRelevantName, activeItem)}
        </NavExpandable>,
      );
    } else {
      rendered.push(
        <NavItem
          preventDefault
          id={currentFullName}
          key={currentFullName}
          groupId={currentFullName}
          className={currentFullName === activeItem ? 'Selected' : ''}
          // itemId={currentFullName}
        >
          {currentRelevantName}
        </NavItem>,
      );
    }
  }

  return rendered;
};

export const DynamicNavbar: React.FC<{
  names: string[];
  onItemSelect: (itemId:string) => void }> = ({
  names, onItemSelect,
}) => {
  const [activeItem, setActiveItem] = useState<string | null>(null);
  // const tree = createTree(names);
  console.log('names', names);
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
              {/* {renderTree(tree, '', '', activeItem)} */}
              {names.length > 0 && names.map((name:string) => {
                if (true) {
                  return (
                    <NavItem
                      preventDefault
                      id={name}
                      key={name}
                      groupId={name}
                      className={name === activeItem ? 'Selected' : ''}
                    >
                      {removeOrg(name)}
                    </NavItem>
                  );
                }
                return null;
              })}
              {/* <NavItem
                preventDefault
                id="test"
                key="test2"
                groupId="test3"
                className={activeItem === 'test3' ? 'Selected' : ''}
              >
                helo
              </NavItem> */}

            </NavList>
          </Nav>
        ) : null }
    </div>
  );
};

export default DynamicNavbar;
