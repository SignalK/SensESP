import { type JsonObject } from "common/jsonTypes";
import { Card } from "components/Card";
import { ToastMessage } from "components/ToastMessage";
import { type JSX } from "preact";
import { useEffect, useId, useState } from "preact/hooks";
import { fetchConfigData, saveConfigData } from "../../common/configAPIClient";
import { Collapse } from "../../components/Collapse";

const CONFIG_PATH = "/System/Signal K Settings";

export function SignalKSettingsPanel(): JSX.Element {
  const [config, setConfig] = useState({});
  const [errorText, setErrorText] = useState("");

  async function handleSave(): Promise<void> {
    try {
      await saveConfigData(CONFIG_PATH, JSON.stringify(config), (e: Error) => {
        setErrorText(e.message);
      });
    } catch (e) {
      setErrorText(e.message);
    }
  }

  async function updateConfig(): Promise<void> {
    try {
      const data = await fetchConfigData(CONFIG_PATH);
      setConfig(data.config);
    } catch (e) {
      setErrorText(e.message);
    }
  }

  useEffect(() => {
    if (Object.keys(config).length === 0) {
      void updateConfig();
    }
  }, [config]);

  return (
    <>
      <ToastMessage
        color="text-bg-danger"
        show={errorText !== ""}
        onHide={() => setErrorText("")}
      >
        <p>{errorText}</p>
      </ToastMessage>

      <div className="vstack gap-4">
        <SKConnectionSettings
          config={config}
          setConfig={setConfig}
          setRequestSave={handleSave}
        />
        <SKAuthToken
          config={config}
          setConfig={setConfig}
          setRequestSave={handleSave}
        />
      </div>
    </>
  );
}

interface SKConnectionSettingsProps {
  config: JsonObject;
  setConfig: (cfg: JsonObject) => void;
  setRequestSave: (b: boolean) => void;
}

function SKConnectionSettings({
  config,
  setConfig,
  setRequestSave,
}: SKConnectionSettingsProps): JSX.Element {
  const [mdns, setMdns] = useState(false);
  const id = useId();

  function setHostname(hostname: string): void {
    setConfig({ ...config, sk_address: hostname });
  }

  function setPort(port: number): void {
    setConfig({ ...config, sk_port: port });
  }

  function handleMDNSChange(event): void {
    setMdns(event.target.checked);
    setConfig({ ...config, use_mdns: event.target.checked });
  }

  return (
    <>
      <Card title="Connection Settings">
        <div className="vstack gap-2">
          <form>
            <div className="mb-3 form-check form-switch">
              <label
                htmlFor={`${id}-mdns`}
                className="form-label"
                data-bs-toggle="collapse"
                data-target={`#${id}-collapse`}
              >
                Automatic server discovery
              </label>
              <input
                type="checkbox"
                className="form-check-input switch"
                id={`${id}-mdns`}
                checked={config.use_mdns === true}
                onChange={handleMDNSChange}
              />
            </div>

            <Collapse id={`${id}-collapse`} expanded={!mdns}>
              <div className="mb-3">
                <label htmlFor={`${id}-hostname`} className="form-label">
                  Hostname
                </label>
                <input
                  type="text"
                  className="form-control"
                  id={`${id}-hostname`}
                  value={String(config.sk_address)}
                  onInput={(e) => setHostname(e.currentTarget.value)}
                />
              </div>
              <div className="mb-3">
                <label htmlFor={`${id}-port`} className="form-label">
                  Port
                </label>
                <input
                  type="number"
                  step={1}
                  className="form-control"
                  value={Number(config.sk_port)}
                  onInput={(e) => setPort(Number(e.currentTarget.value))}
                  id={`${id}-port`}
                />
              </div>
              <div className="mb-3 form-check">
                <input
                  type="checkbox"
                  className="form-check-input"
                  id={`${id}-tls`}
                  disabled
                />
                <label className="form-check-label" htmlFor={`${id}-tls`}>
                  Use TLS
                </label>
              </div>
            </Collapse>

            <button
              type="submit"
              className="btn btn-primary"
              onClick={(e) => {
                e.preventDefault();
                setRequestSave(true);
              }}
            >
              Save
            </button>
          </form>
        </div>
      </Card>
    </>
  );
}

interface SKAuthTokenProps {
  config: JsonObject;
  setConfig: (cfg: JsonObject) => void;
  setRequestSave: (b: boolean) => void;
}

function SKAuthToken({
  config,
  setConfig,
  setRequestSave,
}: SKAuthTokenProps): JSX.Element {
  function handleClearToken(): void {
    setConfig({ ...config, token: "" });
    setRequestSave(true);
  }

  return (
    <Card title="Authentication Token">
      <p>
        Click the button to clear the Signal K authentication token. This causes
        the device to request re-authorization from the Signal K server.
      </p>
      <button className="btn btn-primary" onClick={handleClearToken}>
        Clear Token
      </button>
    </Card>
  );
}
