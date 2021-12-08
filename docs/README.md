# SensESP GitHub Pages documentation

This directory contains the GitHub Pages website for SensESP.

## Rendering the documentation locally

If you want to render the HTML documentation locally, first build the Docker container with the following command:

    docker/build

Then run the following command to start the Dockerized Jekyll server:

    docker/jekyll

Then, navigate to http://0.0.0.0:4000/SensESP in your browser and you should reach a local copy of the documentation site.
