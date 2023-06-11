import React, { useEffect, useState } from 'react';
import { Title } from '@patternfly/react-core';

declare global {
  interface Window { cockpit: any; }
}

export default function ListDBUS() {
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);

  useEffect(() => {
    const externalScript = '../base1/cockpit.js';
    let script = document.querySelector(`script[src="${externalScript}"]`) as HTMLScriptElement;

    const handleScriptLoad = () => {
      const dbus = window.cockpit.dbus('org.freedesktop.DBus');
      const proxy = dbus.proxy();
      proxy.wait().then(() => {
        proxy.call('ListNames').then((names: any[]) => {
          // if name includes skaginn3x, get interfaces
          console.log('names: ', names[0]);
          const skaginn3xNames = names[0].filter((name: string) => name.includes('skaginn3x'));
          const skaginnproxies: any[] = [];
          skaginn3xNames.forEach((name: string) => {
            const subProxy = dbus.proxy(name);
            // store proxy in state
            skaginnproxies.push(subProxy);
          });
          setDbusInterfaces(skaginnproxies);
          console.log(skaginnproxies);
        });
      });
    };

    if (!script) {
      script = document.createElement('script');
      script.src = externalScript;
      script.async = true;
      script.addEventListener('load', handleScriptLoad);
      script.addEventListener('error', (e) => { console.error('Error loading script', e); });
      document.body.appendChild(script);
    } else {
      script.addEventListener('load', handleScriptLoad);
      script.addEventListener('error', (e) => { console.error('Error loading script', e); });
    }
  }, []);

  return (
    <>
      {dbusInterfaces.map((dbusInterface) => (
        <div key={`DBUS ${dbusInterface.iface}`}>
          <Title headingLevel="h2" size="xl">{dbusInterface.iface || 'Unknown name'}</Title>
        </div>
      ))}
    </>
  );
}
