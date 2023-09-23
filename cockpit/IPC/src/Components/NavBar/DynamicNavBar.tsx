/* eslint-disable react/function-component-definition */
import React, { useState } from 'react';
import {
  Nav,
  NavList,
  NavItem,
  NavExpandable,
} from '@patternfly/react-core';
import './DynamicNavBar.css';

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

export const DynamicNavbar: React.FC<{ names: string[]; onItemSelect: (itemId:string) => void }> = ({ names, onItemSelect }) => {
  const [activeItem, setActiveItem] = useState<string | null>(null);
  const tree = createTree(names);

  return (
    <div style={{
      minWidth: '15rem', backgroundColor: '#212427', height: '100%',
    }}
    >
      <Nav
        onSelect={({ groupId }) => {
          setActiveItem(groupId as string);
          onItemSelect(groupId as string);
        }}
        onToggle={({ groupId }) => {
          setActiveItem(groupId as string);
          onItemSelect(groupId as string);
        }}
      >
        <NavList>
          {renderTree(tree, '', '', activeItem)}
        </NavList>
      </Nav>
    </div>
  );
};

export default DynamicNavbar;
