document.onload = function init(){
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    Socket.onmessage = function(event){
        document.getElementById("responseConsole").value += event.data;
    }
}

function syncCmd(){
    Socket.send("#sync");
    document.getElementById("syncButton").disabled=true;
}