import {
  InputDirtyContext,
  InputDirtyContextType,
} from "common/InputDirtyContext";
import { type JSX } from "preact";
import { useContext } from "preact/hooks";

interface FormInputProps {
  id: string;
  label: string;
  type?: string;
  as?: string;
  placeholder?: string;
  readOnly?: boolean;
  value?: string | number;
  step?: number;
  disabled?: boolean;
  checked?: boolean;
  onInput?: (e: JSX.TargetedEvent<HTMLInputElement|HTMLTextAreaElement, Event>) => void;
}

export function FormInput(props: FormInputProps): JSX.Element {

  return (
    <div>
      <label className="form-label" htmlFor={props.id}>
        {props.label}
      </label>

      {props.as === "textarea" ? (
        <textarea
          className="form-control"
          {...props}
        />
      ) : (
        <input
          className="form-control"
          {...props}
        />
      )}
    </div>
  );
}

interface FormFloatInputProps {
  id: string;
  label: string;
}

export function FormFloatInput(props: FormFloatInputProps): JSX.Element {

  return (
    <div className="form-floating mb-3">
      <input
        className="form-control"
        {...props}
      />
      <label className="form-label" htmlFor={props.id}>
        {props.label}
      </label>
    </div>
  );
}

interface FormSelectProps {
  id: string;
  label: string;
  children: React.ReactNode;
}

export function FormSelect(props: FormSelectProps): JSX.Element {

  return (
    <div className="mb-3">
      <label className="form-label" htmlFor={props.id}>
        {props.label}
      </label>
      <select
        className="form-select"
        {...props}
      />
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
      <input
        className="form-check-input"
        {...props}
      />
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
      <input
        className="form-check-input"
        role="switch"
        {...props}
      />
      <label className="form-check-label" htmlFor={props.id}>
        {props.label}
      </label>
    </div>
  );
}
