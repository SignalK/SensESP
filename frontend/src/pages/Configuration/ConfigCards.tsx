import { APP_CONFIG } from "config";
import { type JSX } from "preact";
import { useEffect, useState } from "preact/hooks";
import { ConfigCard } from "./ConfigCard";

const updateCards = async (): Promise<string[]> => {
  try {
    const response = await fetch(APP_CONFIG.config_path);
    if (!response.ok) {
      throw new Error(`HTTP Error ${response.status} ${response.statusText}`);
    }
    const data = await response.json();

    const items = data.keys;
    return items;
  } catch (e) {
    console.log("Error getting config data from server", e);
  }
  return [];
};

export function ConfigCards(): JSX.Element {
  const [cards, setCards] = useState<string[] | null>(null);

  async function updateFunc(): Promise<void> {
    const items = await updateCards();
    setCards(items);
  }

  useEffect(() => {
    void updateFunc();
  }, []);

  if (cards === null) {
    // Display a spinner while waiting for data. Center the spinner
    // in the page.
    return (
      <div
        className="d-flex align-items-center justify-content-center min"
        style={{ height: "100vh" }}
      >
        <div className="spinner-border" role="status">
          <span className="visually-hidden">Loading...</span>
        </div>
      </div>
    );
  }

  return (
    <div className="vstack gap-4">
      {cards.map((card) => (
        <ConfigCard key={card} path={card} />
      ))}
    </div>
  );
}
