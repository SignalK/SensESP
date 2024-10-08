import { APP_CONFIG } from "config";
import { useContext, useEffect, useId, useState } from "preact/hooks";

import {
  RestartRequiredContext,
  RestartRequiredContextProps,
} from "common/RestartRequiredContext";
import { JsonValue, type JsonObject } from "common/jsonTypes";
import { Card } from "components/Card";
import {
  FormCheckboxInput,
  FormNumberInput,
  FormRadioInput,
  FormSelectInput,
  FormTextAreaInput,
  FormTextInput,
} from "components/Form";
import { ToastMessage } from "components/ToastMessage";
import { type JSX } from "preact/compat";

interface ItemsProps {
  type: string;
  enum: string[];
}

interface EditControlProps {
  id: string;
  schema: {
    type: string;
    title: string;
    readOnly?: boolean;
    uniqueItems?: boolean;
    format?: string;
    items?: ItemsProps;
    displayMultiplier?: number;
    displayOffset?: number;
  };
  value: JsonValue;
  setValue: (value: JsonValue) => void;
}

export function EditControl({
  id,
  schema,
  value,
  setValue,
}: EditControlProps): JSX.Element {
  let checked: boolean = false;

  const valueString = String(value);

  switch (schema.type) {
    case "string":
      return (
        <FormTextInput
          key={id}
          label={schema.title}
          value={valueString}
          readOnly={schema.readOnly ?? false}
          setValue={(value: string) => {
            setValue(value);
          }}
        />
      );
    case "number":
      return (
        <FormNumberInput
          key={id}
          label={schema.title}
          value={
            (schema.displayMultiplier ?? 1) * Number(value) +
            (schema.displayOffset ?? 0)
          }
          readOnly={schema.readOnly ?? false}
          setValue={(value: number) => {
            setValue(
              (value - (schema.displayOffset ?? 0)) /
                (schema.displayMultiplier ?? 1),
            );
          }}
        />
      );
    case "integer":
      return (
        <FormNumberInput
          key={id}
          label={schema.title}
          value={
            (schema.displayMultiplier ?? 1) * Number(value) +
            (schema.displayOffset ?? 0)
          }
          readOnly={schema.readOnly ?? false}
          step={1}
          setValue={(value: number) => {
            setValue(
              (value - (schema.displayOffset ?? 0)) /
                (schema.displayMultiplier ?? 1),
            );
          }}
        />
      );
      break;
    case "boolean":
      checked = value === true;
      return (
        <FormCheckboxInput
          key={id}
          type="checkbox"
          label={schema.title}
          checked={checked}
          readOnly={schema.readOnly ?? false}
          setValue={(checked: boolean) => {
            setValue(checked);
          }}
        />
      );
    case "array":
      if (schema.uniqueItems === true) {
        if (schema.format === "radiobutton") {
          return (
            <FormRadioInput
              key={id}
              label={schema.title}
              items={schema.items ?? { type: "string", enum: [] }}
              value={valueString}
              readOnly={schema.readOnly ?? false}
              setValue={(value: string) => {
                setValue(value);
              }}
            />
          );
        } else if (schema.format === "select") {
          return (
            <FormSelectInput
              key={id}
              label={schema.title}
              items={schema.items ?? { type: "string", enum: [] }}
              value={valueString}
              readOnly={schema.readOnly ?? false}
              setValue={(value: string) => {
                setValue(value);
              }}
            />
          );
        } else {
          throw new Error("Unsupported array format");
        }
      } else {
        // Complex arrays are implemented as textareas for now
        let jsonString: string | null;
        if (value === null) {
          jsonString = null;
        } else {
          jsonString = JSON.stringify(value, null, 2);
        }
        const stringToJson = (value: string): JsonValue | null => {
          try {
            return JSON.parse(value);
          } catch (e) {
            console.log("Error parsing JSON", e);
            return null;
          }
        };
        return (
          <FormTextAreaInput
            key={id}
            label={schema.title}
            value={jsonString}
            setValue={(newValue: string | null) => {
              if (newValue === null) {
                setValue(newValue);
              } else {
                // Only update if JSON is different. This allows for
                // whitepace changes.
                const nvJson = stringToJson(newValue ?? "");
                const nvJsonString = JSON.stringify(nvJson, null, 2);
                if (nvJsonString !== jsonString) {
                  setValue(nvJson);
                }
              }
            }}
            readOnly={schema.readOnly ?? false}
          />
        );
      }
      break;
    case "object":
      throw new Error("Unsupported object type");
      break;
    default:
      // same as text
      return (
        <FormTextInput
          key={id}
          label={schema.title}
          value={valueString}
          readOnly={schema.readOnly ?? false}
          setValue={(value: string) => {
            setValue(value);
          }}
        />
      );
  }
}

interface ConfigCardProps {
  path: string;
}

export function ConfigCard({ path }: ConfigCardProps): JSX.Element | null {
  const { setRestartRequired } = useContext<RestartRequiredContextProps>(
    RestartRequiredContext,
  );

  const [config, setConfig] = useState<JsonObject>({});
  const [schema, setSchema] = useState<JsonObject | null>({});
  const [title, setTitle] = useState<string>("");
  const [description, setDescription] = useState<string>("");
  const [saving, setSaving] = useState<boolean>(false);
  const [loading, setLoading] = useState<boolean>(true);
  const [httpErrorText, setHttpErrorText] = useState<string>("");
  const [isDirty, setIsDirty] = useState<boolean>(false);
  const [isValid, setIsValid] = useState<boolean>(true);
  const [componentRequiresRestart, setComponentRequiresRestart] =
    useState<boolean>(false);

  const id = useId();

  const updateFunc = async (path: string): Promise<void> => {
    const response = await fetch(APP_CONFIG.config_path + path);
    if (!response.ok) {
      console.log(`HTTP Error ${response.status} ${response.statusText}`);
      setHttpErrorText(
        `Received error ${response.status} ${response.statusText} when ` +
          `fetching the configuration: ${await response.text()}`,
      );
      return;
    }
    const data = await response.json();
    console.log("Fetched config data", data);

    setConfig(data.config);
    setSchema(data.schema);
    setTitle(data.title);
    setDescription(data.description);
    setComponentRequiresRestart(data.requires_restart);
  };

  useEffect(() => {
    if (Object.keys(config)?.length === 0) {
      console.log(`Fetching config data from ${path}...`);
      void updateFunc(path);
      console.log(`Fetched config data from ${path}`);
    }
  }, [path]);

  useEffect(() => {
    // config is valid if none of the values are null
    const isValid = Object.values(config).every((v) => v !== null);
    setIsValid(isValid);
  }, [config]);

  async function handleSave(e: MouseEvent): Promise<void> {
    e.preventDefault();
    setSaving(true);
    const contentType = "application/json";
    try {
      const response = await fetch(APP_CONFIG.config_path + path, {
        method: "PUT",
        headers: {
          "Content-Type": contentType,
        },
        body: JSON.stringify(config),
      });
      if (!response.ok) {
        console.log("HTTP Error", response.status, response.statusText);
        setHttpErrorText(
          `Received error ${response.status} ${response.statusText} when ` +
            `saving the configuration: ${response.text}`,
        );
        setSaving(false);
        setIsDirty(false);
        return;
      } else {
        if (response.status === 200) {
          setIsDirty(false);
          setSaving(false);
          if (componentRequiresRestart) {
            setRestartRequired(true);
          }
          // TODO: Show success message
        }
      }
    } catch (e) {
      console.log(`Error saving config data to server: ${e.message}`);
      setHttpErrorText(`Error saving config data to server: ` + `${e.message}`);
      setSaving(false);
    }
  }

  if (Object.keys(schema ?? {})?.length === 0) {
    setLoading(true);
  } else {
    setLoading(false);
  }

  const updateConfig = (key: string, value: JsonValue): void => {
    setConfig({ ...config, [key]: value });
  };

  const properties = schema?.properties ?? {};
  const keys = Object.keys(properties);

  return (
    <>
      <ToastMessage
        color="text-bg-danger"
        show={httpErrorText !== ""}
        onHide={() => setHttpErrorText("")}
      >
        <p>Unable to set configuration:</p>
        <p>{httpErrorText}</p>
      </ToastMessage>

      <Card
        id={`${id}-card`}
        key={`${id}-card`}
        loading={loading}
        title={title}
      >
        <form>
          <div
            onInput={() => {
              setIsDirty(true);
            }}
            className="mb-2"
          >
            {description !== "" ? (
              <p dangerouslySetInnerHTML={{ __html: description }}></p>
            ) : null}

            {keys
              .filter((key) => !key.startsWith("_"))
              .map((key) => {
                return (
                  <EditControl
                    id={`${id}-editcontrol-${key}`}
                    key={`${id}-editcontrol-${key}`}
                    schema={properties[key]}
                    value={config[key] ?? null}
                    setValue={(value: JsonValue) => {
                      updateConfig(key, value);
                    }}
                  />
                );
              })}
          </div>
          <div className="d-flex justify-content-begin">
            <button
              className="btn btn-primary me-2"
              type="submit"
              disabled={saving || !isDirty || !isValid}
              onClick={(e: MouseEvent): void => {
                e.stopPropagation();
                void handleSave(e);
              }}
            >
              <span className="me-1" role="status">
                {saving ? "Saving..." : "Save"}
              </span>
              {saving && (
                <span
                  className="spinner-border spinner-border-sm"
                  aria-hidden="true"
                ></span>
              )}
            </button>
          </div>
        </form>
      </Card>
    </>
  );
}
