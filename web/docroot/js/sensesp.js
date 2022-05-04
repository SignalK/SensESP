
var activeEditors = [];
var deviceInfo = null;
var currentPage = "info";
var pagesLoaded = false;
var targets = {};

function ajax(method, url, data, contentType) {
    return new Promise(function (resolve, reject) {
        var request = new XMLHttpRequest();

        request.open(method, url, true);

        request.onload = function () {
            if (request.status === 200) {
                resolve(request.response);
            } else {
                reject(Error(request.statusText));
            }
        };

        request.onerror = function () {
            reject(Error("Network Error"));
        };

        if (contentType) {
            request.setRequestHeader("Content-Type", contentType);
        }

        request.send(data);
    });
}

class NavTarget {
    constructor(key, invokeFunction, link) {
        this.Key = key;
        this.TargetFunction = invokeFunction;
        this.Link = link;
    }
}

function getEmptyContentDiv() {
    var main = document.getElementById("content");
    main.empty();
    globalEditor = null;
    return main;
}

function getSingleJsonEditor(element, config, schema) {
    const editor = new JSONEditor(element,
        {
            'schema': schema,
            'startval': config,
            'no_additional_properties': true,
            'disable_collapse': true,
            'disable_properties': true,
            'disable_edit_json': true,
            'show_opt_in': true,
            'theme': 'bootstrap4'
        });

    return editor;
}

function createEmptyConfigEditorDiv(index) {
    const content = `
    <div class='card mb-3'>
    <div class="card-body">
    <h5 id='card_title_${ index }' class="card-title mb-4">Configuration</h5>
    <div id='card_description_${ index }' class='container ps-0 mb-3'></div>
    <div id='editor_holder_${ index }'></div>
    <div class='col-12'>
    <button id='submit_${ index }' type='button' class='btn btn-primary mt-3'>Save</button>
    </div>
    </div>
    </div>
    `;
    return content;
}

async function loadConfig(configPath) {
    let response = await ajax('GET', '/config' + configPath);
    let json = JSON.parse(response);
    let config = json.config;
    let schema = json.schema;
    let description = json.description;

    return {
        'config': config,
        'schema': schema,
        'description': description
    };
}

function loadAllConfigs(configPaths) {
    let configDataPromises = configPaths.map(configPath => {
        return loadConfig(configPath);
    });

    return configDataPromises;
}

function insertLoaderSpinner(elementId) {
    var element = document.getElementById(elementId);
    element.innerHTML = `
    <div id="loader" class="d-flex justify-content-center">
    <div class="spinner-border" role="status">
      <span class="visually-hidden">Loading...</span>
    </div>
    <span id="loadertext" class="ms-2">Loading...</span>
    </div>`;
}

function clearLoaderSpinner(element) {
    element.innerHTML = "";
}

async function showSingleConfigEditor(configPath, configDataPromise, index) {
    let configData = await configDataPromise;
    let schema = configData.schema;
    if (!schema.title) {
        schema.title = " ";
    }
    let element = document.getElementById(`editor_holder_${ index }`);

    clearLoaderSpinner(element);

    let editor = getSingleJsonEditor(element, configData.config, schema);

    const heading = configPath.substring(1);
    const headingElement = document.getElementById(`card_title_${ index }`);
    headingElement.textContent = heading;

    const descriptionElement = document.getElementById(`card_description_${ index }`);
    descriptionElement.innerHTML = configData.description;

    const buttonElement = document.getElementById(`submit_${ index }`);
    buttonElement.addEventListener('click', function () {
        saveConfig(deviceInfo.Config[index], editor.getValue());
    });

    return editor;
}

function saveConfig(config_path, values) {
    ajax('PUT', '/config' + config_path, JSON.stringify(values), 'application/json')
        .then(response => {
            alert(`Saved configuration for ${ config_path } successfully.`);
        })
        .catch(err => {
            alert(`Error saving configuration ${ config_path }: ${ err.message }`);
        });
}

function showConfig() {
    let main = getEmptyContentDiv();

    // Get the number of config paths
    const numConfigs = deviceInfo.Config.length;

    // load all configs
    let configDataPromises = loadAllConfigs(deviceInfo.Config);

    // create a new card div for each config path

    const cardsHTML = Array.from(Array(numConfigs), (x, i) => {
        return createEmptyConfigEditorDiv(i);
    }).join("\n");

    // insert the card divs into the main div
    main.innerHTML = cardsHTML;

    // insert spinners into each card div

    for (let i = 0; i < numConfigs; i++) {
        insertLoaderSpinner(`editor_holder_${ i }`);
    }

    // Create JSON editors for each config path
    for (let i = 0; i < numConfigs; i++) {
        showSingleConfigEditor(deviceInfo.Config[i], configDataPromises[i], i)
            .then(editor => {
                activeEditors.push(editor);
            });
    }
}

Element.prototype.empty = function () {
    var child = this.lastElementChild;
    while (child) {
        this.removeChild(child);
        child = this.lastElementChild;
    }
}

function executeCommand(name, shouldConfirm) {
    if (shouldConfirm == true && !confirm("Execute " + name + "?")) {
        return;
    }

    showLoader(true, "Executing " + name + "...");

    ajax("GET", "/command?id=" + name)
        .then((r) => {
            showLoader(false);
            alert(r);
        })
        .catch(err => {
            showLoader(false);
            alert(err);
        });
}

function runDeviceCommand(confirmText, command) {
    if (confirm(confirmText)) {
        ajax("GET", "/device/" + command)
            .then(r => {
                alert(r);
            })
            .catch(err => {
                alert(err);
            });
    }
}

function showControl() {
    var div = getEmptyContentDiv();

    var commands = deviceInfo.Commands;
    var appCommands = "";
    if (commands.length > 0) {
        appCommands = `<h5 class="card-title my-4">Application commands</h5>
        <div class="btn-group">`;
        for (var i = 0; i < commands.length; i++) {
            var command = commands[i];
            appCommands += `<§ class="btn btn-primary" onclick="executeCommand('${ command.Name }',${ command.Confirm })" href="#">${ command.Title }</a>`;
        }
        appCommands += "</div>";
    }

    var content = `
    <div class='card'>
        <div class="card-body">
        <h5 class="card-title mb-4">Device commands</h5>
        <a href="#" onclick='runDeviceCommand("Restart device?", "restart");' class="btn btn-primary">Restart</a>
        <a href="#" onclick='runDeviceCommand("Are you sure you want to reset device to factory settings?", "reset");' class="btn btn-danger">Reset to defaults</a>
        </div>
    </div>`;

    var commands = deviceInfo.Commands;

    if (commands.length > 0) {
        content +=
            "<div class='card mt-3'>" +
            "<div class='card-body'>" +
            "<h5 class='card-title mb-4'>Custom commands</h5>";

        for (var i = 0; i < commands.length; i++) {
            var command = commands[i];
            content += `<a class="btn btn-primary" onclick="executeCommand('${ command.Name }',${ command.Confirm })" href="#">${ command.Title }</a>`;
        }

        content += "</div></div>";

    }

    div.innerHTML = content;
}

function showCustom(target) {
    showLoader(true);
    ajax("GET", target.Link)
        .then(v => {
            showLoader(false);
            var div = getEmptyContentDiv();

            div.innerHTML = "<h1>" + target.Key + "</h1>" + v;
        })
        .catch(err => {
            showLoader(false);
            alert(err);
        });
}

function loadInfo() {
    ajax("GET", "/info")
        .then(response => {
            deviceInfo = JSON.parse(response);
            var content = getEmptyContentDiv();
            var groups = {};

            for (const property in deviceInfo.Properties) {
                var propertyObj = deviceInfo.Properties[property];
                propertyObj.Name = property;
                var group = null;
                if (groups[propertyObj.Group] == undefined) {
                    group = groups[propertyObj.Group] =
                    {
                        "Name": propertyObj.Group,
                        "Properties": []
                    };
                }
                else {
                    group = groups[propertyObj.Group];
                }

                group.Properties.push(propertyObj);

                if (property === "Name") {
                    document.getElementById("devicename").innerHTML = propertyObj.Value;
                    document.title = propertyObj.Value + " - WebUI";
                }
            }

            for (const key in groups) {
                var group = groups[key];
                group.Properties.sort((a, b) => a.Order - b.Order);

                content.innerHTML += `<div class='col'><h3>${ group.Name }</h3>`;

                for (i = 0; i < group.Properties.length; i++) {
                    content.innerHTML += `<div class='ms-2 mb-3'><label class='form-label'>${ group.Properties[i].Name }</label><input type='text' readonly class='form-control' value='${ group.Properties[i].Value }'></div>`;
                }

                content.innerHTML += "</div>";
            }

        })
        .catch(err => {
            alert('Device info load failed: ' + err.statusText);
        });
}

function initialize() {
    loadInfo();
    targets["status"] = new NavTarget("status", loadInfo);
    targets["configuration"] = new NavTarget("configuration", showConfig);
    targets["control"] = new NavTarget("control", showControl);


    var mainMenu = document.getElementById("mainmenu");

    mainMenu.addEventListener("click", (e) => {
        var targetKey = e.target.dataset["target"];
        var target = targets[targetKey];
        if (target != undefined) {
            target.TargetFunction(target);
        }
    });
}
