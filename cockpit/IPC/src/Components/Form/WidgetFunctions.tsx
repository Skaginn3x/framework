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
