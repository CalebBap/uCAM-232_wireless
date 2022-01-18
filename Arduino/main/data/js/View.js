class View{
    loadError(){
        document.getElementById("loading_text").innerHTML = "WebSocket failed to connect.<br> Please refresh the page to try again.";
    }

    loadSuccess(){
        document.getElementById("loading_content").style.display = "none";
        document.getElementById("main_content").style.display = "initial";
    }

    showSyncControls(){
        document.getElementById("initial_options").style.display = "none";

        document.getElementById("sync_text").style.display = "initial";

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
        document.getElementById("sync_status").innerText = "Synced";
        document.getElementById("sync_status").style = "color: limegreen";
        document.getElementById("sync_text").style.display = "none";

        document.getElementById("initial_options").style.display = "initial";

        document.getElementById("prev_control_bttn").style.pointerEvents = "auto";
        document.getElementById("prev_control_bttn").disabled = false;
        document.getElementById("prev_control").style.opacity = "1";
        
        document.getElementById("next_control_bttn").style.pointerEvents = "auto";
        document.getElementById("next_control_bttn").disabled = false;
        document.getElementById("next_control").style.opacity = "1";
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