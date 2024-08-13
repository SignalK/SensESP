import { APP_CONFIG } from "config";
import { type JsonObject } from "./jsonTypes";

/**
 * Represents the configuration data.
 */
export interface ConfigData {
  /**
   * The configuration data.
   */
  config: JsonObject;
  /**
   * The schema of the configuration data.
   */
  schema: JsonObject;
  /**
   * The description of the configuration data.
   */
  description: string;
}

/**
 * Fetches the configuration data from the server.
 * @param path - The path of the configuration data.
 * @returns A promise that resolves to the configuration data.
 * @throws An error if there is an issue fetching the configuration data.
 */
export async function fetchConfigData(path: string): Promise<ConfigData> {
  try {
    const response = await fetch(APP_CONFIG.config_path + path);
    if (!response.ok) {
      throw new Error(`HTTP Error ${response.status} ${response.statusText}`);
    }
    return await response.json();
  } catch (e) {
    throw new Error("Error getting config data from the device.");
  }
}

/**
 * Saves the configuration data to the server.
 * @param path - The path of the configuration data.
 * @param string - The configuration data to be saved.
 * @throws An error if there is an issue saving the configuration data.
 */
export async function saveConfigData(
  path: string,
  data: string,
  errorHandler: (e: Error) => void,
  contentType: string = "application/json",
): Promise<boolean> {
  try {
    const response = await fetch(APP_CONFIG.config_path + path, {
      method: "PUT",
      headers: {
        "Content-Type": contentType,
      },
      body: data,
    });
    if (!response.ok) {
      errorHandler(Error(`HTTP Error ${response.status} ${response.statusText}`));
      return false;
    }
  } catch (e) {
    errorHandler(Error(`Error saving config data to server: ${e.message}`));
    return false;
  }
  return true;
}
