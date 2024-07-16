var apiUrl;
var routerId;
var state = [];

function dv(val, def)
{
    if ((val === null) || (val === undefined))
        return def;
    return val;
}

function isSet(val)
{
    if ((val === null) || (val === undefined))
        return false;
    return true;
}

function btnHtml(dir, id, text)
{
    //'<!--<span class="label">Out ' + text +  '</span>-->' +
    return '<span class="btn ' + dir + '" id="span_' + id +'">\n' +
            '<span class="tally" id="tally_' + id +'"><span>&#9679;</span>&nbsp;<span>&#9679;</span>&nbsp;<span>&#9679;</span></span><br>' +
            '<button id="'+id+'">' + text + '</button>\n' +
            '</span>\n';
}

function createButtons(xbar, num_src, num_dst)
{
    var dstButtons = document.getElementById(xbar+"_dst");
    var srcButtons = document.getElementById(xbar+"_src");
    dstButtons.innerHTML="";
    srcButtons.innerHTML="";

    var html='<div class="buttonGrp">';
    for(var i=0;i<num_dst;++i)
    {
        const btn = statusObj.outputs[i];
        if (!isSet(btn))
            continue;

        if (i>1 && (i&7)==0)
            html+='</div><div class=\"buttonGrp\">';

        var label = "Out&nbsp;" + (i + 1);
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

function assignClickEvent(xbar, num_src, num_dst)
{
    for(var i=0;i<num_dst;++i)
    {
        var btn = document.getElementById(xbar + "_dst" + i);
        if (btn===null)
            continue;
        btn.addEventListener('click',
                             fnBtnClickDst.bind(null,
                                                xbar, i
                                                ));
    }
    for(var i=0;i<num_src;++i)
    {
        var btn = document.getElementById(xbar + "_src" + i);
        if (btn===null)
            continue;
        btn.addEventListener('click',
                             fnBtnClickSrc.bind(null,
                                                xbar, i
                                                ));
    }
}

var currentOutput = -1;

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
    for(var i=0;i<nsrc;++i)
    {
        if (statusObj.sources[i].id === id)
            return 1;
    }
    return -1;
}

function selectSource(mtx, src)
{
    var oldSrc = state[xbar].src;
    if(src !== oldSrc)
    {
        var oldBtn = document.getElementById(xbar + "_src" + oldSrc);
        var newBtn = document.getElementById(xbar + "_src" + src);
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
    var oldDst = state[xbar].dst;
    if(dst !== oldDst)
    {
        var oldBtn = document.getElementById(xbar + "_dst" + oldDst);
        var newBtn = document.getElementById(xbar + "_dst" + dst);
        if (oldBtn!==null)
            oldBtn.classList.remove("sel");
        if (newBtn!==null)
            newBtn.classList.add("sel");
        state[xbar].dst = dst;
        console.log("Change destination from " + oldDst + " to " + dst);
    }
    refreshXpoint();
}

function updateActive(buttonId, active)
{
    var container = document.getElementById("span_" + buttonId);
    if (container === null)
        return;
    if (active)
        container.classList.add("act");
    else
        container.classList.remove("act");
}

function updateTally(buttonId, tally)
{
    var leds = document.getElementById("tally_" + buttonId);
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

var nsrc = 0;
var ndst = 0;
var xbar = "p0";
var statusObj = null;

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

    var tmpStatus = {};
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
}



function queryStatusInitial()
{
    var xhr = new XMLHttpRequest();
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
        var dropDown = document.getElementById("panel_select");
        if (dropDown === null)
            return;
        uidList.forEach(function(id)
        {
            var rtrStatus = statusObj.status[id];
            var opt = document.createElement('option');
            var text = id;
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
    var xhr = new XMLHttpRequest();
    xhr.open('GET', apiUrl+"?detail", true);
    xhr.onload = populateDropdown;
    xhr.onerror = function() { setTimeout("loadRouters()",'10000'); console.log("Request failed. Retry in 10s");};
    xhr.send();
}

function initUi() {
    var panel_id="";
    const urlParams = new URLSearchParams(window.location.search);
    routerId = urlParams.get('router');
    var getRequest = new XMLHttpRequest();
    apiUrl = window.location.protocol + "//" + window.location.host + "/router/";


    //clear UI


    state[xbar]= {src:-1, dst:-1};


    //get panel ids
    loadRouters();

    if (routerId !== "") {
        console.log("Querying panel status");
        queryStatusInitial();
    }
}


/*
var cDest = state[xbar].dst;
if (cDest>=0)
{
    selectSource(xbar, statusObj.xpoint[cDest]);
}

//update tallies
return;
var sources = statusObj.sources;
var destinations = statusObj.sources;
var mtx = xbar;
if ( ( undefined !== sources) && ( undefined !== destinations) )
{
    for(var i = 0; i < sources.length; ++i)
    {
        var id = mtx + "_src"+i;
        if (sources[i].tally === undefined)
            break;
        updateTally(id, sources[i].tally);
        updateActive(id, sources[i].active);
    }

    for(var i = 0; i < destinations.length; ++i)
    {
        var id = mtx + "_dst"+i;
        if (sources[i].tally === undefined)
            break;
        updateTally(id, destinations[i].tally);
    }
}
*/

function refreshXpoint() {
    if (routerId === "")
        return;
    var xhr = new XMLHttpRequest();
    xhr.open('GET', apiUrl+routerId+ "/xpoint", true);
    xhr.onerror = function() { setTimeout("refreshXpoint()",'10000'); console.log("Request failed. Retry in 10s");};
    xhr.onload = function()
    {
        if (this.readyState !== 4)
            return;
        if (this.status !== 200)
            return;

        try {
            xpoint = JSON.parse(this.responseText);
        } catch(e)
        {
            console.log("Received invalid JSON. Error: " + e);
            return;
        }
        if (!Array.isArray(xpoint))
        {
            console.log("xpoint uri did not return an array");
            return;
        }
        statusObj.xpoint = xpoint;


        var cDest = state[xbar].dst;
        if (cDest>=0)
        {
            selectSource(xbar, statusObj.xpoint[cDest]);
        }
    };
    xhr.send();
}

function postXPoint(dst, src)
{
    var xhr = new XMLHttpRequest();
    xhr.open('POST', apiUrl+routerId+"/xpoint-set", true);
    var data = new FormData();
    data.append('mtx', xbar);;
    data.append('dst', dst);
    data.append('src', src);
    xhr.onload = refreshXpoint;
    xhr.onerror = function() { console.log("Request failed."); };
    xhr.send(data);
}
