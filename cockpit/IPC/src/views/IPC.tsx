import { Title } from '@patternfly/react-core';
import { useEffect } from 'react';
import { useDbusInterface } from '../Components/Interface/DbusInterface';
import { TFC_DBUS_ORGANIZATION, TFC_DBUS_DOMAIN } from '../variables';
import './IPC.css';
import '@patternfly/react-styles/css/components/TreeView/tree-view.css';
import { Table } from 'src/Components/Table/Table';

export const IPC = () => {

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
      // eslint-disable-next-line react-hooks/exhaustive-deps
   }, [dbusInterface.valid]);

   const { signalList, slotList, connections } = dbusInterface;


   return (
      <>
         <Title headingLevel="h1" size="2xl">IPC - Time For Change</Title>
         {dbusInterface.valid ? <p>Dbus is valid</p> : <p>Dbus is not valid</p>}
         <div className="TableDiv">
            <Table signals={signalList || []} slots={slotList || []} connections={connections || {}} DBUS={dbusInterface} />
         </div>
      </>
   )
}

