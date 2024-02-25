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
            outline: 0;
            border: 1px solid #ccc;
            padding: 10px;
            width: 100%;
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
            display: inline-block;
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

        label>* {
            display: inline;
        }

        form>* {
            display: block;
            margin-bottom: 10px;
        }

        /* .msg{background:#def;border-left:5px solid #59d;padding: 1.5em;} */
        table {
            display: table;
            table-layout: fixed;
            width: 100%;
        }

        table td {
            padding: .5em;
            text-align: left;
        }

        table tbody>:nth-child(2n-1) {
            background: #ddd;
        }

        table th {
            padding: .6em;
            background: #aaa;
            text-align: left;
        }

        .card {
            background-color: #F8F7F9;
            box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, .5);
            padding: 15px 10px 15px 10px;
            margin: 20px;
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
                <li><a href="#/tracker">Tracker</a></li>
                <li><a href="#/app">App</a></li>
                <li><a href="#/firmware">Firmware</a></li>
            </ul>
        </div>
    </nav>
    <div class="container" id="main-content">
    </div>
    <script>
        /*
        let elements = [
          { type: 'text', label:'Text (without value)', name: 'test1', value:'' },
          { type: 'text', label:'Text (with value)', name: 'test2', value:'Test' },
          { type: 'password', label:'Password', name: 'test3', value:'' },
          { type: 'textarea', label:'Textarea', name: 'test4', value:'Hello World' },
          { type: 'select', label:'Select', name: 'test5', options:[["1","1"],["2","2"]] },
          { type: 'button', label:'Confirm', name: 'test6', confirm: 'Do you want to confirm?' disabled: 'false'},
          { type: 'table', label:'Table', name: 'test7', head: ["N","N","N"], data:[["1","1","1"],["2","2","2"],["3","3","3"]] }
        ]
        */

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
        const fetchPage = async (page, post) => {
            let elements = [];
            if (_reloadTimerId) clearTimeout(_reloadTimerId);
            if (page) {
                try {
                    const response = await fetch(`http://${baseurl}/${page}`, {
                        method: post ? 'POST' : 'GET',
                        body: post ? post : undefined
                    });
                    elements = await response.json();
                    createPage(elements);
                }
                catch (error) {
                    console.log('error in fetch');
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
                console.log('error in upload');
                console.log(error);
            }
        }
        const createPage = async (elements) => {
            // UI
            let html = '';
            html += '<div class="card">';
            html += '<form>';
            elements.forEach((obj) => {
                switch (obj.type) {
                    case 'hidden':
                        html += '<input type="' + obj.type + '" id="_' + obj.name + '" name="' + obj.name + '" value="' + obj.value + '" />';
                        break;
                    case 'text':
                    case 'password':
                    case 'file':
                        html += '<p>';
                        html += '<label for="_' + obj.name + '">' + obj.label + '</label>';
                        html += '<input type="' + obj.type + '" id="_' + obj.name + '" name="' + obj.name + '" value="' + obj.value + '" autocomplete="off" ' + obj.attrib + ' />';
                        html += '</p>';
                        break;
                    case 'textarea':
                        html += '<p>';
                        html += '<label for="_' + obj.name + '">' + obj.label + '</label>';
                        html += '<textarea rows="4" id="_' + obj.name + '" name="' + obj.name + '">' + obj.value + '</textarea>';
                        html += '</p>';
                        break;
                    case 'refresh':
                    case 'select':
                        html += '<p>';
                        html += '<label for="_' + obj.name + '">' + obj.label + '</label>';
                        html += '<select name="' + obj.name + '" id="_' + obj.name + '">';
                        html += '</select>';
                        html += '</p>';
                        break;
                    case 'button':
                    case 'submit':
                        html += '<p>';
                        html += '<button type="' + obj.type + '" name="' + obj.name + '" id="_' + obj.name + '" value="' + obj.value + '" ' + (obj.disabled === 'true' ? 'disabled' : '') + '>' + obj.label + '</button>';
                        html += '</p>';
                        break;
                    case 'table':
                        html += '<p>';
                        html += '<table>';
                        if (obj.head) {
                            html += '<thead>';
                            for (let j = 0; j < obj.head.length; ++j) {
                                html += '<th>' + obj.head[j] + '</th>';
                            }
                            html += '</thead>';
                        }
                        html += '<tbody>';
                        for (let i = 0; i < obj.data.length; ++i) {
                            html += '<tr>';
                            for (let j = 0; j < obj.data[i].length; ++j) {
                                html += '<td><span>' + obj.data[i][j] + '</span></td>';
                            }
                            html += '</tr>';
                        }
                        html += '</tbody>';
                        if (obj.foot) {
                            html += '<tfoot>';
                            for (let j = 0; j < obj.foot.length; ++j) {
                                html += '<td>' + obj.foot[j] + '</td>';
                            }
                            html += '</tfoot>';
                        }
                        html += '</table>';
                        html += '</p>';
                        break;
                    case 'title':
                        html += '<h3>' + obj.value + '</h3>';
                        break;
                    case 'alert':
                        html += '<p>' + obj.value + '</p>';
                        break;
                    case 'arrows':
                        html += '<p>';
                        html += '<table>';
                        html += '<tbody>';
                        html += '<tr>';
                        html += '<td></td><td></td>';
                        html += '<td><button id="_' + obj.name + '_up">U</button></td>';
                        html += '<td></td><td></td>';
                        html += '</tr>';
                        html += '<tr>';
                        html += '<td></td>';
                        html += '<td><button id="_' + obj.name + '_left">L</button></td>';
                        html += '<td><button id="_' + obj.name + '_enter">E</button></td>';
                        html += '<td><button id="_' + obj.name + '_right">R</button></td>';
                        html += '<td></td>';
                        html += '</tr>';
                        html += '<tr>';
                        html += '<td></td><td></td>';
                        html += '<td><button id="_' + obj.name + '_down">D</button></td>';
                        html += '<td></td><td></td>';
                        html += '</tr>';
                        html += '</tbody>';
                        html += '</table>';
                        html += '</p>';
                    default:
                        break;
                }
            });
            html += '</form>';
            html += '</div>';
            document.getElementById('main-content').innerHTML = html;
            // Scripts
            let opts = '';
            elements.forEach((obj) => {
                switch (obj.type) {
                    case 'refresh':
                        // populate options
                        obj.options = [];
                        for (let i = 0; i < 60; i++) {
                            obj.options.push([i, i]);
                        }
                        elements.forEach((obj) => {
                            if (obj.type === 'text' || obj.type === 'textarea' || obj.type === 'select' || obj.type === 'refresh') {
                                document.getElementById('_' + obj.name).addEventListener('focusin', (e) => {
                                    if (_reloadTimerId) clearTimeout(_reloadTimerId);
                                });
                                document.getElementById('_' + obj.name).addEventListener('focusout', (e) => {
                                    _reloadTimerId = setTimeout(() => { fetchPage(path[1], serialize()); }, _updateInterval);
                                });
                            }
                        });
                        document.getElementById('_' + obj.name).addEventListener('change', (e) => {
                            _updateInterval = e.target.value * 1000;
                            document.getElementById('_' + obj.name).blur(); // unfocus
                        });
                        if (_updateInterval > 0) _reloadTimerId = setTimeout(() => { fetchPage(path[1], serialize()); }, _updateInterval);
                    case 'select':
                        let opts = '';
                        for (let j = 0; j < obj.options.length; ++j) {
                            let a = obj.options[j];
                            if (a.length == 1) a.push(a[0]);
                            opts += '<option value="' + a[0] + '"' + ((a[0] == obj.value) ? ' selected' : '') + '>' + a[1] + '</option>';
                        }
                        document.getElementById('_' + obj.name).innerHTML = opts;
                        break;
                    case 'button':
                        document.getElementById('_' + obj.name).addEventListener('click', async (e) => {
                            // e.stopImmediatePropagation();
                            e.preventDefault();
                            document.getElementById('_' + obj.name).disabled = true;
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
        }
        window.addEventListener('load', (e) => {
            /*
            console.log(`WebSocket connection to ws://${baseurl}:81`);
            websocket = new WebSocket(`ws://${baseurl}:81`);
            websocket.onopen = (e) => {
                elements = [
                  { type: 'title', value: 'Information' },
                  { type: 'alert', value: 'Websocket connected to the device!' }
                ]
                createPage(elements);
            };
            websocket.onclose = (e) => {
                elements = [
                  { type: 'title', value: 'Information' },
                  { type: 'alert', value: 'Websocket failed to connect to the device!' }
                ]
                createPage(elements);
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
                if (path[1] && path[1] !== '') {
                    fetchPage(path[1], serialize());
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
        function send() {
            let obj = {
                test: 'hello',
                test2: 'world'
            };
            websocket.send(JSON.stringify(obj));
        }
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