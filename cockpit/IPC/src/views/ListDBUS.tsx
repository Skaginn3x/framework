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
  // setTimeout(() => {
  //   const systemBus = window.cockpit.dbus('org.freedesktop.DBus', { bus: 'system', superuser: 'try' });

  //   // Define your match rules
  //   const matchRules = [
  //     "path='/com/skaginn3x/Slots',member='PropertiesChanged'",
  //     // Add other match rules here
  //   ];

  //   // Call BecomeMonitor
  //   systemBus.call(
  //     '/org/freedesktop/DBus',
  //     'org.freedesktop.DBus.Monitoring',
  //     'BecomeMonitor',
  //     [matchRules, 0],
  //   ).done(() => {
  //     // Monitor is now active
  //     console.log('Monitoring DBus signals');
  //   }).fail((err: any) => {
  //     // Handle error
  //     console.error('Error setting up DBus monitor:', err);
  //   });

  //   // Handle incoming signals
  //   systemBus.addEventListener('message', (event: { data: any; }) => {
  //     // Process the message/event
  //     const msg = event.data;

  //     if (msg && msg.member === 'PropertiesChanged') {
  //       console.log(msg);
  //     }
  //   });
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
