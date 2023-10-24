/* eslint-disable react/function-component-definition */
import React from 'react';
import * as reactCore from '@patternfly/react-core';

interface MultiSelectAttributeProps {
  items: string[];
  selectedItems: string[];
  setActiveItems: React.Dispatch<React.SetStateAction<string[]>>;
  attributeName: string;
  activeAttributeMenu: string;
}

const MultiSelectAttribute: React.FC<MultiSelectAttributeProps> = ({
  items,
  selectedItems,
  setActiveItems,
  attributeName,
  activeAttributeMenu,
}) => { // NOSONAR
  const [isMenuOpen, setIsMenuOpen] = React.useState<boolean>(false);
  const toggleRef = React.useRef<HTMLButtonElement>(null);
  const menuRef = React.useRef<HTMLDivElement>(null);
  const containerRef = React.useRef<HTMLDivElement>(null);
  console.log(activeAttributeMenu);

  const handleMenuKeys = (event: KeyboardEvent) => {
    if (isMenuOpen && menuRef.current?.contains(event.target as Node)) {
      if (event.key === 'Escape' || event.key === 'Tab') {
        setIsMenuOpen(!isMenuOpen);
        toggleRef.current?.focus();
      }
    }
  };

  const handleClickOutside = (event: MouseEvent) => {
    if (isMenuOpen && !menuRef.current?.contains(event.target as Node)) {
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
  }, [isMenuOpen, menuRef]);

  const onToggleClick = (ev: React.MouseEvent) => {
    ev.stopPropagation();
    setTimeout(() => {
      if (menuRef.current) {
        const firstElement = menuRef.current.querySelector('li > button:not(:disabled)');
        if (firstElement) {
          (firstElement as HTMLElement).focus();
        }
      }
    }, 0);
    setIsMenuOpen(!isMenuOpen);
  };

  const onSelect = (event: React.MouseEvent | undefined, itemId: string | number | undefined) => {
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
      ref={toggleRef}
      onClick={onToggleClick}
      isExpanded={isMenuOpen}
      badge={selectedItems.length > 0 ? <reactCore.Badge isRead>{selectedItems.length}</reactCore.Badge> : undefined}
      style={{ width: '250px' }}
    >
      {`Filter by ${attributeName}`}
    </reactCore.MenuToggle>
  );

  const menu = (
    <reactCore.Menu ref={menuRef} onSelect={onSelect} selected={selectedItems}>
      <reactCore.MenuContent>
        <reactCore.MenuList>
          {items.map((item) => (
            <reactCore.MenuItem
              hasCheckbox
              key={item}
              isSelected={selectedItems.includes(item)}
              itemId={item}
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
        <reactCore.Popper
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
