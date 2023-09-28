import { useEffect, useId, useState } from "preact/hooks";
import { fetchConfigData, saveConfigData } from "../../common/configAPIClient";

import { type ConfigData } from "common/configAPIClient";
import { type JsonObject } from "common/jsonTypes";
import { Card } from "components/Card";
import { FormInput } from "components/Form";
import { ModalError } from "components/ModalError";
import { type ChangeEvent, type JSX } from "preact/compat";

interface EditControlProps {
  id: string;
  schema: {
    type: string;
    title: string;
    readOnly?: boolean;
  };
  value: string | number;
  onChange: (e: ChangeEvent<HTMLInputElement>) => void;
}

function EditControl({
  id,
  schema,
  value,
  onChange,
}: EditControlProps): JSX.Element {
  let type: string = schema.type;
  let step: number | undefined;
  let as: string | undefined;

  switch (type) {
    case "string":
      type = "text";
      break;
    case "number":
      type = "number";
      break;
    case "integer":
      type = "number";
      step = 1;
      break;
    case "boolean":
      type = "checkbox";
      break;
    case "array":
      as = "textarea";
      break;
    case "object":
      as = "textarea";
      break;
    default:
      type = "text";
      break;
  }

  return (
    <div>
      <FormInput
        type={type}
        as={as}
        id={id}
        label={schema.title}
        value={value}
        readOnly={schema.readOnly ?? false}
        step={step}
        onchange={onChange}
      />
    </div>
  );
}

export default EditControl;

interface CardContentsProps {
  config: JsonObject;
  schema: JsonObject | null;
  description: string;
  setConfig: (config: JsonObject) => void;
}

function CardContents({
  config,
  schema,
  description,
  setConfig,
}: CardContentsProps): JSX.Element {
  const updateConfig = (key: string, value: string | number): void => {
    setConfig({ ...config, [key]: value });
  };

  const properties = schema?.properties ?? {};
  const keys = Object.keys(properties);

  return (
    <>
      {description !== "" || null}

      {keys.map((key) => {
        return (
          <EditControl
            id={key}
            key={key}
            schema={properties[key]}
            value={String(config[key] ?? "")}
            onChange={(event: ChangeEvent<HTMLInputElement>) => {
              updateConfig(key, event.currentTarget.value);
            }}
          />
        );
      })}
    </>
  );
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

  async function handleSave(e: MouseEvent): Promise<void> {
    e.preventDefault();
    setSaving(true);
    await saveConfigData(path, JSON.stringify(config), (e) => {
      console.log("Error saving config data", e);
      setIsDirty(true);
      setHttpErrorText(e.message);
    });
    setIsDirty(false);
    setSaving(false);
  }

  const title = path.slice(1).replace(/\//g, " ▸ ");

  if (
    schema === null ||
    schema === undefined ||
    Object.keys(schema)?.length === 0
  ) {
    return null;
  }

  if (Object.keys(config)?.length === 0) {
    return (
      <div className="d-flex align-items-center justify-content-center min">
        <div className="spinner-border" role="status">
          <span className="visually-hidden">Loading...</span>
        </div>
      </div>
    );
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
        <p>There was an error saving the configuration:</p>
        <p>{httpErrorText}</p>
      </ModalError>

      <Card title={title}>
        <form>
          <div
            onInput={() => {
              setIsDirty(true);
            }}
            className="mb-2"
          >
            <CardContents
              config={config}
              schema={schema}
              description={description}
              setConfig={setConfig}
            />
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
              disabled={saving || !isDirty}
            >
              Save
            </button>
          </div>
        </form>
      </Card>
    </>
  );
}
