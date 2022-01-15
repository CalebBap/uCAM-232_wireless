class View{
    showSyncControls(){
        document.getElementById("prev_control_bttn").style.pointerEvents = "none";
        document.getElementById("prev_control_bttn").disabled = true;
        document.getElementById("prev_control").style.opacity = "0.5";
    }

    showInitialControls(){
        document.getElementById("prev_control_bttn").style.pointerEvents = "auto";
        document.getElementById("prev_control_bttn").disabled = false;
        document.getElementById("prev_control").style.opacity = "1";
    }

    addConsoleText(text){
        console =  document.getElementById("response_console");
        console.value += text;
        console.scrollTop = console.scrollHeight;
    }

    clearTerminal(){
        document.getElementById("response_console").value = "";
    }
}