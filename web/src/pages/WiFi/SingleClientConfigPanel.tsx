import { Collapse } from "components/Collapse";
import { FormCheck, FormInput } from "components/Form";
import { produce } from "immer";
import { type JSX } from "preact";
import { useId } from "preact/hooks";
import { type ISingleClientConfig } from "./WiFiSettingsConfig";

interface SingleClientConfigPanelProps {
  config: ISingleClientConfig;
  setConfig: (config: ISingleClientConfig) => void;
}

export function SingleClientConfigPanel({
  config,
  setConfig,
}: SingleClientConfigPanelProps): JSX.Element {
  const id = useId();

  const useDHCP = config.useDHCP ?? true;

  function updateConfigField(field: string, value): void {
    setConfig(
      produce(config, (draft) => {
        draft[field] = value;
      }),
    );
  }

  return (
    <>
      <div className="vstack gap-2">
        <FormInput
          id={`${id}-name`}
          type="text"
          label="Name"
          placeholder="Network Name"
          value={config.name ?? "WiFi Name"}
          onchange={(event) => {
            updateConfigField("name", event.currentTarget.value);
          }}
        />
        <FormInput
          id={`${id}-password`}
          type="password"
          label="Password"
          placeholder="WiFi Password"
          value={config.password ?? ""}
          onchange={(event) => {
            updateConfigField("password", event.currentTarget.value);
          }}
        />

        <div>
          <FormCheck
            id={`${id}-dhcp`}
            name={id}
            type="radio"
            label="DHCP"
            checked={useDHCP}
            onchange={(event) => {
              updateConfigField("useDHCP", event.currentTarget.checked);
            }}
          />

          <FormCheck
            id={`${id}-static`}
            name={id}
            type="radio"
            label="Static"
            checked={!useDHCP}
            onchange={(event) => {
              updateConfigField("useDHCP", !event.currentTarget.checked);
            }}
          />
        </div>
        <Collapse id={`${id}-collapse`} expanded={!useDHCP}>
          <div>
            <StaticIPConfig
              config={config}
              updateConfigField={updateConfigField}
            />
          </div>
        </Collapse>
      </div>
    </>
  );
}

interface StaticIPConfigProps {
  config: ISingleClientConfig;
  updateConfigField: (field: string, value: string) => void;
}

function StaticIPConfig({
  config,
  updateConfigField,
}: StaticIPConfigProps): JSX.Element {
  const id = useId();
  return (
    <div>
      <FormInput
        id={`${id}-ipAddress`}
        type="text"
        label="IP Address"
        placeholder="IP Address"
        value={config.ipAddress ?? "192.168.0.2"}
        onchange={(event) => {
          updateConfigField("ipAddress", event.currentTarget.value);
        }}
      />

      <FormInput
        id={`${id}-netmask`}
        type="text"
        label="Netmask"
        placeholder="Netmask"
        value={config.netmask ?? "255.255.255.0"}
        onchange={(event) => {
          updateConfigField("netmask", event.currentTarget.value);
        }}
      />

      <FormInput
        id={`${id}-gateway`}
        type="text"
        label="Gateway"
        placeholder="Gateway"
        value={config.gateway ?? "192.168.0.1"}
        onchange={(event) => {
          updateConfigField("gateway", event.currentTarget.value);
        }}
      />

      <FormInput
        id={`${id}-dnsServer`}
        type="text"
        label="DNS Server"
        placeholder="DNS Server"
        value={config.dnsServer ?? "8.8.8.8"}
        onchange={(event) => {
          updateConfigField("dnsServer", event.currentTarget.value);
        }}
      />
    </div>
  );
}
