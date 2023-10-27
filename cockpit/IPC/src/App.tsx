/* eslint-disable react/require-default-props */
import React from 'react';
import './App.css';

import {
  BrowserRouter as Router, Route, Routes, useLocation,
} from 'react-router-dom';
import '@patternfly/react-core/dist/styles/base.css';
import useMediaQuery from '@mui/material/useMediaQuery';
import { ThemeProvider, createTheme } from '@mui/material/styles';
import AlertProvider from './Components/Alert/AlertProvider';
import Alerts from './Components/Alert/Alerts';
import Configurator from './views/Configurator';
import Connections from './views/Connections';
import NotFoundPage from './views/NotFoundPage';
import ListDBUS from './views/ListDBUS';
import IODebug from './views/IODebug';

export type DarkModeType = {
  isDark: boolean;
};

// eslint-disable-next-line react/function-component-definition
const RouterElem:React.FC<DarkModeType> = ({ isDark }) => {
  const location = useLocation();
  const query = new URLSearchParams(location.search);
  const id = query.get('service')?.replace('.html', '') ?? 'default';
  switch (id) {
    case 'connect': return <Connections isDark={isDark} />;
    case 'configure': return <Configurator isDark={isDark} />;
    case 'list': return <ListDBUS isDark={isDark} />;
    case 'debug': return <IODebug isDark={isDark} />;
    default: return <NotFoundPage />;
  }
};

function App() {
  console.log('App Loaded!');
  const prefersDarkMode = useMediaQuery('(prefers-color-scheme: dark)');

  /**
   * Decides wether or not to use dark mode
   * @param val Cockpit shell:style string
   * @returns boolean
   */
  function changeDarkMode(val:string | null) {
    if (val === 'auto') {
      return prefersDarkMode;
    }
    if (val === 'light') {
      return false;
    }
    return true;
  }

  const cockpitDark = localStorage.getItem('shell:style');
  const [isDark, setIsDark] = React.useState<boolean>(changeDarkMode(cockpitDark));

  if (isDark) {
    document.getElementsByTagName('html')[0].classList.add('pf-v5-theme-dark');
  } else {
    document.getElementsByTagName('html')[0].classList.remove('pf-v5-theme-dark');
  }

  const theme = React.useMemo(
    () => createTheme({
      palette: {
        mode: isDark ? 'dark' : 'light',
      },
    }),
    [isDark],
  );

  /**
   * Listens to cockpit changing local storage shell:style to
   */
  window.addEventListener('storage', (event) => {
    if (event.key === 'shell:style') {
      setIsDark(changeDarkMode(event.newValue));
    }
  });

  return (
    <ThemeProvider theme={theme}>
      <AlertProvider>
        <div className="App">
          <Alerts />
          <Router basename="/cockpit/@localhost/IPC">
            <div style={{
              display: 'flex',
              flexDirection: 'column',
              margin: '0px 0px 0px 0px',
              alignItems: 'center',
              minHeight: '100vh',
              backgroundColor: isDark ? '#1b1d21' : 'transparent',
              overflowX: 'scroll',
              minWidth: 'fit-content',
            }}
            >
              <Routes>
                <Route path="/index.html" element={<RouterElem isDark={isDark} />} />
                <Route path="*" element={<NotFoundPage />} />
              </Routes>
            </div>
          </Router>
        </div>
      </AlertProvider>
    </ThemeProvider>
  );
}

export default App;
