import { useEffect, useId, useState } from "preact/hooks";
import { fetchConfigData, saveConfigData } from "../../common/configAPIClient";

import { type ConfigData } from "common/configAPIClient";
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
          value={Number(value)}
          readOnly={schema.readOnly ?? false}
          setValue={(value: number) => {
            setValue(value);
          }}
        />
      );
    case "integer":
      return (
        <FormNumberInput
          key={id}
          label={schema.title}
          value={Number(value)}
          readOnly={schema.readOnly ?? false}
          step={1}
          setValue={(value: number) => {
            setValue(value);
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
    const data: ConfigData = await fetchConfigData(path);
    if (data === null) {
      return;
    }
    setConfig(data.config);
    setSchema(data.schema);
    setDescription(data.description);
  };

  useEffect(() => {
    if (Object.keys(config)?.length === 0) {
      void updateFunc(path);
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
    await saveConfigData(path, JSON.stringify(config), (e) => {
      console.log("Error saving config data", e);
      setIsDirty(true);
      setHttpErrorText(e.message);
    });
    setConfig(config);
    setIsDirty(false);
    setSaving(false);
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
        <p>There was an error saving the configuration:</p>
        <p>{httpErrorText}</p>
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
            <div
              className={
                `spinner-border me-2${saving}` === "" ? "" : " visually-hidden"
              }
              role="status"
            >
              <span className="visually-hidden">Loading...</span>
            </div>

            <button
              className="btn btn-primary"
              type="submit"
              onClick={(e: MouseEvent): void => {
                void handleSave(e);
              }}
              disabled={saving || (!isDirty || !isValid)}
            >
              Save
            </button>
          </div>
        </form>
      </Card>
    </>
  );
}
