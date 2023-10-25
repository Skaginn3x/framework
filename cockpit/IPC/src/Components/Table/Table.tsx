/* eslint-disable react/no-array-index-key */
import React, { useEffect, useRef, useState } from 'react';
import {
  Title,
  Button,
  Bullseye,
  Modal,
  ModalVariant,
  AlertVariant,
  Tooltip,
} from '@patternfly/react-core';
import {
  Thead, Tr, Th, Tbody, Td, Table,
} from '@patternfly/react-table';
import PlusIcon from '@patternfly/react-icons/dist/esm/icons/plus-icon';
import MinusIcon from '@patternfly/react-icons/dist/esm/icons/minus-icon';
import { PencilAltIcon } from '@patternfly/react-icons';
import { SignalType, SlotType, ConnectionType } from '../../Types';
import { useAlertContext } from '../Alert/AlertContext';
import './Table.css';
import SlotModal from './SlotModal';
import ToolBar, { FilterConfig } from './Toolbar';
import FilterModal from './FilterModal';
import { removeOrg } from '../Form/WidgetFunctions';
import MultiSelectAttribute from './ToolbarItems/MultiSelectAttribute';
import TextboxAttribute from './ToolbarItems/TextBoxAttribute';
import emptyStateComponent from './TableItems/EmptyState';

function formatDate(dateStr: string, withTime = false) {
  const date = new Date(dateStr);
  if (!withTime) { return date.toLocaleDateString('de-DE', { day: '2-digit', month: '2-digit', year: 'numeric' }); }

  const formattedDate = date.toLocaleDateString('de-DE', { day: '2-digit', month: 'short', year: 'numeric' });
  const formattedTime = date.toLocaleTimeString('de-DE', { hour: '2-digit', minute: '2-digit' });
  return `${formattedTime} ${formattedDate}`;
}

const columnNames = {
  name: 'Name',
  created_by: 'Created By',
  type: 'Type',
  created_at: 'Created At',
  last_registered: 'Last Registered',
  last_modified: 'Last Modified',
  connected_to: 'Connected To',
  modified_by: 'Modified By',
  description: 'Description',
};

/**
 * @param signals The signals to display in the table
 * @param slots The slots to display in the table
 * @returns
 */
export default function CustomTable({
  signals, slots, connections, DBUS, isDark,
}: { signals: SignalType[], slots: SlotType[], connections: ConnectionType, DBUS: any, isDark: boolean }) {
  const [nameSelection, setNameSelection] = React.useState<string[]>([]);
  const [processSelections, setProcessSelections] = React.useState<string[]>([]);
  const [typeSelection, setTypeSelection] = React.useState<string[]>([]);
  const [isModalOpen, setIsModalOpen] = React.useState(false);
  const [isRemoveModalOpen, setIsRemoveModalOpen] = React.useState(false);
  const [modalSelectedSlots, setModalSelectedSlots] = React.useState<Set<SlotType>>(new Set());
  const [filterModalOpen, setFilterModalOpen] = React.useState(false);
  const [selectedSignal, setSelectedSignal] = React.useState<SignalType | undefined>(undefined);
  const [selectedSlot, setSelectedSlot] = React.useState<string | undefined>(undefined);
  const [page, setPage] = useState(1);
  const [perPage, setPerPage] = useState(20);
  const attributeRefs: Record<string, React.RefObject<HTMLInputElement>> = {
    Name: useRef<HTMLInputElement | null>(null),
    Type: useRef<HTMLInputElement | null>(null),
    Process: useRef<HTMLInputElement | null>(null),
  };

  const [activeAttributeMenu, setActiveAttributeMenu] = React.useState<string>('Name');

  const { addAlert } = useAlertContext();

  /**
   * When the the filters are updated, this function is called to determine if a signal should be displayed
   * @param signal The signal that is being checked
  */
  const onFilter = (signal: SignalType) => {
    const createSafeRegex = (value: string) => {
      try {
        return new RegExp(value, 'i');
      } catch (err) {
        return new RegExp(value.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), 'i');
      }
    };
    let matchesSearchValue;
    if (nameSelection && nameSelection.length !== 0) {
      const searchRegexList = nameSelection.map((value) => createSafeRegex(value));
      matchesSearchValue = searchRegexList.some(
        (regex) => (signal.name + signal.created_by).search(regex) >= 0,
      );
    } else {
      matchesSearchValue = true;
    }
    const matchesTypeSelection = typeSelection.length === 0 || typeSelection.includes(signal.type);
    return (
      matchesSearchValue
      && matchesTypeSelection
      && (processSelections.length === 0 || processSelections.includes(signal.name.split('.').splice(0, 2).join('.')))
    );
  };

  const filteredSignals = (signals ?? []).filter(onFilter);
  const startIndex = (page - 1) * perPage;
  const endIndex = page * perPage;
  const displayedSignals = filteredSignals.slice(startIndex, endIndex);
  const [selectedIndexes, setSelectedIndexes] = useState<[number, number]>([0, -1]); // [signalIndex, slotIndex]
  const signalRefs = displayedSignals.map(() => React.createRef<HTMLTableRowElement>());
  const slotRefs = displayedSignals.reduce((acc, signal, index) => {
    const connectedSlots = connections[signal.name];
    if (connectedSlots) {
      acc[index] = connectedSlots.map(() => React.createRef<HTMLTableRowElement>());
    }
    return acc;
  }, {} as { [key: number]: Array<React.RefObject<HTMLTableRowElement>> });

  useEffect(() => {
    setPage(1);
  }, [nameSelection, typeSelection, processSelections]);

  /**
   * Clear all filters to get back to the original table
   */
  function clearAllFilters() {
    setNameSelection([]);
    setTypeSelection([]);
    setProcessSelections([]);
  }

  /**
   * When the user clicks the plus button to add slots to a signal
   * @param signal The signal to add slots to
   */
  const handlePlusClick = (signal: SignalType) => {
    setSelectedSignal(signal ?? undefined);
    setIsModalOpen(true);
  };

  /**
   * When the user clicks the pencil button to edit filters for a slot
   * @param slot The slot to edit filters for
   */
  const handlePencilClick = (slot: SlotType) => {
    setSelectedSlot(slot.name ?? undefined);
    setFilterModalOpen(true);
  }; // When user wants to edit filters for a slot

  const AddButtonRef = React.createRef<HTMLButtonElement>();
  const removeButtonRef = React.createRef<HTMLButtonElement>();
  const cancelButtonRef = React.createRef<HTMLButtonElement>();
  const handleMinusClick = (slotName: string) => {
    setSelectedSlot(slotName);
    setIsRemoveModalOpen(true);
  };

  /**
   * When the user clicks the remove button when removing slots from a signal
   */
  const handleRemoveSlots = () => {
    if (!selectedSlot) return;
    // look through connections to find the signal that has the slot
    const signal = Object.keys(connections).find((key) => connections[key].includes(selectedSlot));
    DBUS.disconnect(selectedSlot).then(() => {
      addAlert(`Slot ${selectedSlot} removed from ${signal} successfully`, AlertVariant.success);
    }).catch((e: Error) => {
      addAlert(e.message, AlertVariant.danger);
    });
    setIsRemoveModalOpen(false);
    setSelectedIndexes(
      [selectedIndexes[1] - 1 < -1 && selectedIndexes[0] > 0
        ? selectedIndexes[0] - 1 : selectedIndexes[0], selectedIndexes[1] - 1 >= -1 ? selectedIndexes[1] - 1 : -1],
    );
  };

  /**
   * When a user clicks the add button to add slots to a signal
   */
  const handleAddSlots = () => {
    if (!selectedSignal) return;
    modalSelectedSlots.forEach((slot) => {
      DBUS.connect(slot.name, selectedSignal.name).then(() => {
        addAlert(`Slot ${slot.name} added to ${selectedSignal.name} successfully`, AlertVariant.success);
      }).catch((e: Error) => {
        addAlert(e.message, AlertVariant.danger);
      });
    });
    setIsModalOpen(false);
    setModalSelectedSlots(new Set());
  };

  /**
   * When the user clicks the cancel button when removing slots from a signal
   */
  const handleCancelAddSlots = () => {
    setModalSelectedSlots(new Set());
    setIsModalOpen(false);
  };

  function focusOnSlot(globalIndex: number, slotIdx: number) {
    setSelectedIndexes([globalIndex, slotIdx]);
    slotRefs[globalIndex - startIndex][slotIdx].current?.focus();
  } //

  function focusOnSignal(globalIndex: number) {
    setSelectedIndexes([globalIndex, -1]);
    signalRefs[globalIndex - startIndex].current?.focus();
  }

  /**
   * When the user presses the arrow down key. This function will determine what to focus on next
   * @param slotIndex current slot index
   * @param globalSignalIndex current signal index
   */
  function handleArrowDown(slotIndex: number, globalSignalIndex: number) {
    const nextSlotIndex = slotIndex + 1;
    const signalSlots = connections[filteredSignals[globalSignalIndex].name];

    if (signalSlots && nextSlotIndex < signalSlots.length) {
      focusOnSlot(globalSignalIndex, nextSlotIndex);
    } else {
      const nextGlobalSignalIndex = globalSignalIndex + 1;

      if (nextGlobalSignalIndex < filteredSignals.length) {
        setSelectedIndexes([nextGlobalSignalIndex, -1]);

        if (nextGlobalSignalIndex < endIndex) {
          focusOnSignal(nextGlobalSignalIndex);
        } else {
          setPage(page + 1);
        }
      }
    }
  }
  const getPreviousSignalSlots = (globalIndex: number) => connections[filteredSignals[globalIndex].name];

  /**
   * When the user presses the arrow up key. This function will determine what to focus on next
   * @param globalSignalIndex current signal index
   * @param start_index the index of the first signal on the current page
   */
  function focusOnPreviousItem(globalSignalIndex: number, start_index: number) {
    const prevGlobalSignalIndex = globalSignalIndex - 1;

    if (prevGlobalSignalIndex >= 0) {
      const prevSignalSlots = getPreviousSignalSlots(prevGlobalSignalIndex);
      const prevSlotIndex = prevSignalSlots ? prevSignalSlots.length - 1 : -1;

      setSelectedIndexes([prevGlobalSignalIndex, prevSlotIndex]);

      if (prevSlotIndex === -1) {
        if (prevGlobalSignalIndex >= start_index) {
          focusOnSignal(prevGlobalSignalIndex);
        } else {
          setPage(page - 1);
        }
      } else if (prevGlobalSignalIndex >= start_index) {
        focusOnSlot(prevGlobalSignalIndex, prevSlotIndex);
      } else {
        setPage(page - 1);
      }
    }
  }

  /**
   * Handles the arrow up key when the user is focused on a slot
   * Used to determine what to focus on next.
   * @param slotIndex current slot index
   * @param globalSignalIndex current signal index
   */
  function handleArrowUp(slotIndex: number, globalSignalIndex: number) {
    if (slotIndex > 0) {
      focusOnSlot(globalSignalIndex, slotIndex - 1);
    } else if (slotIndex === 0) {
      focusOnSignal(globalSignalIndex);
    } else {
      focusOnPreviousItem(globalSignalIndex, startIndex);
    }
  }

  /**
   * Handles the arrow key pressed to select which handler to call
   * @param event Keyboard event
   * @param globalSignalIndex Current signal index
   * @param slotIndex Current slot index
   */
  const handleKeyDown = (event: React.KeyboardEvent<HTMLTableRowElement>, globalSignalIndex: number, slotIndex: number) => {
    if (event.key === 'ArrowDown') {
      event.preventDefault();
      handleArrowDown(slotIndex, globalSignalIndex);
    } else if (event.key === 'ArrowUp') {
      event.preventDefault();
      handleArrowUp(slotIndex, globalSignalIndex);
    }
  };

  React.useEffect(() => {
    if (isRemoveModalOpen) {
      setTimeout(() => {
        removeButtonRef.current?.focus();
      }, 0);
    }
    function handleRemoveModalKeyDown(event: { key: string; }) {
      if (!isRemoveModalOpen) return;

      if (event.key === 'ArrowRight' || event.key === 'ArrowLeft') {
        if (document.activeElement === removeButtonRef.current) {
          cancelButtonRef.current?.focus();
        } else if (document.activeElement === cancelButtonRef.current) {
          removeButtonRef.current?.focus();
        }
      }
    }

    window.addEventListener('keydown', handleRemoveModalKeyDown);
    return () => window.removeEventListener('keydown', handleRemoveModalKeyDown);
  }, [isRemoveModalOpen]);

  /**
   * When the selected Slot changes, this determines where to focus
   */
  useEffect(() => {
    // Use setTimeout to push the focus command to the end of the JavaScript event queue
    if (isModalOpen) return;
    const timer = setTimeout(() => {
      // if active element is not in the table, do nothing
      if (document.activeElement?.classList.length! > 0
        && (!document.activeElement?.classList.contains('normalRow')
          || !document.activeElement?.classList.contains('smallRow'))) {
        return;
      }
      if (slotRefs[selectedIndexes[0] - startIndex] && slotRefs[selectedIndexes[0] - startIndex][selectedIndexes[1]]) {
        slotRefs[selectedIndexes[0] - startIndex][selectedIndexes[1]].current?.focus();
      } else if (signalRefs[selectedIndexes[0] - startIndex]) {
        signalRefs[selectedIndexes[0] - startIndex].current?.focus();
      }
    }, 0);

    // eslint-disable-next-line consistent-return
    return () => clearTimeout(timer);
  }, [selectedIndexes, slotRefs, page]);

  // Get Unique Types and processes
  const uniqueTypes = [...new Set(signals.map((slot) => slot.type))];
  let AllProcesses = signals.map((signal) => signal.name.split('.').slice(0, 2).join('.'));
  AllProcesses = AllProcesses.concat(slots.map((slot) => slot.name.split('.').slice(0, 2).join('.')));
  const uniqueProcesses = [...new Set(AllProcesses)];

  /**
   * Configuration file for the filters
   * Uses the Toolbar component's FilterConfig type
   */
  const Configs = [
    {
      key: 'Name',
      chips: nameSelection,
      categoryName: 'Name',
      setFiltered: setNameSelection,
      component:
  <TextboxAttribute
    selectedItems={nameSelection}
    setActiveItems={setNameSelection}
    attributeName="Name"
    activeAttributeMenu={activeAttributeMenu}
    innerRef={attributeRefs.Name}
  />,
    },

    {
      key: 'Type',
      chips: typeSelection,
      categoryName: 'Type',
      setFiltered: setTypeSelection,
      component:
  <MultiSelectAttribute
    items={uniqueTypes}
    selectedItems={typeSelection}
    setActiveItems={setTypeSelection}
    attributeName="Type"
    activeAttributeMenu={activeAttributeMenu}
    innerRef={attributeRefs.Type}
  />,
    },

    {
      key: 'Process',
      chips: processSelections,
      categoryName: 'Process',
      setFiltered: setProcessSelections,
      component:
  <MultiSelectAttribute
    items={uniqueProcesses}
    selectedItems={processSelections}
    setActiveItems={setProcessSelections}
    attributeName="Process"
    activeAttributeMenu={activeAttributeMenu}
    innerRef={attributeRefs.Process}
  />,
    },
  ] as FilterConfig[];

  const selectionSignalColor = isDark ? '#555 ' : 'lightgrey';
  const selectionSlotColor = isDark ? '#555 ' : '#CCC';
  const nonSelectionSlotColor = isDark ? '#313131' : '#f6f6f6';
  const empty = emptyStateComponent(clearAllFilters);

  return (
    <>
      <ToolBar
        setPage={setPage}
        page={page}
        setPerPage={setPerPage}
        perPage={perPage}
        filteredItems={filteredSignals}
        filterConfigs={Configs}
        activeAttributeMenu={activeAttributeMenu}
        setActiveAttributeMenu={setActiveAttributeMenu}
        refs={attributeRefs}
      />
      <Table aria-label="Selectable table">
        <Thead>
          <Tr>
            <Th width={35}>{columnNames.name}</Th>
            <Th width={10}>{columnNames.type}</Th>
            <Th width={10}>{columnNames.created_by}</Th>
            <Th width={10}>{columnNames.created_at}</Th>
            <Th width={10}>{columnNames.last_registered}</Th>
            <Th width={20}>{columnNames.description}</Th>
            <Th className="selectionCell topLevelCell" />
          </Tr>
        </Thead>
        <Tbody>
          {displayedSignals.length > 0
            && displayedSignals.map((signal, index) => ( // NOSONAR
              <React.Fragment key={signal.name}>
                <Tr
                  key={signal.name}
                  className="normalRow"
                  ref={signalRefs[index]}
                  tabIndex={0}
                  onFocus={() => setSelectedIndexes([startIndex + index, -1])}
                  onKeyDown={(e) => {
                    if (!e.shiftKey && e.key === 'Enter') {
                      handlePlusClick(signal);
                    }
                    handleKeyDown(e, startIndex + index, -1);
                  }}
                  style={{
                    backgroundColor: selectedIndexes[0] === (startIndex + index) && selectedIndexes[1] === -1
                      ? selectionSignalColor : 'transparent',
                  }}
                >
                  <Td dataLabel={columnNames.name} modifier="truncate" style={{ verticalAlign: 'middle' }}>
                    <Tooltip
                      content={signal.name}
                      enableFlip
                      distance={5}
                      entryDelay={1000}
                    >
                      <div style={{ width: 'min-content' }}>
                        {removeOrg(signal.name) ?? signal.name}
                      </div>
                    </Tooltip>
                  </Td>
                  <Td dataLabel={columnNames.type} modifier="truncate" style={{ verticalAlign: 'middle', paddingLeft: '1rem !important' }}>
                    {signal.type}
                  </Td>
                  <Td dataLabel={columnNames.created_by} modifier="truncate" style={{ verticalAlign: 'middle' }}>
                    {signal.created_by}
                  </Td>
                  <Td dataLabel={columnNames.created_at} modifier="truncate" style={{ verticalAlign: 'middle', paddingLeft: '0.5rem' }}>
                    <Tooltip
                      content={formatDate(signal.created_at, true)}
                      enableFlip
                      distance={5}
                      entryDelay={1000}
                    >
                      <div style={{ width: 'min-content' }}>
                        {formatDate(signal.created_at)}
                      </div>
                    </Tooltip>
                  </Td>
                  {/* eslint-disable-next-line max-len */}
                  <Td dataLabel={columnNames.last_registered} modifier="truncate" style={{ verticalAlign: 'middle', paddingLeft: '0.7rem' }}>
                    <Tooltip
                      content={formatDate(signal.last_registered, true)}
                      enableFlip
                      distance={5}
                      entryDelay={1000}
                    >
                      <div style={{ width: 'min-content' }}>
                        {formatDate(signal.last_registered)}
                      </div>
                    </Tooltip>
                  </Td>
                  <Td dataLabel={columnNames.description} modifier="truncate" style={{ verticalAlign: 'middle' }}>
                    {signal.description}
                  </Td>
                  <Td
                    key={signal.name}
                    dataLabel="Add slots to signal"
                    className="selectionCell"
                    modifier="truncate"
                    style={{ padding: '0.5rem', verticalAlign: 'middle' }}
                  >
                    <div style={{
                      display: 'flex', flexDirection: 'row', alignContent: 'space-between', justifyContent: 'center',
                    }}
                    >
                      <PlusIcon
                        style={{
                          margin: '0', width: '32px', height: '32px', padding: '0.3rem', borderRadius: '0.2rem',
                        }}
                        onClick={() => handlePlusClick(signal)}
                        className={isDark ? 'darkSelectionHover' : 'selectionHover'}
                      />
                    </div>
                  </Td>
                </Tr>
                {connections[signal.name]?.map((slotName: string, slotIndex: number) => (
                  <Tr
                    key={`${signal.name}-${slotName}`}
                    className="smallRow"
                    ref={slotRefs[index][slotIndex]}
                    tabIndex={0}
                    onFocus={() => setSelectedIndexes([startIndex + index, slotIndex])}
                    onKeyDown={(e) => {
                      if (e.shiftKey && e.key === 'Enter') {
                        e.stopPropagation();
                        handlePencilClick(slots.filter((slot) => slot.name === slotName)[0]);
                      } else if (!e.shiftKey && e.key === 'Enter') {
                        e.stopPropagation();
                        handleMinusClick(slotName);
                      } else if (!e.shiftKey) {
                        handleKeyDown(e, startIndex + index, slotIndex);
                      }
                    }}
                    style={{
                      backgroundColor: selectedIndexes[0] === (startIndex + index) && selectedIndexes[1] === slotIndex
                        ? selectionSlotColor : nonSelectionSlotColor,
                    }}
                  >
                    <Td
                      key={`${signal.name}-${slotName}`}
                      dataLabel={columnNames.name}
                      modifier="truncate"
                      style={{ verticalAlign: 'middle' }}
                    >
                      <div style={{ display: 'flex', flexDirection: 'row', alignItems: 'center' }}>
                        <MinusIcon
                          style={{
                            margin: '0', width: '32px', height: '32px', padding: '0.3rem', borderRadius: '0.2rem',
                          }}
                          className={isDark ? 'darkSelectionHoverSignal' : 'selectionHoverSignal'}
                          onClick={() => handleMinusClick(slotName)}
                        />
                        <Tooltip
                          content={slotName}
                          enableFlip
                          distance={5}
                          entryDelay={1000}
                        >
                          <div style={{ width: 'min-content', marginLeft: '1rem' }}>
                            {removeOrg(slotName)}
                          </div>
                        </Tooltip>
                      </div>
                    </Td>

                    {slots.filter((slot) => slot.name === slotName).map((slot) => (
                      <>
                        <Td
                          key={`${signal.name}-${slotName}-type`}
                          dataLabel={columnNames.type}
                          modifier="truncate"
                          style={{ verticalAlign: 'middle', paddingLeft: '1rem !important' }}
                        >
                          {slot.type}
                        </Td>

                        <Td
                          key={`${signal.name}-${slotName}-created_by`}
                          dataLabel={columnNames.created_by}
                          modifier="truncate"
                          style={{ verticalAlign: 'middle', paddingLeft: '0.5rem !important' }}
                        >
                          {slot.created_by}
                        </Td>

                        <Td
                          key={`${signal.name}-${slotName}-created_at`}
                          dataLabel={columnNames.created_at}
                          modifier="truncate"
                          style={{ verticalAlign: 'middle', paddingLeft: '0.7rem !important' }}
                          tooltip={
                            <div style={{ width: 'min-content' }}>
                              {formatDate(slot.created_at, true)}
                            </div> as React.ReactNode
                          }
                        >
                          <Tooltip
                            content={formatDate(slot.created_at, true)}
                            enableFlip
                            distance={5}
                            entryDelay={1000}
                          >
                            <div style={{ width: 'min-content' }}>
                              {formatDate(slot.created_at)}
                            </div>
                          </Tooltip>
                        </Td>

                        <Td
                          key={`${signal.name}-${slotName}-last_registered`}
                          dataLabel={columnNames.last_registered}
                          modifier="truncate"
                          style={{ verticalAlign: 'middle' }}
                          tooltip={null}
                        >
                          <Tooltip
                            content={formatDate(slot.last_registered, true)}
                            enableFlip
                            distance={5}
                            entryDelay={1000}
                          >
                            <div style={{ width: 'min-content' }}>
                              {formatDate(slot.last_registered)}
                            </div>
                          </Tooltip>
                        </Td>

                        <Td
                          key={`${signal.name}-${slotName}--description`}
                          dataLabel={columnNames.description}
                          modifier="truncate"
                          style={{ verticalAlign: 'middle' }}
                        >
                          {slot.description}
                        </Td>
                        <Td
                          key={`${signal.name}-${slotName}--button`}
                          dataLabel="icons"
                          modifier="truncate"
                          style={{ padding: '0.5rem', verticalAlign: 'middle' }}
                        >
                          <div style={{
                            display: 'flex', flexDirection: 'row', alignContent: 'space-between', justifyContent: 'center',
                          }}
                          >
                            <PencilAltIcon
                              style={{
                                margin: '0', width: '32px', height: '32px', padding: '0.3rem', borderRadius: '0.2rem',
                              }}
                              onClick={() => handlePencilClick(slot)}
                              className={isDark ? 'darkSelectionHoverSignal' : 'selectionHoverSignal'}

                            />
                          </div>
                        </Td>
                      </>
                    ))}
                  </Tr>
                ))}
              </React.Fragment>
            ))}
          {filteredSignals.length === 0 && (
            <Tr>
              <Td colSpan={8}>
                <Bullseye>{empty}</Bullseye>
              </Td>
            </Tr>
          )}
        </Tbody>
      </Table>

      <Modal
        variant={ModalVariant.medium}
        className="wideModal"
        isOpen={isModalOpen}
        onEscapePress={() => handleCancelAddSlots()}
        onClose={() => handleCancelAddSlots()}
        aria-label="Add Slots Modal"
        width="90vw"
        header={(
          <Title headingLevel="h1" size="3xl" style={{ color: isDark ? '#EEE' : '#333' }}>
            Add slots to
            {' '}
            {selectedSignal && selectedSignal.name.split('.').slice(3).join('.') !== ''
              ? selectedSignal.name.split('.').slice(3).join('.') : 'signal'}
          </Title>
        )}
        style={{ backgroundColor: isDark ? '#26292d' : '#FFF' }}
        actions={[
          <Button ref={AddButtonRef} key="confirm" variant="primary" onClick={() => handleAddSlots()}>
            Add
          </Button>,
          <Button key="cancel" variant="link" onClick={() => handleCancelAddSlots()}>
            Cancel
          </Button>,
        ]}
      >
        <SlotModal
          slots={slots}
          selectedSlots={modalSelectedSlots}
          setSelectedSlots={setModalSelectedSlots}
          addButtonRef={AddButtonRef}
          signal={selectedSignal}
          isDark={isDark}
        />
      </Modal>

      <Modal
        variant={ModalVariant.small}
        header={(
          <Title headingLevel="h1" size="3xl" style={{ color: isDark ? '#EEE' : '#333' }}>
            Remove slot from signal?
          </Title>
        )}
        isOpen={isRemoveModalOpen}
        onClose={() => setIsRemoveModalOpen(false)}
        aria-label="Remove Slots Modal"
        style={{ backgroundColor: isDark ? '#26292d' : '#EEE', color: isDark ? '#EEE' : '#333' }}
        actions={[
          <Button ref={removeButtonRef} key="confirm" variant="primary" onClick={() => handleRemoveSlots()}>
            Remove
          </Button>,
          <Button ref={cancelButtonRef} key="cancel" variant="link" onClick={() => setIsRemoveModalOpen(false)}>
            Cancel
          </Button>,
        ]}
      >
        Are you sure you want to remove the selected slots from the signal?
      </Modal>

      <Modal
        variant={ModalVariant.small}
        header={(
          <Title headingLevel="h1" size="3xl" style={{ color: isDark ? '#EEE' : '#333' }}>
            Edit signal filters
          </Title>
        )}
        isOpen={filterModalOpen}
        onClose={() => setFilterModalOpen(false)}
        aria-label="Edit Signals Modal"
        style={{ backgroundColor: isDark ? '#26292d' : '#EEE', color: isDark ? '#EEE' : '#333' }}
      >
        <FilterModal slot={selectedSlot} />
      </Modal>

    </>
  );
}
