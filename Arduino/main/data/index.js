var socket = new WebSocket('ws://10.100.0.200:81');
socket.onmessage = function(event){
	document.getElementById("responseConsole").value += event.data;
}

window.addEventListener('beforeunload', function (e) {
	socket.close();
});

/*
function syncCmd(){
    socket.send("#sync");
    document.getElementById("syncButton").disabled=true;
}
*/

function next(){
    document.getElementById("response_console").value += "Next button pressed\n";
}