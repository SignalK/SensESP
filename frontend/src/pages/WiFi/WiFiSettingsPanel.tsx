import {
  RestartRequiredContext,
  RestartRequiredContextProps,
} from "common/RestartRequiredContext";
import { fetchConfigData, saveConfigData } from "common/configAPIClient";
import { JsonValue } from "common/jsonTypes";
import { CollapseCard } from "components/Card";
import {
  FormCheckboxInput,
  FormNumberInput,
  FormTextInput,
} from "components/Form";
import { Tabs } from "components/Tab";
import { ToastMessage } from "components/ToastMessage";
import { produce } from "immer";
import { type JSX } from "preact";
import { useContext, useEffect, useId, useState } from "preact/hooks";
import { NetworkList } from "./NetworkList";
import { SingleClientConfigPanel } from "./SingleClientConfigPanel";
import {
  WiFiSettingsConfig,
  type APSettingsConfig,
  type ClientSettingsConfig,
  type IClientSettingsConfig,
  type ISingleClientConfig,
  type IWiFiSettingsConfig,
} from "./WiFiSettingsConfig";

export function WiFiSettingsPanel(): JSX.Element {
  const [config, setConfig] = useState<IWiFiSettingsConfig>(
    new WiFiSettingsConfig(),
  );
  const [errorText, setErrorText] = useState("");
  const { setRestartRequired } = useContext<RestartRequiredContextProps>(
    RestartRequiredContext,
  );

  async function handleSave(): Promise<void> {
    try {
      await saveConfigData(
        "/System/WiFi Settings",
        JSON.stringify(config),
        (e: Error) => {
          setErrorText(e.message);
        },
      );
      setRestartRequired(true);
    } catch (e) {
      setErrorText(`Failed to save data: ${e.message}`);
    }
  }

  async function updateConfig(): Promise<void> {
    try {
      const data = await fetchConfigData("/System/WiFi Settings");
      const unkData = data.config as unknown;
      const iData = unkData as IWiFiSettingsConfig;

      setConfig(iData); // living dangerously
    } catch (e) {
      setErrorText(`Failed to fetch data: ${e.message}`);
    }
  }

  useEffect(() => {
    void updateConfig();
  }, []);

  function setAPConfig(apSettings: APSettingsConfig): void {
    setConfig(
      produce(config, (draft) => {
        draft.apSettings = apSettings;
      }),
    );
  }

  function setClientConfig(clientSettings: ClientSettingsConfig): void {
    setConfig(
      produce(config, (draft) => {
        draft.clientSettings = clientSettings;
      }),
    );
  }

  return (
    <>
      <ToastMessage
        color="text-bg-warning"
        show={errorText !== ""}
        onHide={() => setErrorText("")}
      >
        <p>{errorText}</p>
      </ToastMessage>

      <div className="mb-3">
        <div className="vstack gap-4">
          <APSettingsPanel config={config.apSettings} setConfig={setAPConfig} />
          <ClientSettingsPanel
            config={config.clientSettings}
            setConfig={setClientConfig}
          />
        </div>
      </div>
      <button type="button" className="btn btn-primary" onClick={handleSave}>
        Save
      </button>
    </>
  );
}

interface APSettingsPanelProps {
  config: APSettingsConfig;
  setConfig: (cfg: APSettingsConfig) => void;
}

function APSettingsPanel({
  config,
  setConfig,
}: APSettingsPanelProps): JSX.Element {
  const id = useId();

  function updateSettingsField(
    field: string,
    value: string | number | boolean,
  ): void {
    setConfig(
      produce(config, (draft) => {
        draft[field] = value;
      }),
    );
  }

  function setExpanded(expanded: boolean): void {
    setConfig(
      produce(config, (draft) => {
        draft.enabled = expanded;
      }),
    );
  }

  return (
    <CollapseCard
      id={`${id}-collapsecard`}
      title={
        <>
          <div className="fw-bold">Access Point</div>
          Create a new WiFi network
        </>
      }
      expanded={config.enabled ?? false}
      setExpanded={setExpanded}
    >
      <form>
        <div className="vstack gap-2">
          <FormTextInput
            label="Name"
            value={config.name ?? ""}
            setValue={(value: JsonValue) =>
              updateSettingsField("name", String(value))
            }
          />

          <FormTextInput
            label="Password"
            type="password"
            value={config.password ?? ""}
            setValue={(value: JsonValue) =>
              updateSettingsField("password", String(value))
            }
          />

          <FormNumberInput
            label="Channel"
            step={1}
            aria-label="Select WiFi channel"
            value={config.channel ?? 1}
            setValue={(value: JsonValue) =>
              updateSettingsField("channel", Number(value))
            }
          />

          <FormCheckboxInput
            label="Hidden"
            type="checkbox"
            checked={config.hidden ?? false}
            setValue={(value: JsonValue) =>
              updateSettingsField("hidden", Boolean(value))
            }
          />

          <FormCheckboxInput
            label="Captive Portal"
            type="checkbox"
            checked={config.captivePortalEnabled ?? false}
            setValue={(value: JsonValue) =>
              updateSettingsField("captivePortalEnabled", Boolean(value))
            }
          />
        </div>
      </form>
    </CollapseCard>
  );
}

interface ClientSettingsPanelProps {
  config: IClientSettingsConfig;
  setConfig: (cfg: IClientSettingsConfig) => void;
}

function ClientSettingsPanel({
  config,
  setConfig,
}: ClientSettingsPanelProps): JSX.Element {
  const [selectedNetwork, setSelectedNetwork] = useState("");
  const [activeTab, setActiveTab] = useState(0);
  const [isScanning, setIsScanning] = useState(false);

  const id = useId();

  function updateClientConfig(
    num: number,
    singleConfig: ISingleClientConfig,
  ): void {
    setConfig(
      produce(config, (draft) => {
        draft.settings[num] = singleConfig;
      }),
    );
  }

  function handleClickNetwork(networkName: string): void {
    setSelectedNetwork(networkName);
    setConfig(
      produce(config, (draft) => {
        draft.settings[activeTab].name = networkName;
      }),
    );
  }

  function handleActiveTab(tabNum: number): void {
    setActiveTab(tabNum);
    setSelectedNetwork(config.settings[tabNum].name);
  }

  function setExpanded(expanded: boolean): void {
    setConfig(
      produce(config, (draft) => {
        draft.enabled = expanded;
      }),
    );
  }

  return (
    <CollapseCard
      id={`${id}-collapseclient`}
      title={
        <>
          <div className="fw-bold">Client</div>
          Connect to existing WiFi
        </>
      }
      expanded={config.enabled ?? false}
      setExpanded={setExpanded}
    >
      <div className="container-fluid">
        <div className="row">
          <div className="col-sm overflow-auto">
            <div className="d-flex align-items-center">
              <h4>Available Networks</h4>
              {isScanning && (
                <div
                  className="spinner-border spinner-border-sm ms-auto"
                  role="status"
                >
                  <span className="visually-hidden">Scanning...</span>
                </div>
              )}
            </div>
            <NetworkList
              selectedNetwork={selectedNetwork}
              setSelectedNetwork={handleClickNetwork}
              isScanning={isScanning}
              setIsScanning={setIsScanning}
            />
          </div>
          <div className="col-sm">
            <Tabs id={`${id}-retabs`}>
              <li
                title="First"
                onClick={() => {
                  handleActiveTab(0);
                }}
              >
                <SingleClientConfigPanel
                  config={config.settings[0]}
                  setConfig={(cfg) => {
                    updateClientConfig(0, cfg);
                  }}
                />
              </li>
              <li
                title="Second"
                onClick={() => {
                  handleActiveTab(1);
                }}
              >
                <SingleClientConfigPanel
                  config={config.settings[1]}
                  setConfig={(cfg) => {
                    updateClientConfig(1, cfg);
                  }}
                />
              </li>
              <li
                title="Third"
                onClick={() => {
                  handleActiveTab(2);
                }}
              >
                <SingleClientConfigPanel
                  config={config.settings[2]}
                  setConfig={(cfg) => {
                    updateClientConfig(2, cfg);
                  }}
                />
              </li>
            </Tabs>
          </div>
        </div>
      </div>
    </CollapseCard>
  );
}
