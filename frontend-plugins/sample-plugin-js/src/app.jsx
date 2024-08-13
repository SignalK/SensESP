import { useState } from "preact/hooks";
import TestPlugin from "./sensesp-plugin.jsx";

export function App() {
  const [count, setCount] = useState(0);

  return (
    <>
      <div>
        <h1>Test Plugin</h1>
        <p>This is a test plugin component!</p>
        <TestPlugin />
      </div>
      <h1>Vite + Preact</h1>
      <div class="card">
        <button onClick={() => setCount((count) => count + 1)}>
          count is {count}
        </button>
        <p>
          Edit <code>src/app.jsx</code> and save to test HMR
        </p>
      </div>
      <p class="read-the-docs">
        Click on the Vite and Preact logos to learn more
      </p>
    </>
  );
}
