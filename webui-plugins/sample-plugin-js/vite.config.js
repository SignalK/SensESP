import { defineConfig } from 'vite'
import preact from '@preact/preset-vite'
import federation from "@originjs/vite-plugin-federation";
import brotli from "rollup-plugin-brotli";


export default defineConfig({
  plugins: [
    preact(),
    federation({
      name: 'sensesp-plugin',
      filename: 'sensesp-plugin.js',
      // Modules to expose
      exposes: {
        './SensESPPlugin': './src/sensesp-plugin.jsx',
      },
      shared: ['preact', 'bootstrap',]
    })],
  build: {
    modulePreload: false,
    minify: true,
    cssCodeSplit: false,
    target: 'esnext',
    rollupOptions: {
      plugins: [
        brotli(),
      ],
    }
  }
})
