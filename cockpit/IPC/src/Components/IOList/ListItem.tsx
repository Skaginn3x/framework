/* eslint-disable react/no-unstable-nested-components */
/* eslint-disable no-param-reassign */
import React, { ChangeEvent, ReactElement } from 'react';
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
import { removeSlotOrg } from 'src/Components/Form/WidgetFunctions';
import StringTinker from 'src/Components/Tinker/StringTinker';
import BoolTinker from 'src/Components/Tinker/BoolTinker';
import NumberTinker from 'src/Components/Tinker/NumberTinker';

interface ListItemProps {
  dbusInterface: any,
  onCheck: (checked: boolean) => void,
  isChecked: boolean,
  data: any,
}

// eslint-disable-next-line react/function-component-definition
const ListItem: React.FC<ListItemProps> = ({
  dbusInterface,
  onCheck, isChecked, data,
}) => {
  const isMobile = window.innerWidth < 768;
  /**
  * Handles the content of the secondary column for booleans
  * @param data The data to be displayed
  * @returns ReactElement to be displayed
  */
  function handleBoolContent(booldata: any): ReactElement {
    return (
      <Tooltip
        content={`Value is ${booldata.Value ? 'true' : 'false'}`}
        enableFlip
        distance={5}
        entryDelay={1000}
      >
        { isChecked
          ? <Circle size="1rem" color={booldata.Value ? 'green' : 'red'} />
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
    if (!isChecked) {
      return (<p style={{ marginBottom: '0px' }}>Unknown</p>);
    }
    return (<p style={{ marginBottom: '0px' }}>{stringdata.Value}</p>);
  }

  /**
   * Handles the content of the secondary column for JSON objects
   * @param data The data to be displayed
   * @returns ReactElement to be displayed
   */
  function handleJSONContent(stringdata: any): ReactElement {
    if (!isChecked) {
      return (<p style={{ marginBottom: '0px' }}>Unknown</p>);
    }

    // Test if string is JSON
    let isJSON = false;
    if (stringdata.Value[0] === '{' || stringdata.Value[0] === '[') {
      try {
        JSON.parse(stringdata.Value);
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
            alignSelf: 'baseline', marginTop: '-.85rem', zIndex: 1, maxWidth: '25vw',
          }}
        >
          {JSON.stringify(JSON.parse(stringdata.Value), null, 2)}
        </ClipboardCopy>
      );
    }

    return (
      <p style={{ marginBottom: '0px' }}>{stringdata.Value}</p>
    );
  }

  /**
   * Handles the content of the secondary column
   * @param data Interface data
   * @returns ReactElement to be displayed
   */
  function getSecondaryContent(interfaceData: any): ReactElement | null {
    const internals = (secData: any) => {
      switch (secData.type) {
        case 'boolean':
          return handleBoolContent(data);

        case 'string':
          return handleJSONContent(data);
          // If the string is not JSON, it falls back to string

        case 'object':
          return handleJSONContent(data);

        case 'number':
        case 'integer':
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
        {internals(interfaceData)}
      </div>
    );
  }

  /**
 * Handles the content of the third column to enable tinkering
 * @param data Interface data
 * @returns ReactElement to be displayed
 */
  function getTinkerInterface(interfaceData: any): React.ReactElement | null {
    const internals = (interfacedata: any) => {
      switch (interfacedata.type) {
        case 'boolean':
          return <BoolTinker data={interfacedata} isChecked={isChecked} />;

        case 'string':
          return <StringTinker data={interfacedata} isChecked={isChecked} />;

        case 'object':
          return <StringTinker data={interfacedata} isChecked={isChecked} />;

        case 'number':
        case 'integer':
        case 'double':
          return <NumberTinker data={interfacedata} isChecked={isChecked} />;

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
        {internals(interfaceData)}
      </div>
    );
  }

  const handleCheckboxChange = (event: ChangeEvent<HTMLInputElement>) => {
    onCheck(event.target.checked);
  };

  return (
    <DataListItem aria-labelledby="check-action-item1" key={dbusInterface.proxy.iface + dbusInterface.process}>
      <DataListItemRow size={10}>
        <DataListCell key="checkbox" style={{ display: 'flex', alignItems: 'center', marginRight: '1rem' }}>
          <input
            type="checkbox"
            onChange={handleCheckboxChange}
            aria-label={`Select ${dbusInterface.proxy.iface}`}
            checked={isChecked}
          />
        </DataListCell>
        <DataListItemCells
          dataListCells={[
            <DataListCell key="primary content" style={{ textAlign: 'left' }}>
              <p className="PrimaryText">{removeSlotOrg(dbusInterface.proxy.iface)}</p>
            </DataListCell>,
            <DataListCell
              key={`${dbusInterface.interfaceName}-secondary-content`}
              className="SecondaryText"
              style={{
                textAlign: 'right',
                height: '100%',
                display: 'flex',
                alignItems: 'center',
              }}
            >
              {getSecondaryContent(dbusInterface)}
            </DataListCell>,
            dbusInterface.direction === 'slot'
              ? (
                <DataListCell
                  key={`${dbusInterface.interfaceName}-tinker-content`}
                  className="TinkerText"
                  style={{
                    textAlign: 'right',
                    height: '100%',
                    display: 'flex',
                    alignItems: 'center',
                  }}
                >
                  { getTinkerInterface(dbusInterface) }
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
