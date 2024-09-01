import federation from "@originjs/vite-plugin-federation";
import preact from "@preact/preset-vite";
import { defineConfig } from "vite";
import mockDevServerPlugin from 'vite-plugin-mock-dev-server';
import tsconfigPaths from 'vite-tsconfig-paths';

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    preact(),
    mockDevServerPlugin(
      {
        // prefix: '^/config',
        log: 'debug',
        // include: 'mock/**/*.mock.{ts,js,cjs,mjs,json,json5}',
      }
    ),
    tsconfigPaths(),
    federation({
      name: 'host-app',
      // A static dummy remote is required or otherwise the dynamic
      // federation won't work.
      remotes: {
        dummy: "dummy.js",
      },
      shared: [ 'preact', 'bootstrap' ]
    })
  ],
  server: {
    proxy: {
      '^/api': { target: 'http://example.com' },
    }
  },
  build: {
    minify: true,
    target: 'esnext',
    // rollupOptions: {
    //   // https://rollupjs.org/configuration-options/
    //   plugins: [
    //     brotli(),
    //   ],
    // },
  }
});
