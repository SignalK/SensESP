import { type JSX } from "preact";
import { useLocation } from "preact-iso";

// import "./header.css"

export function Header(): JSX.Element {
  const { url } = useLocation();

  return (
    <>
      <header className="navbar navbar-expand d-flex flex-wrap justify-content-center bg-body-secondary">
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
              <li className="nav-item">
                <a
                  href="/status"
                  className={`nav-link${
                    url === "/status" ? " active" : " link-body-emphasis"
                  }`}
                >
                  Status
                </a>
              </li>
              <li className="nav-item">
                <a
                  href="/system"
                  className={`nav-link${
                    url === "/system" ? " active" : " link-body-emphasis"
                  }`}
                >
                  System
                </a>
              </li>
              <li className="nav-item">
                <a
                  href="/wifi"
                  className={`nav-link${
                    url === "/wifi" ? " active" : " link-body-emphasis"
                  }`}
                >
                  WiFi
                </a>
              </li>
              <li className="nav-item">
                <a
                  href="/signalk"
                  className={`nav-link${
                    url === "/signalk" ? " active" : " link-body-emphasis"
                  }`}
                >
                  Signal K
                </a>
              </li>
              <li className="nav-item">
                <a
                  href="/configuration"
                  className={`nav-link ${
                    url === "/configuration" ? " active" : " link-body-emphasis"
                  }`}
                >
                  Configuration
                </a>
              </li>
            </ul>
          </nav>
        </div>
      </header>
    </>
  );
}
