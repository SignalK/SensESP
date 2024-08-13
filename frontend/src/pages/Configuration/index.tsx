import { AppPage } from "pages/AppPage";
import { type JSX } from "preact";
import { PageContents } from "../PageContents";
import { PageHeading } from "../PageHeading";
import { ConfigCards } from "./ConfigCards";

export function ConfigurationPage(): JSX.Element {
  return (
    <AppPage>
      <PageHeading title="Configuration" />
      <PageContents>
        <ConfigCards />
      </PageContents>
    </AppPage>
  );
}
