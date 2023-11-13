import { AlertVariant } from '@patternfly/react-core';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';

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

  // eslint-disable-next-line no-param-reassign
  setFormData((prevState: any) => ({
    ...prevState,
    [name]: newData,
  }));

  // set dbus property config to data
  console.log('stringdata: (ss) ', [JSON.stringify(newData), '']);
  const newdbus = window.cockpit.dbus(name, { superuser: 'try' });
  const propProxy = newdbus.proxy(iface, `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/${path}`);

  propProxy.wait().then(() => {
    const stringdata = window.cockpit.variant('(ss)', [JSON.stringify(newData), '']);
    newdbus.call(`/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/${path}`, 'org.freedesktop.DBus.Properties', 'Set', [
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
 * Fetches jsonschema and data from dbus
 * @param name Process Name
 * @param iface Interface Name
 * @param path  DBUS Path
 * @param property  DBUS Property Name
 * @returns Data and Schema { parsedData: any, parsedSchema: any}
 */
export async function fetchDataFromDBus(name: string, iface: string, path: string, property: string) {
  if (!name) return {};
  if (!iface) return {};
  if (!path) return {};
  if (!property) return {};
  const dbus = window.cockpit.dbus(name);
  const OBJproxy = dbus.proxy(iface, `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/${path}`);
  await OBJproxy.wait();
  let parsedData;
  let parsedSchema;

  const { data } = OBJproxy;
  try {
    parsedData = JSON.parse(data[property][0].replace('\\"', '"'));
    parsedSchema = JSON.parse(data[property][1].replace('\\"', '"'));
  } catch (error) {
    console.error('Error parsing data:', error);
    return {};
  }

  if ((parsedData === null && data[property][0].length > 3) || !Object.keys(parsedData).includes(property)) {
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
