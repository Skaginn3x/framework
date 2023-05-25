import { AlertVariant, Button, TextInput, Title } from '@patternfly/react-core';
import React, { useEffect } from 'react';
import { Signal } from './Components/Signal/Signal';
import { Slot } from './Components/Slot/Slot';
import { useDbusInterface } from './Components/Interface/DbusInterface';
import { useAlertContext } from './Components/Alert/AlertContext';

export const Home = () => {
   const [value, setValue] = React.useState('');
   const [signalList, setSignalList] = React.useState<any>([]);
   const [slotList, setSlotList] = React.useState<any>([]);
   const [connections, setConnections] = React.useState<any>([]);
   const [signalListJSX, setSignalListJSX] = React.useState(<></>);
   const [slotListJSX, setSlotListJSX] = React.useState(<></>);
   const [selectedSignal, setSelectedSignal] = React.useState<string>("");
   const [selectedSlots, setSelectedSlots] = React.useState<any>([]);

   const [signalSearchTerm, setSignalSearchTerm] = React.useState("");
   const [slotSearchTerm, setSlotSearchTerm] = React.useState("");
   const { addAlert } = useAlertContext();

   const dbusInterface = useDbusInterface("com.skaginn3x.ipc_ruler", "com.skaginn3x.manager", "/com/skaginn3x/ipc_ruler");

   const handleSignalCheck = (signal: any) => {
      // selectedSignal can be max 1 signal
      //addAlert('Signal Changed', AlertVariant.success);
      setSelectedSignal(signal.name);
      let slots = slotList.filter((slot: any) => slot.connected_to === signal.name)
      setSelectedSlots(slots.map((slot: any) => slot.name));
   }

   const handleSlotCheck = async (slot: any) => {
      // If the slot is already selected, unselect it
      if (selectedSlots.includes(slot.name)) { // if the slot is already selected, disconnect it
         const data = await dbusInterface.disconnect(slot.name);
         if (!data) {
            addAlert('Error disconnecting slot', AlertVariant.danger);
            return
         }
         console.log("Slot disconnected successfully");
         setSelectedSlots(selectedSlots.filter((selectedSlot: any) => selectedSlot !== slot.name));
         setConnections((prevConnections: any) => {
            const newConnections = { ...prevConnections };
            newConnections[selectedSignal] = newConnections[selectedSignal].filter((connectedSlot: string) => connectedSlot !== slot.name);
            return newConnections;
         });
         setSlotList((prevSlotList: any) => {
            const newSlotList = [...prevSlotList];
            const slotIndex = newSlotList.findIndex((slotItem: any) => slotItem.name === slot.name);
            newSlotList[slotIndex].connected_to = "";
            return newSlotList;
         })

      } else { // else select the slot

         const data = await dbusInterface.connect(selectedSignal, slot.name);
         if (!data) {
            addAlert('Error connecting slot', AlertVariant.warning);
            return
         }

         setSelectedSlots([...selectedSlots, slot.name]);
         setConnections((prevConnections: any) => {
            const newConnections = { ...prevConnections };
            // if the slot is already assigned to another signal, remove it from that signal
            Object.keys(newConnections).forEach((signal: string) => {
               newConnections[signal] = newConnections[signal].filter((connectedSlot: string) => connectedSlot !== slot.name);
            })
            newConnections[selectedSignal].push(slot.name);
            return newConnections;
         })

         // remove it from the slot in slotList .connected_to
         setSlotList((prevSlotList: any) => {
            const newSlotList = [...prevSlotList];
            const slotIndex = newSlotList.findIndex((slotObj: any) => slotObj.name === slot.name);
            newSlotList[slotIndex].connected_to = selectedSignal;
            return newSlotList;
         })
      }
   }


   const runCockpitCommand = () => {
      const commandArray = value.split(' '); // splits the string into an array by spaces
      // if rm is in the command, return
      if (commandArray.filter((command: string) => command === "rm").length > 0) {
         console.log("rm is not allowed")
         addAlert('The rm command is not allowed', AlertVariant.danger);
         return
      }
      // @ts-ignore
      cockpit.spawn(commandArray, { err: "message" }).then((data: any) => {
         console.log(data)
      })
   }

   const pollDbus = async () => {
      const data = await dbusInterface.poll();
      if (data) {
         setSlotList(data.slotList);
         setSignalList(data.signalList);
         setConnections(data.connections);
         setSelectedSignal(data.signalList[0].name);
      } else {
         addAlert('DBus connection unsuccessful', AlertVariant.danger);

      }
   }


   useEffect(() => {
      if (dbusInterface.valid) {
         pollDbus()
      }
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, [dbusInterface.valid])


   useEffect(() => {
      if (!signalList || !connections) return
      setSignalListJSX(
         signalList.filter((signal: any) => {
            return Object.values(signal).some((val: any) =>
               val.toString().toLowerCase().includes(signalSearchTerm.toLowerCase())
            );
         })
            .map((signal: any) => {
               return (
                  <Signal
                     signal={signal}
                     onCheck={handleSignalCheck}
                     isChecked={selectedSignal.includes(signal.name)}
                     connections={connections[signal.name]}
                  />
               )
            })
      );
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, [signalList, signalSearchTerm, selectedSignal, connections])

   useEffect(() => {
      setSlotListJSX(
         slotList.filter((slot: any) => {
            // Check if any of the properties match/partially match the search term
            return Object.values(slot).some((val: any) =>
               val.toString().toLowerCase().includes(slotSearchTerm.toLowerCase())
            );
         })
            .map((slot: any) => {
               return (
                  <Slot
                     slot={slot}
                     onCheck={handleSlotCheck}
                     isChecked={selectedSlots.includes(slot.name)}
                  />
               )
            })
      );
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, [slotList, slotSearchTerm, selectedSlots])



   return (
      <>
         <Title headingLevel="h1" size="2xl">IPC - Time For Change</Title>
         <TextInput value={value} type="text" onChange={value => setValue(value)} aria-label="test-input" style={{ width: "20vw", margin: "1rem" }} />
         <Button variant="primary" onClick={runCockpitCommand} style={{ margin: "1rem" }}>
            Run Command
         </Button>
         {dbusInterface.valid ? <p>Dbus is valid</p> : <p>Dbus is not valid</p>}
         {selectedSignal.length > 0 ? <p>Selected signal: {selectedSignal}</p> : <p>No signal selected</p>}

         <div style={{ display: "flex", flexDirection: "row", maxWidth: "100vw" }}>
            <div style={{ minWidth: "30rem" }}>
               <Title headingLevel="h2" size="lg">Signals</Title>
               <TextInput value={signalSearchTerm} type="text" onChange={value => setSignalSearchTerm(value)} aria-label="Signal search" style={{ width: "20vw", margin: "1rem" }} />
               {signalListJSX}
            </div>
            <div style={{ minWidth: "30rem" }}>
               <Title headingLevel="h2" size="lg">Slots</Title>
               <TextInput value={slotSearchTerm} type="text" onChange={value => setSlotSearchTerm(value)} aria-label="Slot search" style={{ width: "20vw", margin: "1rem" }} />
               {slotListJSX}
            </div>
         </div>

      </>
   )
}

