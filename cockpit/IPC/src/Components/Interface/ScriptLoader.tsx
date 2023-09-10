type ScriptLoadCallback = (allNames: string[]) => void;

export const loadExternalScript = (callback: ScriptLoadCallback) => {
  const externalScript = '../base1/cockpit.js';
  let script = document.querySelector(`script[src="${externalScript}"]`) as HTMLScriptElement;

  const handleScriptLoad = () => {
    const dbus = window.cockpit.dbus('org.freedesktop.DBus');
    const proxy = dbus.proxy();
    proxy.wait().then(() => {
      proxy.call('ListNames').then((Allnames: any[]) => {
        callback(Allnames[0]);
      });
    });
  };

  if (!script) {
    script = document.createElement('script');
    script.src = externalScript;
    script.async = true;
    script.addEventListener('load', handleScriptLoad);
    script.addEventListener('error', (e) => { console.error('Error loading script', e); });
    document.body.appendChild(script);
  } else {
    handleScriptLoad();
    script.addEventListener('error', (e) => { console.error('Error loading script', e); });
  }
};

export default loadExternalScript;
