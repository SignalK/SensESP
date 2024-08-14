import { AppPage } from "pages/AppPage";
import { type JSX } from "preact";
import { PageContents } from "../PageContents";
import { PageHeading } from "../PageHeading";
import { WiFiSettingsPanel } from "./WiFiSettingsPanel";

export function WiFiConfigPage(): JSX.Element {
  return (
    <AppPage>
      <PageHeading title="WiFi Configuration" />
      <PageContents>
        <WiFiSettingsPanel />
      </PageContents>
    </AppPage>
  );
}
