import { RouteInstruction } from "App";
import Modal from "bootstrap/js/dist/modal";
import NavPathContext from "common/NavPathContext";
import {
  RestartRequiredContext,
  RestartRequiredContextProps,
} from "common/RestartRequiredContext";
import { type JSX } from "preact";
import { useContext, useState } from "preact/hooks";

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
  const [modalMessage, setModalMessage] = useState<string>("");

  async function handleRestart(e: MouseEvent): Promise<void> {
    e.preventDefault();
    const response = await fetch("/api/device/restart", { method: "POST" });
    setModalMessage("Device is restarting.");
    const modal = Modal.getOrCreateInstance(`#modalRestarting`);
    modal.show();
    // Wait for the device to restart and then reload the page.
    setTimeout(() => {
      window.location.reload();
    }, 5000);
  }

  return (
    <>
        <div id="modalRestarting" className="modal fade">
          <div className="modal-dialog">
            <div className="modal-content">
              <div className="modal-header">
                <h5 className="modal-title">Restarting</h5>
                <button
                  type="button"
                  className="btn-close"
                  data-bs-dismiss="modal"
                  aria-label="Close"
                ></button>
              </div>
              <div className="modal-body">
                <p>{modalMessage}</p>
              </div>
              <div className="modal-footer">
                <button
                  type="button"
                  className="btn btn-secondary"
                  data-bs-dismiss="modal"
                >
                  Close
                </button>
              </div>
            </div>
          </div>
        </div>
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
          </nav>
          {restartRequired && (
            <button
              className="btn btn-danger"
              type="button"
              onClick={(e: MouseEvent): void => {
                e.stopPropagation();
                void handleRestart(e);
              }}
            >
              Restart
            </button>
          )}
        </div>
      </header>
    </>
  );
}
