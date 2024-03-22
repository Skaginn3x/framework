import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
/* eslint-disable no-continue */
import React, { useEffect, useState } from 'react';
import {
  Title,
  Nav,
  NavGroup,
  NavItem,
  Drawer,
  DrawerPanelContent,
  DrawerContent,
  DrawerContentBody,
  Button,
  Modal,
  AlertVariant,
} from '@patternfly/react-core';
import { DownloadIcon, UploadIcon } from '@patternfly/react-icons';
import 'bootstrap/dist/css/bootstrap.min.css';
import 'bootstrap-icons/font/bootstrap-icons.css';
import './Configurator.css';
import Hamburger from 'hamburger-react';
import {
  getData, handleNullValue, sortItems, updateFormData,
} from 'src/Components/Form/WidgetFunctions';
import { useDarkMode } from 'src/Components/Simple/DarkModeContext';
import AceEditor from 'react-ace';
import FormGenerator from '../Components/Form/Form';
import { useAlertContext } from '../Components/Alert/AlertContext';

import 'ace-builds/src-noconflict/mode-json';
import 'ace-builds/src-noconflict/snippets/json';
import 'ace-builds/src-noconflict/theme-twilight';
import 'ace-builds/src-noconflict/theme-github';
import 'ace-builds/src-noconflict/theme-github_dark';
import 'ace-builds/src-noconflict/ext-language_tools';

declare global {
  interface Window { cockpit: any; }
}

// eslint-disable-next-line react/function-component-definition
const Configurator: React.FC = () => {
  const { addAlert } = useAlertContext();
  const { isDark } = useDarkMode();
  // const [names, setNames] = useState<Map<string, string>>(new Map());
  const [isDrawerExpanded, setIsDrawerExpanded] = useState(true);
  const [formSubmissionCount, setFormSubmissionCount] = useState(0);
  const [isRawModalOpen, setIsRawModalOpen] = useState(false);
  const [rawData, setRawData] = useState<string>('{}');
  const [rawErrorMessage, setRawErrorMessage] = useState<string>('');
  const toggleDrawer = () => {
    setIsDrawerExpanded(!isDrawerExpanded);
  };

  const [formData, setFormData] = useState<any>({});
  const [schemas, setSchemas] = useState<any>({});

  function loadData() {
    getData().then((allData) => {
      // data is an array of { name, data: { schemas, data } }
      allData.forEach(({ name, data }) => {
        setSchemas((prevState: any) => {
          const newSchemas = { ...prevState };
          const filteredSchemas = Object.keys(data.schemas).reduce((acc: any, key: string) => {
            if (!key.includes('/Filter')) {
              acc[key] = data.schemas[key];
            }
            return acc;
          }, {});
          if (newSchemas[name]) {
            newSchemas[name] = { ...newSchemas[name], ...filteredSchemas };
          } else {
            newSchemas[name] = filteredSchemas;
          }
          return newSchemas;
        });

        setFormData((prevState: any) => {
          const newFormData = { ...prevState };
          const filteredData = Object.keys(data.data).reduce((acc: any, key: string) => {
            if (!key.includes('/Filter')) {
              acc[key] = data.data[key];
            }
            return acc;
          }, {});
          if (newFormData[name]) {
            newFormData[name] = { ...newFormData[name], ...filteredData };
          } else {
            newFormData[name] = filteredData;
          }
          return newFormData;
        });
      });
    });
  }

  useEffect(() => {
    loadData();
  }, []);

  const [activeItem, setActiveItem] = React.useState(Object.keys(schemas)[0]);
  const [activeItemProcess, setActiveItemProcess] = React.useState<string | undefined>(undefined);
  const onSelect = (selectedItem: {
    groupId: string | number;
    itemId: string | number;
  }) => {
    if (selectedItem.itemId) {
      setActiveItem(selectedItem.itemId as string);
      setIsDrawerExpanded(false);
    }
    if (selectedItem.groupId) {
      setActiveItemProcess(selectedItem.groupId as string);
    }
    if (!selectedItem.groupId && !selectedItem.itemId) {
      setActiveItemProcess(undefined);
    }
  };

  /**
   * Posts data to dbus
   * @param data Configuration values
   */
  function handleSubmit(data: any) {
    // eslint-disable-next-line no-param-reassign
    data = handleNullValue(data);
    // Find interface name from dbus name
    if (!activeItem) return;

    updateFormData(
      activeItemProcess, // Process name
      `${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.Config`, // Interface name
      activeItem, // path
      'Value', // property
      data, // Data
      setFormData,
      addAlert,
    );
    setTimeout(() => {
      loadData();
    }, 100);
  }

  /**
   * Reads schemas, and gets the processes from the keys
   * Uses string splitting, which is not ideal, but no other option is available
   * @returns string[] of processes
   */
  function getProcesses(): string[] {
    const fullNameUnique = Object.keys(schemas).filter((value, index, self) => value && self.indexOf(value) === index);
    // 4. Sort the list alphabetically
    return sortItems(fullNameUnique);
  }

  function getPaths(process:string): string[] {
    if (Object.keys(schemas).includes(process)) {
      return sortItems(Object.keys(schemas[process] || []));
    }
    return [];
  }

  function prettify(name: string) {
    if (!name) return '';
    let newName = name.replaceAll('_', ' ');
    newName = newName.split(' ').map((word) => word.charAt(0).toUpperCase() + word.slice(1)).join(' ');
    return newName;
  }

  useEffect(() => {
    if (!schemas || !activeItem) return;
    setFormSubmissionCount((count) => count + 1);
  }, [activeItem, schemas]);

  useEffect(() => {
    if (activeItem && schemas[activeItem]) {
      setRawData(JSON.stringify(formData[activeItem], null, 2));
    }
  }, [activeItem]);

  const panelContent = (
    <DrawerPanelContent style={{ backgroundColor: '#212427' }}>
      <div style={{
        minWidth: '15rem', backgroundColor: '#212427', height: '-webkit-fill-available',
      }}
      >
        <Nav onSelect={(_, item) => onSelect(item)} aria-label="Grouped global">
          <NavGroup title="Processes">
            <NavItem
              preventDefault
              to="#all"
              key="all-navItem"
              groupId={undefined}
              isActive={activeItemProcess === undefined}
            >
              All
            </NavItem>
            {getProcesses().map((name) => (
              <NavItem
                preventDefault
                to={`#${name}`}
                key={`${name}-navItem`}
                groupId={name}
                isActive={activeItemProcess === name}
              >
                {name.split('.').slice(3).join('.')}
              </NavItem>
            ))}
          </NavGroup>
          <NavGroup title="Schemas">
            {activeItemProcess ? sortItems(getPaths(activeItemProcess))
              .map((name: string) => (
                <NavItem
                  preventDefault
                  to={`#${name}`}
                  key={`${name}-navItem`}
                  itemId={name}
                  isActive={activeItem === name}
                >
                  {name ? prettify(name.split('/').splice(-1)[0]) : name}
                </NavItem>
              ))
              : sortItems(Object.keys(schemas)).map((process) => (
                sortItems(Object.keys(schemas[process] || {})).map((path) => (
                  <NavItem
                    preventDefault
                    to={`#${path}`}
                    key={`${process}-${path}-navItem`}
                    groupId={process}
                    itemId={path}
                    isActive={activeItem === path && activeItemProcess === process}
                  >
                    {path ? `${process.split('.').slice(3).join('.')} ${prettify(path.split('/').splice(-1)[0])}` : path }
                  </NavItem>
                ))
              ))}
          </NavGroup>
        </Nav>
      </div>
    </DrawerPanelContent>
  );
  function saveRawJSON(data: string) {
    const blob = new Blob([data], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const now = new Date();
    const a = document.createElement('a');
    a.href = url;
    a.download = `${activeItem}-${now.toISOString()}.json`;
    a.click();
    URL.revokeObjectURL(url);
  }

  function openRawDataJSON() {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.onchange = (event: any) => {
      const file = event.target.files[0];
      const reader = new FileReader();
      reader.onload = (e: any) => {
        console.log(e);
        setRawData(e.target.result);
      };
      reader.readAsText(file);
    };
    input.click();
  }

  return (
    <div style={{
      height: '100vh',
      fontFamily: '"RedHatText", helvetica, arial, sans-serif !important',
      width: '100%',
    }}
    >
      <Modal
        isOpen={isRawModalOpen}
        aria-label="Raw edit modal"
        onClose={() => setIsRawModalOpen(false)}
        onEscapePress={() => setIsRawModalOpen(false)}
        disableFocusTrap
        style={{ color: isDark ? '#EEE' : '#111' }}
        variant="medium"
        header={(
          <Title headingLevel="h1" size="3xl" style={{ color: isDark ? '#EEE' : '#333' }}>
            {`Edit Raw - ${activeItem && activeItemProcess ? activeItemProcess.split('.').slice(3).join('.') : 'Error - Unknown name'}`}
          </Title>
        )}
        actions={[
          <Button
            key="sendbtn"
            variant="primary"
            onClick={() => {
              try {
                const result = JSON.parse(rawData);
                handleSubmit(result.config);
                setIsRawModalOpen(false);
              } catch (e: any) {
                console.error('Error parsing JSON:', e);
                setRawErrorMessage(e.message);
                addAlert(`Parsing Error: ${e.message}`, AlertVariant.danger);
              }
            }}
          >
            Send
          </Button>,
          <Button
            key="closebtn"
            variant="link"
            onClick={() => setIsRawModalOpen(false)}
          >
            Close
          </Button>,
          <Button
            key="save-btn"
            variant="plain"
            onClick={() => saveRawJSON(rawData)}
          >
            <DownloadIcon style={{ width: '1.2rem', height: '1.2rem', marginRight: '0.5rem' }} />
            Save
          </Button>,
          <Button
            key="open-btn"
            variant="plain"
            onClick={() => openRawDataJSON()}
          >
            <UploadIcon style={{ width: '1.2rem', height: '1.2rem', marginRight: '0.5rem' }} />
            Open
          </Button>,
        ]}
      >
        <AceEditor
          placeholder="Data"
          mode="json"
          theme={isDark ? 'github_dark' : 'github'}
          name={`ace-editor-${activeItem || 'unknown'}`}
          fontSize={14}
          lineHeight={19}
          showGutter
          highlightActiveLine
          showPrintMargin={false}
          style={{ width: '100%', height: '35rem' }}
          value={rawData}
          onChange={(value) => {
            setRawData(value);
            try {
              JSON.parse(value);
              setRawErrorMessage('');
            } catch (e: any) {
              setRawErrorMessage(e.message);
            }
          }}
          enableBasicAutocompletion
          enableLiveAutocompletion
          enableSnippets
          // showLineNumbers
          tabSize={2}
        />
        <p style={{ color: '#EE1111', margin: '0rem' }}>
          {rawErrorMessage}
        </p>
      </Modal>
      <Drawer isExpanded={isDrawerExpanded} position="right">
        <DrawerContent panelContent={panelContent}>
          <DrawerContentBody>
            <div style={{
              minWidth: '300px',
              flex: 1,
              height: '100%',
              width: isDrawerExpanded ? 'calc(100% - 28rem)' : '100%',
              transition: 'width 0.2s ease-in-out',
            }}
            >
              <Title
                headingLevel="h1"
                size="2xl"
                style={{ marginBottom: '2rem', color: isDark ? '#EEE' : '#111' }}
              >
                Configurator
              </Title>
              <div style={{
                position: 'fixed',
                right: isDrawerExpanded ? '29.5rem' : '1.5rem',
                transition: 'right 0.2s ease-in-out',
                top: '0rem',
                zIndex: '10000',
              }}
              >
                <Hamburger
                  toggled={isDrawerExpanded}
                  toggle={toggleDrawer}
                  size={30}
                  color={isDark ? '#EEE' : undefined}
                />
              </div>
              <div style={{
                width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center', color: isDark ? '#EEE' : '#111',
              }}
              >
                {activeItem && activeItemProcess
                  ? (
                    <div style={{ minWidth: '350px', width: '50vw', maxWidth: '600px' }} key={`${activeItem}div`}>
                      <Title headingLevel="h2" size="lg" style={{ marginBottom: '1rem', padding: '0.5rem' }}>
                        {`${activeItemProcess.split('.').slice(3).join('.')} ${prettify(activeItem.split('/').splice(-1)[0])}`}
                      </Title>
                      <Button
                        variant="tertiary"
                        onClick={() => setIsRawModalOpen(true)}
                        style={{ marginBottom: '1rem' }}
                      >
                        Edit Raw
                      </Button>
                      <FormGenerator
                        inputSchema={schemas[activeItemProcess][activeItem]}
                        key={`${activeItem}-form-${formSubmissionCount}`}
                        onSubmit={(data: any) => handleSubmit(data.values.config)}
                        values={formData[activeItemProcess][activeItem]}
                        intKey={formSubmissionCount}
                      />
                      <div style={{ marginBottom: '2rem' }} />
                    </div>
                  )
                  : <div style={{ minWidth: '350px', width: '50vw', maxWidth: '600px' }} key="emptyDiv" />}

              </div>
            </div>
          </DrawerContentBody>
        </DrawerContent>
      </Drawer>
    </div>
  );
};

export default Configurator;
