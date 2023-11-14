import { RouteInstruction } from "App";
import NavPathContext from "common/NavPathContext";
import { type JSX } from "preact";
import { useContext, useEffect, useState } from "preact/hooks";

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
              {routes.map((route) => (
                <RouteLink key={route.path} route={route} />
              ))}
            </ul>
          </nav>
        </div>
      </header>
    </>
  );
}
