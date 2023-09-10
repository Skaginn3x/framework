import React, { useEffect, useState } from 'react';
import { Title } from '@patternfly/react-core';
import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';

declare global {
  interface Window { cockpit: any; }
}

const connectToDBusNames = (names: string[], dbus: any) => {
  const proxies: any[] = [];
  names.forEach((name) => {
    const proxy = dbus.proxy(name);
    proxies.push(proxy);
  });
  return proxies;
};

export default function ListDBUS() {
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);

  useEffect(() => {
    const callback = (allNames: string[]) => {
      const dbus = window.cockpit.dbus('org.freedesktop.DBus');
      const filteredNames = allNames.filter((name: string) => name.includes('config'));
      const proxies = connectToDBusNames(filteredNames, dbus);
      setDbusInterfaces(proxies);
    };

    loadExternalScript(callback);
  }, []);

  return (
    <>
      <Title headingLevel="h1" size="2xl">DBUS Names</Title>
      {dbusInterfaces.map((dbusInterface) => (
        <div key={`DBUS ${dbusInterface.iface}`}>
          <Title headingLevel="h2" size="xl">{dbusInterface.iface || 'Unknown name'}</Title>
        </div>
      ))}
    </>
  );
}
