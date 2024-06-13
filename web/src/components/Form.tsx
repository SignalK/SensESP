import { JsonValue } from "common/jsonTypes";
import { type JSX } from "preact";
import { useEffect, useId, useState } from "preact/hooks";

interface ItemsProps {
  type: string;
  enum: string[];
}

interface FormRadioInputProps {
  label: string;
  value: string;
  readOnly?: boolean;
  setValue: (value: string) => void;
  items: ItemsProps;
}

export function FormRadioInput(props: FormRadioInputProps): JSX.Element {
  const id = useId();
  // Generate a control group block with inline radio buttons
  return (
    <fieldset className="mb-3">
      <legend className="form-label">{props.label}</legend>
      {props.items?.enum.map((item, index) => (
        <div className="form-check form-check-inline" key={index}>
          <input
            className="form-check-input"
            type="radio"
            id={id + item}
            name={item}
            value={item}
            disabled={props.readOnly}
            checked={props.value === item}
            onChange={(e) => props.setValue(e.currentTarget.value)}
          />
          <label className="form-check-label" htmlFor={id + item}>
            {item}
          </label>
        </div>
      ))}
    </fieldset>
  );
}

interface FormSelectInputProps {
  label: string;
  value: string;
  readOnly?: boolean;
  items: ItemsProps;
  setValue: (value: string) => void;
}

export function FormSelectInput(props: FormSelectInputProps): JSX.Element {
  const id = useId();
  // Generate a select control
  return (
    <div className="form-floating mb-3">
      <select
        className="form-select"
        id={id}
        value={props.value}
        readOnly={props.readOnly}
        onChange={(e) => props.setValue(e.currentTarget.value)}
      >
        {props.items?.enum.map((item, index) => (
          <option value={item} key={index}>
            {item}
          </option>
        ))}
      </select>
      <label className="form-label" htmlFor={id}>
        {props.label}
      </label>
    </div>
  );
}

interface FormCheckboxInputProps {
  label: string;
  type: string;
  checked: boolean;
  readOnly?: boolean;
  setValue: (value: JsonValue) => void;
}

export function FormCheckboxInput(props: FormCheckboxInputProps): JSX.Element {
  const id = useId();
  // Generate a checkbox control
  return (
    <div className="form-check">
      <input
        className="form-check-input"
        type={props.type || "checkbox"}
        id={id}
        checked={props.checked}
        disabled={props.readOnly}
        onChange={(e) => props.setValue(!props.checked)}
      />
      <label className="form-check-label" htmlFor={id}>
        {props.label}
      </label>
    </div>
  );
}

export interface FormTextAreaInputProps {
  key: string;
  label: string;
  value: string | null;
  setValue: (value: string | null) => void;
  readOnly?: boolean;
}

export function FormTextAreaInput(props: FormTextAreaInputProps): JSX.Element {
  const id = useId();
  let onInput: (e: Event) => void = () => {};
  const [state, setState] = useState<string | null>(null);

  useEffect(() => {
    if (props.value !== null && props.value !== state) {
      setState(props.value);
    }
  }, [props.value]);

  // If our value is string, we can show it as is and call the default
  // onInput handler. If it's an array of arrays of numbers, we need to
  // convert it to a string and parse it back into the array of arrays
  // of numbers.
  onInput = (e) => {
    const value = (e.target as HTMLTextAreaElement).value;
    setState(value);
    props.setValue(value);
  };

  // Generate a textarea control
  return (
    <div className="form-floating mb-3">
      <textarea
        className={`form-control ${state === null ? "border-danger" : ""}`}
        id={id}
        value={state ?? ""}
        rows={8}
        style={
          props.value === null
            ? { borderColor: "red", height: "100%" }
            : { height: "100%" }
        }
        readOnly={props.readOnly}
        onChange={onInput}
      />
      <label className="form-label" htmlFor={id}>
        {props.label}
      </label>
    </div>
  );
}

interface FormTextInputProps {
  label: string;
  readOnly?: boolean;
  type?: string;
  value: JsonValue;
  disabled?: boolean;
  setValue: (value: JsonValue) => void;
}

export function FormTextInput(props: FormTextInputProps): JSX.Element {
  const id = useId();
  const inputProps = {
    id: id,
    type: props.type || "text",
    defaultValue: String(props.value),
    readOnly: props.readOnly,
    disabled: props.disabled,
    onChange: (e: Event) =>
      props.setValue((e.currentTarget as HTMLInputElement).value),
  };

  return (
    <div className="form-floating mb-3">
      <input className="form-control" {...inputProps} />
      <label className="form-label" htmlFor={id}>
        {props.label}
      </label>
    </div>
  );
}

interface FormNumberInputProps {
  label: string;
  readOnly?: boolean;
  value: JsonValue;
  disabled?: boolean;
  step?: number;
  setValue: (value: JsonValue) => void;
}

export function FormNumberInput(props: FormNumberInputProps): JSX.Element {
  const id = useId();
  const decimals = props.step
    ? Math.max(0, Math.ceil(Math.log10(1 / (props.step || 1))))
    : 3;
  const inputProps = {
    id: id,
    type: "number",
    defaultValue: String(Number(props.value).toFixed(decimals)),
    readOnly: props.readOnly,
    disabled: props.disabled,
    step: props.step,
    onChange: (e: Event) =>
      props.setValue(Number((e.currentTarget as HTMLInputElement).value)),
  };
  return (
    <div className="form-floating mb-3">
      <input className="form-control" {...inputProps} />
      <label className="form-label" htmlFor={id}>
        {props.label}
      </label>
    </div>
  );
}

interface FormSelectProps {
  label: string;
  children: React.ReactNode;
}

export function FormSelect(props: FormSelectProps): JSX.Element {
  const id = useId();
  return (
    <div className="mb-3">
      <label className="form-label" htmlFor={id}>
        {props.label}
      </label>
      <select className="form-select" {...props} />
      {props.children}
    </div>
  );
}

interface FormCheckProps {
  id: string;
  name: string;
  type: string;
  checked: boolean;
  label: string;
  handleValueChange: (value: string) => void;
}

export function FormCheck(props: FormCheckProps): JSX.Element {
  return (
    <div className="form-check">
      <input className="form-check-input" {...props} />
      <label className="form-check-label" htmlFor={props.id}>
        {props.label}
      </label>
    </div>
  );
}

export function FormSwitch(
  props: JSX.IntrinsicAttributes & JSX.HTMLAttributes<HTMLInputElement>,
): JSX.Element {
  return (
    <div className="form-check form-switch mb-3">
      <input className="form-check-input" role="switch" {...props} />
      <label className="form-check-label" htmlFor={props.id}>
        {props.label}
      </label>
    </div>
  );
}
