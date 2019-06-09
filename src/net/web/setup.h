#include <pgmspace.h>
const char PAGE_setup[] PROGMEM = R"=====(
<html>
    <head>
        <title>Configure SensESP Sensor</title>

<style>
/* Remove default bullets */
ul, #tree {
  list-style-type: none;
}

/* Remove margins and padding from the parent ul */
#tree {
  margin: 0;
  padding: 0;
}

/* Style the caret/arrow */
.caret, .selectable {
  cursor: pointer; 
  user-select: none; /* Prevent text selection */
}


/* Create the caret/arrow with a unicode, and style it */
.caret::before {
  content: "\25B6";
  color: black;
  display: inline-block;
  margin-right: 6px;
}

/* Rotate the caret/arrow icon when clicked on (using JavaScript) */
.caret-down::before {
  transform: rotate(90deg); 
}

/* Hide the nested list */
.nested {
  display: none;
}

/* Show the nested list when the user clicks on the caret/arrow (with JavaScript) */
.active {
  display: block;
}    
</style>

<script src="/js/jsoneditor.min.js"></script>
<script src="/js/sensesp.js"></script>
</head>

<body onload="showConfigTree()">
<h1>SensESP Sensor</h1>
<div id="mountNode">
</div>
</body>
</html>

)=====";
