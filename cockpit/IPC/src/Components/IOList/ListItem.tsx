/* eslint-disable react/no-unstable-nested-components */
/* eslint-disable no-param-reassign */
import React, { ReactElement } from 'react';
import {
  ClipboardCopy,
  ClipboardCopyVariant,
  DataListAction,
  DataListCell,
  DataListItem,
  DataListItemCells,
  DataListItemRow,
  Dropdown,
  DropdownItem,
  DropdownList,
  MenuToggleElement,
  Tooltip,
} from '@patternfly/react-core';
import Circle from 'src/Components/Simple/Circle';
import { removeSlotOrg } from 'src/Components/Form/WidgetFunctions';
import CustomMenuToggle from 'src/Components/Dropdown/CustomMenuToggle';
import StringTinker from 'src/Components/Tinker/StringTinker';
import BoolTinker from 'src/Components/Tinker/BoolTinker';
import NumberTinker from 'src/Components/Tinker/NumberTinker';

interface ListItemProps {
  dbusInterface: any,
  index: number,
  activeDropdown: number | null,
  dropdownRefs: any,
  onToggleClick: any,
  setModalOpen: any,
}

// eslint-disable-next-line react/function-component-definition
const ListItem: React.FC<ListItemProps> = ({
  dbusInterface, index, activeDropdown, dropdownRefs, onToggleClick, setModalOpen,
}) => {
  const isMobile = window.innerWidth < 768;
  /**
  * Handles the content of the secondary column for booleans
  * @param data The data to be displayed
  * @returns ReactElement to be displayed
  */
  function handleBoolContent(data: any): ReactElement {
    return (
      <Tooltip
        content={`Value is ${data.Value ? 'true' : 'false'}`}
        enableFlip
        distance={5}
        entryDelay={1000}
      >
        <Circle size="1rem" color={data.Value ? 'green' : 'red'} />
      </Tooltip>
    );
  }

  /**
   * Handles the content of the secondary column for strings
   * @param data The data to be displayed
   * @returns ReactElement to be displayed
   */
  function handleStringContent(data: any): ReactElement {
    return (
      <p style={{ marginBottom: '0px' }}>{data.Value}</p>
    );
  }

  /**
   * Handles the content of the secondary column for JSON objects
   * @param data The data to be displayed
   * @returns ReactElement to be displayed
   */
  function handleJSONContent(data: any): ReactElement {
    // Test if string is JSON
    let isJSON = false;
    if (data.Value[0] === '{' || data.Value[0] === '[') {
      try {
        JSON.parse(data.Value);
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
          {JSON.stringify(JSON.parse(data.Value), null, 2)}
        </ClipboardCopy>
      );
    }

    return (
      <p style={{ marginBottom: '0px' }}>{data.Value}</p>
    );
  }

  /**
   * Handles the content of the secondary column
   * @param data Interface data
   * @returns ReactElement to be displayed
   */
  function getSecondaryContent(data: any): ReactElement | null {
    const internals = (interfacedata: any) => {
      switch (interfacedata.type) {
        case 'boolean':
          return handleBoolContent(interfacedata.proxy.data);

        case 'string':
          return handleJSONContent(interfacedata.proxy.data);
          // If the string is not JSON, it falls back to string

        case 'object':
          return handleJSONContent(interfacedata.proxy.data);

        case 'number':
        case 'integer':
        case 'double':
          return handleStringContent(interfacedata.proxy.data);

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
        {internals(data)}
      </div>
    );
  }

  /**
 * Handles the content of the third column to enable tinkering
 * @param data Interface data
 * @returns ReactElement to be displayed
 */
  function getTinkerInterface(data: any): React.ReactElement | null {
    const internals = (interfacedata: any) => {
      switch (interfacedata.type) {
        case 'boolean':
          return <BoolTinker data={interfacedata} />;

        case 'string':
          return <StringTinker data={interfacedata} />;

        case 'object':
          return <StringTinker data={interfacedata} />;

        case 'number':
        case 'integer':
        case 'double':
          return <NumberTinker data={interfacedata} />;

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
        {internals(data)}
      </div>
    );
  }
  const onSelect = (_event: React.MouseEvent<Element, MouseEvent> | undefined, value: string | number | undefined) => {
    console.log(value);
  };

  return (
    <DataListItem aria-labelledby="check-action-item1" key={dbusInterface.proxy.iface + dbusInterface.process}>
      <DataListItemRow size={10}>
        <DataListItemCells
          dataListCells={[
            <DataListCell key="primary content" style={{ textAlign: 'left' }}>
              <p className="PrimaryText">{removeSlotOrg(dbusInterface.proxy.iface)}</p>
            </DataListCell>,
            <DataListCell
              key={`${dbusInterface.interfaceName}-secondary-content`}
              className="FullSizeCell"
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
        <DataListAction
          aria-labelledby="check-action-item1 check-action-action1"
          id="check-action-action1"
          aria-label="Actions"
          isPlainButtonAction
        >
          <Dropdown
            className="DropdownItem"
            key={`${dbusInterface.proxy.iface}${dbusInterface.process}`}
            toggle={(toggleRef: React.Ref<MenuToggleElement>) => ( // NOSONAR
              <CustomMenuToggle
                toggleRef={(ref) => {
                  if (typeof toggleRef === 'function') {
                    toggleRef(ref);
                  }
                  dropdownRefs.current[index] = ref; // Store the ref for the dropdown
                }}
                onClick={() => onToggleClick(index)}
                isExpanded={dbusInterface.dropdown}
              />
            )}
            isOpen={activeDropdown === index}
            onSelect={onSelect}
            popperProps={{ enableFlip: true }}
          >
            <DropdownList>
              <DropdownItem key="history" style={{ textDecoration: 'none' }} isDisabled> View History </DropdownItem>
              <DropdownItem
                key={`watch-${dbusInterface.interfaceName}-dd`}
                style={{ textDecoration: 'none' }}
                onMouseDown={() => setModalOpen(index)}
              >
                Watch
              </DropdownItem>
            </DropdownList>
          </Dropdown>
        </DataListAction>
      </DataListItemRow>
    </DataListItem>
  );
};

export default ListItem;
