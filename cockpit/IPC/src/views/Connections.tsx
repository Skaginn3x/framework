import React, { useEffect, useState } from 'react';
import { QuestionCircleIcon } from '@patternfly/react-icons';
import {
  Button, Modal, Title, Tooltip,
} from '@patternfly/react-core';

import { DarkModeType } from 'src/App';
// import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import CustomTable from '../Components/Table/Table';
import useDbusInterface from '../Components/Interface/DbusInterface';
import { TFC_DBUS_ORGANIZATION, TFC_DBUS_DOMAIN } from '../variables';
import './Connections.css';

// eslint-disable-next-line react/function-component-definition
const Connections:React.FC<DarkModeType> = ({ isDark }) => {
  const busName = `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.ipc_ruler`;
  const interfaceName = `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.manager`;
  const objectPath = `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/ipc_ruler`;

  const dbusInterface = useDbusInterface(busName, interfaceName, objectPath);
  const [timezone, setTimezone] = useState<string>('Atlantic/Reykjavik');
  const pollDbus = async () => {
    await dbusInterface.poll();
  };
  useEffect(() => {
    if (dbusInterface.valid) {
      pollDbus();
      try {
        const dbus = window.cockpit.dbus('org.freedesktop.timedate1');
        const proxy = dbus.proxy();
        proxy.wait().then(() => {
          setTimezone(proxy.Timezone);
        });
      } catch (e) {
        console.log(e);
      }
    }
  }, [dbusInterface.valid]);

  const { signalList, slotList, connections } = dbusInterface;
  const [modalIsOpen, setModalIsOpen] = React.useState<boolean>(false);

  const TooltipContent = (
    <>
      <Title headingLevel="h1" size="2xl" style={{ color: isDark ? '#EEE' : '#111' }}>
        Keyboard Shortcuts
      </Title>
      <br />
      <strong>Arrow keys:</strong>
      <ul>
        <li>Up/Down - Move up/down in table</li>
      </ul>
      <strong>If slot is selected:</strong>
      <ul>
        <li>Enter - Connect signal to slot</li>
      </ul>
      <strong>If signal is selected:</strong>
      <ul>
        <li>Enter - Disconnect signal from slot</li>
        <li>Shift + Enter - Open Filter Dialog</li>
      </ul>
      <strong>General:</strong>
      <ul>
        <li>Ctrl + F - Open search</li>
      </ul>
    </>
  );

  return (
    <>
      <Tooltip
        content="Keyboard Shortcuts"
        title="Keyboard Shortcuts"
      >
        <QuestionCircleIcon
          size={16}
          color={isDark ? '#EEE' : '#111'}
          style={{ position: 'absolute', top: '1rem', left: '1rem' }}
          onClick={() => setModalIsOpen(true)}
        />
      </Tooltip>
      <Title headingLevel="h1" size="2xl" style={{ color: isDark ? '#EEE' : '#111' }}>
        Connections
      </Title>
      <div className="TableDiv">
        <CustomTable
          signals={signalList || []}
          slots={slotList || []}
          connections={connections || {}}
          DBUS={dbusInterface}
          isDark={isDark}
          timezone={timezone}
        />
      </div>
      <Modal
        isOpen={modalIsOpen}
        aria-label="Keyboard Shortcut Information Modal"
        onEscapePress={() => setModalIsOpen(false)}
        onClose={() => setModalIsOpen(false)}
        style={{ color: isDark ? '#EEE' : '#111' }}
        variant="small"
        actions={[
          <Button
            key="close"
            variant="primary"
            onClick={() => setModalIsOpen(false)}
          >
            Close
          </Button>,
        ]}
      >
        {TooltipContent}
      </Modal>
    </>
  );
};

export default Connections;
