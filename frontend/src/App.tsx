import NavPathContext from "common/NavPathContext";
import { RestartRequiredProvider } from "common/RestartRequiredContext";
import { Header } from "components/Header";
import { ConfigurationPage } from "pages/Configuration";
import { SignalKPage } from "pages/SignalK";
import { StatusPage } from "pages/Status";
import { SystemPage } from "pages/System";
import { WiFiConfigPage } from "pages/WiFi";
import { JSX } from "preact";
import { Route, Router, RouterOnChangeArgs, route } from "preact-router";
import { useEffect, useState } from "preact/hooks";
import {
  __federation_method_getRemote,
  __federation_method_setRemote,
  __federation_method_unwrapDefault,
} from "virtual:__federation__";

export interface RouteInstruction {
  name: string;
  path: string;
  componentName: string;
  loadPath?: string;
  component?: () => JSX.Element;
}

type KnownComponents = {
  [key: string]: () => JSX.Element;
};

const KNOWN_COMPONENTS: KnownComponents = {
  StatusPage,
  SystemPage,
  WiFiConfigPage,
  SignalKPage,
  ConfigurationPage,
};

interface RedirectProps {
  path: string;
  to: string;
}

function Redirect({ to }: RedirectProps): JSX.Element {
  useEffect(() => {
    route(to);
    // Also update the browser URL
    window.history.pushState({}, "", to);
  }, []);

  return <></>;
}

export function App(): JSX.Element {
  const [routes, setRoutes] = useState<RouteInstruction[]>([]);
  const [navPath, setNavPath] = useState<string>("/");

  useEffect(() => {
    // Fetch routes from the backend API
    void (async () => {
      const res = await fetch("/api/routes");
      const data = await res.json();
      setRoutes(data);

      const populatedRoutes: RouteInstruction[] = [];
      for (const route of data) {
        if (route.loadPath) {
          const remoteInfo = {
            url: () => Promise.resolve(route.loadPath),
            format: "esm",
            from: "vite",
          };

          __federation_method_setRemote(route.componentName, remoteInfo);
          const module = await __federation_method_getRemote(
            route.componentName,
            "./SensESPPlugin",
          );
          const unwrapped = await __federation_method_unwrapDefault(module);
          populatedRoutes.push({ ...route, component: unwrapped });
        } else if (route.componentName in KNOWN_COMPONENTS) {
          populatedRoutes.push({
            ...route,
            component: KNOWN_COMPONENTS[route.componentName],
          });
        } else {
          // We don't know about this component - throw an error
          throw new Error(`Unknown component: ${route.componentName}`);
        }
      }

      setRoutes(populatedRoutes);
    })();
  }, []);

  const [routeComponents, setRouteComponents] = useState<JSX.Element[]>([]);

  useEffect(() => {
    // Always add a redirection from root to the first route
    const newRouteComponents: JSX.Element[] = [
      <Redirect path="/" to={routes[0].path} />,
    ];
    routes.forEach((route) => {
      if (route.component) {
        newRouteComponents.push(
          <Route path={route.path} component={route.component} />,
        );
      }
    });
    setRouteComponents(newRouteComponents);
  }, [routes]);

  async function handleRouteChange(e: RouterOnChangeArgs): Promise<void> {
    setNavPath(e.url);
  }

  if (routes.length === 0) {
    return (
      <div className="d-flex align-items-center justify-content-center min">
        <div className="spinner-border" role="status">
          <span className="visually-hidden">Loading...</span>
        </div>
      </div>
    );
  }

  return (
    <>
      <RestartRequiredProvider>
        <NavPathContext.Provider value={navPath}>
          <Header routes={routes} />
        </NavPathContext.Provider>
        {routes.length === 0 ? null : (
          <Router onChange={handleRouteChange}>{routeComponents}</Router>
        )}
      </RestartRequiredProvider>
    </>
  );
}
