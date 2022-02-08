
var globalEditor = null;
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

function editConfig(config_path) {

    var main = getEmptyContentDiv();

    showLoader(true, "Loading configuration from device...");

    ajax('GET', '/config' + config_path)

        .then(response => {

            showLoader(false);
            var json = JSON.parse(response);
            var config = json.config;
            var schema = json.schema;

            if (Object.keys(schema).length == 0) {
                alert(`No schema available for ${config_path}`);
                return;
            }

            var description = "";
            if(json.description != undefined && json.description != "")
            {
                description = `<div class='container'>${json.description}</div>`
            }

            main.innerHTML = `<div class='row g-3 m-4'>
            <h1>Configuration for ${config_path}</h1>
        ${description}
        <div id='editor_holder'></div>
        <div class='col-12'>
        <button id='submit' type='button' class='btn btn-primary'>Save</button>
        <span id='valid_indicator' class='label ms-5'></span>
        </div>
        </div>`;

            globalEditor = new JSONEditor(document.getElementById('editor_holder'),
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

            document.getElementById('submit').addEventListener('click', function () {
                saveConfig(config_path, globalEditor.getValue());
            });

            // Hook up the validation indicator to update its 
            // status whenever the editor changes
            globalEditor.on('change', function () {
                // Get an array of errors from the validator
                var errors = globalEditor.validate();
                var indicator = document.getElementById('valid_indicator');

                // Not valid
                if (errors.length) {
                    indicator.className = 'text-danger';
                    indicator.textContent = 'not valid';
                }
                // Valid
                else {
                    indicator.className = 'text-success';
                    indicator.textContent = 'valid';
                }
            })
        })
        .catch(err => {
            showLoader(false);
            alert(`Error retrieving configuration ${config_path}: ${err.message}`);
        });

}


function saveConfig(config_path, values) {
    showLoader(true, "Saving configuration...");
    ajax('PUT', '/config' + config_path, JSON.stringify(values), 'application/json')
        .then(response => {
            showLoader(false);
        })
        .catch(err => {
            showLoader(false);
            alert(`Error saving configuration ${config_path}: ${err.message}`);
        });
}


Element.prototype.empty = function () {
    var child = this.lastElementChild;
    while (child) {
        this.removeChild(child);
        child = this.lastElementChild;
    }
}

function showLoader(show, status) {
    var loader = document.getElementById("loader");
    if (show) {
        if (status == null || status == undefined) {
            status = "Loading...";
        }

        loadertext.innerHTML = status;
        loader.classList.remove("visually-hidden");
    }
    else {
        loader.classList.add("visually-hidden");
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
            appCommands += `<a class="btn btn-primary" onclick="executeCommand('${command.Name}',${command.Confirm})" href="#">${command.Title}</a>`;
        }
        appCommands += "</div>";
    }
    var content = `
    <div class='card'>
        <div class="card-body">
        <h5 class="card-title my-4">Device commands</h5>
        <p>These commands allows you to restart device or reset its configuration.</p>
        <div class="btn-group">
        <a href="#" onclick='runDeviceCommand("Restart device?", "restart");' class="btn btn-primary">Restart</a>
        <a href="#" onclick='runDeviceCommand("Are you sure you want to reset device to factory settings?", "reset");' class="btn btn-danger">Reset to defaults</a>
        </div>
        ${appCommands}
        </div>
    </div>`;

    

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
                if(groups[propertyObj.Group] == undefined)
                {
                    group = groups[propertyObj.Group] = 
                    {
                        "Name" : propertyObj.Group,
                        "Properties" : []
                    };
                }
                else
                {
                    group = groups[propertyObj.Group];
                }

                group.Properties.push(propertyObj);

                if (property === "Name") {
                    document.getElementById("devicename").innerHTML = propertyObj.Value;
                    document.title = propertyObj.Value + " - WebUI";
                }
            }

            for(const key in groups)
            {
                var group = groups[key];
                group.Properties.sort((a,b) => a.Order - b.Order);

                content.innerHTML += `<div class='col'><h3>${group.Name}</h3>`;

                for(i = 0; i < group.Properties.length; i++)
                {
                    content.innerHTML += `<div class='ms-2 mb-3'><label class='form-label'>${group.Properties[i].Name}</label><input type='text' readonly class='form-control' value='${group.Properties[i].Value}'></div>`;
                }

                content.innerHTML += "</div>";
            }

            if (!pagesLoaded) {
                pagesLoaded = true;
                var mainMenu = document.getElementById("mainmenu");

                for (const page in deviceInfo.Pages) {
                    var link = deviceInfo.Pages[page];
                    var navItem = document.createElement("li");
                    navItem.innerHTML = "<a href='#' data-target='" + page + "' class='nav-link' aria-current='page'>" + page + "</a></li>";
                    mainMenu.append(navItem);
                    targets[page] = new NavTarget(page, showCustom, link);
                }

                var configmenu = document.getElementById("configmenu");
                var lastRoot = null;
                for (var i = 0; i < deviceInfo.Config.length; i++) {
                    var configPath = deviceInfo.Config[i];
                    var parts = configPath.split("/");
                    var rootName = parts[1];
                    rootName = rootName[0].toUpperCase() + rootName.substring(1);
                    if (lastRoot == null || lastRoot != rootName) {
                        if (lastRoot != null) {
                            configmenu.innerHTML += "<li><hr class='dropdown-divider'></li>";
                        }
                        configmenu.innerHTML += "<li class='d-flex align-items-center mb-1'><svg class='ms-2' width='16' height='16' viewBox='0 0 32 32'><use xlink:href='#i-settings'/></svg>&nbsp;<span class='text-center'>" + rootName + "</span></li>";
                        lastRoot = rootName;
                    }
                    var linkName = parts[parts.length - 1];
                    linkName = linkName[0].toUpperCase() + linkName.substring(1);
                    configmenu.innerHTML += "<li><a class='dropdown-item d-flex align-items-center' onclick='editConfig(\"" + configPath + "\");' href='#'><span class='d-inline-block bg-primary rounded-circle' style='width: .5em;height:0.5em'></span>&nbsp;" + linkName + "</a></li>";
                }

                var dropDown = document.getElementById("configdrop");
                dropDown.addEventListener("click", () => {
                    if (configmenu.classList.contains("show")) {
                        configmenu.classList.remove("show");
                    }
                    else {
                        configmenu.classList.add("show");
                    }
                })
            }

            showLoader(false);
        })
        .catch(err => {
            alert('Device info load failed: ' + err.statusText);
        });
}

function initialize() {
    loadInfo();
    targets["status"] = new NavTarget("status", loadInfo);
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