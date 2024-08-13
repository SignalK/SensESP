import { type JSX } from "preact";

interface AppPageProps {
  children: React.ReactNode;
}

export function AppPage({ children }: AppPageProps): JSX.Element {
  return <div className="container-sm p-3">{children} </div>;
}
