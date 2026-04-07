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
  }, [to]);

  return <></>;
}

export function App(): JSX.Element {
  const [routes, setRoutes] = useState<RouteInstruction[]>([]);
  const [navPath, setNavPath] = useState<string>("/");
  const [error, setError] = useState<string>("");

  useEffect(() => {
    void (async () => {
      try {
        const res = await fetch("/api/routes");
        if (!res.ok) {
          setError(`Failed to load routes: ${res.status} ${res.statusText}`);
          return;
        }
        const data = await res.json();

        const populatedRoutes: RouteInstruction[] = [];
        for (const routeDef of data) {
          if (routeDef.loadPath) {
            const remoteInfo = {
              url: () => Promise.resolve(routeDef.loadPath),
              format: "esm",
              from: "vite",
            };

            __federation_method_setRemote(routeDef.componentName, remoteInfo);
            const module = await __federation_method_getRemote(
              routeDef.componentName,
              "./SensESPPlugin",
            );
            const unwrapped =
              await __federation_method_unwrapDefault(module);
            populatedRoutes.push({ ...routeDef, component: unwrapped });
          } else if (routeDef.componentName in KNOWN_COMPONENTS) {
            populatedRoutes.push({
              ...routeDef,
              component: KNOWN_COMPONENTS[routeDef.componentName],
            });
          } else {
            console.error(`Unknown component: ${routeDef.componentName}`);
          }
        }

        setRoutes(populatedRoutes);
      } catch (e) {
        setError(`Failed to load routes: ${(e as Error).message}`);
      }
    })();
  }, []);

  const [routeComponents, setRouteComponents] = useState<JSX.Element[]>([]);

  useEffect(() => {
    if (routes.length === 0) {
      return;
    }
    const newRouteComponents: JSX.Element[] = [
      <Redirect path="/" to={routes[0].path} />,
    ];
    routes.forEach((routeDef) => {
      if (routeDef.component) {
        newRouteComponents.push(
          <Route path={routeDef.path} component={routeDef.component} />,
        );
      }
    });
    setRouteComponents(newRouteComponents);
  }, [routes]);

  function handleRouteChange(e: RouterOnChangeArgs): void {
    setNavPath(e.url);
  }

  if (error) {
    return (
      <div className="d-flex align-items-center justify-content-center min">
        <div className="alert alert-danger" role="alert">
          <h4 className="alert-heading">Error</h4>
          <p>{error}</p>
        </div>
      </div>
    );
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
        <Router onChange={handleRouteChange}>{routeComponents}</Router>
      </RestartRequiredProvider>
    </>
  );
}
