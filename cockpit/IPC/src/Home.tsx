import { Button, TextInput, Title } from '@patternfly/react-core';
import React, { useEffect } from 'react';
import { Signal } from './Components/Signal/Signal';
import { Slot } from './Components/Slot/Slot';
import { useDbusInterface } from './Components/Interface/DbusInterface';

export const Home = () => {
   const [value, setValue] = React.useState('');
   const [signalList, setSignalList] = React.useState<any>([]);
   const [slotList, setSlotList] = React.useState<any>([]);
   const [signalListJSX, setSignalListJSX] = React.useState(<></>);
   const [slotListJSX, setSlotListJSX] = React.useState(<></>);
   const [selectedSignals, setSelectedSignals] = React.useState<any>([]);
   const [selectedSlots, setSelectedSlots] = React.useState<any>([]);

   const [signalSearchTerm, setSignalSearchTerm] = React.useState("");
   const [slotSearchTerm, setSlotSearchTerm] = React.useState("");


   const dbusInterface = useDbusInterface("com.skaginn3x.ipc_ruler", "com.skaginn3x.manager", "/com/skaginn3x/ipc_ruler");

   const handleSignalCheck = (signal: any) => {
      // selectedSignals can be max 1 signal
      setSelectedSignals([signal.name]);
      let slots = slotList.filter((slot: any) => slot.connected_to === signal.name)
      setSelectedSlots(slots.map((slot: any) => slot.name))


      // If the signal is already selected, unselect it
      // if (selectedSignals.includes(signal)) {
      //    setSelectedSignals(selectedSignals.filter((selectedSignal: any) => selectedSignal !== signal));
      // } else { // else select the signal
      //    setSelectedSignals([...selectedSignals, signal]);
      // }
   }
   const handleSlotCheck = (slot: any) => {
      // If the slot is already selected, unselect it
      if (selectedSlots.includes(slot.name)) {
         setSelectedSlots(selectedSlots.filter((selectedSlot: any) => selectedSlot !== slot.name));
      } else { // else select the slot
         setSelectedSlots([...selectedSlots, slot.name]);
      }
   }


   const runCockpitCommand = () => {
      console.log("Running command")
      const commandArray = value.split(' '); // splits the string into an array by spaces
      // if rm is in the command, return
      if (commandArray.filter((command: string) => command === "rm").length > 0) {
         console.log("rm is not allowed")
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
      } else {
         console.log("dbus object proxy is not valid")
      }
   }

   const connectSignals = async () => {
      selectedSlots.forEach(async (slot: any) => {
         const success = await dbusInterface.connect(selectedSignals[0], slot);
         if (success) {
            console.log("Successfully connected " + selectedSignals[0] + " to " + slot)
         } else {
            console.log("Failed to connect " + selectedSignals[0] + " to " + slot)
         }
      })
   }



   useEffect(() => {
      if (dbusInterface.valid) {
         pollDbus()
      }
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, [dbusInterface.valid])


   useEffect(() => {
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
                     isChecked={selectedSignals.includes(signal.name)}
                  />
               )
            })
      );
   }, [signalList, signalSearchTerm, selectedSignals])

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
         <Button variant="primary" onClick={connectSignals} style={{ margin: "1rem" }} disabled={selectedSignals.length === 0 || selectedSlots === 0}>
            Connect Signal to Slot{selectedSignals.length > 0 && selectedSlots.length > 0 ? "s" : ""}
         </Button>
         {dbusInterface.valid ? <p>Dbus is valid</p> : <p>Dbus is not valid</p>}
         {selectedSignals.length > 0 ? <p>Selected signals: {selectedSignals.map((signal: any) => signal).join(", ")}</p> : <p>No signals selected</p>}
         {selectedSlots.length > 0 ? <p>Selected slots: {selectedSlots.map((slot: any) => slot).join(", ")}</p> : <p>No slots selected</p>}

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

