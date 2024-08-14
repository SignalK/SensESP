import { AppPage } from "pages/AppPage";
import { type JSX } from "preact";
import { PageContents } from "../PageContents";
import { PageHeading } from "../PageHeading";
import { InfoGroups } from "./InfoGroups";

export function StatusPage(): JSX.Element {
  return (
    <AppPage>
      <PageHeading title="Device Status" />
      <PageContents>
        <InfoGroups />
      </PageContents>
    </AppPage>
  );
}
