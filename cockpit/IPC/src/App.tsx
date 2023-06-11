import React from 'react';
import './App.css';

import {
  BrowserRouter as Router, Route, Routes, useLocation,
} from 'react-router-dom';
import '@patternfly/react-core/dist/styles/base.css';
import AlertProvider from './Components/Alert/AlertProvider';
import Alerts from './Components/Alert/Alerts';
import Configurator from './views/Configurator';
import IPC from './views/IPC';
import NotFoundPage from './views/NotFoundPage';
import ListDBUS from './views/ListDBUS';

function RouterElem() {
  console.log('RouterElem');
  const location = useLocation();
  const query = new URLSearchParams(location.search);
  const id = query.get('service')?.replace('.html', '') || 'default';
  switch (id) {
    case 'connect': return <IPC />;
    case 'configure': return <Configurator />;
    case 'list': return <ListDBUS />;
    default: return <NotFoundPage />;
  }
}

function App() {
  console.log('App Loaded!');
  return (
    <AlertProvider>
      <div className="App">
        <Alerts />
        <Router basename="/cockpit/@localhost/IPC">
          <div style={{
            display: 'flex',
            flexDirection: 'column',
            margin: '0px 0px 0px 0px',
            alignItems: 'center',
            overflowX: 'hidden',
            overflowY: 'scroll',
          }}
          >
            <Routes>
              <Route path="/index.html" element={<RouterElem />} />
              <Route path="*" element={<NotFoundPage />} />
            </Routes>
          </div>
        </Router>
      </div>
    </AlertProvider>
  );
}

export default App;
