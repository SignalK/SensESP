import { JsonObject } from 'common/jsonTypes';
import { defineMock } from 'vite-plugin-mock-dev-server'

let hostname = "localhost";
let authenticationSettings: JsonObject = {
  "authEnabled": false,
  "username": "",
  "password": ""
};

export default defineMock(
  [
    {
      url: '/api/config',
      method: 'GET',
      body: {
        keys: [
          "/System/WiFi Settings",
          "/System/Signal K Settings",
          "/Transforms/Linear"
        ]
      },
      delay: 800,
    },
    {
      url: '/api/config/System/Signal K Settings',
      method: 'GET',
      body: {
        "config": {
          "sk_address": "oppi4.local",
          "sk_port": 3000,
          "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJkZXZpY2UiOiI0NTMxODliOC04NzNkLTkxMzUtNDFjYy1jYzFjMzg4ZWU1YzUiLCJpYXQiOjE2NTA4OTg3MjB9.5ENWogKknEbHf9Ppc-xl5zgJvBrzy0WcIH0tyymcfVg",
          "client_id": "453189b8-873d-9135-41cc-cc1c388ee5c5",
          "polling_href": ""
        },
        "description": ""
      },
      delay: 600,
    },
    {
      url: '/api/config/system/hostname',
      response(req, resp) {
        if (req.method === 'GET') {
          const doc = {
            "config": {
              "value": hostname
            },
            "schema": {},
            "description": "The hostname of the device."
          };
          resp.end(JSON.stringify(doc));
        } else if (req.method === 'POST') {
          const doc = req.body;
          console.log(doc);
          hostname = doc.value;
          resp.end();
        }
      }
    },
    {
      url: '/api/config/system/authentication',
      response(req, resp) {
        if (req.method === 'GET') {
          const doc = {
            "config": authenticationSettings,
            "schema": {},
            "description": "Auth."
          };
          resp.end(JSON.stringify(doc));
        } else if (req.method === 'POST') {
          const doc = req.body;
          console.log(doc);
          authenticationSettings = doc;
          resp.end();
        }
      }
    },
    {
      url: '/api/config/System/WiFi Settings',
      method: 'GET',
      body: {
        "config": {
          "apSettings": {
            "enabled": false,
            "name": "My New Network",
            "password": "mypassword",
            "channel": "Auto",
            "hidden": false
          },
          "clientSettings": {
            "enabled": true,
            "settings": [
              {
                name: "Hat Labs Sensors",
                password: "mypassword",
                useDHCP: true,
                ipAddress: "",
                netmask: "",
                gateway: "",
                dnsServer: "",
              },
              {
                name: "Network 11",
                password: "mypassword2",
                useDHCP: false,
                ipAddress: "10.34.21.2",
                netmask: "255.255.255.0",
                gateway: "10.34.21.1",
                dnsServer: "10.34.21.1",
              },
              {
                name: "My Phone Hotspot",
                password: "mypassword3",
                useDHCP: true,
                ipAddress: "",
                netmask: "",
                gateway: "",
                dnsServer: "",
              },
            ]
          }

        },
        "description": ""
      },
      delay: 1200,
    },
    {
      url: '/api/config/Transforms/Linear',
      method: 'GET',
      body: {
        "config": {
          "offset": 0,
          "multiplier": 1,
          "value": 80.20713806
        },
        "schema": {
          "type": "object",
          "properties": {
            "multiplier": {
              "title": "Multiplier",
              "type": "number",
              "readOnly": false
            },
            "offset": {
              "title": "Constant offset",
              "type": "number",
              "readOnly": false
            },
            "value": {
              "title": "Last value",
              "type": "number",
              "readOnly": true
            }
          }
        },
        "description": "Linear transform custom description"
      },
      delay: 400,
    },
  ]
)
