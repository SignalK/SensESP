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
        <SKSSLSettings
          config={config}
          setConfig={setConfig}
          setRequestSave={handleSave}
          onConfigUpdate={updateConfig}
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

interface SKSSLSettingsProps {
  config: JsonObject;
  setConfig: (cfg: JsonObject) => void;
  setRequestSave: (b: boolean) => void;
  onConfigUpdate: () => Promise<void>;
}

function SKSSLSettings({
  config,
  setConfig,
  setRequestSave,
  onConfigUpdate,
}: SKSSLSettingsProps): JSX.Element {
  const [resetting, setResetting] = useState(false);
  const id = useId();

  function handleSSLChange(event): void {
    setConfig({ ...config, ssl_enabled: event.target.checked });
  }

  function handleVerifyChange(event): void {
    setConfig({ ...config, tofu_enabled: event.target.checked });
  }

  async function handleResetTOFU(): Promise<void> {
    setResetting(true);
    try {
      const response = await fetch("/api/signalk/reset-tofu", {
        method: "POST",
      });
      if (response.ok) {
        // Refresh config to show cleared fingerprint
        await onConfigUpdate();
      }
    } catch (e) {
      console.error("Failed to reset TOFU:", e);
    }
    setResetting(false);
  }

  const hasFingerprint =
    config.tofu_fingerprint && config.tofu_fingerprint !== "";
  const fingerprintDisplay = hasFingerprint
    ? String(config.tofu_fingerprint).substring(0, 16) + "..."
    : "Not stored";

  return (
    <Card title="SSL/TLS Security">
      <div className="vstack gap-2">
        <form>
          <div className="mb-3 form-check form-switch">
            <input
              type="checkbox"
              className="form-check-input switch"
              id={`${id}-ssl`}
              checked={config.ssl_enabled === true}
              onChange={handleSSLChange}
            />
            <label htmlFor={`${id}-ssl`} className="form-check-label">
              SSL/TLS Enabled
            </label>
            <div className="form-text">
              Enable for HTTPS/WSS connections. Auto-detected when server
              redirects HTTP to HTTPS.
            </div>
          </div>

          <div className="mb-3 form-check form-switch">
            <input
              type="checkbox"
              className="form-check-input switch"
              id={`${id}-verify`}
              checked={config.tofu_enabled === true}
              onChange={handleVerifyChange}
              disabled={!config.ssl_enabled}
            />
            <label htmlFor={`${id}-verify`} className="form-check-label">
              Verify Server Certificate (TOFU)
            </label>
            <div className="form-text">
              When enabled, the server certificate fingerprint is captured on
              first connection and verified on subsequent connections.
            </div>
          </div>

          {config.ssl_enabled && (
            <div className="mb-3">
              <label className="form-label">Stored Fingerprint</label>
              <div className="input-group">
                <input
                  type="text"
                  className="form-control"
                  value={fingerprintDisplay}
                  disabled
                />
                <button
                  className="btn btn-outline-danger"
                  type="button"
                  onClick={handleResetTOFU}
                  disabled={!hasFingerprint || resetting}
                >
                  {resetting ? "Resetting..." : "Reset TOFU"}
                </button>
              </div>
              <div className="form-text">
                Reset if the server certificate has changed legitimately.
              </div>
            </div>
          )}

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
  );
}
