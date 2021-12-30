# SensESP GitHub Pages Documentation

This directory contains the GitHub Pages website for SensESP.

## Rendering the Documentation Locally

If you just want to read the documentation, you don't need to run it on your computer - just come to this website. But if you do need to be able to render the HTML documentation locally:

1. You must have Docker installed and running.

2. You must clone the SensESP GitHub repository to your computer.

3. From /SensESP/docs on your computer, build the Docker container with the following command:

    docker/build

4. Each time you want to bring up the local copy of the site, go to /SensESP/docs and run the following command to start the Dockerized Jekyll server:

    docker/jekyll

5. Finally, point your browser to http://0.0.0.0:4000/SensESP/ and you should see a local copy of the site.
