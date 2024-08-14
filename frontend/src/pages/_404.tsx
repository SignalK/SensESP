import { type JSX } from "preact";
import { AppPage } from "./AppPage";
import { PageContents } from "./PageContents";
import { PageHeading } from "./PageHeading";

export function NotFound(): JSX.Element {
  return (
    <AppPage>
      <PageHeading title="&nbsp;" />
      <PageContents>
        <div>
          <h1>404: Not Found</h1>
          <p>Sorry!</p>
        </div>
      </PageContents>
    </AppPage>
  );
}
