import { type JsonObject } from "common/jsonTypes";
import { Card } from "components/Card";
import { ModalError } from "components/ModalError";
import { type JSX } from "preact";
import { useContext, useEffect, useId, useState } from "preact/hooks";
import { fetchConfigData, saveConfigData } from "../../common/configAPIClient";
import { Collapse } from "../../components/Collapse";

export function SignalKSettingsPanel(): JSX.Element {
  const [config, setConfig] = useState({});
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
        "/System/Signal K Settings",
        JSON.stringify(config),
        handleError,
      );
      setRequestSave(false);
    }
  }, [config, requestSave]);

  async function updateConfig(): Promise<void> {
    try {
      const data = await fetchConfigData("/System/Signal K Settings");
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

      <div className="vstack gap-4">
        <SKConnectionSettings
          config={config}
          setConfig={setConfig}
          setRequestSave={setRequestSave}
        />
        <SKAuthToken
          config={config}
          setConfig={setConfig}
          setRequestSave={setRequestSave}
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

  function handleMDNSChange(event): void {
    setMdns(event.target.checked);
    setConfig({ ...config, mdns: event.target.checked });
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
                checked={config.mdns === true}
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
        the device to request a new token from the Signal K server.
      </p>
      <button className="btn btn-primary" onClick={handleClearToken}>
        Clear Token
      </button>
    </Card>
  );
}
