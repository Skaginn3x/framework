/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable react/jsx-no-undef */
/* eslint-disable react/no-unstable-nested-components */
import React, { useEffect, useState } from 'react';
import {
  DataList,
  DataListItem,
  DataListItemCells,
  DataListItemRow,
  DataListCell,
  DataListAction,
  Dropdown,
  DropdownItem,
  Title,
} from '@patternfly/react-core';
import EllipsisVIcon from '@patternfly/react-icons/dist/esm/icons/ellipsis-v-icon';
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

export default function IODebug() {
  const [dbusInterfaces, setDbusInterfaces] = useState<any[]>([]);

  useEffect(() => {
    const callback = (allNames: string[]) => {
      const dbus = window.cockpit.dbus('org.freedesktop.DBus');
      const filteredNames = allNames.filter((name: string) => name.includes('config')); // && name.includes('_slot_')
      const proxies = connectToDBusNames(filteredNames, dbus);
      const interfaces = proxies.map((proxy: any) => ({
        proxy,
        dropdown: false,
        forcestate: null,
      }));
      setDbusInterfaces(interfaces);
    };

    loadExternalScript(callback);
  }, []);

  const setDropdownAtIndex = (index: number, bool:boolean) => {
    const updatedInterfaces = [...dbusInterfaces];
    updatedInterfaces[index].dropdown = bool;
    setDbusInterfaces(updatedInterfaces);
  };

  const onFocus = (index:number) => {
    setDropdownAtIndex(index, false);
    const element = document.getElementById('toggle-initial-selection');
    element?.focus();
  };

  //   return (
  //     <>
  //       {dbusInterfaces.map((dbusInterface) => (
  //         <div key={`DBUS ${dbusInterface.iface}`}>
  //           <Title headingLevel="h2" size="xl">{dbusInterface.iface || 'Unknown name'}</Title>
  //         </div>
  //       ))}
  //     </>
  //   );
  return (
    <>
      <Title headingLevel="h1" size="2xl" style={{ marginBottom: '2rem' }}>DBUS IO Debugging Tool</Title>
      <DataList aria-label="Checkbox and action data list example">
        {dbusInterfaces.map((dbusInterface:any, index:number) => (
          <DataListItem aria-labelledby="check-action-item1" key={dbusInterface.proxy.iface}>
            <DataListItemRow size={20}>
              <DataListItemCells
                dataListCells={[
                  <DataListCell key="primary content" style={{ textAlign: 'right' }}>
                    <span id="check-action-item1">{dbusInterface.proxy.iface}</span>
                  </DataListCell>,
                  <DataListCell key="secondary content 1" style={{ textAlign: 'right' }}>
                    Secondary content.
                  </DataListCell>,
                ]}
              />
              <DataListAction
                aria-labelledby="check-action-item1 check-action-action1"
                id="check-action-action1"
                aria-label="Actions"
                isPlainButtonAction
              >
                <Dropdown
                  onSelect={() => onFocus(index)}
                  onBlur={() => setDropdownAtIndex(index, false)}
                  key={`${dbusInterface.proxy.iface}-DD`}
                  toggle={<EllipsisVIcon aria-hidden="true" onClick={() => setDropdownAtIndex(index, !dbusInterface.dropdown)} />}
                  isOpen={dbusInterface.dropdown}
                  // onChange={() => toggleDropdownAtIndex(index)}
                  dropdownItems={[
                    <DropdownItem key="action">Action</DropdownItem>,
                    <DropdownItem key="disabled action" isDisabled>
                      Disabled Action
                    </DropdownItem>,
                  ]}
                />
              </DataListAction>
            </DataListItemRow>
          </DataListItem>
        ))}
      </DataList>
    </>
  );
}
