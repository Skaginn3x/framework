import './App.css';


import { BrowserRouter as Router, Route, Routes } from 'react-router-dom';
import "@patternfly/react-core/dist/styles/base.css";
import { Button, Modal, ModalVariant, TextInput } from '@patternfly/react-core';
import { DropDownTest } from './Components/Dropdown/DropDownTest';
import React, { useEffect, useState } from 'react';



function App() {

  const externalScript = "../base1/cockpit.js"
  const state = useExternalScript(externalScript);
  console.log(state)
  // create dbus connection
  return (
    <div className="App">

      <Router>
        <div style={{ display: "flex", flexDirection: "column", margin: "0px 0px 0px 0px", alignItems: "center", overflow: "hidden", height: "100vh" }}>
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


const Home = () => {
  const [value, setValue] = React.useState('');
  const [isModalOpen, setIsModalOpen] = React.useState(false);

  const handleModalToggle = () => {
    setIsModalOpen(!isModalOpen);
  };


  const runCockpitCommand = () => {
    console.log("Running command")
    const commandArray = value.split(' '); // splits the string into an array by spaces
    // if rm is in the command, return
    if (commandArray.filter((command: string) => command === "rm").length > 0) {
      console.log("rm is not allowed")
      return
    }
    // @ts-ignore
    cockpit.spawn(commandArray, { err: "message" }).then((data: any) => {
      console.log(data)
    })
  }


  const sendDbusMessage = () => {
    console.log("Sending dbus message")

    // @ts-ignore
    const dbus = cockpit.dbus("org.freedesktop.Notifications", { bus: "session" });
    let dbusObjectProxy = dbus.proxy("org.freedesktop.Notifications", "/org/freedesktop/Notifications");

    dbusObjectProxy.wait(() => {
      const icon = '';
      const summary = 'DBUS React Test';
      const body = value;
      const actions: any = [];
      const hints: any = {};
      const timeout = -1; // -1 to use default timeout

      // The Notify method is from the org.freedesktop.Notifications interface
      dbusObjectProxy.Notify("React Cockpit App", 0, icon, summary, body, actions, hints, timeout)
        .done((response: string) => {
          console.log("Notification sent successfully: ", response);
        })
        .fail((error: string) => {
          console.error("Failed to send notification: ", error);
        });
    });
  }


  return (
    <>
      <h2> Skaginn3x Taipan login smthn </h2>
      <div style={{ width: "30%" }}></div>
      <TextInput value={value} type="text" onChange={value => setValue(value)} aria-label="test-input" style={{ width: "20vw", margin: "1rem" }} />
      <Button variant="primary" onClick={runCockpitCommand} style={{ marginBottom: "1rem" }}>
        Run Command
      </Button>
      <Button variant="primary" onClick={sendDbusMessage} style={{ marginBottom: "1rem" }}>
        Send DBUS Message
      </Button>
      <Button variant="primary" onClick={handleModalToggle} style={{ marginBottom: "1rem" }}>
        Show basic modal
      </Button>
      <Modal
        title="Basic modal"
        isOpen={isModalOpen}
        onClose={handleModalToggle}
        variant={ModalVariant.medium}
        actions={[
          <Button key="confirm" variant="primary" onClick={handleModalToggle}>
            Confirm
          </Button>,
          <Button key="cancel" variant="link" onClick={handleModalToggle}>
            Cancel
          </Button>
        ]}
      >
        Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore
        magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo
        consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla
        pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est
        laborum.
      </Modal >
      <DropDownTest />

    </>
  )
}



const useExternalScript = (url: string) => {
  let [state, setState] = useState(url ? "loading" : "idle");

  useEffect(() => {
    if (!url) {
      setState("idle");
      return;
    }

    let script = document.querySelector(`script[src="${url}"]`) as HTMLScriptElement;

    const handleScript = (e: { type: string; }) => {
      setState(e.type === "load" ? "ready" : "error");
    };

    if (!script) {
      script = document.createElement("script") as HTMLScriptElement;
      script.type = "application/javascript";
      script.src = url;
      script.async = true;
      document.body.appendChild(script);
      script.addEventListener("load", handleScript);
      script.addEventListener("error", handleScript);
    }

    script.addEventListener("load", handleScript);
    script.addEventListener("error", handleScript);

    return () => {
      script.removeEventListener("load", handleScript);
      script.removeEventListener("error", handleScript);
    };
  }, [url]);

  return state;
};