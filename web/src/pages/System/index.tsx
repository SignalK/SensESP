import {
  ConfigData,
  fetchConfigData,
  saveConfigData,
} from "common/configAPIClient";
import { JsonObject } from "common/jsonTypes";
import { ButtonCard } from "components/Card";
import { FormInput, FormSwitch } from "components/Form";
import { produce } from "immer";
import { AppPage } from "pages/AppPage";
import { type JSX } from "preact";
import { ChangeEvent } from "preact/compat";
import { useEffect, useId, useState } from "preact/hooks";
import { ModalError } from "../../components/ModalError";
import { PageContents } from "../PageContents";
import { PageHeading } from "../PageHeading";

export function SystemPage(): JSX.Element {
  return (
    <AppPage>
      <PageHeading title="System Settings" />
      <PageContents>
        <SystemCards />
      </PageContents>
    </AppPage>
  );
}

function SystemCards(): JSX.Element {
  const id = useId();

  return (
    <div className="vstack gap-4">
      <DeviceNameCard />
      <AuthCard />
      <RestartCard />
      <ResetCard />
    </div>
  );

  function DeviceNameCard(): JSX.Element {
    const [origData, setOrigData] = useState<JsonObject>({});
    const [data, setData] = useState<JsonObject>({});
    const [errorMessage, setErrorMessage] = useState<string>("");

    const configPath = "/system/hostname";

    const isDirty = origData !== data;

    const fetchData = async (): Promise<void> => {
      const data: ConfigData = await fetchConfigData(configPath);
      // Get data.config and set it to config
      setOrigData(data.config);
      setData(data.config);
    };

    async function handleSaveData(): Promise<void> {
      const success = await saveConfigData(configPath, JSON.stringify(data), (e) => {
        console.log("Error saving config data", e);
        setErrorMessage(e.message);
      });
      if (success) {
        setOrigData(data);
      }
    }

    const hostname = String(data?.value ?? "");
    function setHostname(hostname: string): void {
      // Use immer to update the hostname property of the config object
      setData(
        produce(data, (draft) => {
          draft.value = hostname;
        }),
      );
    }

    useEffect(() => {
      void fetchData();
    }, []);

    return (
      <SystemSettingsCard
        title={<h5 className="card-title">Device Name</h5>}
        isDirty={isDirty}
        handleSaveData={handleSaveData}
        errorMessage={errorMessage}
        setErrorMessage={setErrorMessage}
      >
        <p className="card-text">
          The device name is used to identify this device on the network. It is
          used both as a hostname (e.g. <code>my-device.local</code>) and as an
          identifying name in the Signal K network.
        </p>
        <FormInput
          label="Hostname"
          id={`${id}-name`}
          type="text"
          value={hostname}
          onInput={(e: ChangeEvent<HTMLInputElement>) =>
            setHostname(e.currentTarget.value)
          }
        />
      </SystemSettingsCard>
    );
  }
}

function AuthCard(): JSX.Element {
  const [origData, setOrigData] = useState<JsonObject>({});
  const [data, setData] = useState<JsonObject>({});
  const [errorMessage, setErrorMessage] = useState<string>("");
  const id = useId();

  const configPath = "/system/authentication";

  const isDirty = origData !== data;

  const authEnabled = data?.authEnabled === true;
  const username = String(data?.username ?? "");
  const password = String(data?.password ?? "");

  const fetchData = async (): Promise<void> => {
    const data: ConfigData = await fetchConfigData(configPath);
    // Get data.config and set it to config
    setOrigData(data.config);
    setData(data.config);
  };

  useEffect(() => {
    void fetchData();
  }, []);

  async function handleSaveData(): Promise<void> {
    const success = await saveConfigData(configPath, JSON.stringify(data), (e) => {
      console.log("Error saving config data", e);
      setErrorMessage(e.message);
    });
    if (success) {
      setOrigData(data);
    }
  }

  function setAuthEnabled(authEnabled: boolean): void {
    setData(
      produce(data, (draft) => {
        draft.authEnabled = authEnabled;
      }),
    );
  }

  function setUsername(username: string): void {
    setData(
      produce(data, (draft) => {
        draft.username = username;
      }),
    );
  }

  function setPassword(password: string): void {
    setData(
      produce(data, (draft) => {
        draft.password = password;
      }),
    );
  }

  return (
    <SystemSettingsCard
      title={<h5 className="card-title">Authentication</h5>}
      isDirty={isDirty}
      handleSaveData={handleSaveData}
      errorMessage={errorMessage}
      setErrorMessage={setErrorMessage}
    >
      <p className="card-text">
        Authentication is used to restrict access to the configuration
        interface. You should disable authentication only if you are using this
        device on a trusted private network.
      </p>
      <div>
        <FormSwitch
          label="Enable authentication"
          id={`${id}-enableAuth`}
          type="checkbox"
          checked={authEnabled}
          onInput={(e) => {
            setAuthEnabled(e.currentTarget.checked);
          }}
        />
      </div>
      <div className="row">
        <div className="col-sm-6">
          <FormInput
            label="Username"
            id={`${id}-username`}
            type="text"
            placeholder="Username"
            value={username}
            disabled={!authEnabled}
            onInput={(e: ChangeEvent<HTMLInputElement>) => {
              setUsername(e.currentTarget.value);
            }}
          />
        </div>
        <div className="col-sm-6">
          <FormInput
            label="Password"
            id={`${id}-password`}
            type="password"
            placeholder="Password"
            value={password}
            disabled={!authEnabled}
            onInput={(e: ChangeEvent<HTMLInputElement>) => {
              setPassword(e.currentTarget.value);
            }}
          />
        </div>
      </div>
    </SystemSettingsCard>
  );
}

interface SystemCardProps {
  title?: React.ReactNode;
  children: React.ReactNode;
}

function SystemCard({ children }: SystemCardProps): JSX.Element {
  return <div className="card">{children}</div>;
}

interface SystemSettingsCardProps {
  title: React.ReactNode;
  isDirty: boolean;
  handleSaveData: () => Promise<void>;
  errorMessage: string;
  setErrorMessage: (message: string) => void;
  children: React.ReactNode;
}

function SystemSettingsCard({
  title,
  isDirty,
  handleSaveData,
  errorMessage,
  setErrorMessage,
  children,
}: SystemSettingsCardProps): JSX.Element {
  const [saving, setSaving] = useState<boolean>(false);

  const id = useId();

  async function handleSave(e: MouseEvent): Promise<void> {
    e.preventDefault();
    setSaving(true);
    await handleSaveData();
    setSaving(false);
  }

  return (
    <>
      <ModalError
        id={`${id}-modal`}
        title="Error"
        show={errorMessage !== ""}
        onHide={() => {
          setErrorMessage("");
        }}
      >
        <p>There was an error saving the configuration:</p>
        <p>{errorMessage}</p>
      </ModalError>

      <SystemCard>
        <div className="card-header">{title}</div>
        <div className="card-body">
          {children}
          <button
            type="button"
            className="btn btn-primary mt-3"
            disabled={!isDirty}
            onClick={handleSave}
          >
            {saving ? (
              <>
                <span
                  className="spinner-border spinner-border-sm"
                  role="status"
                  aria-hidden="true"
                ></span>
                Saving...
              </>
            ) : (
              "Save"
            )}
          </button>
        </div>
      </SystemCard>
    </>
  );
}

/**
 * Component for restarting the device.
 */
function RestartCard(): JSX.Element {
  const [httpErrorText, setHttpErrorText] = useState("");

  const id = useId();

  async function handleRestart(): Promise<void> {
    console.log("Restarting the device");

    try {
      const response = await fetch("/api/device/restart", { method: "POST" });
      if (!response.ok) {
        setHttpErrorText(`${response.status} ${response.statusText}`);
        return;
      }
      console.log("Restarted the device");
      // wait for 5 seconds and attempt a reload
      setTimeout(() => {
        window.location.reload();
      }, 5000);
    } catch (e) {
      setHttpErrorText(`Error restarting the device: ${e}`);
    }
  }

  return (
    <>
      <ModalError
        id={`${id}-modal`}
        title="Error"
        show={httpErrorText !== ""}
        onHide={() => {
          setHttpErrorText("");
        }}
      >
        <p>Failed to restart the device:</p>
        <p>{httpErrorText}</p>
      </ModalError>
      <ButtonCard
        title="Restart the device"
        buttonText="Restart"
        isButtonEnabled={true}
        onClick={() => {
          void handleRestart();
        }}
      >
        <p className="card-text">
          Restarting the device will take a few seconds. If you are connected to
          the device&apos;s WiFi access point, you may have to manually
          reconnect.
        </p>
      </ButtonCard>
    </>
  );
}

/**
 * Component for resetting to factory defaults.
 */
function ResetCard(): JSX.Element {
  const [httpErrorText, setHttpErrorText] = useState("");

  const id = useId();

  async function handleReset(): Promise<void> {
    console.log("Resetting the device");

    try {
      const response = await fetch("/api/device/reset", { method: "POST" });
      if (!response.ok) {
        setHttpErrorText(`${response.status} ${response.statusText}`);
        return;
      }
      console.log("Reset the device");
      // wait for 5 seconds and attempt a reload
      setTimeout(() => {
        window.location.reload();
      }, 5000);
    } catch (e) {
      setHttpErrorText(`Error Resetting the device: ${e}`);
    }
  }

  return (
    <>
      <ModalError
        id={`${id}-modal`}
        title="Error"
        show={httpErrorText !== ""}
        onHide={() => {
          setHttpErrorText("");
        }}
      >
        <p>Failed to reset the device:</p>
        <p>{httpErrorText}</p>
      </ModalError>
      <ButtonCard
        title="Reset the device to factory defaults"
        buttonText="Reset"
        buttonVariant="danger"
        isButtonEnabled={true}
        onClick={() => {
          void handleReset();
        }}
      >
        <p className="card-text">
          <strong>Warning:</strong> This will reset the device to factory
          defaults, erasing all configuration and data. You will need to
          reconfigure the device after resetting.
        </p>
      </ButtonCard>
    </>
  );
}
