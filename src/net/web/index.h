#include <pgmspace.h>
const char PAGE_index[] PROGMEM = R"=====(
<html>
<head>
<title>SensESP SignalK Sensor</title>
</head>
<body>
<h1>SensESP SignalK Sensor</h1>
Your options:
<ul>
<li><a href="/info">Device information</a></li>
<li><a href="/setup">Configure device</a></li>
<li><a href="/device/restart" onclick="return confirm('Restart the device?')">Restart sensor</a></li>
<li><a href="/device/reset" onclick="return confirm('Are you sure you want to reset device to factory settings?')">Reset sensor</a></li>
</ul>
</body>
</html>

)=====";
