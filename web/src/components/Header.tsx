import { RouteInstruction } from "App";
import NavPathContext from "common/NavPathContext";
import {
  RestartRequiredContext,
  RestartRequiredContextProps,
} from "common/RestartRequiredContext";
import { type JSX } from "preact";
import { useContext, useState } from "preact/hooks";
import { ToastMessage } from "./ToastMessage";

function RouteLink({ route }: { route: RouteInstruction }): JSX.Element {
  const navPath = useContext(NavPathContext);

  return (
    <li className="nav-item">
      <a
        href={route.path}
        className={`nav-link${
          navPath === route.path ? " active" : " link-body-emphasis"
        }`}
      >
        {route.name}
      </a>
    </li>
  );
}

type HeaderProps = {
  routes: RouteInstruction[];
};

export function Header({ routes }: HeaderProps): JSX.Element {
  const { restartRequired, setRestartRequired } =
    useContext<RestartRequiredContextProps>(RestartRequiredContext);
  const [showToast, setShowToast] = useState(false);

  async function handleRestart(e: MouseEvent): Promise<void> {
    e.preventDefault();
    const response = await fetch("/api/device/restart", { method: "POST" });
    setShowToast(true);
    // Wait for the device to restart and then reload the page.
    setTimeout(() => {
      window.location.reload();
    }, 5000);
  }

  return (
    <>
      <ToastMessage
        color="text-bg-warning"
        show={showToast}
        onHide={() => setShowToast(false)}
      >
        The device is restarting. Please wait...
      </ToastMessage>

      <header className="navbar navbar-expand sticky-top d-flex flex-wrap justify-content-center bg-body-secondary">
        <div className="container px-3">
          <a
            href="/"
            className="navbar-brand d-flex align-items-center mb-0 ms-2 me-4 text-decoration-none"
          >
            <img
              src="/SensESP_logo_symbol.svg"
              width="45"
              height="34"
              alt="SensESP logo"
              className="d-inline-block align-items-center"
            />
          </a>
          <nav className="navbar-nav mb-0">
            <ul className="nav nav-pills">
              {routes.map((route) => (
                <RouteLink key={route.path} route={route} />
              ))}
            </ul>
            {restartRequired && (
              <button
                className="btn btn-warning"
                type="button"
                onClick={(e: MouseEvent): void => {
                  e.stopPropagation();
                  void handleRestart(e);
                }}
              >
                Restart
              </button>
            )}
          </nav>
        </div>
      </header>
    </>
  );
}
