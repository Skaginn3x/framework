/* eslint-disable react/no-unknown-property */
/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable react/jsx-no-undef */
/* eslint-disable react/no-unstable-nested-components */
import cockpit from 'pkg/lib/cockpit';
import React, {
  useEffect, useMemo, useRef, useState,
} from 'react';
import {
  Title,
  Divider,
  DataList,
  Spinner,
  AlertVariant,
} from '@patternfly/react-core';
import './IODebug.css';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import { loadExternalScript, parseXMLInterfaces } from 'src/Components/Interface/ScriptLoader';
import ListItem from 'src/Components/IOList/ListItem';
import { useAlertContext } from 'src/Components/Alert/AlertContext';
import TextboxAttribute from 'src/Components/Table/ToolbarItems/TextBoxAttribute';
import MultiSelectAttribute from 'src/Components/Table/ToolbarItems/MultiSelectAttribute';
import ToolBar, { FilterConfig } from 'src/Components/Table/Toolbar';
import { useDarkMode } from 'src/Components/Simple/DarkModeContext';
import useDbusInterface from "../Components/Interface/DbusInterface";
import {DBusEndpoint, SlotType, ipcDBusPath, SlotInterface} from "../Types";

// eslint-disable-next-line react/function-component-definition
const IODebug: React.FC = () => {
  const serviceManager = `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.ipc_ruler`;
  const interfaceManager = `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.manager`;
  const pathManager = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/ipc_ruler`;
  const dbusInterface = useDbusInterface(serviceManager, interfaceManager, pathManager);

  const interfaceSlot = `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.Slot`;
  const interfaceSignal = `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.Signal`;

  const { signalList, slotList } = dbusInterface;

  const { addAlert } = useAlertContext();
  const { isDark } = useDarkMode();

  useEffect(() => {
    dbusInterface.poll().then(() => {
      console.log('poll signal&slot list complete', signalList, slotList);
    }).catch((e: any) => {
      console.log('error: ', e);
    });
  }, [dbusInterface.valid]);

  return (
    <div style={{
      minHeight: '100vh',
      fontFamily: '"RedHatText", helvetica, arial, sans-serif !important',
      width: '100%',
    }}
    >
      <div style={{
        minWidth: '300px',
        flex: 1,
        height: '100%',
        width: '100%',
        transition: 'width 0.2s ease-in-out',
      }}
      >
        <Title headingLevel="h1" size="2xl" style={{ marginBottom: '1rem', color: isDark ? '#EEE' : '#111' }}>
          IO Debugging Tool
        </Title>
        <div style={{
          width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center',
        }}
        >
          <Divider />
          <Title headingLevel="h2" size="xl" style={{ marginBottom: '2rem', color: isDark ? '#EEE' : '#111' }}>
            Slots
          </Title>
          <DataList aria-label="Checkbox and action data list example" key="slots">
            { slotList.map((slot: SlotType) => {
                return (
                  <ListItem
                    endpoint={new SlotInterface(slot)}
                    dataType={slot.type}
                    tinker={true}
                    key={`${slot.name}-${slot.created_by}-List-Slot`}
                  />
                );
            })
              }
          </DataList>
          <Divider />
          <Title headingLevel="h2" size="xl" style={{ marginBottom: '2rem', color: isDark ? '#EEE' : '#111' }}>
            Signals
          </Title>
          {/*<DataList aria-label="Checkbox and action data list example" key="signals">*/}
          {/*  {dbusInterfaces.length > 0 ? dbusInterfaces.map((dbusInterface: any) => {*/}
          {/*    if (!dbusInterface.hidden && dbusInterface.direction === 'signal') {*/}
          {/*      return (*/}
          {/*        <ListItem*/}
          {/*          dbusInterface={dbusInterface}*/}
          {/*          key={`${dbusInterface.interfaceName}-${dbusInterface.process}-List-Signal`}*/}
          {/*          isChecked={dbusInterface.listener}*/}
          {/*          data={dbusInterface.data}*/}
          {/*          onCheck={() => (dbusInterface.listener*/}
          {/*            ? unsubscribe(dbusInterface.interfaceName)*/}
          {/*            : subscribe(dbusInterface.interfaceName)*/}
          {/*          )}*/}
          {/*        />*/}
          {/*      );*/}
          {/*    }*/}
          {/*    return null;*/}
          {/*  })*/}
          {/*    : <Spinner />}*/}
          {/*</DataList>*/}
          <div style={{ height: '10rem' }} />
        </div>
      </div>
    </div>
  );
};

export default IODebug;
