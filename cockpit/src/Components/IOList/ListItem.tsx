/* eslint-disable react/no-unstable-nested-components */
/* eslint-disable no-param-reassign */
import React, {ChangeEvent, ReactElement, useEffect, useState} from 'react';
import {
  ClipboardCopy,
  ClipboardCopyVariant,
  DataListCell,
  DataListItem,
  DataListItemCells,
  DataListItemRow,
  Tooltip,
} from '@patternfly/react-core';
import Circle from 'src/Components/Simple/Circle';
import {removeOrg, removeSlotOrg} from 'src/Components/Form/WidgetFunctions';
import StringTinker from 'src/Components/Tinker/StringTinker';
import BoolTinker from 'src/Components/Tinker/BoolTinker';
import NumberTinker from 'src/Components/Tinker/NumberTinker';
import {DBusEndpoint} from "../../Types";

interface ListItemProps {
  endpoint: DBusEndpoint,
  dataType: string,
  tinker?: boolean,
}

// eslint-disable-next-line react/function-component-definition
const ListItem: React.FC<ListItemProps> = ({
  endpoint, dataType, tinker
}) => {
  const isMobile = window.innerWidth < 768;
  let [enabled, setEnabled] = useState(false);
  let [data, setData] = useState<any>();

  useEffect(() => {
    if (enabled) {
      endpoint.connect().then(() => {
        console.log('connected to endpoint: ', endpoint.displayName);
        endpoint.client.call(endpoint.path, 'org.freedesktop.DBus.Properties', 'Get', endpoint.interface, 'Value').then((value: any) => {
          console.log(value);
        }).catch((e: any) => {
          console.error(`error getting value for ${endpoint.interface}: `, e);
        });
        endpoint.client.subscribe({ path_namespace: endpoint.path, member: 'PropertiesChanged' }, (path, intf, signal, args) => {
          // check if args have the right format
          if (!args[0] || args[0] !== endpoint.interface) {
            console.error('invalid interface in args: ', args);
            return;
          }
          if (!args[1] || !('Value' in args[1]) || !('v' in args[1]['Value'])) {
            console.error('invalid value in args: ', args);
            return;
          }
          setData(args[1]['Value']['v']);
          // console.log('signal: ', path, intf, signal, args, args[1]['Value']['v']);
        });
        // endpoint.proxy.addEventListener('changed', (event, data) => {
        //   console.log('signal: ', event, data);
        // });
        // endpoint.client.watch({ path_namespace: endpoint.path, interface: endpoint.interface }).then((data: any) => {
        //   console.log('watched data: ', data);
        // });
        // endpoint.client.addEventListener('notify', (data: any) => {
        //   console.log('notified data: ', data, endpoint.interface);
        // });
      });
    }
    else {
      endpoint.disconnect();
    }
  }, [endpoint, endpoint.client, enabled]);

  /**
  * Handles the content of the secondary column for booleans
  * @param booldata The data to be displayed
  * @returns ReactElement to be displayed
  */
  function handleBoolContent(booldata: any): ReactElement {
    return (
      <Tooltip
        content={`Value is ${booldata ? 'true' : 'false'}`} // NOSONAR
        enableFlip
        distance={5}
        entryDelay={1000}
      >
        { booldata !== null
          ? <Circle size="1rem" color={booldata ? 'green' : 'red'} />
          : <Circle size="1rem" color="#888888" />}
      </Tooltip>
    );
  }

  /**
   * Handles the content of the secondary column for strings
   * @param data The data to be displayed
   * @returns ReactElement to be displayed
   */
  function handleStringContent(stringdata: any): ReactElement {
    if (stringdata === null) {
      return (<p style={{ marginBottom: '0px' }}>Unknown</p>);
    }
    return (<p style={{ marginBottom: '0px' }}>{stringdata}</p>);
  }

  /**
   * Handles the content of the secondary column for JSON objects
   * @param data The data to be displayed
   * @returns ReactElement to be displayed
   */
  function handleJSONContent(stringdata: any): ReactElement {
    if (stringdata === null) {
      return (<p style={{ marginBottom: '0px' }}>Unknown</p>);
    }

    // Test if string is JSON
    let isJSON = false;
    if (stringdata[0] === '{' || stringdata[0] === '[') {
      try {
        JSON.parse(stringdata);
        isJSON = true;
      } catch {
        isJSON = false;
      }
    }

    if (isJSON) {
      return (
        <ClipboardCopy
          isCode
          hoverTip="Copy"
          isReadOnly
          clickTip="Copied"
          variant={ClipboardCopyVariant.expansion}
          style={{
            alignSelf: 'baseline', marginTop: '-.25rem', zIndex: 1, maxWidth: '25vw',
          }}
        >
          {JSON.stringify(JSON.parse(stringdata), null, 2)}
        </ClipboardCopy>
      );
    }

    return (
      <p style={{ marginBottom: '0px' }}>{stringdata}</p>
    );
  }

  /**
   * Handles the content of the secondary column
   * todo make data type dynamically instead of statically
   * todo for example backend can send how the data type should be treated by others
   * todo can this be enum
   * @param dataType 'unknown' | 'bool' | 'int64_t' | 'uint64_t' | 'double' | 'string' | 'json'
   * @returns ReactElement to be displayed
   */
  function getSecondaryContent(data: any, dataType: string): ReactElement | null {
    const internals = (data: any, dataType: string) => {
      switch (dataType) {
        case 'bool':
          return handleBoolContent(data);
        case 'json':
          return handleJSONContent(data);
        case 'string':
        case 'number':
        case 'int64_t':
        case 'uint64_t':
        case 'double':
          return handleStringContent(data);

        default:
          return <>Type Error</>;
      }
    };

    return (
      <div style={{
        height: '100%',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'end',
        padding: isMobile ? '10px' : '0px',
      }}
      >
        {internals(data, dataType)}
      </div>
    );
  }

  /**
   * Handles the content of the third column to enable tinkering
   * todo refer to todos above
   * @param endpoint DBus endpoint
   * @param dataType 'unknown' | 'bool' | 'int64_t' | 'uint64_t' | 'double' | 'string' | 'json'
   * @returns ReactElement to be displayed
   */
  function getTinkerInterface(data: any, dataType: string, endpoint: DBusEndpoint): React.ReactElement | null {
    const internals = (data: any, dataType: string, endpoint: DBusEndpoint) => {
      switch (dataType) {
        case 'bool':
          return <BoolTinker endpoint={endpoint} data={data} />;

        // case 'string':
        // case 'object':
        //   return isChecked ? <StringTinker interfaceData={interfacedata}/> : null;
        //
        // case 'int64_t':
        // case 'uint64_t':
        // case 'double':
        //   return isChecked ? <NumberTinker interfaceData={interfacedata}/> : null;

        default:
          return <>Type Error</>;
      }
    };
    return (
      <div style={{
        height: '100%',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'start',
      }}
      >
        {internals(data, dataType, endpoint)}
      </div>
    );
  }


  return (
    <DataListItem aria-labelledby="check-action-item1" key={endpoint.interface}>
      <DataListItemRow size={10}>
        <DataListCell key="checkbox" style={{ display: 'flex', alignItems: 'center', marginRight: '1rem' }}>
          <input
            type="checkbox"
            aria-label={`Select ${endpoint.interface}`}
            checked={enabled}
            onChange={(e: ChangeEvent<HTMLInputElement>) => {
              setEnabled(e.target.checked);
              console.log('checked', e.target.checked, enabled);
            }}
          />
        </DataListCell>
        <DataListItemCells
          dataListCells={[
            <DataListCell key="primary content" style={{ textAlign: 'left' }}>
              <p className="PrimaryText">{endpoint.displayName}</p>
            </DataListCell>,
            enabled ? (<DataListCell
              key={`${endpoint.interface}-secondary-content`}
              className="SecondaryText"
              style={{
                textAlign: 'right',
                height: '100%',
                display: 'flex',
                alignItems: 'center',
              }}
            >
              {getSecondaryContent(data, dataType)}
            </DataListCell>) : null,
            tinker && enabled
              ? (
                <DataListCell
                  key={`${endpoint.interface}-tinker-content`}
                  className="TinkerText"
                  style={{
                    textAlign: 'right',
                    height: '100%',
                    display: 'flex',
                    alignItems: 'center',
                  }}
                >
                  { getTinkerInterface(data, dataType, endpoint) }
                </DataListCell>
              )
              : null,
          ]}
        />
      </DataListItemRow>
    </DataListItem>
  );
};

export default ListItem;
