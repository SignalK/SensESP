// import "preact/debug"; // <-- Add this line at the top of your main entry file

import { render } from "preact";
import { App } from "./App";

// import "bootstrap/dist/css/bootstrap.css";
import "../css/styles.css";

// import "bootstrap/dist/js/bootstrap.js";

import "bootstrap/js/dist/collapse";
import "bootstrap/js/dist/modal";
import "bootstrap/js/dist/tab";

render(<App />, document.body);
