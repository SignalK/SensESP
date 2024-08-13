import { ToastMessage } from "components/ToastMessage";
import { APP_CONFIG } from "config";
import { type JSX } from "preact";
import { useEffect, useId, useState } from "preact/hooks";
import { InfoGroup, type InfoItemData } from "./InfoGroup";

async function updateGroups(): Promise<Map<string, InfoItemData[]>> {
  const response = await fetch(APP_CONFIG.info_path);
  if (!response.ok) {
    throw new Error(`HTTP Error ${response.status} ${response.statusText}`);
  }
  const data = await response.json();

  // Make data an array of InfoItemData objects
  const infoItems = data.map((item: Record<string, any>) => {
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

  // collect unique groups from the sorted items
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

  const id = useId();

  async function timerFunc(): Promise<void> {
    // fetch updated group items from server
    if (errorText !== "") {
      return; // don't update if there's an error
    }
    try {
      const groupedItems = await updateGroups();
      setGroups(groupedItems);
    } catch (e) {
      setErrorText(e.message);
    }
  }

  useEffect(() => {
    if (errorText !== "") {
      console.log("Error: ", errorText);
    }
  }, [errorText]);

  useEffect(() => {
    if (Array.from(groups.keys()).length === 0 && errorText === "") {
      timerFunc();
    }
    const interval = setInterval(() => {
      timerFunc();
    }, 5000);
    return () => {
      clearInterval(interval);
    };
  }, [groups, errorText]);

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
