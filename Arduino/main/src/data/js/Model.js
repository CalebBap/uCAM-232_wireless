class Model{
    constructor(){
        this.socket = null;
        this.fsm_state = 0;
    }

    connectWebSocket(){
        this.socket = new WebSocket('ws://10.100.0.200:81');

        var timer = setTimeout(function() {
            view.loadError();
        }, 5000);

        this.socket.onerror = function(event){
            clearTimeout(timer);
            view.loadError();
        }

        this.socket.onopen = function(event){
            clearTimeout(timer);
            view.loadSuccess();
        }

        this.socket.onmessage = function(event){
            controller.handleWebSocketMessage(event.data);
        }
    }

    closeWebSocket(){
        this.socket.close();
    }

    syncCmd(){
        try{
            this.socket.send("#sync");
        }
        catch(error){
            console.error(error);
        }
    }

    initCmd(){
        let colour_type = document.getElementById("colour_type").value;
        let res_resolution = document.getElementById("raw_resolutions");
        let jpeg_resolution = document.getElementById("jpeg_resolutions");

        let resolution = (colour_type === "J") ? jpeg_resolution.value : res_resolution.value;

        try{
            this.socket.send("#init:" + colour_type + "," + resolution);
        }
        catch(error){
            console.error(error);
        }
    }

    initialiseOptionSelection(colour_value, raw_res_value, jpeg_res_value){
        let all_options_selected = false;

        if( (colour_value === "") || (colour_value === undefined) ){
            view.setSelectionState("raw_resolutions", false);
            view.clearElementValue("raw_resolutions");

            view.setSelectionState("jpeg_resolutions", false);
            view.clearElementValue("jpeg_resolutions");

            all_options_selected = false;
        }
        else if(colour_value === "J"){
            view.setSelectionState("jpeg_resolutions", true);

            view.setSelectionState("raw_resolutions", false);
            view.clearElementValue("raw_resolutions");

            all_options_selected = (jpeg_res_value !== "");
        }
        else{
            view.setSelectionState("raw_resolutions", true);

            view.setSelectionState("jpeg_resolutions", false);
            view.clearElementValue("jpeg_resolutions");

            all_options_selected = (raw_res_value !== "");
        }

        if(all_options_selected){
            view.setButtonState("next_control", "next_control_bttn", true);
        }
        else{
            view.setButtonState("next_control", "next_control_bttn", false);
        }
    }

    fsm(value){
        this.fsm_state += value;
        if(this.fsm_state < 0){
            this.fsm_state = 0;
        }

        const nextBttnPressed = value == 1;
    
        switch(this.fsm_state){
            case 0:
                view.showSyncControls();
                break;
            case 1:
                if(nextBttnPressed){
                    this.syncCmd();
                    view.waitForSync();
                }
                else{
                    view.showSyncControls();
                }
                break;
            case 2:
                view.showInitialiseControls();
                this.initialiseOptionSelection();
                break;
            case 3:
                if(nextBttnPressed){
                    this.initCmd();
                    view.waitForInit();
                }
                else{
                    this.fsm(-1);
                }
                break;
            case 4:
                view.showSnapshotControls();
                break;
            case 5:
                view.showGetPictureControls();
                break;
        }
    }
}