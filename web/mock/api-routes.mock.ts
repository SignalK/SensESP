import { defineMock } from 'vite-plugin-mock-dev-server'
import { readFile } from 'fs/promises'

export default defineMock(
  [
    {
      url: '/api/routes',
      method: 'GET',
      body: [
        { name: "Status", path: "/status", componentName: "StatusPage" },
        { name: "System", path: "/system", componentName: "SystemPage" },
        { name: "WiFi", path: "/wifi", componentName: "WiFiConfigPage" },
        { name: "Signal K", path: "/signalk", componentName: "SignalKPage" },
        { name: "Configuration", path: "/configuration", componentName: "ConfigurationPage" },
        { name: "SensESP Plugin", path: "/sensesp-plugin/sensesp-plugin", componentName: "SensESPPluginPage", loadPath: "/api/plugins/sensesp-plugin/sensesp-plugin.js" },
      ]
      ,
      delay: 100,
    },
    {
      url: '/api/plugins/sensesp-plugin/sensesp-plugin.js',
      method: 'GET',
      headers: {
        'Content-Type': 'application/javascript'
      },
      body: await readFile('../webui-plugins/sample-plugin-js/dist/assets/sensesp-plugin.js', 'utf-8'),
    },
    {
      url: '/api/plugins/sensesp-plugin/__federation_fn_import.js',
      method: 'GET',
      headers: {
        'Content-Type': 'application/javascript'
      },
      body: await readFile('../webui-plugins/sample-plugin-js/dist/assets/__federation_fn_import.js', 'utf-8'),
    },
    {
      url: '/api/plugins/sensesp-plugin/__federation_shared_preact-8_xvI4no.js',
      method: 'GET',
      headers: {
        'Content-Type': 'application/javascript'
      },
      body: await readFile('../webui-plugins/sample-plugin-js/dist/assets/__federation_shared_preact-8_xvI4no.js', 'utf-8'),
    },
    {
      url: '/api/plugins/sensesp-plugin/preload-helper-xR9xMGMN.js',
      method: 'GET',
      headers: {
        'Content-Type': 'application/javascript'
      },
      body: await readFile('../webui-plugins/sample-plugin-js/dist/assets/preload-helper-xR9xMGMN.js', 'utf-8'),
    },
    {
      url: '/api/plugins/sensesp-plugin/__federation_expose_SensESPPlugin-jc4KNWOO.js',
      method: 'GET',
      headers: {
        'Content-Type': 'application/javascript'
      },
      body: await readFile('../webui-plugins/sample-plugin-js/dist/assets/__federation_expose_SensESPPlugin-jc4KNWOO.js', 'utf-8'),
    }
  ]
)
