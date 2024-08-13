import { createContext } from "preact";

export interface InputDirtyContextType {
  isInputDirty: boolean;
  setInputDirty: (value: boolean) => void;
}

export const InputDirtyContext = createContext<InputDirtyContextType>({
  isInputDirty: false,
  setInputDirty: (v) => {},
});
