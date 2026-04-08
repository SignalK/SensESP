import { ToastMessage } from "components/ToastMessage";
import { APP_CONFIG } from "config";
import { type JSX } from "preact";
import { useEffect, useRef, useState } from "preact/hooks";
import { InfoGroup, type InfoItemData } from "./InfoGroup";

async function fetchGroups(): Promise<Map<string, InfoItemData[]>> {
  const response = await fetch(APP_CONFIG.info_path);
  if (!response.ok) {
    throw new Error(`HTTP Error ${response.status} ${response.statusText}`);
  }
  const data = await response.json();

  const infoItems = data.map((item: Record<string, unknown>) => {
    return {
      name: item.name,
      value: item.value,
      group: item.group,
      order: item.order,
    };
  });

  const sortedItems = infoItems.sort((a: InfoItemData, b: InfoItemData) => {
    return a.order < b.order ? -1 : 1;
  });

  const groups = new Map<string, InfoItemData[]>();
  for (const item of sortedItems) {
    if (!groups.has(item.group)) {
      groups.set(item.group, []);
    }
    groups.get(item.group)?.push(item);
  }

  return groups;
}

export function InfoGroups(): JSX.Element {
  const [groups, setGroups] = useState<Map<string, InfoItemData[]>>(new Map());
  const [errorText, setErrorText] = useState("");
  const errorRef = useRef(errorText);
  errorRef.current = errorText;

  useEffect(() => {
    let cancelled = false;

    async function poll(): Promise<void> {
      if (errorRef.current !== "") {
        return;
      }
      try {
        const groupedItems = await fetchGroups();
        if (!cancelled) {
          setGroups(groupedItems);
        }
      } catch (e) {
        if (!cancelled) {
          setErrorText((e as Error).message);
        }
      }
    }

    void poll();
    const interval = setInterval(() => void poll(), 5000);
    return () => {
      cancelled = true;
      clearInterval(interval);
    };
  }, []);

  if (errorText !== "") {
    return (
      <div
        className="d-flex align-items-center justify-content-center min"
        style="height: 100vh"
      >
        <div className="alert alert-danger" role="alert">
          <h4 className="alert-heading">Error</h4>
          <p>{errorText}</p>
        </div>
      </div>
    );
  }

  if (groups.size === 0) {
    // Display a spinner while waiting for data. Center the spinner
    // in the page.

    return (
      <div
        className="d-flex align-items-center justify-content-center min"
        style="height: 100vh"
      >
        <div className="spinner-border" role="status">
          <span className="visually-hidden">Loading...</span>
        </div>
      </div>
    );
  }

  return (
    <>
      <ToastMessage
        color="text-bg-danger"
        show={errorText !== ""}
        onHide={() => setErrorText("")}
      >
        <p>{errorText}</p>
      </ToastMessage>

      <div>
        <div className="vstack gap-4">
          {Array.from(groups.keys()).map((name_) => {
            return (
              <InfoGroup key={name_} name={name_} items={groups.get(name_)!} />
            );
          })}
        </div>
      </div>
    </>
  );
}
