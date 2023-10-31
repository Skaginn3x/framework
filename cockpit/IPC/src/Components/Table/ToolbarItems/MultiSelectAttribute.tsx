/* eslint-disable react/function-component-definition */
import React from 'react';
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

  const focusOnFirst = () => {
    if (innerRef.current) {
      const firstElement = innerRef.current.querySelector('li input[type="checkbox"]:not(:disabled)');
      if (firstElement) {
        (firstElement as HTMLElement).focus();
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

  const onToggleClick = () => {
    setIsMenuOpen(!isMenuOpen);
    setTimeout(() => {
      if (!isMenuOpen) {
        focusOnFirst();
      }
    }, 0);
  };

  const onFocus = () => {
    setIsMenuOpen(true);
    setTimeout(() => {
      focusOnFirst();
    }, 0);
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

  const toggle = (
    <reactCore.MenuToggle
      ref={innerRef}
      onFocus={(e: any) => {
        // if mouse key is down, don't focus
        // if sourceCapabilities fires touch events, don't focus
        if (e.nativeEvent.sourceCapabilities === null) {
          onFocus();
        }
      }}
      onClick={(ev) => {
        ev.stopPropagation();
        ev.preventDefault();
        onToggleClick();
      }}
      isExpanded={isMenuOpen}
      badge={selectedItems.length > 0 ? <reactCore.Badge isRead>{selectedItems.length}</reactCore.Badge> : undefined}
      style={{ width: '250px' }}
    >
      {`Filter by ${attributeName}`}
    </reactCore.MenuToggle>
  );

  const menu = (
    <reactCore.Menu
      ref={innerRef}
      onSelect={(_, val) => onSelect(val)}
      selected={selectedItems}
    >
      <reactCore.MenuContent>
        <reactCore.MenuList>
          {items.map((item) => (
            <reactCore.MenuItem
              hasCheckbox
              key={item}
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
