import { JsonValue } from "common/jsonTypes";
import { Collapse } from "components/Collapse";
import { FormCheckboxInput, FormTextInput } from "components/Form";
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
        <FormTextInput
          type="text"
          label="Network Name"
          value={config.name ?? "WiFi Name"}
          setValue={(value: JsonValue) => {
            updateConfigField("name", value);
          }}
        />
        <FormTextInput
          type="password"
          label="Password"
          value={config.password ?? ""}
          setValue={(value: JsonValue) => {
            updateConfigField("password", value);
          }}
        />

        <div>
          <FormCheckboxInput
            type="radio"
            label="DHCP"
            checked={useDHCP}
            setValue={(checked: JsonValue) => {
              updateConfigField("useDHCP", checked);
            }}
          />

          <FormCheckboxInput
            type="radio"
            label="Static"
            checked={!useDHCP}
            setValue={(checked: JsonValue) => {
              updateConfigField("useDHCP", !checked);
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
  return (
    <div>
      <FormTextInput
        type="text"
        label="IP Address"
        value={config.ipAddress ?? "192.168.0.2"}
        setValue={(value: JsonValue) => {
          updateConfigField("ipAddress", String(value));
        }}
      />

      <FormTextInput
        type="text"
        label="Netmask"
        value={config.netmask ?? "255.255.255.0"}
        setValue={(value: JsonValue) => {
          updateConfigField("netmask", String(value));
        }}
      />

      <FormTextInput
        type="text"
        label="Gateway"
        value={config.gateway ?? "192.168.0.1"}
        setValue={(value: JsonValue) => {
          updateConfigField("gateway", String(value));
        }}
      />

      <FormTextInput
        type="text"
        label="DNS Server"
        value={config.dnsServer ?? "8.8.8.8"}
        setValue={(value: JsonValue) => {
          updateConfigField("dnsServer", String(value));
        }}
      />
    </div>
  );
}
