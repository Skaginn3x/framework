import React, { useEffect } from 'react';
import { Switch, Title } from '@patternfly/react-core';
import { DarkModeType } from 'src/App';
import CustomTable from '../Components/Table/Table';
import useDbusInterface from '../Components/Interface/DbusInterface';
import { TFC_DBUS_ORGANIZATION, TFC_DBUS_DOMAIN } from '../variables';
import './IPC.css';

// eslint-disable-next-line react/function-component-definition
const IPC:React.FC<DarkModeType> = ({ isDark, setIsDark }) => {
  const busName = `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.ipc_ruler`;
  const interfaceName = `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.manager`;
  const objectPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/ipc_ruler`;

  const dbusInterface = useDbusInterface(busName, interfaceName, objectPath);

  const pollDbus = async () => {
    await dbusInterface.poll();
  };

  useEffect(() => {
    if (dbusInterface.valid) {
      pollDbus();
    }
  }, [dbusInterface.valid]);

  const { signalList, slotList, connections } = dbusInterface;

  return (
    <>
      <Title headingLevel="h1" size="2xl" style={{ marginBottom: '2rem', color: isDark ? '#EEE' : '#111' }}>
        IPC - Time For Change
      </Title>
      <Switch
        onChange={(_, state) => setIsDark(state)}
        isChecked={isDark}
      />
      <div className="TableDiv">
        <CustomTable
          signals={signalList || []}
          slots={slotList || []}
          connections={connections || {}}
          DBUS={dbusInterface}
          isDark={isDark}
        />
      </div>
    </>
  );
};

export default IPC;
