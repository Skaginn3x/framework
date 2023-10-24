/* eslint-disable react/function-component-definition */
import React, { useEffect } from 'react';
import FilterIcon from '@patternfly/react-icons/dist/esm/icons/filter-icon';
import * as reactCore from '@patternfly/react-core';

import { SignalType, SlotType } from '../../Types';
import './Toolbar.css';

export type FilterConfig = {
  key: string;
  component: JSX.Element;
  chips: string[] | undefined;
  categoryName: string;
  setFiltered: React.Dispatch<React.SetStateAction<any[]>>;
};

type ToolBarProps = {
  setPage: React.Dispatch<React.SetStateAction<number>>;
  page: number;
  setPerPage: React.Dispatch<React.SetStateAction<number>>;
  perPage: number;
  filteredItems: SignalType[] | SlotType[];
  filterConfigs: FilterConfig[];
  activeAttributeMenu: string;
  setActiveAttributeMenu: React.Dispatch<React.SetStateAction<string>>;
  refs: Record<string, React.RefObject<HTMLInputElement>>;
};

const ToolBar: React.FC<ToolBarProps> = ({ // NOSONAR
  setPage,
  page,
  setPerPage,
  perPage,
  filteredItems,
  filterConfigs,
  activeAttributeMenu,
  setActiveAttributeMenu,
  refs,
}) => {
  const toolbarPagination = (
    <reactCore.Pagination
      perPageOptions={[
        { title: '10', value: 10 },
        { title: '20', value: 20 },
        { title: '50', value: 50 },
        { title: '100', value: 100 },
        { title: '250', value: 250 },
        { title: 'All', value: filteredItems.length },
      ]}
      itemCount={filteredItems.length}
      perPage={perPage}
      page={page}
      onSetPage={(_event, pageNumber) => setPage(pageNumber)}
      onPerPageSelect={(_event, perPageSelect) => {
        setPerPage(perPageSelect);
        setPage(1); // Reset to first page when changing items per page
      }}
      widgetId="attribute-search-mock-pagination"
      isCompact
    />
  );

  // Set up Type single select
  const [isTypeMenuOpen, setIsTypeMenuOpen] = React.useState<boolean>(false);
  const typeToggleRef = React.useRef<HTMLButtonElement>(null);
  const typeMenuRef = React.useRef<HTMLDivElement>(null);

  const handleTypeMenuKeys = (event: KeyboardEvent) => {
    if (isTypeMenuOpen && typeMenuRef.current?.contains(event.target as Node)) {
      if (event.key === 'Escape' || event.key === 'Tab') {
        setIsTypeMenuOpen(!isTypeMenuOpen);
        typeToggleRef.current?.focus();
      }
    }
  };

  const handleTypeClickOutside = (event: MouseEvent) => {
    if (isTypeMenuOpen && !typeMenuRef.current?.contains(event.target as Node)) {
      setIsTypeMenuOpen(false);
    }
  };

  React.useEffect(() => {
    window.addEventListener('keydown', handleTypeMenuKeys);
    window.addEventListener('click', handleTypeClickOutside);
    return () => {
      window.removeEventListener('keydown', handleTypeMenuKeys);
      window.removeEventListener('click', handleTypeClickOutside);
    };
  }, [isTypeMenuOpen, typeMenuRef]);

  // Set up attribute selector
  const [isAttributeMenuOpen, setIsAttributeMenuOpen] = React.useState(false);
  const attributeToggleRef = React.useRef<HTMLButtonElement>(null);
  const attributeMenuRef = React.useRef<HTMLDivElement>(null);
  const attributeContainerRef = React.useRef<HTMLDivElement>(null);

  const handleAttribueMenuKeys = (event: KeyboardEvent) => {
    if (!isAttributeMenuOpen) {
      return;
    }
    if (
      attributeMenuRef.current?.contains(event.target as Node)
      || attributeToggleRef.current?.contains(event.target as Node)
    ) {
      if (event.key === 'Escape' || event.key === 'Tab') {
        setIsAttributeMenuOpen(!isAttributeMenuOpen);
        attributeToggleRef.current?.blur();
      }
      if (event.key === 'Enter') {
        setIsAttributeMenuOpen(!isAttributeMenuOpen);
        refs[activeAttributeMenu].current?.focus();
      }
    }
  };

  const handleAttributeClickOutside = (event: MouseEvent) => {
    if (isAttributeMenuOpen && !attributeMenuRef.current?.contains(event.target as Node)) {
      setIsAttributeMenuOpen(false);
    }
  };

  React.useEffect(() => {
    window.addEventListener('keydown', handleAttribueMenuKeys);
    window.addEventListener('click', handleAttributeClickOutside);
    return () => {
      window.removeEventListener('keydown', handleAttribueMenuKeys);
      window.removeEventListener('click', handleAttributeClickOutside);
    };
  }, [isAttributeMenuOpen, attributeMenuRef]);

  const onAttributeToggleClick = (ev: React.MouseEvent) => {
    ev.preventDefault();
    ev.stopPropagation(); // Stop handleClickOutside from handling
    setTimeout(() => {
      if (attributeMenuRef.current) {
        const firstElement = attributeMenuRef.current.querySelector('li > button:not(:disabled)');
        if (firstElement) {
          (firstElement as HTMLElement).focus();
        }
      }
    }, 50);
    setIsAttributeMenuOpen(!isAttributeMenuOpen);
  };

  // Event handler for ctrl+f to open attribute menu
  useEffect(() => {
    const handleTableKeyDown = (event: any) => {
      if (event.ctrlKey && event.key === 'f') {
        event.preventDefault();
        onAttributeToggleClick(event);
      }
    };

    window.addEventListener('keydown', handleTableKeyDown);
    return () => {
      window.removeEventListener('keydown', handleTableKeyDown);
    };
  }, []);

  const attributeToggle = (
    <reactCore.MenuToggle
      ref={attributeToggleRef}
      onClick={onAttributeToggleClick}
      isExpanded={isAttributeMenuOpen}
      icon={<FilterIcon />}
    >
      {activeAttributeMenu}
    </reactCore.MenuToggle>
  );
  const attributeMenu = (
    <reactCore.Menu
      ref={attributeMenuRef}
      onSelect={(_ev, itemId) => {
        setActiveAttributeMenu(itemId as string ?? '');
        setIsAttributeMenuOpen(!isAttributeMenuOpen);
        setTimeout(() => {
          refs[itemId as string]?.current?.focus(); // Focus on the input
        }, 50);
      }}
    >
      <reactCore.MenuContent>
        <reactCore.MenuList>
          {filterConfigs.map((attribute) => (
            <reactCore.MenuItem itemId={attribute.key} key={attribute.key}>
              {attribute.categoryName}
            </reactCore.MenuItem>
          ))}
        </reactCore.MenuList>
      </reactCore.MenuContent>
    </reactCore.Menu>
  );

  const attributeDropdown = (
    <div ref={attributeContainerRef}>
      <reactCore.Popper
        trigger={attributeToggle}
        popper={attributeMenu}
        appendTo={attributeContainerRef.current ?? undefined}
        isVisible={isAttributeMenuOpen}
      />
    </div>
  );

  const [isProcessMenuOpen, setIsProcessMenuOpen] = React.useState<boolean>(false);
  const processToggleRef = React.useRef<HTMLButtonElement>(null);
  const processMenuRef = React.useRef<HTMLDivElement>(null);

  const handleProcessMenuKeys = (event: KeyboardEvent) => {
    if (isProcessMenuOpen && processMenuRef.current?.contains(event.target as Node)) {
      if (event.key === 'Escape' || event.key === 'Tab') {
        setIsProcessMenuOpen(!isProcessMenuOpen);
        processToggleRef.current?.focus();
      }
    }
  };

  const handleProcessClickOutside = (event: MouseEvent) => {
    if (isProcessMenuOpen && !processMenuRef.current?.contains(event.target as Node)) {
      setIsProcessMenuOpen(false);
    }
  };

  React.useEffect(() => {
    window.addEventListener('keydown', handleProcessMenuKeys);
    window.addEventListener('click', handleProcessClickOutside);
    return () => {
      window.removeEventListener('keydown', handleProcessMenuKeys);
      window.removeEventListener('click', handleProcessClickOutside);
    };
  }, [isProcessMenuOpen, processMenuRef]);

  return (
    <reactCore.Toolbar
      id="attribute-search-filter-toolbar"
      clearAllFilters={() => {
        console.log('Clearing all filters');
        filterConfigs.forEach((filter) => filter.setFiltered([]));
      }}
    >
      <reactCore.ToolbarContent>
        <reactCore.ToolbarToggleGroup toggleIcon={<FilterIcon />} breakpoint="xl">
          <reactCore.ToolbarGroup variant="filter-group">
            <reactCore.ToolbarItem>{attributeDropdown}</reactCore.ToolbarItem>
            {filterConfigs.map((filter) => {
              let chipsArray: string[] | undefined;
              if (Array.isArray(filter.chips) && filter.chips.length > 0) {
                chipsArray = filter.chips;
              }
              return (
                <reactCore.ToolbarFilter
                  key={`${filter.key}-Toolbar`}
                  chips={chipsArray}
                  deleteChip={(_, chip) => {
                    if (filter.chips?.filter((item: any) => item !== chip).length === 0) {
                      filter.setFiltered([]);
                      console.log('No chips left');
                      return;
                    }
                    if (filter.chips) {
                      filter.setFiltered(filter.chips.filter((item: any) => item !== chip));
                    }
                  }}
                  deleteChipGroup={() => filter.setFiltered([])}
                  categoryName={filter.categoryName}
                >
                  {filter.component}
                </reactCore.ToolbarFilter>
              );
            })}
          </reactCore.ToolbarGroup>
        </reactCore.ToolbarToggleGroup>
        <reactCore.ToolbarItem variant="pagination">{toolbarPagination}</reactCore.ToolbarItem>
      </reactCore.ToolbarContent>
    </reactCore.Toolbar>
  );
};

export default ToolBar;
