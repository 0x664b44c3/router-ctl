var apiUrl;
var wsUrl="";
var routerId;
var state = [];
var refreshTimer = null;
var currentOutput = -1;

//for websocket object
var socket = null;

/* get value of a variable or a default if variable is null or undefined */
function dv(val, def)
{
    if ((val === null) || (val === undefined))
        return def;
    return val;
}

/* get value of a variable or a default if variable is null or undefined */
function isSet(val)
{
    if ((val === null) || (val === undefined))
        return false;
    return true;
}

//maybe move this out of here, this is UI view code
function btnHtml(dir, id, text)
{
    //'<!--<span class="label">Out ' + text +  '</span>-->' +
    return '<span class="btn ' + dir + '" id="span_' + id +'">\n' +
            '<span class="tally" id="tally_' + id +'"><span>&#9679;</span>&nbsp;<span>&#9679;</span>&nbsp;<span>&#9679;</span></span><br>' +
            '<button id="'+id+'">' + text + '</button>\n' +
            '</span>\n';
}

//maybe move this out of here, this is UI view code
function createButtons(xbar, num_src, num_dst)
{
    let dstButtons = document.getElementById(xbar+"_dst");
    let srcButtons = document.getElementById(xbar+"_src");
    dstButtons.innerHTML="";
    srcButtons.innerHTML="";

    let html='<div class="buttonGrp">';
    for(let i=0;i<num_dst;++i)
    {
        const btn = statusObj.outputs[i];
        if (!isSet(btn))
            continue;

        if (i>1 && (i&7)==0)
            html+='</div><div class=\"buttonGrp\">';

        let label = "Out&nbsp;" + (i + 1);
        if (isSet(btn.label) && btn.label !== "")
            label = btn.label;

        html+=btnHtml("dst", xbar + "_dst"+i, label);
    }
    html+="</div>";

    dstButtons.innerHTML = html;

    html='<div class="buttonGrp">';

    for(i=0;i<num_src;++i)
    {
        const btn = statusObj.inputs[i];
        if (!isSet(btn))
            continue;

        label = "In&nbsp;" + (i + 1);
        if (isSet(btn.label) && btn.label !== "")
            label = btn.label;

        if (i>1 && (i&7)==0)
            html+='</div><div class=\"buttonGrp\">';

        html+=btnHtml("src", xbar + "_src"+i, label);
    }
    html+="</div>";
    srcButtons.innerHTML = html;
}

//maybe move this out of here, this is UI logic
function assignClickEvent(xbar, num_src, num_dst)
{
    for(let i=0;i<num_dst;++i)
    {
        let btn = document.getElementById(xbar + "_dst" + i);
        if (btn===null)
            continue;
        btn.addEventListener('click',
                             fnBtnClickDst.bind(null,
                                                xbar, i
                                                ));
    }

    for(i=0;i<num_src;++i)
    {
        btn = document.getElementById(xbar + "_src" + i);
        if (btn===null)
            continue;
        btn.addEventListener('click',
                             fnBtnClickSrc.bind(null,
                                                xbar, i
                                                ));
    }
}

function fnBtnClickDst(mtx, key)
{
    //we can always safely do this
    selectSource(mtx, -1);
    selectDest(mtx, key);

    if (!isSet(statusObj))
        return;
    const xpoint = statusObj.xpoint;
    if (!Array.isArray(xpoint))
        return;
    selectSource(mtx, xpoint[key]);
    console.log("XBar: " + mtx + "\nDest: " +  + key);
}

function fnBtnClickSrc(mtx, key)
{
    if (!isSet(statusObj))
    {
        console.log("Warning: status obj not set");
        return;
    }
    const xpoint = statusObj.xpoint;
    if (!Array.isArray(xpoint))
    {
        console.log("xpoint status array not set");

    }
    selectSource(mtx, key);
    statusObj.xpoint[state[mtx].dst] = key;
    postXPoint(state[xbar].dst, key);
    console.log("XBar: " + mtx + "\nSource: " +  + key);
}

function sourceIdx(id)
{
    for(let i=0;i<nsrc;++i)
    {
        if (statusObj.sources[i].id === id)
            return 1;
    }
    return -1;
}

function selectSource(mtx, src)
{
    let oldSrc = state[xbar].src;
    if(src !== oldSrc)
    {
        let oldBtn = document.getElementById(xbar + "_src" + oldSrc);
        let newBtn = document.getElementById(xbar + "_src" + src);
        if (oldBtn!==null)
            oldBtn.classList.remove("sel");
        if (newBtn!==null)
            newBtn.classList.add("sel");
        state[xbar].src = src;
        console.log("Change source from " + oldSrc + " to " + src);
    }
}

function selectDest(mtx, dst)
{
    let oldDst = state[xbar].dst;
    if(dst !== oldDst)
    {
        let oldBtn = document.getElementById(xbar + "_dst" + oldDst);
        let newBtn = document.getElementById(xbar + "_dst" + dst);
        if (oldBtn!==null)
            oldBtn.classList.remove("sel");
        if (newBtn!==null)
            newBtn.classList.add("sel");
        state[xbar].dst = dst;
        console.log("Change destination from " + oldDst + " to " + dst);
    }
    const url = new URL(location.href);
    url.searchParams.set("dst", dst);

    // Same as `location.replace` without reloading.
    history.replaceState(null, '', url);
    refreshXpoint();
}

function updateActive(buttonId, active)
{
    let container = document.getElementById("span_" + buttonId);
    if (container === null)
        return;
    if (active)
        container.classList.add("act");
    else
        container.classList.remove("act");
}

function updateTally(buttonId, tally)
{
    let leds = document.getElementById("tally_" + buttonId);
    if (leds === undefined)
        return;
    if (leds === null)
        return;

    leds.classList.remove("red");
    leds.classList.remove("green");
    leds.classList.remove("both");

    tally = tally & 3;

    switch(tally)
    {
    case 1:
        leds.classList.add("red");
        break;
    case 2:
        leds.classList.add("green");
        break;
    case 3:
        leds.classList.add("both");
        break;
    default:
        break;
    }
}

let nsrc = 0;
let ndst = 0;
let xbar = "p0";
let statusObj = null;

function clearUi()
{
    nsrc = 0;
    ndst = 0;
    xbar = "p0";
    createButtons(xbar, nsrc,ndst);
}

function initPanel()
{
    if (this.readyState !== 4)
        return;

    let tmpStatus = {};
    if (this.status !== 200)
    {
        console.log("backend error");
        return;
    }

    try {
        tmpStatus  = JSON.parse(this.responseText);
    } catch(e)
    {
        console.log("Error: " + e);
        return;
    }

    if (typeof tmpStatus.status === 'object' && tmpStatus.status !== null)
    {
        statusObj = tmpStatus.status[routerId];
    }


    if (!isSet(statusObj.inputs))
        return;
    if (!isSet(statusObj.outputs))
        return;

    //initialize the UI
    clearUi();

    nsrc = dv(statusObj.inputs.length, 0);
    ndst = dv(statusObj.outputs.length, 0);
    createButtons(xbar, nsrc, ndst);

    assignClickEvent(xbar, nsrc, ndst);

    if (refreshTimer !== null)
        clearInterval(refreshTimer);
    refreshTimer = setInterval(refreshXpoint, 2500);

    //reselect the last selected destination upon page reload
    const urlParams = new URLSearchParams(window.location.search);
    let sel_dst = urlParams.get('dst');
    console.log(sel_dst);
    if (isSet(sel_dst))
        selectDest(xbar, parseInt(dv(sel_dst, -1)));
}



function queryStatusInitial()
{
    let xhr = new XMLHttpRequest();
    xhr.open('GET', apiUrl+"?detail", true);
    xhr.onload = initPanel;
    xhr.onerror = function() { setTimeout("queryStatus()",'10000'); console.log("Request failed. Retry in 10s");};
    xhr.send();
}

//initialize the router selection dropdown
function populateDropdown()
{
    if (this.readyState !== 4)
        return;
    if (this.status === 200)
    {
        try {
            statusObj = JSON.parse(this.responseText);
        } catch(e)
        {
            console.log("Received invalid JSON. Error: " + e);
            return;
        }
        const uidList = statusObj.uids;

        if (!Array.isArray(uidList))
        {
            console.log("uid list not present");
            return;
        }
        let dropDown = document.getElementById("panel_select");
        if (dropDown === null)
            return;
        uidList.forEach(function(id)
        {
            let rtrStatus = statusObj.status[id];
            let opt = document.createElement('option');
            let text = id;
            if (rtrStatus !== null) {
                if (typeof rtrStatus.label === 'string') {
                    const label = rtrStatus.label;
                    if(label.length)
                        text = label;
                }
            }
            opt.value = id;
            opt.innerHTML = text;
            dropDown.appendChild(opt);
        }
        );
        dropDown.value = routerId;
        dropDown.addEventListener(
             'change',
             function() { this.form.submit(); },
             false
          );
        console.log(statusObj);
    }
    else
    {
        console.log("Could not load router list.");
    }
}

//load router list
function loadRouters()
{
    let xhr = new XMLHttpRequest();
    xhr.open('GET', apiUrl+"?detail", true);
    xhr.onload = populateDropdown;
    xhr.onerror = function() { setTimeout("loadRouters()",'10000'); console.log("Request failed. Retry in 10s");};
    xhr.send();
}


function onWsMessage(event) {

    console.log("Message from server ", event.data);

    //if we have a valid subscription response we can diable the periodic update refreshTimer
    // if (refreshTimer !== null)
    //     clearInterval(refreshTimer);
}

function wsSubscribeSingleRouter( router )
{
    if (!socket)
        return;
    if (socket.readyState !== WebSocket.OPEN)
    {
        return;
    }

    let msg={
        action: "subscribe",
        router: router,
        clear: true
    };
    socket.send(JSON.stringify(msg));
}

function onWsConnected()
{
    console.log("WS connection established.");
    socket.onclose = function()
    {
        console.log("WS Closed. Trying to reconnect");
        // establishWsConnection();
    };

    wsSubscribeSingleRouter(routerId);
}

//try to connect to the websocket interface
function establishWsConnection()
{
    //clean up socket if already existing
    if (socket !== null) {
        socket.onclose=function(){};
        socket.close();
        delete socket;
    }

    try {
        socket = new WebSocket(wsUrl);
    } catch(e) {
        console.log("Could not create websocket object. Error: " + e);
        socket = null;
    }
    if (!socket) {
        return;
    }
    console.log("Got a websocket, attaching event listeners");

    //we should have a socket at this point

    socket.addEventListener("open", (event) => {console.log("WS conn."); onWsConnected();});
    socket.addEventListener("error", (event) => {console.log("WS Error", event); });

    // Listen for messages
    socket.addEventListener("message", (event) => {onWsMessage(event)});
}



function initUi() {
    let panel_id="";
    const urlParams = new URLSearchParams(window.location.search);

    routerId = urlParams.get('router');
    let baseUrl = window.location.protocol + "//" + window.location.host;

    const apiEndpoint = "router";

    apiUrl = baseUrl + "/" + apiEndpoint;


    let wsProtocol = 'ws://';
    if (window.location.protocol === 'https:') {
        wsProtocol = 'wss://';
    }
    wsUrl = wsProtocol + window.location.host + "/ws/" + apiEndpoint;


    //clear UI


    state[xbar]= {src:-1, dst:-1};


    //get panel ids
    loadRouters();

    if (routerId !== "") {
        console.log("Querying panel status");
        queryStatusInitial();
        establishWsConnection();
    }
}

function refreshXpoint() {
    if (routerId === "")
        return;
    let xhr = new XMLHttpRequest();
    let onlineBox = document.getElementById("onlinebox");
    let alarmBox = document.getElementById("alarm");
    let panelStatus = {};
    xhr.open('GET', apiUrl+"/"+routerId, true);
    xhr.onerror = function() {
        setTimeout("refreshXpoint()",'10000'); console.log("Request failed. Retry in 10s");
        if (isSet(onlineBox))
        {
            onlineBox.innerHTML = "OFFLINE";
            onlineBox.classList.remove("green");
            onlineBox.classList.add("red");
        }
        if (isSet(alarmBox))
        {
            alarmBox.classList.remove("red");
            alarmBox.classList.remove("green");
        }
    };
    xhr.onload = function()
    {
        if (this.readyState !== 4)
            return;
        if (this.status !== 200)
            return;

        try {
            panelStatus = JSON.parse(this.responseText);
        } catch(e)
        {
            console.log("Received invalid JSON. Error: " + e);
            return;
        }

        if (isSet(panelStatus.xpoint) && Array.isArray(panelStatus.xpoint))
        {
            statusObj.xpoint = panelStatus.xpoint;
        } else {
            console.log("xpoint uri did not return an array");
            return;
        }

        if (isSet(onlineBox))
        {
            onlineBox.innerHTML = "ONLINE";
            onlineBox.classList.remove("red");
            onlineBox.classList.add("green");
        }
        if (isSet(alarmBox))
        {
            var alm = parseInt(panelStatus.alarm);
            if (alm===0)
            {
                alarmBox.classList.remove("red");
                alarmBox.classList.remove("green");
            } else {
                alarmBox.classList.add("red");
            }

        }

        let cDest = state[xbar].dst;
        if (cDest>=0)
        {
            selectSource(xbar, statusObj.xpoint[cDest]);
        }
    };
    xhr.send();
}

function postXPoint(dst, src)
{
    let xhr = new XMLHttpRequest();
    xhr.open('POST', apiUrl+"/"+routerId+"/xpoint-set", true);
    let data = new FormData();
    data.append('mtx', xbar);;
    data.append('dst', dst);
    data.append('src', src);
    xhr.onload = refreshXpoint;
    xhr.onerror = function() { console.log("Request failed."); };
    xhr.send(data);
}


/*
let cDest = state[xbar].dst;
if (cDest>=0)
{
    selectSource(xbar, statusObj.xpoint[cDest]);
}

//update tallies
return;
let sources = statusObj.sources;
let destinations = statusObj.sources;
let mtx = xbar;
if ( ( undefined !== sources) && ( undefined !== destinations) )
{
    for(let i = 0; i < sources.length; ++i)
    {
        let id = mtx + "_src"+i;
        if (sources[i].tally === undefined)
            break;
        updateTally(id, sources[i].tally);
        updateActive(id, sources[i].active);
    }

    for(let i = 0; i < destinations.length; ++i)
    {
        let id = mtx + "_dst"+i;
        if (sources[i].tally === undefined)
            break;
        updateTally(id, destinations[i].tally);
    }
}
*/
