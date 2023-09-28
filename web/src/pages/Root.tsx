import { type JSX } from "preact";
import { useEffect } from "preact/hooks";
import { AppPage } from "./AppPage";
import { PageContents } from "./PageContents";
import { PageHeading } from "./PageHeading";

interface RedirectRootProps {
  destination: string;
}

export function RedirectRoot({ destination }: RedirectRootProps): JSX.Element {
  useEffect(() => {
    const timeout = setTimeout(() => {
      // 👇️ redirects to an external URL
      window.location.replace(destination);
    }, 0);

    return () => {
      clearTimeout(timeout);
    };
  }, [destination]);

  // this shenaningans allows to display a spinner that doesn't jump when
  // the redirect happens
  return (
    <>
      <AppPage>
        <PageHeading title="&nbsp;" />
        <PageContents>
          <div
            className="d-flex align-items-center justify-content-center min"
            style="height: 100vh"
          >
            <div className="spinner-border" role="status">
              <span className="visually-hidden">Loading...</span>
            </div>
          </div>
        </PageContents>
      </AppPage>
    </>
  );
}
