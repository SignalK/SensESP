import { type JSX } from "preact";

interface PageContentsProps {
  children: JSX.Element;
}

export function PageContents(props: PageContentsProps): JSX.Element {
  return props.children;
}
