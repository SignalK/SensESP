import { type JSX } from "preact";

interface PageHeadingProps {
  title: string;
  subtitle?: string;
}

export function PageHeading(props: PageHeadingProps): JSX.Element {
  return <h1 className="display-6">{props.title}</h1>;
}
