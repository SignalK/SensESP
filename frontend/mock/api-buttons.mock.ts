import { defineMock } from "vite-plugin-mock-dev-server";

export default defineMock([
  {
    url: "/api/buttons",
    method: "GET",
    body: [
      { name: "reset-trip", title: "Reset Trip Counter", mustConfirm: false },
      { name: "clear-alarms", title: "Clear All Alarms", mustConfirm: true },
    ],
    delay: 100,
  },
  {
    url: "/api/buttons/:name",
    method: "POST",
    body: { status: "ok" },
    delay: 500,
  },
]);
