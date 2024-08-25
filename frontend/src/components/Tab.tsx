import { type JSX } from "preact";
import { useState } from "preact/hooks";

interface TabItemProps {
  id: string;
  title: string;
  active?: boolean;
  target: string;
  onClick: () => void;
}

export function TabItem({
  id,
  title,
  active,
  target,
  onClick,
}: TabItemProps): JSX.Element {
  return (
    <li className="nav-item">
      <button
        className={`nav-link${(active ?? false) ? " active" : ""}`}
        id={id}
        type="button"
        data-bs-toggle="tab"
        data-bs-target={target}
        role="tab"
        onClick={onClick}
      >
        {title}
      </button>
    </li>
  );
}

interface TabPanelProps {
  id: string;
  active?: boolean;
  children: JSX.Element;
}

function TabPanel({ id, active, children }: TabPanelProps): JSX.Element {
  return (
    <div
      className={`tab-pane fade${(active ?? false) ? " show active" : ""}`}
      id={id}
      role="tabpanel"
    >
      {children}
    </div>
  );
}

interface TabsProps {
  id?: string;
  children: JSX.Element[];
}

export function Tabs({ id, children }: TabsProps): JSX.Element {
  const [activeTab, setActiveTab] = useState(0);

  function generateTabItems(): JSX.Element[] {
    return children.map((child, idx) => {
      return (
        <TabItem
          id={`${id}-tab-${idx}`}
          key={child.props.title}
          title={child.props.title}
          target={`#${id}-tab-panel-${idx}`}
          active={idx === activeTab}
          onClick={() => {
            setActiveTab(idx);
            child.props.onClick();
          }}
        />
      );
    });
  }

  function generateTabPanels(): JSX.Element[] {
    return children.map((child, idx) => {
      return (
        <TabPanel
          id={`${id}-tab-panel-${idx}`}
          key={idx}
          active={idx === activeTab}
        >
          {child.props.children}
        </TabPanel>
      );
    });
  }

  return (
    <>
      <ul className="nav nav-tabs" id={id}>
        {generateTabItems()}
      </ul>
      <div className="tab-content" id={`${id}-tab-panels`}>
        {generateTabPanels()}
      </div>
    </>
  );
}
