import './App.css';

import { BrowserRouter as Router, Route, Routes } from 'react-router-dom';
import "@patternfly/react-core/dist/styles/base.css";
import { Home } from './Home';



function App() {

  return (
    <div className="App">

      <Router>
        <div style={{ display: "flex", flexDirection: "column", margin: "0px 0px 0px 0px", alignItems: "center", overflowX: "hidden", overflowY: "scroll", height: "100vh" }}>
          {
            <Routes>
              <Route path="*" element={<Home />} />
              <Route path="/" element={<Home />} />
            </Routes>

          }
        </div>
      </Router>
    </div>
  );
}

export default App;

