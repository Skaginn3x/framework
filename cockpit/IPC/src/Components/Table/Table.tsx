import React, { useEffect, useRef, useState } from 'react';
import {
   SearchInput,
   EmptyState,
   EmptyStateIcon,
   Title,
   EmptyStateBody,
   EmptyStatePrimary,
   Button,
   Bullseye,
   Modal,
   ModalVariant,
   AlertVariant
} from '@patternfly/react-core';
import { TableComposable, Thead, Tr, Th, Tbody, Td } from '@patternfly/react-table';
import SearchIcon from '@patternfly/react-icons/dist/esm/icons/search-icon';
import PlusIcon from '@patternfly/react-icons/dist/esm/icons/plus-icon';
import MinusIcon from '@patternfly/react-icons/dist/esm/icons/minus-icon';
import { SignalType, SlotType, ConnectionType } from 'src/Types';
import { useAlertContext } from '../Alert/AlertContext';
import "./Table.css"
import ModalContent from './ModalContent';
import { ToolBar } from './Toolbar';

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
export const Table: React.FC<{ signals: SignalType[], slots: SlotType[], connections: ConnectionType, DBUS: any }> = ({ signals, slots, connections, DBUS }) => {

   const [searchValue, setSearchValue] = React.useState('');
   const [processSelections, setProcessSelections] = React.useState<string[]>([]);
   const [typeSelection, setTypeSelection] = React.useState('');
   const [isModalOpen, setIsModalOpen] = React.useState(false);
   const [isRemoveModalOpen, setIsRemoveModalOpen] = React.useState(false);
   const [modalSelectedSlots, setModalSelectedSlots] = React.useState<Set<SlotType>>(new Set());
   const [selectedSignal, setSelectedSignal] = React.useState<SignalType | undefined>(undefined);
   const [selectedSlot, setSelectedSlot] = React.useState<string | undefined>(undefined);
   const [page, setPage] = useState(1);
   const [perPage, setPerPage] = useState(10);
   const searchBoxRef = useRef<HTMLInputElement | null>(null);


   const [activeAttributeMenu, setActiveAttributeMenu] = React.useState<string>('Name');

   const { addAlert } = useAlertContext();

   const onSearchChange = (value: string) => {
      setSearchValue(value);
   };

   const onFilter = (signal: SignalType) => {
      let searchValueInput: RegExp;
      try {
         searchValueInput = new RegExp(searchValue, 'i');
      } catch (err) {
         searchValueInput = new RegExp(searchValue.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), 'i');
      }
      const matchesSearchValue = (signal.name + signal.created_by).search(searchValueInput) >= 0;

      const matchesTypeSelection = typeSelection === '' || signal.type === typeSelection;

      return (
         matchesSearchValue && matchesTypeSelection && (processSelections.length === 0 || processSelections.includes(signal.name.split('.')[0]))
      );
   };
   const filteredSignals = (signals || []).filter(onFilter);

   useEffect(() => {
      setPage(1);
   }, [searchValue, typeSelection, processSelections]);

   // Set up name search input
   const searchInput = (
      <SearchInput
         placeholder="Filter by name"
         value={searchValue}
         ref={searchBoxRef}
         onKeyDown={(e) => {
            if (e.key === 'Enter' && filteredSignals.length > 0) {
               e.preventDefault();
               e.stopPropagation();
               // set focus on first row
               signalRefs[0]?.current?.focus();
            }
            if (e.key === 'Escape') {
               e.preventDefault();
               e.stopPropagation();
               onSearchChange('');
               searchBoxRef.current?.blur();
            }

         }}
         onChange={(_event, value) => onSearchChange(value)}
         onClear={() => onSearchChange('')}
      />
   );

   const emptyState = (
      <EmptyState>
         <EmptyStateIcon icon={SearchIcon} />
         <Title size="lg" headingLevel="h4">
            No results found
         </Title>
         <EmptyStateBody>No results match the filter criteria. Clear all filters and try again.</EmptyStateBody>
         <EmptyStatePrimary>
            <Button
               variant="link"
               onClick={() => {
                  setSearchValue('');
                  setTypeSelection('');
                  setProcessSelections([]);
               }}
            >
               Clear all filters
            </Button>
         </EmptyStatePrimary>
      </EmptyState>
   );

   const handlePlusClick = (signal: SignalType) => {
      setSelectedSignal(signal || undefined);
      setIsModalOpen(true);
   }

   const AddButtonRef = React.createRef<HTMLButtonElement>();
   const removeButtonRef = React.createRef<HTMLButtonElement>();
   const cancelButtonRef = React.createRef<HTMLButtonElement>();
   const handleMinusClick = (slotName: string) => {
      setSelectedSlot(slotName);
      setIsRemoveModalOpen(true);
   }

   const handleRemoveSlots = () => {
      if (!selectedSlot) return;
      // change focus to slot above or if none, the signal above
      let signal = selectedSignal?.name;
      if (!signal) {
         const slot = slots.filter(slot => slot.name === selectedSlot);
         if (slot.length === 1) {
            signal = slot[0].connected_to
         }
      }
      DBUS.disconnect(selectedSlot).then(() => {
         addAlert("Slot " + selectedSlot + " removed from " + signal + " successfully", AlertVariant.success);
      }).catch((e: Error) => {
         addAlert(e.message, AlertVariant.danger);
      });
      setIsRemoveModalOpen(false);
      setSelectedIndexes(
         [selectedIndexes[1] - 1 < -1 && selectedIndexes[0] > 0 ? selectedIndexes[0] - 1 : selectedIndexes[0],
         selectedIndexes[1] - 1 >= -1 ? selectedIndexes[1] - 1 : -1]
      );

   }
   const handleAddSlots = () => {
      if (!selectedSignal) return;
      modalSelectedSlots.forEach(slot => {
         DBUS.connect(slot.name, selectedSignal.name).then(() => {
            addAlert("Slot " + slot.name + " added to " + selectedSignal.name + " successfully", AlertVariant.success);
         }).catch((e: Error) => {
            addAlert(e.message, AlertVariant.danger)
         });
      });
      setIsModalOpen(false);
      setModalSelectedSlots(new Set());
   }

   const handleCancelAddSlots = () => {
      setModalSelectedSlots(new Set());
      setIsModalOpen(false);
   }

   useEffect(() => {
      const handleKeyDown = (event: any) => {
         if (event.ctrlKey && event.key === "f" && !isModalOpen) {
            event.preventDefault();
            // Ensure that searchBoxRef.current is available
            setActiveAttributeMenu('Name');
            setTimeout(() => {
               searchBoxRef.current?.focus();
            }, 0);
         }
      };

      window.addEventListener("keydown", handleKeyDown);
      return () => {
         window.removeEventListener("keydown", handleKeyDown);
      };
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, []);


   const startIndex = (page - 1) * perPage;
   const endIndex = page * perPage;
   const displayedSignals = filteredSignals.slice(startIndex, endIndex);


   const [selectedIndexes, setSelectedIndexes] = useState<[number, number]>([0, -1]); // [signalIndex, slotIndex]
   const signalRefs = displayedSignals.map(() => React.createRef<HTMLTableRowElement>());
   const slotRefs = displayedSignals.reduce((acc, signal, index) => {
      const slots = connections[signal.name];
      if (slots) {
         acc[index] = slots.map(() => React.createRef<HTMLTableRowElement>());
      }
      return acc;
   }, {} as { [key: number]: Array<React.RefObject<HTMLTableRowElement>> });

   const handleKeyDown = (event: React.KeyboardEvent<HTMLTableRowElement>, globalSignalIndex: number, slotIndex: number) => {

      const focusOnSlot = (globalIndex: number, slotIdx: number) => {
         setSelectedIndexes([globalIndex, slotIdx]);
         slotRefs[globalIndex - startIndex][slotIdx].current?.focus();
      };

      const focusOnSignal = (globalIndex: number) => {
         setSelectedIndexes([globalIndex, -1]);
         signalRefs[globalIndex - startIndex].current?.focus();
      };

      const getPreviousSignalSlots = (globalIndex: number) => connections[filteredSignals[globalIndex].name];

      const handleArrowDown = () => {
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
      };

      const handleArrowUp = () => {
         if (slotIndex > 0) {
            focusOnSlot(globalSignalIndex, slotIndex - 1);
         } else if (slotIndex === 0) {
            focusOnSignal(globalSignalIndex);
         } else {
            const prevGlobalSignalIndex = globalSignalIndex - 1;

            if (prevGlobalSignalIndex >= 0) {
               const prevSignalSlots = getPreviousSignalSlots(prevGlobalSignalIndex);
               const prevSlotIndex = prevSignalSlots ? prevSignalSlots.length - 1 : -1;

               setSelectedIndexes([prevGlobalSignalIndex, prevSlotIndex]);

               if (prevSlotIndex === -1) {
                  if (prevGlobalSignalIndex >= startIndex) {
                     focusOnSignal(prevGlobalSignalIndex);
                  } else {
                     setPage(page - 1);
                  }
               } else {
                  if (prevGlobalSignalIndex >= startIndex) {
                     focusOnSlot(prevGlobalSignalIndex, prevSlotIndex);
                  } else {
                     setPage(page - 1);
                  }
               }
            }
         }
      };

      if (event.key === 'ArrowDown') {
         event.preventDefault();
         handleArrowDown();
      } else if (event.key === 'ArrowUp') {
         event.preventDefault();
         handleArrowUp();
      }
   };

   React.useEffect(() => {
      if (isRemoveModalOpen) {
         setTimeout(() => {
            removeButtonRef.current?.focus();
         }, 0);
      }
      function handleKeyDown(event: { key: string; }) {
         if (!isRemoveModalOpen) return;

         if (event.key === "ArrowRight" || event.key === "ArrowLeft") {
            if (document.activeElement === removeButtonRef.current) {
               cancelButtonRef.current?.focus();
            } else if (document.activeElement === cancelButtonRef.current) {
               removeButtonRef.current?.focus();
            }
         }
      }

      window.addEventListener("keydown", handleKeyDown);
      return () => window.removeEventListener("keydown", handleKeyDown);
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, [isRemoveModalOpen]);


   useEffect(() => {
      // Use setTimeout to push the focus command to the end of the JavaScript event queue
      if (isModalOpen) return;
      const timer = setTimeout(() => {
         // if active element is not in the table, do nothing
         if (document.activeElement?.classList.length! > 0 && (!document.activeElement?.classList.contains('normalRow') || !document.activeElement?.classList.contains('smallRow'))) return;
         if (slotRefs[selectedIndexes[0] - startIndex] && slotRefs[selectedIndexes[0] - startIndex][selectedIndexes[1]])
            slotRefs[selectedIndexes[0] - startIndex][selectedIndexes[1]].current?.focus();
         else {
            if (signalRefs[selectedIndexes[0] - startIndex])
               signalRefs[selectedIndexes[0] - startIndex].current?.focus();
         }
      }, 0);

      // Clear the timer when the component unmounts or when selectedIndexes changes
      return () => clearTimeout(timer);
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, [selectedIndexes, slotRefs, page]);


   const attributes = ["Name", "Type", "Process"]
   const toolbar = ToolBar(setSearchValue, searchValue, setTypeSelection, typeSelection, setProcessSelections, processSelections, setActiveAttributeMenu, activeAttributeMenu, searchInput, setPage, page, setPerPage, perPage, attributes, signals, filteredSignals);


   return (
      <React.Fragment>
         {toolbar}
         <TableComposable aria-label="Selectable table">
            <Thead>
               <Tr>
                  <Th className='selectionCell' />
                  <Th width={30} colSpan={1}>{columnNames.name}</Th>
                  <Th width={10}>{columnNames.type}</Th>
                  <Th width={10}>{columnNames.created_by}</Th>
                  <Th width={10}>{columnNames.created_at}</Th>
                  <Th width={10}>{columnNames.last_registered}</Th>
                  <Th width={25}>{columnNames.description}</Th>
                  <Th className='selectionCell' />
               </Tr>
            </Thead>
            <Tbody>
               {displayedSignals.length > 0 &&
                  displayedSignals.map((signal, index) => (
                     <React.Fragment key={signal.name}>
                        <Tr key={signal.name}
                           className="normalRow"
                           ref={signalRefs[index]}
                           tabIndex={0}
                           onFocus={() => setSelectedIndexes([startIndex + index, -1])}
                           onKeyDown={(e) => {
                              if (e.key === 'Enter') {
                                 handlePlusClick(signal);
                              }
                              handleKeyDown(e, startIndex + index, -1);
                           }}
                           style={{ backgroundColor: selectedIndexes[0] === (startIndex + index) && selectedIndexes[1] === -1 ? 'lightgray' : 'white' }}
                        >
                           <Td dataLabel={columnNames.name} modifier="truncate" style={{ verticalAlign: "middle" }} colSpan={2}>
                              {signal.name.split('.').slice(3).join('.') || signal.name}
                           </Td>
                           <Td dataLabel={columnNames.type} modifier="truncate" style={{ verticalAlign: "middle" }}>
                              {signal.type}
                           </Td>
                           <Td dataLabel={columnNames.created_by} modifier="truncate" style={{ verticalAlign: "middle" }}>
                              {signal.created_by}
                           </Td>
                           <Td dataLabel={columnNames.created_at} modifier="truncate" style={{ verticalAlign: "middle" }}>
                              {formatDate(signal.created_at)}
                           </Td>
                           <Td dataLabel={columnNames.last_registered} modifier="truncate" style={{ verticalAlign: "middle" }}>
                              {formatDate(signal.last_registered)}
                           </Td>
                           <Td dataLabel={columnNames.description} modifier="truncate" style={{ verticalAlign: "middle" }}>
                              {signal.description}
                           </Td>
                           <Td key={signal.name} onClick={() => handlePlusClick(signal)} dataLabel={"Add slots to signal"} className="selectionCell selectionHover" modifier="truncate" style={{ padding: "0.5rem", verticalAlign: "middle" }}>
                              <PlusIcon style={{ margin: "0" }} />
                           </Td>
                        </Tr>
                        {connections[signal.name] && connections[signal.name].map((slotName: string, slotIndex: number) => (
                           <Tr key={`${signal.name}-${slotName}`}
                              className="smallRow"
                              ref={slotRefs[index][slotIndex]}
                              tabIndex={0}
                              onFocus={() => setSelectedIndexes([startIndex + index, slotIndex])}
                              onKeyDown={(e) => {
                                 if (e.key === 'Enter') {
                                    e.stopPropagation();
                                    handleMinusClick(slotName);
                                 } else {
                                    handleKeyDown(e, startIndex + index, slotIndex);
                                 }
                              }}
                              style={{ backgroundColor: selectedIndexes[0] === (startIndex + index) && selectedIndexes[1] === slotIndex ? 'lightgray' : '#f6f6f6' }}
                           >
                              <Td key={"Selection" + slotIndex} onClick={() => handleMinusClick(slotName)} dataLabel={"Remove slot from signal"} className="smallSelectionCell" style={{ padding: "0.5rem" }}>
                                 <MinusIcon style={{ margin: "0" }} />
                              </Td>
                              <Td key={`${signal.name}-${slotName}-${slotIndex}`} dataLabel={columnNames.name} modifier="truncate" style={{ paddingLeft: "2rem", verticalAlign: "middle" }}>
                                 {slotName}
                              </Td>
                              {slots.filter(slot => slot.name === slotName).map(slot => (
                                 <>
                                    <Td key={`${signal.name}-${slotName}-${slotIndex}-type`} dataLabel={columnNames.type} modifier="truncate" style={{ verticalAlign: "middle" }}>
                                       {slot.type}
                                    </Td>
                                    <Td key={`${signal.name}-${slotName}-${slotIndex}-created_by`} dataLabel={columnNames.created_by} modifier="truncate" style={{ verticalAlign: "middle" }}>
                                       {slot.created_by}
                                    </Td>
                                    <Td key={`${signal.name}-${slotName}-${slotIndex}-created_at`} dataLabel={columnNames.created_at} modifier="truncate" style={{ verticalAlign: "middle" }}>
                                       {formatDate(slot.created_at)}
                                    </Td>
                                    <Td key={`${signal.name}-${slotName}-${slotIndex}-last_registered`} dataLabel={columnNames.last_registered} modifier="truncate" style={{ verticalAlign: "middle" }}>
                                       {formatDate(slot.last_registered)}
                                    </Td>
                                    <Td key={`${signal.name}-${slotName}-${slotIndex}-description`} dataLabel={columnNames.description} modifier="truncate" style={{ verticalAlign: "middle" }} colSpan={2}>
                                       {slot.description}
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
                        <Bullseye>{emptyState}</Bullseye>
                     </Td>
                  </Tr>
               )}
            </Tbody>
         </TableComposable>

         <Modal
            variant={ModalVariant.medium}
            className="wideModal"
            title={"Add slots to " + (selectedSignal && selectedSignal.name.split('.').slice(3).join('.') !== "" ? selectedSignal.name.split('.').slice(3).join('.') : "signal")}
            isOpen={isModalOpen}
            onEscapePress={() => handleCancelAddSlots()}

            onClose={() => handleCancelAddSlots()}
            actions={[
               <Button ref={AddButtonRef} key="confirm" variant="primary" onClick={() => handleAddSlots()}>
                  Add
               </Button>,
               <Button key="cancel" variant="link" onClick={() => handleCancelAddSlots()}>
                  Cancel
               </Button>
            ]}
         >
            <ModalContent slots={slots} selectedSlots={modalSelectedSlots} setSelectedSlots={setModalSelectedSlots} addButtonRef={AddButtonRef} signal={selectedSignal} />
         </Modal>

         <Modal
            variant={ModalVariant.small}
            title="Remove slots from signal?"
            isOpen={isRemoveModalOpen}
            onClose={() => setIsRemoveModalOpen(false)}

            actions={[
               <Button ref={removeButtonRef} key="confirm" variant="primary" onClick={() => handleRemoveSlots()}>
                  Remove
               </Button>,
               <Button ref={cancelButtonRef} key="cancel" variant="link" onClick={() => setIsRemoveModalOpen(false)}>
                  Cancel
               </Button>
            ]}
         >
            Are you sure you want to remove the selected slots from the signal?
         </Modal>

      </React.Fragment>
   );
};


export function formatDate(dateStr: string) {
   const date = new Date(dateStr);

   const formattedDate = date.toLocaleDateString('de-DE', { day: '2-digit', month: '2-digit', year: 'numeric' });
   const formattedTime = date.toLocaleTimeString('de-DE', { hour: '2-digit', minute: '2-digit' });

   return `${formattedTime} ${formattedDate}`
}
