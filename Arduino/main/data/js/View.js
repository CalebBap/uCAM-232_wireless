class View{
    showSyncControls(){
        document.getElementById("prev_control_bttn").style.pointerEvents = "none";
        document.getElementById("prev_control_bttn").disabled = true;
        document.getElementById("prev_control").style.opacity = "0.5";
    }

    waitForSync(){
        document.getElementById("next_control_bttn").style.pointerEvents = "none";
        document.getElementById("next_control_bttn").disabled = true;
        document.getElementById("next_control").style.opacity = "0.5";
        document.getElementById("sync_status").innerText = "...";
        document.getElementById("sync_status").style = "color:white";
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