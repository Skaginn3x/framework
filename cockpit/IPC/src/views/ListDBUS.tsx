import React, { useEffect, useState } from 'react';
import { Title } from '@patternfly/react-core';
import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import { DarkModeType } from 'src/App';

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

// eslint-disable-next-line react/function-component-definition
const ListDBUS:React.FC<DarkModeType> = ({ isDark }) => {
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

  // // run after 2sec
  // setTimeout(async () => {
  //   const client = window.cockpit.dbus('com.skaginn3x.tfc.operations.def', { bus: 'system', superuser: 'try' });

  //   const watch = client.watch('/com/skaginn3x/Slots');
  //   console.log(watch);
  // }, 2000);

  return (
    <div style={{ color: isDark ? '#EEE' : '#111' }}>
      <Title headingLevel="h1" size="2xl">DBUS Names</Title>
      {dbusInterfaces.map((dbusInterface) => (
        <div key={`DBUS ${dbusInterface.iface}`}>
          <Title headingLevel="h2" size="xl">{dbusInterface.iface || 'Unknown name'}</Title>
        </div>
      ))}
    </div>
  );
};

export default ListDBUS;
