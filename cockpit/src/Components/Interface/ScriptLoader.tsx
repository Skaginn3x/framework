import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';

type ScriptLoadCallback = (allNames: string[]) => void;

export const loadExternalScript = (callback: ScriptLoadCallback) => {
  const externalScript = '../base1/cockpit.js';
  let script = document.querySelector(`script[src="${externalScript}"]`) as HTMLScriptElement;

  const handleScriptLoad = () => {
    const dbus = window.cockpit.dbus('org.freedesktop.DBus');
    const proxy = dbus.proxy();
    proxy.wait().then(() => {
      proxy.call('ListNames').then((Allnames: any[]) => {
        console.log(Allnames);
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

/**
 * Parses DBUS XML strings to extract interfaces
 * @param xml XML string
 * @returns Array of interfaces with name and value type { name: string, valueType: string}
 */
export const parseXMLInterfaces = (xml: string): { name: string, valueType: string }[] => {
  const parser = new DOMParser();
  const xmlDoc = parser.parseFromString(xml, 'text/xml');
  const interfaceElements = xmlDoc.querySelectorAll(`interface[name^="${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}."]`);
  const interfaces: { name: string, valueType: string }[] = [];
  interfaceElements.forEach((element) => {
    const name = element.getAttribute('name');
    const valueType = element.querySelector('property[name="Value"]')?.getAttribute('type') ?? 'unknown';

    if (name) {
      interfaces.push({ name, valueType });
    }
  });

  return interfaces;
};

export default loadExternalScript;
