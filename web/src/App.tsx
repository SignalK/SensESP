import { Header } from "components/Header";
import { ConfigurationPage } from "pages/Configuration";
import { RedirectRoot } from "pages/Root";
import { SignalKPage } from "pages/SignalK";
import { StatusPage } from "pages/Status";
import { SystemPage } from "pages/System";
import { WiFiConfigPage } from "pages/WiFi";
import { NotFound } from "pages/_404.jsx";
import { type JSX } from "preact";
import { LocationProvider, Route, Router } from "preact-iso";

export function App(): JSX.Element {
  return (
    <LocationProvider>
      <Header />
      <Router>
        <RedirectRoot path="/" destination="/status" />
        <StatusPage path="/status" />
        <SystemPage path="/system" />
        <WiFiConfigPage path="/wifi" />
        <SignalKPage path="/signalk" />
        <ConfigurationPage path="/configuration" />
        <Route default component={NotFound} />
      </Router>
    </LocationProvider>
  );
}
