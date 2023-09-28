import { immerable } from "immer";

export interface IWiFiSettingsConfig {
  apSettings: IAPSettingsConfig;
  clientSettings: IClientSettingsConfig;
}

export class WiFiSettingsConfig implements IWiFiSettingsConfig {
  apSettings: APSettingsConfig;
  clientSettings: ClientSettingsConfig;

  constructor() {
    this.apSettings = new APSettingsConfig();
    this.clientSettings = new ClientSettingsConfig();
  }
}

export interface IAPSettingsConfig {
  enabled: boolean;
  name: string;
  password: string;
  channel: number;
  hidden: boolean;
}

export class APSettingsConfig implements IAPSettingsConfig {
  enabled: boolean;
  name: string;
  password: string;
  channel: number;
  hidden: boolean;

  constructor() {
    this.enabled = false;
    this.name = "";
    this.password = "";
    this.channel = 1;
    this.hidden = false;
  }
}

export interface IClientSettingsConfig {
  enabled: boolean;
  settings: ISingleClientConfig[]; // expected length of 3
}

export class ClientSettingsConfig implements IClientSettingsConfig {
  enabled: boolean;
  settings: SingleClientConfig[]; // expected length of 3

  constructor() {
    this.enabled = false;
    this.settings = [
      new SingleClientConfig(),
      new SingleClientConfig(),
      new SingleClientConfig(),
    ];
  }
}

export interface ISingleClientConfig {
  name: string;
  password: string;
  useDHCP: boolean;
  ipAddress: string;
  netmask: string;
  gateway: string;
  dnsServer: string;
}

export class SingleClientConfig implements ISingleClientConfig {
  [immerable] = true;
  name: string;
  password: string;
  useDHCP: boolean;
  ipAddress: string;
  netmask: string;
  gateway: string;
  dnsServer: string;

  constructor() {
    this.name = "";
    this.password = "";
    this.useDHCP = true;
    this.ipAddress = "";
    this.netmask = "";
    this.gateway = "";
    this.dnsServer = "";
  }
}
