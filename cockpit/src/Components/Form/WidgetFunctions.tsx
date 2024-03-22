import { AlertVariant } from '@patternfly/react-core';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';
import { loadExternalScript } from '../Interface/ScriptLoader';
/* eslint-disable import/prefer-default-export */
function determineDefaultValue(nextKey: any): any {
  return typeof nextKey === 'number' ? [] : {};
}

/**
   * Uses StoreKeys to navigate store and returns appropriate data.
   * @param obj Store
   * @param keys StoreKeys (as List)
   * @returns CurentStore (as JSON)
   */
export function getNestedValue(obj: any, keys: Array<any>): any {
  let currentValue = obj;

  keys.forEach((key, i) => {
    if (currentValue === null) {
      currentValue = determineDefaultValue(keys[i + 1]);
    }
    if (currentValue[key] === undefined) {
      currentValue[key] = i < keys.length - 1 ? determineDefaultValue(keys[i + 1]) : undefined;
    }
    currentValue = currentValue[key];
  });

  return currentValue;
}

/**
   * Remove org from dbus name
   * @param name  string
   * @returns string
   */
export function removeOrg(name: string) {
  // com.skaginn3x.config.etc.tfc.operation_mode.def.state_machine
  // return etc.tfc.operation_mode.def.state_machine
  if (name.split('.').length > 4) {
    return name.split('.').slice(3).join('.');
  }
  if (name.split('.').length >= 3) {
    return name.split('.').slice(2).join('.');
  }
  return name;
}

/**
   * Remove org from dbus slot interface
   * @param name  string
   * @returns string
   */
export function removeSlotOrg(name: string) {
  // com.skaginn3x.config.etc.tfc.operation_mode.def.state_machine
  // return etc.tfc.operation_mode.def.state_machine
  if (name.split('.').length > 4) {
    return name.split('.').slice(2).join('.');
  }
  return name;
}

/**
 * Update form data and dbus property for jsonschema
 * @param name  Process Name
 * @param iface Interface Name
 * @param path  Dbus Path
 * @param property Property Name
 * @param newData  Data to be updated
 * @param setFormData React Hook to set form data
 * @param addAlert Alert context hook
 * @returns Nothing
 */
export const updateFormData = (
  name: string | undefined,
  iface: string,
  path: string,
  property: string,
  newData: any,
  setFormData:
  React.Dispatch<any>,
  addAlert: any,
) => {
  if (newData === undefined || !name) return;

  // Unwrap config object if it exists (for nested schemas)
  if (newData && newData.config) {
    // eslint-disable-next-line no-param-reassign
    newData = newData.config;
  }
  setFormData((prevState: any) => ({
    ...prevState,
    [name]: { config: newData },
  }));

  // set dbus property config to data
  console.log('stringdata: (s) ', JSON.stringify(newData));
  const newdbus = window.cockpit.dbus(name, { superuser: 'try' });
  const propProxy = newdbus.proxy(iface, path);

  propProxy.wait().then(() => {
    const stringdata = window.cockpit.variant('s', JSON.stringify(newData));
    newdbus.call(path, 'org.freedesktop.DBus.Properties', 'Set', [
      iface, // The interface name
      property, // The property name
      stringdata, // The new value
    ]).then(() => {
      addAlert('Property updated successfully', AlertVariant.success);
    }).catch((error: any) => {
      addAlert('Failed to update property', AlertVariant.danger);
      console.error('Failed to update property:', error);
    });
  }).catch((error: any) => {
    addAlert('Failed to get DBUS proxy', AlertVariant.danger);
    console.error('Failed to get DBUS proxy:', error);
  });
};

/**
 *  Gets Schemas and Data from proxies
 * @param proxies Cockpit Proxies object
 * @param paths Array of strings
 * @returns {schemas: {}, data: {}}
 */
export function getDataFromProxies(proxies: any, paths: string[]) {
  const allData = { schemas: {}, data: {} } as any;
  paths.forEach((path) => {
    try {
      let parsedData = JSON.parse(proxies[path].Value.replace('\\"', '"'));
      const parsedSchema = JSON.parse(proxies[path].Schema.replace('\\"', '"'));
      if ((parsedData === null && proxies[path].Value.length > 3) || !Object.keys(parsedData).includes('config')) {
        parsedData = { config: parsedData };
      }
      allData.schemas[path] = parsedSchema;
      allData.data[path] = parsedData;
    } catch (error) {
      console.error('Error parsing data:', error, proxies[path].Value, proxies[path].Schema);
    }
  });
  return allData;
}

/**
 *  Sorts strings with numbers
 * @param items List of strings to sort
 * @returns Sorted List of strings
 */
export function sortItems(items:string[]) {
  return items.sort((a, b) => {
    const regex = /([^\d]+)(\d+)$/;
    const matchA = regex.exec(a);
    const matchB = regex.exec(b);

    if (matchA && matchB) {
      // Compare non-numeric parts
      if (matchA[1] < matchB[1]) return -1;
      if (matchA[1] > matchB[1]) return 1;

      // Compare numeric parts
      return parseInt(matchA[2], 10) - parseInt(matchB[2], 10);
    }

    // Fallback to regular string comparison if one or both don't match the pattern
    return a.localeCompare(b);
  });
}

interface ProxyData {
  schemas: any;
  data: any;
}
interface NameDataPair {
  name: string;
  data: ProxyData;
}
/**
 * Gets data from dbus
 * @returns Promise of NameDataPair[]
 */
export function getData(): Promise<NameDataPair[]> {
  return new Promise((resolve, reject) => {
    loadExternalScript((allNames: string[]) => {
      const filteredNames = allNames.filter((name) => (name.includes(`${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.config`)
        || name.includes(`${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.tfc`))
        && !name.includes('ipc_ruler'));

      const sortedNames = sortItems(filteredNames);
      const proxyPromises: Promise<NameDataPair>[] = sortedNames.map((name) => new Promise((resolveProxy) => {
        const dbus = window.cockpit.dbus(name);
        const proxies = dbus.proxies(`${TFC_DBUS_DOMAIN}.${TFC_DBUS_ORGANIZATION}.Config`);

        setTimeout(() => {
          const paths = Object.keys(proxies);
          const allProcessData: ProxyData = getDataFromProxies(proxies, paths);
          resolveProxy({ name, data: allProcessData });
        }, 100);
      }));

      Promise.all(proxyPromises).then(resolve).catch(reject);
    });
  });
}

/**
 * Fetches jsonschema and data from dbus
 * @param name Process Name
 * @param iface Interface Name
 * @param path  DBUS Path
 * @param property  DBUS Property Name
 * @returns Data and Schema { parsedData: any, parsedSchema: any}
 */
export async function fetchDataFromDBus(name: string, iface: string, path: string) {
  if (!name) return {};
  if (!iface) return {};
  if (!path) return {};
  const dbus = window.cockpit.dbus(name);
  const OBJproxy = dbus.proxy(iface, path);
  await OBJproxy.wait();
  let parsedData;
  let parsedSchema;

  const { data } = OBJproxy;
  try {
    parsedData = JSON.parse(data.Value.replace('\\"', '"'));
    parsedSchema = JSON.parse(data.Schema.replace('\\"', '"'));
  } catch (error) {
    console.error('Error parsing data:', error, data.Value, data.Schema);
    return {};
  }

  if ((parsedData === null && data.Value.length > 3) || !Object.keys(parsedData).includes('config')) {
    parsedData = { config: parsedData };
  }

  return { parsedData, parsedSchema };
}

/**
   * Finds all internal null values and sets them to actual null.
   * This is need because UI-schema handles null weirdly
   * @param data Configuration values
   * @returns updated Configuration values
   */
export function handleNullValue(data: any): any {
  if (Array.isArray(data)) {
    return data.filter((item) => item != null && item !== undefined).map(handleNullValue);
  }
  if (typeof data === 'object' && data !== null) {
    const newObj: any = {};
    // eslint-disable-next-line no-restricted-syntax, guard-for-in
    for (const key in data) {
      const childObj = handleNullValue(data[key]);

      if (key === 'internal_null_value_do_not_use' && childObj === null) {
        return null; // Set the entire object to null if this key is present and its value is null
      }

      if (childObj !== undefined) {
        newObj[key] = childObj;
      }
    }
    return newObj;
  }
  return data;
}
