class View{
    loadError(){
        document.getElementById("loading_text").innerHTML = "WebSocket failed to connect.<br> Please refresh the page to try again.";
    }

    loadSuccess(){
        document.getElementById("loading_content").style.display = "none";
        document.getElementById("main_content").style.display = "block";
    }

    showSyncControls(){
        document.getElementById("initialise_options").style.display = "none";

        document.getElementById("sync_text").style.display = "initial";
        document.getElementById("control_title").innerText = "Sync with Camera";

        this.setButtonState("prev_control", "prev_control_bttn", false);
        this.setButtonState("next_control", "next_control_bttn", true);
    }

    waitForSync(){
        this.setButtonState("next_control", "next_control_bttn", false);

        document.getElementById("sync_status").innerText = "...";
        document.getElementById("sync_status").style = "color:white";
    }

    showInitialiseControls(){
        document.getElementById("sync_status").innerText = "Synced";
        document.getElementById("sync_status").style = "color: limegreen";
        document.getElementById("sync_text").style.display = "none";

        document.getElementById("initialise_options").style.display = "flex";
        document.getElementById("control_title").innerText = "Initialise Camera";

        this.setButtonState("prev_control", "prev_control_bttn", true);
    }

    waitForInit(){
        this.setButtonState("prev_control", "prev_control_bttn", false);
        this.setButtonState("next_control", "next_control_bttn", false);
        
        this.setSelectionState("colour_type", false);
        this.setSelectionState("raw_resolutions", false);
        this.setSelectionState("jpeg_resolutions", false);
    }

    setButtonState(div_id, button_id, enable){
        if(enable){
            document.getElementById(button_id).style.pointerEvents = "auto";
            document.getElementById(button_id).disabled = false;
            document.getElementById(div_id).style.opacity = "1";
        }
        else{
            document.getElementById(button_id).style.pointerEvents = "none";
            document.getElementById(button_id).disabled = true;
            document.getElementById(div_id).style.opacity = "0.5";
        }
    }

    setSelectionState(select_id, enable){
        if(enable){
            document.getElementById(select_id).disabled = false;
            document.getElementById(select_id).style.opacity = "1";
        }
        else{
            document.getElementById(select_id).disabled = true;
            document.getElementById(select_id).style.opacity = "0.5";
        }
    }

    addConsoleText(text){
        response_console =  document.getElementById("response_console");
        response_console.value += text;
        console.scrollTop = console.scrollHeight;
    }

    clearElementValue(id){
        document.getElementById(id).value = "";
    }
}