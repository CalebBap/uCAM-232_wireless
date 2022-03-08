class Controller{
    handleWebSocketMessage(data){
        if(data.charAt(0) == '#'){
            controller.handleCommand(data);
        }
        else{
            view.addConsoleText(data);
        }
    }

    handleControlBttn(value){
        model.fsm(value);
    }

    handleCommand(command){
        switch(command){
            case "#sync_failed":
                model.fsm(-1);
                break;
            case "#synced":
                model.fsm(1);
                break;
            case "#init_failed":
                model.fsm(-1);
                break;
            case "#initialised":
                model.fsm(1);
                break;
        }
    }

    handleInitialiseSelection(){
        let colour_value = document.getElementById("colour_type").value;
        let raw_res_value = document.getElementById("raw_resolutions").value;
        let jpeg_res_value = document.getElementById("jpeg_resolutions").value;
        model.initialiseOptionSelection(colour_value, raw_res_value, jpeg_res_value);
    }

    handleSnapshotSelection(){
        let snapshot_checkbox = document.getElementById("get_snapshot_input");

        if(snapshot_checkbox.checked){
            view.showSnapshotSkipFrames(true);
        }
        else{
            view.showSnapshotSkipFrames(false);
        }
    }

    validateSkipFramesInput(){
        const integer_regex = new RegExp("^(0|[1-9][0-9]*)$");
        
        let num_skip_frames = document.getElementById("snapshot_skip_frames").value;
        
        if(!integer_regex.test(num_skip_frames)){
            alert("Invalid number of frames entered.");
            return false;
        }

        return true;
    }
    
    handleClearBttn(){
        view.clearElementValue("response_console");
    }
}

let controller = new Controller();
let view = new View();
let model = new Model();

model.connectWebSocket();

window.addEventListener('load', function (e) {
    model.fsm(0);
});

window.addEventListener('beforeunload', function (e) {
    model.closeWebSocket();
});