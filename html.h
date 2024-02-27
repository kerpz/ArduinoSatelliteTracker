#ifndef HTML_H
#define HTML_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
    <title>ESP8266</title>
    <meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
    <style>
        body {
            color: #333;
            font-family: Century Gothic, sans-serif;
            font-size: 1em;
            line-height: 24px;
            margin: 0;
            padding: 0;
        }

        /* nav {background: #3861d1;color: #fff;font-size: 1.3em;padding: 15px;text-align: center;margin: auto;display: block;max-width: 600px;} */
        /* nav b {font-size: 1.5em;margin-bottom: 15px;display: block;} */
        nav {
            display: flex;
            position: relative;
            justify-content: space-between;
            align-items: center;
            background-color: #333;
            color: #fff;
            padding: 5px;
            max-width: 600px;
            margin: auto;
        }

        nav b {
            font-size: 1.5rem;
            margin: .5rem;
        }

        .nav-links {
            height: 100%;
        }

        .nav-links ul {
            display: flex;
            margin: 0;
            padding: 0;
        }

        .nav-links li {
            list-style: none;
        }

        .nav-links li a {
            display: block;
            text-decoration: none;
            color: #fff;
            padding: 1rem;
        }

        .nav-links li:hover {
            background-color: #555;
        }

        .toggle-button {
            position: absolute;
            top: .75rem;
            right: 1rem;
            display: none;
            flex-direction: column;
            justify-content: space-between;
            width: 30px;
            height: 21px;
        }

        .toggle-button .bar {
            height: 3px;
            width: 100%;
            background-color: #fff;
            border-radius: 10px;
        }

        @media (max-width: 800px) {
            nav {
                flex-direction: column;
                align-items: flex-start;
            }

            .toggle-button {
                display: flex;
            }

            .nav-links {
                display: none;
                width: 100%;
            }

            .nav-links ul {
                width: 100%;
                flex-direction: column;
            }

            .nav-links ul li {
                text-align: center;
            }

            .nav-links ul li a {
                padding: .5rem 1rem;
            }

            .nav-links.active {
                display: flex;
            }
        }

        .container {
            max-width: 600px;
            margin: auto;
        }

        /* @media(min-width:1200px){.container{margin: auto;}} */
        /* @media(min-width:768px) and (max-width:1200px){.container{margin: auto;}} */
        textarea,
        input,
        select {
            width: 100%;
            outline: 0;
            border: 1px solid #ccc;
            padding: 10px;
            box-sizing: border-box;
        }

        input[type='checkbox'] {
            float: left;
            width: 20px;
        }

        textarea:focus,
        input:focus,
        select:focus {
            border-color: rgb(188, 194, 196);
        }

        button {
            background: rgb(68, 71, 72);
            border-radius: 4px;
            cursor: pointer;
            transition-duration: 0.4s;
            border: 0;
            color: #fff;
            display: block;
            margin: 2px 0;
            padding: 10px 0px 11px;
            width: 100%;
            font-size: 1.5em;
            text-align: center;
            text-decoration: none;
        }

        button:hover {
            background: rgb(94, 97, 98);
        }

        button:active,
        button:focus {
            background: rgb(83, 85, 86);
        }

        button:disabled {
            background: rgb(68, 71, 72);
            opacity: 0.6;
            cursor: not-allowed;
        }

        form>* {
            margin-bottom: 10px;
        }

        .grid-container {
            margin-top: 20px;
            grid-template-columns: auto auto auto auto auto;
            grid-gap: 5px;
            align-items: center;
        }

        .grid-container>div {
            padding: .1em;
            text-align: left;
        }

        .label {
            grid-column: 1 / span 2;
        }

        .item {
            grid-column: 3 / span 3;
        }

        .full {
            grid-column: 1 / span 5;
        }

        .col1 {
            grid-column: 1;
        }

        .col2 {
            grid-column: 2;
        }

        .col3 {
            grid-column: 3;
        }

        .col4 {
            grid-column: 4;
        }

        .col5 {
            grid-column: 5;
        }

        .card {
            background-color: #F8F7F9;
            box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, .5);
            padding: 15px 10px 15px 10px;
            margin: 20px;
        }

        .collapsible {
            background-color: #F8F7F9;
            color: #222;
            width: 100%;
            text-align: left;
            font-size: 16px;
            font-weight: bold;
        }

        .collapsible>span {
            font-size: 16px;
            float: right;
        }

        .collapsible:hover,
        .collapsible:active,
        .collapsible:focus {
            background-color: #F8F7F9;
        }

        .alert {
            padding: 10px;
            font-size: 13px;
            border-radius: 4px;
            background-color: #d8d8d8;
        }
    </style>
</head>

<body>
    <nav>
        <b>ESP8266</b>
        <a href="#" class="toggle-button">
            <span class="bar"></span>
            <span class="bar"></span>
            <span class="bar"></span>
        </a>
        <div class="nav-links">
            <ul>
                <li><a href="#/system">System</a></li>
                <li><a href="#/config">Config</a></li>
                <li><a href="#/app">App</a></li>
                <li><a href="#/firmware">Firmware</a></li>
            </ul>
        </div>
    </nav>
    <div class="container" id="main-content">
    </div>
    <script>
        let test = [{
            label: 'Group 1', name: 'group1', value: 1, elements: [
                { type: 'alert', value: 'Hello World' },
                { type: 'text', label: 'Key', value: 'key', attrib: 'disabled' },
                { type: 'text', label: 'Text', name: 'text', value: 'text' },
                { type: 'hidden', name: 'hidden', value: 'text' },
                { type: 'password', label: 'Password', name: 'password', value: '' },
                { type: 'textarea', label: 'Textarea', name: 'textarea', value: 'Hello World' },
                { type: 'select', label: 'Select', name: 'select', options: [["1", "1"], ["2", "2"]] },
                { type: 'button', label: 'Confirm', name: 'button', confirm: 'Do you want to confirm?', disabled: 'false' },
                { type: 'button', label: 'Disabled', name: 'button', disabled: 'true' }
            ]
        },
        {
            label: 'Group 2', name: 'group2', value: 1, elements: [
                { type: 'arrows' }
            ]
        }
        ]

        // global variables
        let websocket;
        let path = [];
        const baseurl = window.location.hostname || 'tracker.local';
        let _updateInterval = 2000;
        let _reloadTimerId;

        const serialize = (inObj) => {
            let elements = document.querySelectorAll("input, select, textarea"); // exclude button
            let obj = { ...inObj };
            elements.forEach((element) => {
                let name = element.name;
                let value = element.value;
                if (name) obj[name] = value;
            });
            return JSON.stringify(obj);
        }
        const fetchPage = async (page, post, create = true) => {
            let elements = [];
            if (_reloadTimerId) clearTimeout(_reloadTimerId);
            if (page !== 'test') {
                try {
                    const response = await fetch(`http://${baseurl}/${page}`, {
                        method: post ? 'POST' : 'GET',
                        body: post ? post : undefined
                    });
                    elements = await response.json();
                    if (create) createPage(elements);
                    else {
                        elements.forEach((group) => {
                            group.elements.forEach((obj) => {
                                if (obj.type === 'text' && obj.attrib === 'disabled') {
                                    document.getElementById('_' + obj.name).value = obj.value;
                                }
                                else if (obj.type === 'button' && obj.disabled === 'false') {
                                    document.getElementById('_' + obj.name).disabled = false;
                                }
                                if (obj.type === 'refresh') {
                                    _reloadTimerId = setTimeout(() => { fetchPage(path[1], serialize(), false); }, _updateInterval);
                                }
                            });
                        });
                    }
                }
                catch (error) {
                    console.log(error);
                }
            }
        }
        const fileUpload = async (fileInput) => {
            try {
                const formData = new FormData();
                formData.append('file', fileInput.files[0]);
                const response = await fetch(`http://${baseurl}/upload`, {
                    method: 'POST',
                    body: formData
                });
                elements = await response.json();
                createPage(elements);
            }
            catch (error) {
                console.log('error in fileUpload');
            }
        }
        const send = (obj) => {
            /*
            let obj = {
                test: 'hello',
                test2: 'world'
            };
            */
            websocket.send(JSON.stringify(obj));
        }
        const createPage = async (groups) => {
            // UI
            let html = '';
            html += '<form>';
            groups.forEach((group) => {
                html += '<div class="card">';
                html += '<button type="button" class="collapsible" id="_' + group.name + '_button">' + group.label + '<span>' + (group.value ? '-' : '+') + '</span></button>';
                html += '<input type="hidden" name="' + group.name + '" id="_' + group.name + '" value=' + group.value + ' />';
                html += '<div class="grid-container" id="_' + group.name + '_content">';
                group.elements.forEach((obj) => {
                    switch (obj.type) {
                        case 'hidden':
                            html += '<input type="' + obj.type + '" id="_' + obj.name + '" name="' + obj.name + '" value="' + obj.value + '" />';
                            break;
                        case 'text':
                        case 'password':
                        case 'file':
                            html += '<div class="label">' + obj.label + '</div>';
                            html += '<div class="item"><input type="' + obj.type + '" id="_' + obj.name + '" name="' + obj.name + '" value="' + obj.value + '" autocomplete="off" ' + (obj.accept ? ' accept="' + obj.accept + '" ' : '') + (obj.attrib ? obj.attrib : '') + ' /></div>';
                            break;
                        case 'textarea':
                            html += '<div class="label">' + obj.label + '</div>';
                            html += '<div class="item"><textarea rows="4" id="_' + obj.name + '" name="' + obj.name + '">' + obj.value + '</textarea></div>';
                            break;
                        case 'refresh':
                        case 'select':
                            html += '<div class="label">' + obj.label + '</div>';
                            html += '<div class="item"><select name="' + obj.name + '" id="_' + obj.name + '"></select></div>';
                            break;
                        case 'button':
                        case 'submit':
                            html += '<div class="label"></div>';
                            html += '<div class="item"><button type="' + obj.type + '" name="' + obj.name + '" id="_' + obj.name + '" value="' + obj.value + '" ' + (obj.disabled === 'true' ? 'disabled' : '') + '>' + obj.label + '</button></div>';
                            break;
                        case 'arrows':
                            html += '<div class="col3"><button id="_' + obj.name + '_up">U</button></div>';
                            html += '<div class="col2"><button id="_' + obj.name + '_left">L</button></div>';
                            html += '<div class="col3"><button id="_' + obj.name + '_enter">E</button></div>';
                            html += '<div class="col4"><button id="_' + obj.name + '_right">R</button></div>';
                            html += '<div class="col3"><button id="_' + obj.name + '_down">D</button></div>';
                            break;
                        case 'alert':
                            html += '<div class="full"><div class="alert">' + obj.value + '</div></div>';
                            break;
                        default:
                            break;
                    }
                });
                html += '</div>';
                html += '</div>';
            });
            html += '</form>';
            document.getElementById('main-content').innerHTML = html;
            // Scripts
            groups.forEach((group) => {
                group.elements.forEach((obj) => {
                    const el = document.getElementById('_' + obj.name);
                    switch (obj.type) {
                        case 'refresh':
                            // populate options
                            obj.options = [];
                            for (let i = 0; i < 60; i++) {
                                obj.options.push([i, i]);
                            }
                            _updateInterval = parseInt(obj.value) * 1000;
                            group.elements.forEach((obj) => {
                                if (obj.type === 'text' || obj.type === 'textarea' || obj.type === 'select' || obj.type === 'refresh') {
                                    const _el = document.getElementById('_' + obj.name);
                                    _el.addEventListener('focusin', (e) => {
                                        if (_reloadTimerId) clearTimeout(_reloadTimerId);
                                    });
                                    _el.addEventListener('focusout', (e) => {
                                        _reloadTimerId = setTimeout(() => { fetchPage(path[1], serialize(), false); }, _updateInterval);
                                    });
                                }
                            });
                            el.addEventListener('change', (e) => {
                                _updateInterval = e.target.value * 1000;
                                el.blur(); // unfocus
                            });
                            if (_updateInterval > 0) _reloadTimerId = setTimeout(() => { fetchPage(path[1], serialize(), false); }, _updateInterval);
                        case 'select':
                            let opts = '';
                            for (let j = 0; j < obj.options.length; ++j) {
                                let a = obj.options[j];
                                if (a.length == 1) a.push(a[0]);
                                opts += '<option value="' + a[0] + '"' + ((a[0] == obj.value) ? ' selected' : '') + '>' + a[1] + '</option>';
                            }
                            el.innerHTML = opts;
                            break;
                        case 'button':
                            el.addEventListener('click', async (e) => {
                                // e.stopImmediatePropagation();
                                e.preventDefault();
                                el.disabled = true;
                                // violation in ms
                                if (obj.confirm) {
                                    if (!confirm(obj.confirm)) {
                                        return;
                                    }
                                }
                                if (obj.name === 'upload') {
                                    const fileInput = document.querySelector('#_file');
                                    await fileUpload(fileInput);
                                }
                                else {
                                    let inObj = {};
                                    inObj[obj.name] = obj.value;
                                    await fetchPage(path[1], serialize(inObj));
                                }
                            });
                            break;
                        case 'arrows':
                            document.getElementById('_' + obj.name + '_up').addEventListener('click', async (e) => {
                                e.preventDefault();
                                await fetchPage(path[1], '{"' + obj.name + '":"1"}');
                            });
                            document.getElementById('_' + obj.name + '_down').addEventListener('click', async (e) => {
                                e.preventDefault();
                                await fetchPage(path[1], '{"' + obj.name + '":"2"}');
                            });
                            document.getElementById('_' + obj.name + '_left').addEventListener('click', async (e) => {
                                e.preventDefault();
                                await fetchPage(path[1], '{ "' + obj.name + '":"3"}');
                            });
                            document.getElementById('_' + obj.name + '_right').addEventListener('click', async (e) => {
                                e.preventDefault();
                                await fetchPage(path[1], '{"' + obj.name + '":"4"}');
                            });
                            document.getElementById('_' + obj.name + '_enter').addEventListener('click', async (e) => {
                                e.preventDefault();
                                await fetchPage(path[1], '{"' + obj.name + '":"5"}');
                            });
                        default:
                            break;
                    }
                });

                document.getElementById('_' + group.name + '_content').style.display = (group.value) ? 'grid' : 'none';
                document.getElementById('_' + group.name + '_button').addEventListener('click', async (e) => {
                    const newValue = (group.value) ? 0 : 1;
                    document.getElementById('_' + group.name).value = newValue;
                    group.value = newValue;
                    createPage(groups);
                });
            });
        }
        window.addEventListener('load', (e) => {
            /*
            console.log(`WebSocket connection to ws://${baseurl}:81`);
            websocket = new WebSocket(`ws://${baseurl}:81`);
            websocket.onopen = (e) => {
                console.log('onopen');
            };
            websocket.onclose = (e) => {
                console.log('onclose');
                // setTimeout(initWebSocket, 2000);
            };
            websocket.onmessage = (e) => {
                console.log(event.data)
                let obj = JSON.parse(event.data);
            };
            */

            const hash = location.hash.split("#");
            if (hash[1]) {
                path = hash[1].split("/");
                if (path[1]) {
                    if (path[1] === 'test') createPage(test);
                    else if (path[1] !== '') fetchPage(path[1], serialize());
                }
            }
            else {
                window.location = '#/system';
            }
        });
        window.addEventListener("hashchange", (e) => {
            const hash = location.hash.split("#");
            if (hash[1]) {
                path = hash[1].split("/");
                fetchPage(path[1], '{}');
            }
        });

        const toggleButton = document.getElementsByClassName('toggle-button')[0]
        const navLinks = document.getElementsByClassName('nav-links')[0]
        toggleButton.addEventListener('click', () => {
            navLinks.classList.toggle('active')
        });
    </script>
</body>

</html>
)rawliteral";

#endif