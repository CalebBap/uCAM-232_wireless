var socket = new WebSocket('ws://10.100.0.200:81');
socket.onmessage = function(event){
    addConsoleText(event.data);
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

function prev(){
    addConsoleText("Previous button pressed\n");
}

function next(){
    addConsoleText("Next button pressed\n");
}

function addConsoleText(text){
    console =  document.getElementById("response_console");
    console.value += text;
    console.scrollTop = console.scrollHeight;
}

function clearTerminal(){
    document.getElementById("response_console").value = "";
}