/* eslint-disable react/function-component-definition */
import React, { useEffect } from 'react';
import * as reactCore from '@patternfly/react-core';
import { Popper } from '@patternfly/react-core';

interface MultiSelectAttributeProps {
  items: string[];
  selectedItems: string[];
  setActiveItems: React.Dispatch<React.SetStateAction<string[]>>;
  attributeName: string;
  activeAttributeMenu: string;
  innerRef: React.RefObject<HTMLInputElement>;
}

const MultiSelectAttribute: React.FC<MultiSelectAttributeProps> = ({
  items,
  selectedItems,
  setActiveItems,
  attributeName,
  activeAttributeMenu,
  innerRef,
}) => { // NOSONAR
  const [isMenuOpen, setIsMenuOpen] = React.useState<boolean>(false);
  const toggleRef = React.useRef<HTMLButtonElement>(null);

  const containerRef = React.useRef<HTMLDivElement>(null);

  const handleMenuKeys = (event: KeyboardEvent) => {
    if (isMenuOpen && innerRef.current?.contains(event.target as Node)) {
      if (event.key === 'Escape' || event.key === 'Tab') {
        setIsMenuOpen(!isMenuOpen);
        toggleRef.current?.focus();
      }
    }
  };

  const handleClickOutside = (event: MouseEvent) => {
    if (isMenuOpen && !innerRef.current?.contains(event.target as Node)) {
      setIsMenuOpen(false);
    }
  };

  React.useEffect(() => {
    window.addEventListener('keydown', handleMenuKeys);
    window.addEventListener('click', handleClickOutside);
    return () => {
      window.removeEventListener('keydown', handleMenuKeys);
      window.removeEventListener('click', handleClickOutside);
    };
  }, [isMenuOpen, innerRef]);

  const onToggleClick = (ev: React.MouseEvent) => {
    ev.stopPropagation();
    setTimeout(() => {
      if (innerRef.current) {
        const firstElement = innerRef.current.querySelector('li > button:not(:disabled)');
        if (firstElement) {
          (firstElement as HTMLElement).focus();
        }
      }
    }, 0);
    // setIsMenuOpen(!isMenuOpen);
  };

  // on select, add or remove the item from the selectedItems
  const onSelect = (itemId: string | number | undefined) => {
    if (typeof itemId === 'undefined') {
      return;
    }

    const itemStr = itemId.toString();
    setActiveItems(
      selectedItems.includes(itemStr)
        ? selectedItems.filter((item) => item !== itemStr)
        : [itemStr, ...selectedItems],
    );
  };

  const firstItemRef = React.useRef<HTMLLIElement>(null);
  const toggle = (
    <reactCore.MenuToggle
      ref={innerRef}
      onFocus={() => {
        setIsMenuOpen(true);
      }}
      onClick={onToggleClick}
      isExpanded={isMenuOpen}
      badge={selectedItems.length > 0 ? <reactCore.Badge isRead>{selectedItems.length}</reactCore.Badge> : undefined}
      style={{ width: '250px' }}
    >
      {`Filter by ${attributeName}`}
    </reactCore.MenuToggle>
  );

  useEffect(() => { // On menu open, focus the first item
    if (isMenuOpen) {
      setTimeout(() => {
        firstItemRef.current?.focus();
      }, 50);
    }
  }, [isMenuOpen]);

  const menu = (
    <reactCore.Menu
      ref={innerRef}
      onSelect={(_, val) => onSelect(val)}
      selected={selectedItems}
    >
      <reactCore.MenuContent>
        <reactCore.MenuList>
          {items.map((item, index:number) => (
            <reactCore.MenuItem
              hasCheckbox
              key={item}
              ref={index === 0 ? firstItemRef : undefined}
              isSelected={selectedItems.includes(item)}
              itemId={item}
              onKeyDown={(event) => {
                // adds or removes the item from the selectedItems
                if (event.key === 'Enter') {
                  event.preventDefault();
                  onSelect(item);
                }
              }}
            >
              {item}
            </reactCore.MenuItem>
          ))}
        </reactCore.MenuList>
      </reactCore.MenuContent>
    </reactCore.Menu>
  );

  console.log('MULTI:', activeAttributeMenu, attributeName);

  return (
    activeAttributeMenu === attributeName ? (
      <div ref={containerRef}>
        <Popper
          trigger={toggle}
          popper={menu}
          appendTo={containerRef.current ?? undefined}
          isVisible={isMenuOpen}
        />
      </div>
    )
      : null
  );
};

export default MultiSelectAttribute;
