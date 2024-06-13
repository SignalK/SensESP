import { APP_CONFIG } from "config";
import { useEffect, useId, useState } from "preact/hooks";

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
import { ModalError } from "components/ModalError";
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
  let step: number | undefined;
  let as: string | undefined;
  let checked: boolean = false;

  let valueString = String(value);

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
            (Number(value) - (schema.displayOffset ?? 0)) /
            (schema.displayMultiplier ?? 1)
          }
          readOnly={schema.readOnly ?? false}
          setValue={(value: number) => {
            setValue(
              value * (schema.displayMultiplier ?? 1) +
                (schema.displayOffset ?? 0),
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
            Number(value) * (schema.displayMultiplier ?? 1) +
            (schema.displayOffset ?? 0)
          }
          readOnly={schema.readOnly ?? false}
          step={1}
          setValue={(value: number) => {
            setValue(
              value / (schema.displayMultiplier ?? 1) -
                (schema.displayOffset ?? 0),
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
  const [config, setConfig] = useState<JsonObject>({});
  const [schema, setSchema] = useState<JsonObject | null>({});
  const [description, setDescription] = useState<string>("");
  const [saving, setSaving] = useState<boolean>(false);
  const [httpErrorText, setHttpErrorText] = useState<string>("");
  const [isDirty, setIsDirty] = useState<boolean>(false);
  const [isValid, setIsValid] = useState<boolean>(true);

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
    if (response.status === 202 && data.status === "pending") {
      // The configuration is being updated, so wait and try again
      setHttpErrorText("Can't handle an async response yet.");
      return;
    } else {
      setConfig(data.config);
      setSchema(data.schema);
      setDescription(data.description);
    }
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

  async function pollSaveResult(url: string): Promise<void> {
    const response = await fetch(url);
    if (!response.ok) {
      console.log(`HTTP Error ${response.status} ${response.statusText}`);
      setHttpErrorText(
        `Received error ${response.status} ${response.statusText} when ` +
          `fetching the configuration: ${response.text}`,
      );
      return;
    }
    const data = await response.json();
    if (response.status === 202 && data.status === "pending") {
      // The configuration is being updated, so wait and try again
      setTimeout(() => {
        void pollSaveResult(url);
      }, 500);
    } else {
      setSaving(false);
      setIsDirty(false);
    }
  }

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
        console.log("Config data saved to server");
        if (response.status === 202) {
          const data = await response.json();
          const pollUrl = data.poll;
          console.log("About to poll", pollUrl);
          setTimeout(() => {
            void pollSaveResult(pollUrl);
          }, 500);
        }
      }
    } catch (e) {
      console.log(`Error saving config data to server: ${e.message}`);
      setHttpErrorText(`Error saving config data to server: ` + `${e.message}`);
      setSaving(false);
    }
  }

  const title = path.slice(1).replace(/\//g, " ▸ ");

  if (Object.keys(config)?.length === 0) {
    console.log("No config data");
    return (
      <div className="d-flex align-items-center justify-content-center min">
        <div className="spinner-border" role="status">
          <span className="visually-hidden">Loading...</span>
        </div>
      </div>
    );
  }

  if (
    schema === null ||
    schema === undefined ||
    Object.keys(schema)?.length === 0
  ) {
    return null;
  }

  const updateConfig = (key: string, value: JsonValue): void => {
    setConfig({ ...config, [key]: value });
  };

  const properties = schema?.properties ?? {};
  const keys = Object.keys(properties);

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
        {httpErrorText}
      </ModalError>

      <Card id={`${id}-card`} key={`${id}-card`} title={title}>
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

            {keys.map((key) => {
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
              onClick={(e: MouseEvent): void => {
                e.stopPropagation();
                void handleSave(e);
              }}
              disabled={saving || !isDirty || !isValid}
            >
              Save
            </button>
            <div
              className={
                `spinner-border me-2${saving}` === "" ? "" : " visually-hidden"
              }
              role="status"
            >
              <span className="visually-hidden">Loading...</span>
            </div>
          </div>
        </form>
      </Card>
    </>
  );
}
