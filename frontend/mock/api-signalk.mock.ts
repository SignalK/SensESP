import { defineMock } from 'vite-plugin-mock-dev-server'

export default defineMock(
  [
    {
      url: '/api/signalk/status',
      delay: 800,
      method: 'GET',
      body: () => {
        const statusChoices = [
          "connected",
          "disconnected",
          "connecting",
          "unknown",
          "unauthorized",
          "error",
          "authenticating",
        ];
        const randomStatus = statusChoices[
          Math.floor(Math.random() * statusChoices.length)
        ];
        // create rx and tx deltas with random values between 0 and 100000
        const randomRxDeltas = Math.floor(Math.random() * 100000);
        const randomTxDeltas = Math.floor(Math.random() * 100000);
        const data = {
          connectionStatus: randomStatus,
          numRxDeltas: randomRxDeltas,
          numTxDeltas: randomTxDeltas,
        };
        return data;
      }

    },
  ]
);
