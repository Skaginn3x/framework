import React from 'react';
import FilterIcon from '@patternfly/react-icons/dist/esm/icons/filter-icon';


import {
   Toolbar,
   ToolbarContent,
   ToolbarItem,
   Menu,
   MenuContent,
   MenuList,
   MenuItem,
   MenuToggle,
   Popper,
   Pagination,
   Badge,
   ToolbarGroup,
   ToolbarFilter,
   ToolbarToggleGroup,
} from '@patternfly/react-core';

import { SignalType, SlotType } from '../../Types';



export function ToolBar(
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
   filteredItems: SignalType[] | SlotType[]) {


   const toolbarPagination = (
      <Pagination
         titles={{ paginationTitle: 'Attribute search pagination' }}
         perPageOptions={[
            { title: '10', value: 10 },
            { title: '20', value: 20 },
            { title: '50', value: 50 },
            { title: '100', value: 100 },
            { title: '250', value: 250 },
            { title: 'All', value: filteredItems.length }
         ]}
         itemCount={filteredItems.length}
         perPage={perPage}
         page={page}
         onSetPage={(_event, pageNumber) => setPage(pageNumber)}
         onPerPageSelect={(_event, perPage) => {
            setPerPage(perPage);
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
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, [isTypeMenuOpen, typeMenuRef]);

   const onTypeToggleClick = (ev: React.MouseEvent) => {
      ev.stopPropagation();
      setTimeout(() => {
         if (typeMenuRef.current) {
            const firstElement = typeMenuRef.current.querySelector('li > button:not(:disabled)');
            firstElement && (firstElement as HTMLElement).focus();
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
      <MenuToggle
         ref={typeToggleRef}
         onClick={onTypeToggleClick}
         isExpanded={isTypeMenuOpen}
         style={
            {
               width: '200px'
            } as React.CSSProperties
         }
      >
         Filter by Type
      </MenuToggle>
   );

   let uniqueTypes: string[] = []
   items.forEach((item) => {
      if (!uniqueTypes.includes(item.type)) {
         uniqueTypes.push(item.type);
      }
   });
   const typeMenu = (
      <Menu ref={typeMenuRef} id="attribute-search-type-menu" onSelect={onTypeSelect} selected={typeSelection}>
         <MenuContent>
            <MenuList>
               {uniqueTypes.map((type) => (
                  <MenuItem itemId={type}>{type}</MenuItem>
               ))}
            </MenuList>
         </MenuContent>
      </Menu>
   );

   const typeSelect = (
      <div ref={typeContainerRef}>
         <Popper
            trigger={typeToggle}
            popper={typeMenu}
            appendTo={typeContainerRef.current || undefined}
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
         attributeMenuRef.current?.contains(event.target as Node) ||
         attributeToggleRef.current?.contains(event.target as Node)
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
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, [isAttributeMenuOpen, attributeMenuRef]);

   const onAttributeToggleClick = (ev: React.MouseEvent) => {
      ev.stopPropagation(); // Stop handleClickOutside from handling
      setTimeout(() => {
         if (attributeMenuRef.current) {
            const firstElement = attributeMenuRef.current.querySelector('li > button:not(:disabled)');
            firstElement && (firstElement as HTMLElement).focus();
         }
      }, 0);
      setIsAttributeMenuOpen(!isAttributeMenuOpen);
   };

   const attributeToggle = (
      <MenuToggle
         ref={attributeToggleRef}
         onClick={onAttributeToggleClick}
         isExpanded={isAttributeMenuOpen}
         icon={<FilterIcon />}
      >
         {activeAttributeMenu}
      </MenuToggle>
   );
   const attributeMenu = (
      // eslint-disable-next-line no-console
      <Menu
         ref={attributeMenuRef}
         onSelect={(_ev, itemId) => {
            setActiveAttributeMenu(itemId?.toString() as 'Name' | 'Type' | 'Process');
            setIsAttributeMenuOpen(!isAttributeMenuOpen);
         }}
      >
         <MenuContent>
            <MenuList>
               {attributes.map((attribute) => (
                  <MenuItem itemId={attribute}>{attribute}</MenuItem>
               ))
               }
            </MenuList>
         </MenuContent>
      </Menu>
   );

   const attributeDropdown = (
      <div ref={attributeContainerRef}>
         <Popper
            trigger={attributeToggle}
            popper={attributeMenu}
            appendTo={attributeContainerRef.current || undefined}
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
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, [isProcessMenuOpen, processMenuRef]);

   const onProcessMenuToggleClick = (ev: React.MouseEvent) => {
      ev.stopPropagation();
      setTimeout(() => {
         if (processMenuRef.current) {
            const firstElement = processMenuRef.current.querySelector('li > button:not(:disabled)');
            firstElement && (firstElement as HTMLElement).focus();
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
            ? processSelections.filter(selection => selection !== itemStr)
            : [itemStr, ...processSelections]
      );
   }

   const processToggle = (
      <MenuToggle
         ref={processToggleRef}
         onClick={onProcessMenuToggleClick}
         isExpanded={isProcessMenuOpen}
         {...(processSelections.length > 0 && { badge: <Badge isRead>{processSelections.length}</Badge> })}
         style={
            {
               width: '200px'
            } as React.CSSProperties
         }
      >
         Filter by Process
      </MenuToggle>
   );

   function getProcesses(signal: SignalType[] | SlotType[]) {
      const processes: string[] = [];
      signal.forEach((signal) => {
         const process = signal.name.split(".")[0];
         if (!processes.includes(process)) {
            processes.push(process);
         }
      });
      return processes;
   }

   const processMenu = (
      <Menu
         ref={processMenuRef}
         id="attribute-search-process-menu"
         onSelect={onProcessMenuSelect}
         selected={processSelections}
      >
         <MenuContent>
            <MenuList>
               {getProcesses(items).map((name, index) => {
                  return <MenuItem hasCheck isSelected={processSelections.includes(name)} itemId={name}>
                     {name}
                  </MenuItem>
               })
               }
            </MenuList>
         </MenuContent>
      </Menu>
   );

   const processSelect = (
      <div ref={processContainerRef}>
         <Popper
            trigger={processToggle}
            popper={processMenu}
            appendTo={processContainerRef.current || undefined}
            isVisible={isProcessMenuOpen}
         />
      </div>
   );
   return <Toolbar
      id="attribute-search-filter-toolbar"
      clearAllFilters={() => {
         setSearchValue('');
         setTypeSelection('');
         setProcessSelections([]);
      }}
   >
      <ToolbarContent>
         <ToolbarToggleGroup toggleIcon={<FilterIcon />} breakpoint="xl">
            <ToolbarGroup variant="filter-group">
               <ToolbarItem>{attributeDropdown}</ToolbarItem>
               <ToolbarFilter
                  chips={searchValue !== '' ? [searchValue] : ([] as string[])}
                  deleteChip={() => setSearchValue('')}
                  deleteChipGroup={() => setSearchValue('')}
                  categoryName="Name"
                  showToolbarItem={activeAttributeMenu === 'Name'}
               >
                  {searchInputTextbox}
               </ToolbarFilter>
               <ToolbarFilter
                  chips={typeSelection !== '' ? [typeSelection] : ([] as string[])}
                  deleteChip={() => setTypeSelection('')}
                  deleteChipGroup={() => setTypeSelection('')}
                  categoryName="Status"
                  showToolbarItem={activeAttributeMenu === 'Type'}
               >
                  {typeSelect}
               </ToolbarFilter>
               <ToolbarFilter
                  chips={processSelections}
                  deleteChip={(category, chip) => onProcessMenuSelect(undefined, chip as string)}
                  deleteChipGroup={() => setProcessSelections([])}
                  categoryName="Process"
                  showToolbarItem={activeAttributeMenu === 'Process'}
               >
                  {processSelect}
               </ToolbarFilter>
            </ToolbarGroup>
         </ToolbarToggleGroup>
         <ToolbarItem variant="pagination">{toolbarPagination}</ToolbarItem>
      </ToolbarContent>
   </Toolbar>;
}
