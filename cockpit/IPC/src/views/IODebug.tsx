/* eslint-disable react/no-unknown-property */
/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable react/jsx-no-undef */
/* eslint-disable react/no-unstable-nested-components */
import React, {
  useEffect, useRef, useState,
} from 'react';
import {
  Title,
  Divider,
} from '@patternfly/react-core';
import './IODebug.css';
import { DarkModeType } from 'src/App';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import loadExternalScript from 'src/Components/Interface/ScriptLoader';

interface SubType {
  [key: string]: { data: any };
}

// eslint-disable-next-line react/function-component-definition
const IODebug: React.FC<DarkModeType> = ({ isDark }) => {
  const [processDBUS, setProcessDBUS] = useState<any>();
  const [subscriptions, setSubscriptions] = useState<SubType>({});
  const subscriptionsRef = useRef(subscriptions);

  useEffect(() => {
    loadExternalScript(() => {
      setProcessDBUS(window.cockpit.dbus(null));
    });
  }, []);

  useEffect(() => {
    if (!processDBUS) return;

    const match = {
      path_namespace: `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}`,
    };
    processDBUS.watch(match);

    const notifyHandler = (data: any) => {
      const removePath = data.detail[Object.keys(data.detail)[0]];
      const ifaceName = Object.keys(removePath)[0];
      if (subscriptionsRef.current[ifaceName]) {
        setSubscriptions((prevState: any) => {
          const newState = { ...prevState };
          newState[ifaceName].data = removePath[ifaceName].Value;
          return newState;
        });
      }
    };

    processDBUS.addEventListener('notify', notifyHandler);

    // eslint-disable-next-line consistent-return
    return () => {
      processDBUS.removeEventListener('notify', notifyHandler);
    };
  }, [processDBUS]);

  useEffect(() => {
    subscriptionsRef.current = subscriptions;
  }, [subscriptions]);

  const subscribe = (interfaceName: string) => {
    console.log('Subscribed to: ', interfaceName);
    setSubscriptions((prevState) => ({
      ...prevState,
      [interfaceName]: { data: null },
    }));
  };

  const unsubscribe = (interfaceName: string) => {
    console.log('Unsubscribed to: ', interfaceName);
    setSubscriptions((prevState) => {
      const newState = { ...prevState };
      delete newState[interfaceName];
      return newState;
    });
  };

  // subscribe after 4 sec then never unsubscribe
  useEffect(() => {
    setTimeout(() => {
      console.log('SUBBINBING');
      subscribe('com.skaginn3x.signal_source.def.bool.square_wave_2000ms');
      subscribe('com.skaginn3x.signal_source.def.bool.square_wave_200ms');
      subscribe('com.skaginn3x.signal_source.def.bool.square_wave_500ms');
      subscribe('com.skaginn3x.signal_source.def.bool.square_wave_400ms');
      unsubscribe('com.skaginn3x.signal_source.def.bool.square_wave_400ms');
    }, 4000);
  }, []);

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
        {/* <ToolBar
          filteredItems={dbusInterfaces}
          filterConfigs={Configs}
          activeAttributeMenu={activeAttributeMenu}
          setActiveAttributeMenu={setActiveAttributeMenu}
          refs={filterRefs}
        /> */}
        <div style={{
          width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center',
        }}
        >
          <Divider />
          { processDBUS && (
            <div style={{
              width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center',
            }}
            >
              <Title headingLevel="h1" size="2xl" style={{ marginBottom: '1rem', color: isDark ? '#EEE' : '#111' }}>
                Values
              </Title>
              { subscriptions && Object.keys(subscriptions).map((key) => (
                subscriptions[key].data !== null
                  ? (
                    <div
                      key={key}
                      style={{
                        display: 'flex', flexDirection: 'row', alignItems: 'center', color: isDark ? '#EEE' : '#222',
                      }}
                    >
                      <p style={{ marginRight: '1rem' }}>{key}</p>
                      <p>{subscriptions[key].data.toString()}</p>
                    </div>
                  )
                  : null)) }
            </div>
          ) }
        </div>

      </div>
    </div>
  );
};

export default IODebug;
