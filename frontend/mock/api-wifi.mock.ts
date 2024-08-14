import { defineMock } from 'vite-plugin-mock-dev-server'

const network_data = {
  "networks": [
    {
      "ssid": "Boat Network", "rssi": -45, "encryption": 3, "channel": 1, "bssid": "11:22:33:44:55:66"
    },
    {
      "ssid": "Home Network", "rssi": -45, "encryption": 3, "channel": 1, "bssid": "11:22:33:44:55:66"
    },
    {
      "ssid": "Other Network", "rssi": -45, "encryption": 3, "channel": 1, "bssid": "11:22:33:44:55:66"
    },
    {
      "ssid": "Boat Network", "rssi": -46, "encryption": 3, "channel": 1, "bssid": "11:22:33:44:55:66"
    },
    {
      "ssid": "Other Network 2", "rssi": -69, "encryption": 3, "channel": 6, "bssid": "11:22:33:44:55:66"
    },
    {
      "ssid": "Random Network", "rssi": -78, "encryption": 3, "channel": 6, "bssid": "11:22:33:44:55:66"
    },
    {
      "ssid": "Boat Network", "rssi": -78, "encryption": 3, "channel": 6, "bssid": "11:22:33:44:55:66"
    },
    {
      "ssid": "Home Network", "rssi": -78, "encryption": 3, "channel": 6, "bssid": "11:22:33:44:55:66"
    },
    {
      "ssid": "Other Network", "rssi": -79, "encryption": 3, "channel": 6, "bssid": "11:22:33:44:55:66"
    },
    {
      "ssid": "Other Network 3", "rssi": -84, "encryption": 3, "channel": 6, "bssid": "11:22:33:44:55:66"
    }
  ]
};

let scanning = false;
let scan_start_time = 0;

export default defineMock(
  [
    {
      url: '/api/wifi/scan',
      response(req, resp) {
          scanning = true;
          scan_start_time = Date.now();
          // Return status code 202 Accepted with the body "SCAN STARTED".
          resp.statusCode = 202;
          resp.end("SCAN STARTED");
      }
    },
    {
      url: '/api/wifi/scan-results',
      response(req, resp) {
        if (!scanning) {
          // Return 400 Bad Request with the body "SCAN FAILED".
          resp.statusCode = 400;
          resp.end("SCAN FAILED");
        } else {
          // If scanning and less than 3 seconds have passed, return status code 202 Accepted with the body "SCAN RUNNING".
          if (Date.now() - scan_start_time < 3000) {
            resp.statusCode = 202;
            resp.end("SCAN RUNNING");
          } else {
            // If scanning and more than 4 seconds have passed, return status code 200 OK with the body of the scan results.
            // Remove network_data rows with a 30% chance.
            let filtered_data = { ...network_data };
            filtered_data.networks = network_data.networks.filter(() => Math.random() > 0.3);
            scanning = false;
            resp.statusCode = 200;
            resp.setHeader('Content-Type', 'application/json');
            resp.end(JSON.stringify(filtered_data));
          }
        }
      }
    },
  ]
);

