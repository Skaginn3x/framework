/**
 * @param problem The problem of the error
 * @param name The name of the error
 * @param message The message of the error
 */
export default interface ErrorType {
  problem: string | null | undefined,
  name: string,
  message: string,
}

/**
 * @param name The name of the signal
 * @param description The description of the signal
 * @param type The type of the signal
 * @param created_by The user who created the signal
 * @param created_at The time the signal was created
 * @param last_registered The time the signal was last registered
 */
export interface SignalType {
  name: string,
  description?: string,
  type: 'unknown' | 'bool' | 'int64_t' | 'uint64_t' | 'double' | 'string' | 'json',
  created_by: string,
  created_at: string,
  last_registered: string,
}

/**
 * @param name - The name of the slot
 * @param description The description of the slot
 * @param type The type of the slot (bool, int64_t, uint64_t, double, string, json)
 * @param created_by The user who created the slot
 * @param created_at The time the slot was created
 * @param last_registered The time the slot was last registered
 * @param connected_to The signal the slot is connected to
 * @param modified_by The user who last modified the slot
 */
export interface SlotType {
  name: string,
  description?: string,
  type: string,
  created_by: string,
  created_at: string,
  last_registered: string,
  last_modified: string,
  connected_to: string,
  modified_by: string,
}

/**
 * @param x The x coordinate of the point
 * @param y The y coordinate of the point
 */
export type Point = {
  x: number;
  y: number;
};

/**
 * @param id The id of the node
 * @param text The text of the node
 * @param children The children of the node
 * @param data The data of the node
 */
export type TreeNodeProps = {
  id: string;
  text: string;
  children?: TreeNodeProps[];
  data?: any;
};

export interface ConnectionType {
  [key: string]: string[];
}

export const CockpitDBUSTypes = {
  y: 'Byte',
  b: 'Boolean',
  n: 'Int16',
  q: 'UInt16',
  i: 'Int32',
  u: 'UInt32',
  x: 'Int64',
  t: 'UInt64',
  d: 'Double',
  s: 'String',
  o: 'Object Path',
  g: 'Signature',
  ay: 'Array of Bytes',
};
