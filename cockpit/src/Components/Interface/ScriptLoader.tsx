import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import cockpit from 'cockpit';

// class DBusInterface {
//     private dbus: any;
//     private proxy: any;
//     private name: string;
//
//     constructor(dbus: any, name: string) {
//         this.name = name;
//         this.dbus = dbus;
//         this.proxy = this.dbus.proxy(name);
//     }
//
//     public getProxy() {
//         return this.proxy;
//     }
//
//     public getName() {
//         return this.name;
//     }
//
// }
//
// enum BusType {
//     SYSTEM = 'system',
//     SESSION = 'session'
// }
//
// class DBus {
//     private dbus: any;
//
//     constructor(bus: BusType = BusType.SYSTEM) {
//         this.dbus = cockpit.dbus(null, );
//     }
//
//
// }


type ScriptLoadCallback = (allNames: string[]) => void;

export const loadExternalScript = (callback: ScriptLoadCallback) => {
  const dbus = cockpit.dbus('org.freedesktop.DBus');
  const proxy = dbus.proxy();
  proxy.wait().then(() => {
    proxy.call('ListNames').then((Allnames: any[]) => {
      callback(Allnames[0]);
    });
  });
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
