import './App.css';

import { BrowserRouter as Router, Route, Routes, useLocation } from 'react-router-dom';
import "@patternfly/react-core/dist/styles/base.css";
import AlertProvider from './Components/Alert/AlertProvider';
import Alerts from './Components/Alert/Alerts';
import { Subscribe } from './views/Subscribe';
import { IPC } from './views/IPC';
import NotFoundPage from './views/NotFoundPage';


function App() {

  return (
    <AlertProvider>
      <div className="App">
        <Alerts />
        <Router basename='/cockpit/@localhost/IPC'>
          <div style={{ display: "flex", flexDirection: "column", margin: "0px 0px 0px 0px", alignItems: "center", overflowX: "hidden", overflowY: "scroll" }}>
            {
              <Routes>
                <Route path="/index.html" element={<RouterElem />} />
                <Route path="*" element={<NotFoundPage />} />
              </Routes>
            }
          </div>
        </Router>
      </div>
    </AlertProvider>
  );
}

const RouterElem = () => {

  const location = useLocation();
  const query = new URLSearchParams(location.search);
  const id = query.get('service')?.replace(".html", "") || "default";
  switch (id) {
    case "connect": return <IPC />;
    case "configure": return <NotFoundPage />;
    case "list": return <Subscribe />;
    default: return <NotFoundPage />;
  }
}

export default App;

