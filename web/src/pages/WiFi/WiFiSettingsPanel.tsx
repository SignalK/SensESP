import { fetchConfigData, saveConfigData } from "common/configAPIClient";
import { CollapseCard } from "components/Card";
import { FormInput, FormSwitch } from "components/Form";
import { ModalError } from "components/ModalError";
import { Tabs } from "components/Tab";
import { produce } from "immer";
import { type JSX } from "preact";
import { useEffect, useId, useState } from "preact/hooks";
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
  const [requestSave, setRequestSave] = useState(false);
  const [errorText, setErrorText] = useState("");

  const id = useId();

  function handleError(e: Error): void {
    setErrorText(e.message);
  }

  useEffect(() => {
    if (requestSave) {
      // save config data to server
      void saveConfigData(
        "/System/WiFi Settings",
        JSON.stringify(config),
        handleError,
      );
      setRequestSave(false);
    }
  }, [config, requestSave]);

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
      <ModalError
        id={`${id}-modal`}
        title="Error"
        show={errorText !== ""}
        onHide={() => {
          setErrorText("");
        }}
      >
        <p>{errorText}</p>
      </ModalError>
      <div className="mb-3">
        <div className="vstack gap-4">
          <APSettingsPanel config={config.apSettings} setConfig={setAPConfig} />
          <ClientSettingsPanel
            config={config.clientSettings}
            setConfig={setClientConfig}
          />
        </div>
      </div>
      <button
        type="button"
        className="btn btn-primary"
        onClick={() => {
          setRequestSave(true);
        }}
      >
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

  function handleApSettingsChange(
    field,
  ): (event: JSX.TargetedEvent<HTMLInputElement, Event>) => void {
    return (event) => {
      setConfig({
        ...config,
        [field]:
          event.currentTarget?.type === "checkbox"
            ? event.currentTarget?.checked
            : event.currentTarget?.value,
      });
    };
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
          <FormInput
            id={`${id}-name`}
            label="Name"
            type="text"
            placeholder="Network Name"
            value={config.name ?? ""}
            onchange={handleApSettingsChange("name")}
          />

          <FormInput
            id={`${id}-password`}
            label="Password"
            type="password"
            placeholder="Network Password"
            value={config.password ?? ""}
            onchange={handleApSettingsChange("password")}
          />

          <FormInput
            id={`${id}-channel`}
            label="Channel"
            type="number"
            aria-label="Select WiFi channel"
            value={config.channel ?? 1}
            onchange={handleApSettingsChange("channel")}
          />

          <FormSwitch
            id={`${id}-hidden`}
            label="Hidden"
            type="checkbox"
            checked={config.hidden ?? false}
            onChange={handleApSettingsChange("hidden")}
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
                <div className="spinner-border spinner-border-sm ms-auto" role="status">
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
