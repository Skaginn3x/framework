import React from 'react';
import FilterIcon from '@patternfly/react-icons/dist/esm/icons/filter-icon';

import * as reactCore from '@patternfly/react-core';

import { SignalType, SlotType } from '../../Types';

export default function ToolBar( // NOSONAR
  setSearchValue: React.Dispatch<React.SetStateAction<string>>,
  searchValue: string,
  setTypeSelection: React.Dispatch<React.SetStateAction<string>>,
  typeSelection: string,
  setProcessSelections: React.Dispatch<React.SetStateAction<string[]>>,
  processSelections: string[],
  setActiveAttributeMenu: React.Dispatch<React.SetStateAction<string>>,
  activeAttributeMenu: string,
  searchInputTextbox: JSX.Element,
  setPage: React.Dispatch<React.SetStateAction<number>>,
  page: number,
  setPerPage: React.Dispatch<React.SetStateAction<number>>,
  perPage: number,
  attributes: string[],
  items: SignalType[] | SlotType[],
  filteredItems: SignalType[] | SlotType[],
) {
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
  const typeContainerRef = React.useRef<HTMLDivElement>(null);

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

  const onTypeToggleClick = (ev: React.MouseEvent) => {
    ev.stopPropagation();
    setTimeout(() => {
      if (typeMenuRef.current) {
        const firstElement = typeMenuRef.current.querySelector('li > button:not(:disabled)');
        if (firstElement) {
          (firstElement as HTMLElement).focus();
        }
      }
    }, 0);
    setIsTypeMenuOpen(!isTypeMenuOpen);
  };

  function onTypeSelect(event: React.MouseEvent | undefined, itemId: string | number | undefined) {
    if (typeof itemId === 'undefined') {
      return;
    }

    setTypeSelection(itemId.toString());
    setIsTypeMenuOpen(!isTypeMenuOpen);
  }

  const typeToggle = (
    <reactCore.MenuToggle
      ref={typeToggleRef}
      onClick={onTypeToggleClick}
      isExpanded={isTypeMenuOpen}
      style={
        {
          width: '200px',
        } as React.CSSProperties
      }
    >
      Filter by Type
    </reactCore.MenuToggle>
  );

  const uniqueTypes: string[] = [];
  items.forEach((item) => {
    if (!uniqueTypes.includes(item.type)) {
      uniqueTypes.push(item.type);
    }
  });
  const typeMenu = (
    <reactCore.Menu ref={typeMenuRef} id="attribute-search-type-menu" onSelect={onTypeSelect} selected={typeSelection}>
      <reactCore.MenuContent>
        <reactCore.MenuList>
          {uniqueTypes.map((type) => (
            <reactCore.MenuItem itemId={type} key={type}>{type}</reactCore.MenuItem>
          ))}
        </reactCore.MenuList>
      </reactCore.MenuContent>
    </reactCore.Menu>
  );

  const typeSelect = (
    <div ref={typeContainerRef}>
      <reactCore.Popper
        trigger={typeToggle}
        popper={typeMenu}
        appendTo={typeContainerRef.current ?? undefined}
        isVisible={isTypeMenuOpen}
      />
    </div>
  );

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
        attributeToggleRef.current?.focus();
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
    ev.stopPropagation(); // Stop handleClickOutside from handling
    setTimeout(() => {
      if (attributeMenuRef.current) {
        const firstElement = attributeMenuRef.current.querySelector('li > button:not(:disabled)');
        if (firstElement) {
          (firstElement as HTMLElement).focus();
        }
      }
    }, 0);
    setIsAttributeMenuOpen(!isAttributeMenuOpen);
  };

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
    // eslint-disable-next-line no-console
    <reactCore.Menu
      ref={attributeMenuRef}
      onSelect={(_ev, itemId) => {
        setActiveAttributeMenu(itemId?.toString() as 'Name' | 'Type' | 'Process');
        setIsAttributeMenuOpen(!isAttributeMenuOpen);
      }}
    >
      <reactCore.MenuContent>
        <reactCore.MenuList>
          {attributes.map((attribute) => (
            <reactCore.MenuItem itemId={attribute} key={attribute}>{attribute}</reactCore.MenuItem>
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
  const processContainerRef = React.useRef<HTMLDivElement>(null);

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

  const onProcessMenuToggleClick = (ev: React.MouseEvent) => {
    ev.stopPropagation();
    setTimeout(() => {
      if (processMenuRef.current) {
        const firstElement = processMenuRef.current.querySelector('li > button:not(:disabled)');
        if (firstElement) {
          (firstElement as HTMLElement).focus();
        }
      }
    }, 0);
    setIsProcessMenuOpen(!isProcessMenuOpen);
  };

  function onProcessMenuSelect(event: React.MouseEvent | undefined, itemId: string | number | undefined) {
    if (typeof itemId === 'undefined') {
      return;
    }

    const itemStr = itemId.toString();

    setProcessSelections(
      processSelections.includes(itemStr)
        ? processSelections.filter((selection) => selection !== itemStr)
        : [itemStr, ...processSelections],
    );
  }

  const processToggle = (
    <reactCore.MenuToggle
      ref={processToggleRef}
      onClick={onProcessMenuToggleClick}
      isExpanded={isProcessMenuOpen}
      badge={processSelections.length > 0 ? <reactCore.Badge isRead>{processSelections.length}</reactCore.Badge> : undefined}
      style={
        {
          width: '200px',
        } as React.CSSProperties
      }
    >
      Filter by Process
    </reactCore.MenuToggle>
  );

  function getProcesses(signals: SignalType[] | SlotType[]) {
    const processes: string[] = [];
    signals.forEach((signal) => {
      const process = signal.name.split('.')[0];
      if (!processes.includes(process)) {
        processes.push(process);
      }
    });
    return processes;
  }

  const processMenu = (
    <reactCore.Menu
      ref={processMenuRef}
      id="attribute-search-process-menu"
      onSelect={onProcessMenuSelect}
      selected={processSelections}
    >
      <reactCore.MenuContent>
        <reactCore.MenuList>
          {getProcesses(items).map((name) => (
            <reactCore.MenuItem
              hasCheckbox
              key={name}
              isSelected={processSelections.includes(name)}
              itemId={name}
            >
              {name}
            </reactCore.MenuItem>
          ))}
        </reactCore.MenuList>
      </reactCore.MenuContent>
    </reactCore.Menu>
  );

  const processSelect = (
    <div ref={processContainerRef}>
      <reactCore.Popper
        trigger={processToggle}
        popper={processMenu}
        appendTo={processContainerRef.current ?? undefined}
        isVisible={isProcessMenuOpen}
      />
    </div>
  );
  return (
    <reactCore.Toolbar
      id="attribute-search-filter-toolbar"
      clearAllFilters={() => {
        setSearchValue('');
        setTypeSelection('');
        setProcessSelections([]);
      }}
    >
      <reactCore.ToolbarContent>
        <reactCore.ToolbarToggleGroup toggleIcon={<FilterIcon />} breakpoint="xl">
          <reactCore.ToolbarGroup variant="filter-group">
            <reactCore.ToolbarItem>{attributeDropdown}</reactCore.ToolbarItem>
            <reactCore.ToolbarFilter
              chips={searchValue !== '' ? [searchValue] : ([] as string[])}
              deleteChip={() => setSearchValue('')}
              deleteChipGroup={() => setSearchValue('')}
              categoryName="Name"
              showToolbarItem={activeAttributeMenu === 'Name'}
            >
              {searchInputTextbox}
            </reactCore.ToolbarFilter>
            <reactCore.ToolbarFilter
              chips={typeSelection !== '' ? [typeSelection] : ([] as string[])}
              deleteChip={() => setTypeSelection('')}
              deleteChipGroup={() => setTypeSelection('')}
              categoryName="Status"
              showToolbarItem={activeAttributeMenu === 'Type'}
            >
              {typeSelect}
            </reactCore.ToolbarFilter>
            <reactCore.ToolbarFilter
              chips={processSelections}
              deleteChip={(category, chip) => onProcessMenuSelect(undefined, chip as string)}
              deleteChipGroup={() => setProcessSelections([])}
              categoryName="Process"
              showToolbarItem={activeAttributeMenu === 'Process'}
            >
              {processSelect}
            </reactCore.ToolbarFilter>
          </reactCore.ToolbarGroup>
        </reactCore.ToolbarToggleGroup>
        <reactCore.ToolbarItem variant="pagination">{toolbarPagination}</reactCore.ToolbarItem>
      </reactCore.ToolbarContent>
    </reactCore.Toolbar>
  );
}
