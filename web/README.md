# SensESP Mock Web Environment

This directory is for development and testing of the HTML and Javascript code that is utilized in the
configuration UI of SensESP.  

# Setting up mock development environment

If you have a need to make changes to the SensESP web UI, configure a web server on your machine
to point to the "docroot" directory in this web directory.  You can then make web requests as normal
and test or modify the behavior of the Javascript without the need to communicate with an actual
sensor running SensESP.

Once you are done, you can run the "webUI" build target to convert the required files to C header files
that can be used by the SensESP source code.


# Rebuilding for deployment

Rebuilding the Javascript files requires the use of the NodeJS utility Terser.  Install it using
the follownig command:

`npm install terser -g`


To build a new version of the web UI, issue the following command in your PlatformIO terminal:

`pio run -t webUI`

For information on how this build target works, see the project file extra_script.py

# Files used by WebUI

The following files converted to header files for use by the SensESP web UI.  See the 
src/net/web directory for the results of this conversion.

1. docroot/index.html
1. docroot/js/jsoneditor.min.js
1. docroot/js/sensesp.js
1. docroot/setup/index.html

The other directories contain mock files that would be returned by the MCU to a browser making the request.
