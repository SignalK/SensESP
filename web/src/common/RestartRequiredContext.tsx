import { createContext, JSX } from "preact";
import { useState } from "preact/hooks";

export interface RestartRequiredContextProps {
  restartRequired: boolean;
  setRestartRequired: (value: boolean) => void;
}

export const RestartRequiredContext = createContext<RestartRequiredContextProps>({
  restartRequired: false,
  setRestartRequired: () => {},
});

export const RestartRequiredProvider = ({
  children,
}: {
  children: JSX.Element;
}) => {
  const [restartRequired, setRestartRequired] = useState<boolean>(false);

  return (
    <RestartRequiredContext.Provider
      value={{ restartRequired, setRestartRequired }}
    >
      {children}
    </RestartRequiredContext.Provider>
  );
};
