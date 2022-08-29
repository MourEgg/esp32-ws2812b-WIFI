const char htmlCode[] = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body {
                font-family: Arial, Helvetica, sans-serif;
            }
            @media screen and (max-width: 576px) {
            .container {
                    width: 100%;
                }
            }
            @media screen and (min-width: 576px) {
            .container {
                    max-width: 540px;
                }
            }
            @media screen and (min-width: 768px) {
                .container {
                    max-width: 720px;
                }
            }
            @media screen and (min-width: 992px) {
                .container {
                    max-width: 960px;
                }
            }
            @media screen and (min-width: 1200px) {
            .container {
                    max-width: 1140px;
                }
            }
            .container {
                margin-left: auto;
                margin-right: auto;
                display: flex;
                flex-direction: column;
                align-items: center;
            }
            .inputs {
                display: flex;
                flex-direction: column;
                align-items: center;
                max-width: 400px;
                width: 100%;
            }
            .btn {
                color: white;
                background-color: #0d6efd;
                border-color: #0d6efd;
                padding: 1rem 1rem;
                font-size: 1.25rem;
                border-radius: .3rem;
                margin-top: 20px;
                width: 100%;
                border: none;
            }
            .btn--close {
                margin-top: 0;
                width: auto;
                margin-left: auto;
                margin-right: 0;
            }
            .btn--set {
                width: auto;
                margin-top: 0;
            }
            .colorInput {
                height: 65px;
                width: 100%;
            }
            .slider {
                width: 100%;
            }
            .input-group {
                display: flex;
                justify-content: space-between;
                width: 100%;
            }
            .input-group .btn {
                width: 45%;
            }
            .modal {
                display: none;
                position: absolute;
                top: 0;
                left: 0;
                width: 100%;
                height: 100%;
                background-color: rgba(0, 0, 0, 0.8);
            }
            .modal--visible {
                display: initial;
            }
            .modal__dialog {
                max-width: 500px;
                margin: 1.75rem auto;
            }
            .modal__content {
                position: relative;
                display: flex;
                flex-direction: column;
                pointer-events: auto;
                background-color: #fff;
                background-clip: padding-box;
                border: 1px solid rgba(0,0,0,.2);
                border-radius: 0.3rem;
                outline: 0;
                padding: 20px;
            }
            .input-row {
                display: flex;
                width: 100%;
                justify-content: space-between;
                align-items: center;
                margin: 30px 0;
            }
            p {
                margin-bottom: 5px;
            }
            h1 {
                font-size: 50px;
                margin-top: 30px;
            }
            .btn:hover {
                background-color: #0b5ed7;
                border-color: #0b5ed7;
            }
            @media screen and (max-width: 768px) {
                .inputs {
                    width: 100%;
                    }
                .btn {
                    padding: 3rem 3rem;
                    margin-top: 40px;
                }
                .btn--close, .btn--set {
                    padding: 0.5rem 2rem;
                    margin-top: 0;
                }
                h1 {
                    display: none;
                }
                .number-input {
                    max-width: 60px;
                }
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>LED Strip Control panel</h1>
            <button id="settings" class="btn btn--close" type="button">Settings</button>
            <div class="inputs">
                <p>Color Input</p>
                <input id="colorInput" class="colorInput" type="color" value="#ff0000">
                <p>Brightness</p>
                <input type="range" min="0" max="255" value="255" class="slider" id="slider">
                <p>Speed</p>
                <input type="range" min="1" max="100" value="50" class="slider" id="speed">
                <button id="white" class="btn btn-send" type="button">White</button>
                <div class="input-group">
                    <button id="sinelon" class="btn btn-send" type="button">Sinelon</button>
                    <button id="rainbow" class="btn btn-send" type="button">Rainbow</button>
                </div>
                <button id="off" class="btn btn-send" type="button">ON/OFF</button>
            </div>
        </div>
        <div id="modal" class="modal">
            <div class="modal__dialog">
                <div class="modal__content">
                    <div class="input-row">
                        <button id="info" type="button" class="btn btn--set">Info</button>
                        <button id="close" type="button" class="btn btn--close">X</button>
                    </div>
                    <div class="input-row">
                        <label for="time">Alarm time</label>
                        <input id="time" type="time">
                    </div>
                    <div class="input-row">
                        <label for="offset">Start before(min)</label>
                        <input class="number-input" id="offset" type="number" placeholder="15min">
                    </div>
                    <div class="input-row">
                        <label for="weekdays">Alarm on weekdays</label>
                        <input id="weekdays" type="checkbox">  
                    </div>
                    <div class="input-row">
                        <button type="button" class="btn btn--set" id="disable-once">Disable Once</button>
                        <button type="button" class="btn btn--set" id="alarm-set">Set alarm</button>
                    </div>
                </div>
            </div>
        </div>
    </body>



    <script>
    var colorInput = document.getElementById("colorInput");
    var off = document.getElementById("off");
    var buttons = document.getElementsByClassName("btn-send");
    var slider = document.getElementById("slider");
    var speed = document.getElementById("speed");
    var settings = document.getElementById("settings");
    var close = document.getElementById("close");
    var disableOnce = document.getElementById("disable-once");
    var info = document.getElementById("info");

    var setAlarm = document.getElementById("alarm-set");

    colorInput.addEventListener("input", throttle(getColor, 30), false);
    slider.addEventListener("input", throttle(getBrightness, 30), false);
    speed.addEventListener("input", throttle(getSpeed, 30), false);
    settings.addEventListener("click", toggleModal);
    close.addEventListener("click", toggleModal);
    info.addEventListener("click", getInfo);


    setAlarm.addEventListener("click", sendAlarmSettings);

    for (var i = 0; i < buttons.length; i++) {
        buttons[i].addEventListener('click', function(e) {
            let id = e.target.id;
            sendGet(id);
        }, false);
    }

    function getColor (){
    var theColor = colorInput.value;
    sendGet(`color/${theColor.substring(1)}`);
    }

    function sendAlarmSettings(){
        let weekdays = document.getElementById("weekdays").checked;
        let time = document.getElementById("time").value;
        let offset = document.getElementById("offset").value;
        if (offset == "") {
            offset = 15;
        }
        if (weekdays == true) {
            weekdays = 1;
        }
        else {
            weekdays = 0;
        }
        sendGet(`time/${time}`);
        sendGet(`offset/${offset}`);
        sendGet(`weekdays/${weekdays}`);
    }

    function getInfo() {
        var client = new XMLHttpRequest();
        client.open("GET", "/info", true);
        client.send();

        client.onreadystatechange = function() {
            if(this.readyState == this.HEADERS_RECEIVED) {
                var currTime = client.getResponseHeader("CurrentTime");
                var alarmTime = client.getResponseHeader("AlarmTime");
                var offsetTime = client.getResponseHeader("OffsetTime");
                let weekdays = client.getResponseHeader("WeekdaysAlarm");
                let retWeekdays = weekdays == 1 ? "True" : "False";
                alert("Current Time: " + currTime + "\nAlarm Time: " + alarmTime + "\nOffset Time: " + offsetTime + " (0 = alarm off)" + "\nAlarm on weekdays: " + retWeekdays);
            }
        }
    }

    function toggleModal(){
        let modal = document.getElementById("modal")
        if(modal.classList.contains("modal--visible")) {
            modal.classList.remove("modal--visible");
        }
        else {
            modal.classList.add("modal--visible");
        }
    }

    function getBrightness(){
        var brightness = slider.value;
        sendGet(`brightness/${brightness}`);
    }

    function getSpeed(){
        let lspeed = speed.value;
        sendGet(`speed/${lspeed}`);
    }

    function sendGet(value) {
        var xmlHttp = new XMLHttpRequest();
        xmlHttp.open( "GET", `/${value}`, true ); // false for synchronous request
        xmlHttp.send( null );
    }

    function throttle (callback, limit) {
        var waiting = false;                      // Initially, we're not waiting
        return function () {                      // We return a throttled function
            if (!waiting) {                       // If we're not waiting
                callback.apply(this, arguments);  // Execute users function
                waiting = true;                   // Prevent future invocations
                setTimeout(function () {          // After a period of time
                    waiting = false;              // And allow future invocations
                }, limit);
            }
        }
    }
    </script>
</html>
)=====";