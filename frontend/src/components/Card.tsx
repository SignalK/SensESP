import { type JSX } from "preact";
import { useId, type ReactNode } from "preact/compat";
import { Collapse } from "./Collapse";

interface CardProps {
  id?: string;
  key?: string;
  loading?: boolean;
  title: string;
  children: ReactNode;
}

export function Card({ title, children, loading }: CardProps): JSX.Element {
  if (loading) {
    return (
      <div className="card">
        <div className="card-header">
          <div className="placeholder-glow">
            <span className="placeholder col-7"></span>
          </div>
        </div>
        <div className="card-body">
          <div className="placeholder-glow">
            <span class="placeholder col-7"></span>
            <span class="placeholder col-4"></span>
            <span class="placeholder col-4"></span>
            <span class="placeholder col-6"></span>
            <span class="placeholder col-8"></span>
          </div>
        </div>
      </div>
    );
  } else {
    return (
      <div className="card">
        <div className="card-header">{title}</div>
        <div className="card-body">{children}</div>
      </div>
    );
  }
}

interface ButtonCardProps {
  title: string;
  buttonText: string;
  buttonVariant?: string;
  isButtonEnabled: boolean;
  onClick: () => void;
  children: ReactNode;
}

export function ButtonCard({
  title,
  buttonText,
  buttonVariant = "primary",
  isButtonEnabled,
  onClick,
  children,
}: ButtonCardProps): JSX.Element {
  return (
    <Card title={title}>
      {children}
      <button
        type="button"
        className={`btn btn-${buttonVariant} mt-3`}
        disabled={!isButtonEnabled}
        onClick={onClick}
      >
        {buttonText}
      </button>
    </Card>
  );
}

interface ButtonTitleProps {
  title: string;
  description?: string;
}

export function ButtonTitle({
  title,
  description,
}: ButtonTitleProps): JSX.Element {
  return (
    <>
      <strong>{title}</strong>
      {description != null && (
        <>
          <br /> <small>{description}</small>
        </>
      )}
    </>
  );
}

interface CollapseCardProps {
  id: string;
  title: string | JSX.Element;
  children: ReactNode;
  expanded: boolean;
  setExpanded: (expanded: boolean) => void;
}

/**
 * Render a collapsible card component that can be triggered to show or hide its content.
 *
 * The collapse behavior is controlled by a checkbox in the card header.
 */

export function CollapseCard({
  id,
  title,
  children,
  expanded,
  setExpanded,
}: CollapseCardProps): JSX.Element {
  return (
    <div className="card">
      <div className="card-header justify-content-between align-items-start">
        <div className="d-flex">
          <div className="me-auto">
            <div className="card-title">{title}</div>
          </div>
          <div className="col-auto">
            <CheckToggle expanded={expanded} setExpanded={setExpanded} />
          </div>
        </div>
      </div>
      <Collapse id={id} expanded={expanded}>
        <div className="card-body">{children}</div>
      </Collapse>
    </div>
  );
}

interface CheckToggleProps {
  expanded: boolean;
  setExpanded: (expanded: boolean) => void;
}

function CheckToggle({ expanded, setExpanded }: CheckToggleProps): JSX.Element {
  const id = useId();

  return (
    <div>
      <input
        className="form-check-input"
        type="checkbox"
        id={id}
        checked={expanded}
        onClick={() => {
          setExpanded(!expanded);
        }}
      />
    </div>
  );
}
