import { useEffect, useState } from 'react';

type DbusProxy = any; // Replace with the correct type for your dbus proxy objects

interface DbusInterfaceHook {
   poll: () => Promise<any>;
   registerSignal: (signalName: string, signalByte: number) => Promise<boolean>;
   registerSlot: (slotName: string, slotByte: number) => Promise<boolean>;
   connect: (signalName: string, slotName: string) => Promise<boolean>;
   disconnect: (signalName: string) => Promise<boolean>;
   valid: boolean;
}

export const useDbusInterface = (busName: string, interfaceName: string, objectPath: string): DbusInterfaceHook => {
   const [dbusObjectProxy, setDbusObjectProxy] = useState<DbusProxy | null>(null);
   const [valid, setValid] = useState<boolean>(false);

   useEffect(() => {
      const externalScript = "../base1/cockpit.js";
      let script = document.querySelector(`script[src="${externalScript}"]`) as HTMLScriptElement;

      const handleScript = (e: Event) => {
         if (e.type === "load") {
            //@ts-ignore
            const newDbus = cockpit.dbus(busName);
            const newDbusObjectProxy = newDbus.proxy(interfaceName, objectPath);
            newDbusObjectProxy.wait().then(() => {
               setDbusObjectProxy(newDbusObjectProxy);
               setValid(newDbusObjectProxy.valid);
            });
         }
      };

      if (!script) {
         script = document.createElement("script");
         script.src = externalScript;
         script.async = true;
         script.addEventListener("load", handleScript);
         script.addEventListener("error", handleScript);
         document.body.appendChild(script);
      } else {
         script.addEventListener("load", handleScript);
         script.addEventListener("error", handleScript);
      }

      return () => {
         script.removeEventListener("load", handleScript);
         script.removeEventListener("error", handleScript);
      };
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, []);

   const registerSignal = async (signalName: string, signalByte: number): Promise<boolean> => {
      try {
         if (dbusObjectProxy) {
            const response = await dbusObjectProxy.Register_signal(signalName, signalByte);
            console.log("Signal registered successfully: ", response);
            return true;
         }
      } catch (error) {
         console.error("Failed to register signal: ", error);
      }
      return false;
   };

   const registerSlot = async (slotName: string, slotByte: number): Promise<boolean> => {
      try {
         if (dbusObjectProxy) {
            const response = await dbusObjectProxy.Register_slot(slotName, slotByte);
            console.log("Slot registered successfully: ", response);
            return true;
         }
      } catch (error) {
         console.error("Failed to register slot: ", error);
      }
      return false;
   };

   const connect = async (signalName: string, slotName: string): Promise<boolean> => {
      try {
         if (dbusObjectProxy !== null && dbusObjectProxy.valid) {
            await dbusObjectProxy.Connect(slotName, signalName)
            return true;
         }
         console.log("dbus object proxy is null or not valid")
      } catch (error: any) {
         console.error("Failed to connect: ", error.message);
      }
      return false;
   };

   const disconnect = async (signalName: string): Promise<boolean> => {
      try {
         if (dbusObjectProxy) {
            const response = await dbusObjectProxy.Disconnect(signalName);
            console.log("Disconnected successfully: ", response);
            return true;
         }
      } catch (error) {
         console.error("Failed to disconnect: ", error);
      }
      return false;
   };

   const poll = (): Promise<any> => {
      return new Promise((resolve, reject) => {
         if (dbusObjectProxy && dbusObjectProxy.valid) {
            console.log("dbus object proxy is valid, polling")
            // change dbusObjectProxy.data from string to object
            let slotList = JSON.parse(dbusObjectProxy.data.Slots);
            let signalList = JSON.parse(dbusObjectProxy.data.Signals);
            let connections = JSON.parse(dbusObjectProxy.data.Connections);
            resolve({ slotList, signalList, connections });
         } else {
            console.log("dbus object proxy is not valid");
            reject("dbus object proxy is not valid");
         }

         // Add a timeout
         setTimeout(() => {
            reject("pollDbus timeout");
         }, 1000);  // 1 second timeout for polling
      });
   };

   return { poll, registerSignal, registerSlot, connect, disconnect, valid };
};
