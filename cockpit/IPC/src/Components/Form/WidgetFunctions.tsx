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
  return name;
}

export function removeSlotOrg(name: string) {
  // com.skaginn3x.config.etc.tfc.operation_mode.def.state_machine
  // return etc.tfc.operation_mode.def.state_machine
  if (name.split('.').length > 4) {
    return name.split('.').slice(2).join('.');
  }
  return name;
}

/**
   * Update form data and dbus property config
   * @param name string
   * @param newData Object
   * @returns void
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
  if (!newData || !name) return;

  // Unwrap config object if it exists (for nested schemas)
  if (newData.config) {
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

export async function fetchDataFromDBus(name: string, iface: string, path: string, property: string) {
  console.log('name = ', name);
  console.log('iface = ', iface);
  const dbus = window.cockpit.dbus(name);
  console.log('dbus = ', dbus);
  const OBJproxy = dbus.proxy(iface, `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}/${path}`);
  await OBJproxy.wait();
  console.log('proxy = ', OBJproxy);

  const { data } = OBJproxy;
  let parsedData = JSON.parse(data[property][0].replace('\\"', '"'));
  const parsedSchema = JSON.parse(data[property][1].replace('\\"', '"'));

  if (!Object.keys(parsedData).includes(property)) {
    parsedData = { config: parsedData };
  }

  return { parsedData, parsedSchema };
}
