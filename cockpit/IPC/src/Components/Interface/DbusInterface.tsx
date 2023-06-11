import { useEffect, useState } from 'react';
import { ConnectionType, SignalType, SlotType } from '../../Types';

interface DbusInterfaceHook {
  poll: () => Promise<any>;
  registerSignal: (signalName: string, description: string, signalByte: number) => Promise<void | Error>;
  registerSlot: (slotName: string, description: string, slotByte: number) => Promise<void | Error>;
  connect: (slotName: string, signalName: string) => Promise<void | Error>;
  disconnect: (slotName: string) => Promise<void | Error>;
  signalList: SignalType[];
  slotList: SlotType[];
  connections: ConnectionType;
  valid: boolean;
}

const useDbusInterface = (busName: string, interfaceName: string, objectPath: string): DbusInterfaceHook => {
  const [dbusObjectProxy, setDbusObjectProxy] = useState<any | null>(null);
  const [valid, setValid] = useState<boolean>(false);

  const [slotList, setSlotList] = useState<SlotType[]>([]);
  const [signalList, setSignalList] = useState<SignalType[]>([]);
  const [connections, setConnections] = useState<ConnectionType>({});

  const [error, setError] = useState<Error | null>(null);

  const tryToConnectToDbus = async () => {
    // @ts-ignore
    const newDbus = cockpit.dbus(busName);
    const newDbusObjectProxy = newDbus.proxy(interfaceName, objectPath);
    newDbusObjectProxy.wait().then(() => {
      setError(null);
      setDbusObjectProxy(newDbusObjectProxy);
      setValid(newDbusObjectProxy.valid);
    }).catch((e: any) => {
      if (!error) {
        setError(e);
      }
      setDbusObjectProxy(null);
      setValid(false);
    });
  };

  useEffect(() => {
    if (error !== null) {
      console.error(error);
    }
  }, [error]);

  useEffect(() => {
    const externalScript = '../base1/cockpit.js';
    let script = document.querySelector(`script[src="${externalScript}"]`) as HTMLScriptElement;

    const handleScript = (e: Event) => {
      if (e.type === 'load') {
        // @ts-ignore
        const newDbus = cockpit.dbus(busName);
        const newDbusObjectProxy = newDbus.proxy(interfaceName, objectPath);
        newDbusObjectProxy.wait().then(() => {
          setDbusObjectProxy(newDbusObjectProxy);
          setValid(newDbusObjectProxy.valid);
        });
      }
    };

    if (!script) {
      script = document.createElement('script');
      script.src = externalScript;
      script.async = true;
      script.addEventListener('load', handleScript);
      script.addEventListener('error', handleScript);
      document.body.appendChild(script);
    } else {
      script.addEventListener('load', handleScript);
      script.addEventListener('error', handleScript);
    }

    return () => {
      script.removeEventListener('load', handleScript);
      script.removeEventListener('error', handleScript);
    };
  }, []);

  async function updateData() {
    if (dbusObjectProxy && dbusObjectProxy.valid) {
      console.info('dbus object proxy is valid, polling');
      setSlotList(await JSON.parse(dbusObjectProxy.data.Slots));
      setSignalList(await JSON.parse(dbusObjectProxy.data.Signals));
      setConnections(await JSON.parse(dbusObjectProxy.data.Connections));
    } else {
      throw new Error('dbus object proxy is not valid');
    }
  }

  const handleChanged = () => {
    updateData();
  };

  useEffect(() => {
    if (dbusObjectProxy && dbusObjectProxy.valid) {
      // Handles changes to dbus properties
      dbusObjectProxy.addEventListener('changed', handleChanged);
    }
    // Cleanup
    return () => {
      if (dbusObjectProxy && dbusObjectProxy.valid) {
        dbusObjectProxy.removeEventListener('changed', handleChanged);
      }
    };
  }, [dbusObjectProxy]); // Re-run this effect when dbusObjectProxy changes

  useEffect(() => {
    const checkConnection = setInterval(() => {
      if (!valid) { tryToConnectToDbus(); }
    }, 1000);

    return () => {
      clearInterval(checkConnection);
    };
  }, [valid]);

  const retry = <T extends any>(func: () => Promise<T>, retries: number = 3): Promise<T | Error> => func().catch(async (e: Error) => {
    if (retries > 0) {
      console.log(`Retry after error: ${e}`);
      await new Promise((resolve) => {
        setTimeout(resolve, 1000);
      });
      return retry(func, retries - 1);
    }
    tryToConnectToDbus();
    throw e;
  });

  const registerSignal = async (signalName: string, description: string, signalByte: number): Promise<void | Error> => retry(async () => {
    if (dbusObjectProxy) {
      await dbusObjectProxy.RegisterSignal(signalName, description, signalByte);
    } else {
      throw new Error('DBus proxy object is null');
    }
  });

  const registerSlot = async (slotName: string, description: string, slotByte: number): Promise<void | Error> => retry(async () => {
    if (dbusObjectProxy) {
      await dbusObjectProxy.RegisterSlot(slotName, description, slotByte);
    } else {
      throw new Error('DBus proxy object is null');
    }
  });

  const connect = async (slotName: string, signalName: string): Promise<void | Error> => retry(async () => {
    if (dbusObjectProxy) {
      await dbusObjectProxy.Connect(slotName, signalName);
    } else {
      throw new Error('DBus proxy object is null');
    }
  });

  const disconnect = async (slotName: string): Promise<void | Error> => retry(async () => {
    if (dbusObjectProxy) {
      await dbusObjectProxy.Disconnect(slotName);
    } else {
      throw new Error('DBus proxy object is null');
    }
  });

  const poll = async () => {
    await retry(async () => {
      updateData();
    });
  };

  return {
    poll, registerSignal, registerSlot, connect, disconnect, valid, slotList, signalList, connections,
  };
};
export default useDbusInterface;
