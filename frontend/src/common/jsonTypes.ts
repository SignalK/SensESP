export type JsonValue =
  | string
  | number
  | boolean
  | null
  | JsonArray
  | JsonObject;

export type JsonArray = Array<JsonValue>;

// eslint-disable-next-line @typescript-eslint/no-empty-object-type
export interface JsonObject extends Record<string, JsonValue> {}
