import BSCollapse from "bootstrap/js/dist/collapse";
import { type JSX } from "preact";
import { useEffect } from "preact/hooks";

interface ReCollapseProps {
  id: string;
  children: React.ReactNode;
  expanded: boolean;
}

/**
 * Renders a collapsible component that can be triggered to show or hide its content.
 * @param {string} id - The id of the collapsible component.
 * @param {React.ReactNode} children - The content to be displayed inside the collapsible component.
 * @param {boolean} expanded - Determines whether the collapsible component should be collapsed or expanded.
 * @returns {JSX.Element} - The collapsible component.
 */
export function Collapse({
  id,
  children,
  expanded,
}: ReCollapseProps): JSX.Element {
  useEffect(() => {
    const collapseEl = document.getElementById(id) ?? "";
    const bsCollapse = new BSCollapse(collapseEl, { toggle: false });
    if (expanded) {
      bsCollapse.show();
    } else {
      bsCollapse.hide();
    }
  });

  return (
    <div className="collapse" id={id}>
      {children}
    </div>
  );
}
