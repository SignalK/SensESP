
function ajax(method, url, data, contentType) {
  return new Promise(function(resolve, reject) {
    var request = new XMLHttpRequest();

    request.open(method, url, true);

    request.onload = function() {
        if (request.status === 200) {
            resolve(request.response);
        } else {
            reject(Error(request.statusText));
        }
    };

    request.onerror = function() {
      reject(Error("Network Error"));
    };

    if (contentType) {
        request.setRequestHeader("Content-Type", contentType);
    }

    request.send(data);
  });
}            


class TreeList {
    
    constructor(main, pathList) {

      this.main = main;
      this.main.appendChild(document.createElement('div'));
      this.root = document.createElement('ul');
      this.root.id = 'tree';
      this.main.appendChild(this.root);

      // Build the tree structure out of the configuration list
      for (var i = 0; i < pathList.length; i++) {
         var entry = pathList[i];
         var fullPath = entry;
         var parts = entry.split('/');
         var nodeName = parts.pop();
         var section = this.main;
         parts.shift(); // Get rid of first blank entry (due to forward slash)...
         while (parts.length > 0) {
            var sectionName = parts.shift();
            section = this.findNode(section, sectionName);
         }
         this.addEntry(section, nodeName, fullPath);
      }

      // Add section expand/collapse to the "folder" nodes
      var toggler = document.getElementsByClassName("caret");
      for (var i = 0; i < toggler.length; i++) {
          toggler[i].addEventListener("click", function() {
              this.parentElement.querySelector(".nested").classList.toggle("active");
              this.classList.toggle("caret-down");
          });
      }

    }
   
    makeSectionNode(name) {
        var section = document.createElement('li');
        var caret = document.createElement('span');
        caret.className = 'caret';
        caret.innerHTML = name;
        section.appendChild(caret);
        var ul = document.createElement('ul');
        ul.className = 'nested';
        section.appendChild(ul);
        return section;
    }
   
    addEntry(section, name, fullPath) {
       var entry = document.createElement('li');
       entry.innerHTML = name;
       section.children[1].appendChild(entry);
       entry.className = 'selectable';
       entry.addEventListener('click', function () { editConfig(fullPath); } )
    }
   
    findNode(sectionRoot, nodeName) {
       if (nodeName != '') {
          var i;
          var nextEntry;
          var searchChildren = sectionRoot.children[1].childNodes;
          for (i = 0; i < searchChildren.length; i++) {
             nextEntry = searchChildren[i]
             if (nextEntry.childNodes[0].textContent == nodeName) {
                return nextEntry;
             }
          }
          // If we get here, the node does not yet exist. Add it...
          nextEntry = this.makeSectionNode(nodeName);
          sectionRoot.children[1].appendChild(nextEntry);
          return nextEntry;
       }
      else {
          return sectionRoot;
      }
    }
  
 }
 
var globalEditor = null;

function getEmptyMountDiv() {

    var main = document.getElementById("mountNode");
    main.empty();
    globalEditor = null;
    return main;
}


function editConfig(config_path) {

    var main = getEmptyMountDiv();

    ajax('GET', '/config' + config_path)

    .then(response => {

        var json = JSON.parse(response);
        var config = json.config;
        var schema = json.schema;

        if (Object.keys(schema).length == 0) {
            alert(`No schema available for ${config_path}`);
            showConfigTree();
            return;
        }

        if (!schema.title) {
            schema.title = `Configuration for ${config_path}`;
        }

        main.innerHTML = `
        <div class='row'>
        <div id='editor_holder' class='medium-12 columns'></div>        
        </div>
        <div class='row'>
        <div class='medium-12-columns'>
        <button id='submit' class='tiny'>Save</button>
        <button id='cancel' class='tiny'>Cancel</button>
        <span id='valid_indicator' class='label'></span>
        </div>
        </div>
      `;

        globalEditor = new JSONEditor(document.getElementById('editor_holder'), 
                                        {   
                                        'schema': schema, 
                                        'startval': config,
                                        'no_additional_properties': true,
                                        'disable_collapse': true,
                                        'disable_properties': true,
                                        'disable_edit_json': true,
                                        'show_opt_in': true,
                                        });        

        document.getElementById('submit').addEventListener('click',function() {
            saveConfig(config_path, globalEditor.getValue());
        });

        document.getElementById('cancel').addEventListener('click',function() {
            showConfigTree();
        });
      
      // Hook up the validation indicator to update its 
      // status whenever the editor changes
      globalEditor.on('change',function() {
        // Get an array of errors from the validator
        var errors = globalEditor.validate();
        
        var indicator = document.getElementById('valid_indicator');
        
        // Not valid
        if(errors.length) {
          indicator.className = 'label alert';
          indicator.textContent = 'not valid';
        }
        // Valid
        else {
          indicator.className = 'label success';
          indicator.textContent = 'valid';
        }
      })
    })
    .catch(err => {
        alert(`Error retrieving configuration ${config_path}: ${err.message}`);
        showConfigTree();
    });

}


function saveConfig(config_path, values) {

    ajax('PUT', '/config' + config_path, JSON.stringify(values), 'application/json')
    .then(response => {
        showConfigTree();
    })
    .catch(err => {
        alert(`Error saving configuration ${config_path}: ${err.message}`);
        showConfigTree();
    });
}


Element.prototype.empty = function() {
    var child = this.lastElementChild;  
    while (child) { 
       this.removeChild(child); 
       child = this.lastElementChild; 
    }
}


function showConfigTree() {

    var main = getEmptyMountDiv();

    ajax('GET', '/config')
    .then(response => {
        var json = JSON.parse(response);
        var configList = json.keys;
        configList.sort();
        return configList;
    })
    .then(configList => {

        var treeList = new TreeList(main, configList);

    })
    .catch(err => {
        alert('Error: ' + err.statusText);
    });

}
