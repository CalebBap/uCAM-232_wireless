try{
    var socket = new WebSocket('ws://10.100.0.200:81');
}
catch(error){
    console.error(error);
}

socket.onmessage = function(event){
    addConsoleText(event.data);
}

window.addEventListener('load', function (e) {
    fsm(0);
});

window.addEventListener('beforeunload', function (e) {
	try{
        socket.close();
    }
    catch(error){
        console.error(error);
    }
});

function syncCmd(){
    try{
        socket.send("#sync");
    }
    catch(error){
        console.error(error);
    }
}

function prev(){
    fsm_state -= 1;
}

function next(){
    fsm_state += 1;
}

function addConsoleText(text){
    console =  document.getElementById("response_console");
    console.value += text;
    console.scrollTop = console.scrollHeight;
}

function clearTerminal(){
    document.getElementById("response_console").value = "";
}

function fsm(value){
    if ( typeof fsm.fsm_state == 'undefined' ) {
        fsm.fsm_state = 0;
    }

    fsm.fsm_state += value;

    if(fsm.fsm_state < 0){
        fsm.fsm_state = 0;
    }

    switch(fsm.fsm_state){
        case 0:
            document.getElementById("prev_control_bttn").style.pointerEvents = "none";
            document.getElementById("prev_control_bttn").disabled = true;
            document.getElementById("prev_control").style.opacity = "0.5";
            break;
        case 1:
            syncCmd();
            document.getElementById("prev_control_bttn").style.pointerEvents = "auto";
            document.getElementById("prev_control_bttn").disabled = false;
            document.getElementById("prev_control").style.opacity = "1";
            break;
    }
}