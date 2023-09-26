import React, { useEffect, useRef, useState } from 'react';
import {
  Thead, Tr, Th, Tbody, Td, Table,
} from '@patternfly/react-table';
import {
  SearchInput,
  Tooltip,
} from '@patternfly/react-core';
import { SignalType, SlotType } from '../../Types';
import ToolBar from './Toolbar';

interface ModalType {
  slots: SlotType[];
  selectedSlots: Set<SlotType>;
  setSelectedSlots: React.Dispatch<React.SetStateAction<Set<SlotType>>>;
  addButtonRef: React.RefObject<HTMLButtonElement>;
  signal: SignalType | undefined;
  isDark: boolean
}

function formatDate(dateStr: string) {
  const date = new Date(dateStr);

  const formattedDate = date.toLocaleDateString('de-DE', { day: '2-digit', month: '2-digit', year: 'numeric' });
  const formattedTime = date.toLocaleTimeString('de-DE', { hour: '2-digit', minute: '2-digit' });

  return `${formattedTime} ${formattedDate}`;
}

export default function SlotModal({
  slots, selectedSlots, setSelectedSlots, addButtonRef, signal, isDark,
}: ModalType) {
  const [focusedIndex, setFocusedIndex] = useState<number>(0);
  const [searchValue, setSearchValue] = React.useState('');
  const [processSelections, setProcessSelections] = React.useState<string[]>([]);
  const [page, setPage] = React.useState(1);
  const [perPage, setPerPage] = React.useState(10);
  const searchBoxRef = useRef<HTMLInputElement | null>(null);
  const [activeAttributeMenu, setActiveAttributeMenu] = React.useState<string>('Name');
  const [typeSelection, setTypeSelection] = React.useState<string>('');

  const onSearchChange = (value: string) => {
    setSearchValue(value);
  };

  const onFilter = (slot: SlotType) => {
    let searchValueInput: RegExp;
    try {
      searchValueInput = new RegExp(searchValue, 'i');
    } catch (err) {
      searchValueInput = new RegExp(searchValue.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), 'i');
    }
    const matchesSearchValue = (slot.name + slot.created_by).search(searchValueInput) >= 0;

    return (
      matchesSearchValue && (processSelections.length === 0 || processSelections.includes(slot.name.split('.')[0]))
    );
  };
  // remove types that dont match. Also remove the slot that is already connected to the signal
  const filteredSlots = slots.filter((slot: any) => slot.type === signal?.type && slot.connected_to !== signal?.name).filter(onFilter);
  // Array to hold refs for each row
  const rowRefs = filteredSlots.map(() => React.createRef<HTMLTableRowElement>());

  useEffect(() => {
    setPage(1);
  }, [searchValue, processSelections]);

  // Set up name search input
  const searchInput = (
    <SearchInput
      placeholder="Filter by name"
      value={searchValue}
      ref={searchBoxRef}
      onKeyDown={(e) => {
        if (e.key === 'Enter' && filteredSlots.length > 0) {
          e.preventDefault();
          e.stopPropagation();
          // set focus on first row
          rowRefs[0]?.current?.focus();
        }
        if (e.key === 'Escape') {
          e.preventDefault();
          e.stopPropagation();
          onSearchChange('');
          searchBoxRef.current?.blur();
          setTimeout(() => {
            rowRefs[focusedIndex]?.current?.focus();
          }, 0);
        }
      }}
      onChange={(_event, value) => onSearchChange(value)}
      onClear={() => onSearchChange('')}
    />
  );

  React.useEffect(() => {
    if (window.document.activeElement === searchBoxRef.current) {
      return;
    }
    if (focusedIndex >= 0 && focusedIndex < filteredSlots.length) {
      rowRefs[focusedIndex].current?.focus();
    }
  }, [focusedIndex]);

  const handleSelect = (slot: SlotType, isSelecting: boolean) => {
    const newSelectedSlots = new Set(selectedSlots);
    if (isSelecting) {
      newSelectedSlots.add(slot);
    } else {
      newSelectedSlots.delete(slot);
    }
    setSelectedSlots(newSelectedSlots);
  };

  const isSlotSelected = (slot: SlotType) => selectedSlots.has(slot);

  useEffect(() => {
    const handleKeyDown = (event: any) => {
      if (event.ctrlKey && event.key === 'f') {
        event.preventDefault();
        // Ensure that searchBoxRef.current is available
        setActiveAttributeMenu('Name');
        setTimeout(() => {
          searchBoxRef.current?.focus();
        }, 0);
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
    };
  }, []);

  const startIndex = (page - 1) * perPage;
  const endIndex = page * perPage;
  const displayedSlots = filteredSlots.slice(startIndex, endIndex);

  const focusOnRow = (index: number) => {
    setFocusedIndex(index);
    rowRefs[index].current?.focus();
  };

  function handleArrowDown(rowIndex: number) {
    const nextRowIndex = rowIndex + 1;
    if (nextRowIndex < filteredSlots.length) {
      if (nextRowIndex < perPage) {
        focusOnRow(nextRowIndex);
      } else {
        setPage(page + 1);
        setFocusedIndex(0);
      }
    }
  }

  function handleArrowUp(rowIndex: number) {
    const prevRowIndex = rowIndex - 1;
    if (prevRowIndex >= -1) {
      if (prevRowIndex >= 0) {
        focusOnRow(prevRowIndex);
      } else {
        if (page === 1) { return; }
        setPage(page - 1);
        setFocusedIndex(perPage - 1);
      }
    }
  }

  const handleKeyDown = (event: React.KeyboardEvent<HTMLTableRowElement>, rowIndex: number) => {
    if (event.key === 'ArrowDown') {
      event.preventDefault();
      handleArrowDown(rowIndex);
    } else if (event.key === 'ArrowUp') {
      event.preventDefault();
      handleArrowUp(rowIndex);
    } else if (event.key === 'Enter') {
      const slot = displayedSlots[rowIndex];
      const isSelecting = !selectedSlots.has(slot);
      handleSelect(slot, isSelecting);
      // if focus is on search, move focus to first row, if focus is on a row, move focus to add
    } else if (event.key === 'Tab' && window.document.activeElement === searchBoxRef.current) {
      event.preventDefault();
      rowRefs[focusedIndex].current?.focus();
    } else if (event.key === 'Tab' && window.document.activeElement === rowRefs[focusedIndex].current) {
      event.preventDefault();
      addButtonRef.current?.focus();
    }
  };

  const [oldPage, setOldPage] = useState(-1);

  useEffect(() => {
    setOldPage(page);
  }, [page]);

  useEffect(() => {
    if (window.document.activeElement === searchBoxRef.current) {
      return;
    }
    // When the page changes, focus the first row (or the last row when going back a page)
    const newFocusIndex = page > oldPage ? 0 : perPage - 1;
    setFocusedIndex(newFocusIndex);
    rowRefs[newFocusIndex]?.current?.focus();
  }, [page]);

  const attributes = ['Name', 'Process'];
  const toolbar = ToolBar(
    setSearchValue,
    searchValue,
    setTypeSelection,
    typeSelection,
    setProcessSelections,
    processSelections,
    setActiveAttributeMenu,
    activeAttributeMenu,
    searchInput,
    setPage,
    page,
    setPerPage,
    perPage,
    attributes,
    slots,
    filteredSlots,
  );

  return (
    <>
      <div style={{
        display: 'flex',
        flexDirection: 'row',
        justifyContent: 'center',
        backgroundColor: isDark ? '#1b1d21' : '#FFF',
      }}
      >
        {toolbar}
      </div>
      {filteredSlots.length > 0
        ? (
          <Table variant="compact">
            <Thead>
              <Tr>
                <Th />
                <Th>Name</Th>
                <Th>Type</Th>
                <Th>Connected To</Th>
                <Th>Created By</Th>
                <Th>Created At</Th>
                <Th>Last Registered</Th>
                <Th>Description</Th>
              </Tr>
            </Thead>
            <Tbody>
              {displayedSlots.map((slot: SlotType, rowIndex: number) => (
                <Tr
                  key={slot.name}
                  tabIndex={rowIndex === focusedIndex ? 0 : -1}
                  ref={rowRefs[rowIndex]}
                  onKeyDown={(event) => handleKeyDown(event, rowIndex)}
                >
                  <Td
                    tabIndex={-1}
                    select={{
                      rowIndex,
                      onSelect: (_event, isSelecting) => handleSelect(slot, isSelecting),
                      isSelected: isSlotSelected(slot),
                    }}
                    style={{ paddingLeft: '1rem !important' }}
                  />
                  <Td>
                    <Tooltip
                      content={slot.name}
                    >
                      <div>{slot.name.split('.').slice(3).join('.') || slot.name}</div>
                    </Tooltip>
                  </Td>
                  <Td>{slot.type}</Td>
                  <Td>
                    <Tooltip
                      content={slot.connected_to}
                    >
                      <div>{slot.connected_to.split('.').slice(3).join('.') || slot.connected_to}</div>
                    </Tooltip>
                  </Td>
                  <Td>{slot.created_by}</Td>
                  <Td>{formatDate(slot.created_at)}</Td>
                  <Td>{formatDate(slot.last_registered)}</Td>
                  <Td>{slot.description}</Td>
                </Tr>
              ))}
            </Tbody>
          </Table>
        )
        : <div>No slots available</div>}
    </>
  );
}
