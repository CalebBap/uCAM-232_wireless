class Controller {
    image;
    constructor() {
        this.image = new CameraImage();
    }

    handleWebSocketData(data) {
        const new_data = new Uint8Array(data);
        if (this.image.processData(new_data)) {
            model.fsm(0);
        }
    }

    handleWebSocketMessage(data) {
        if (data.charAt(0) === '#') {
            controller.handleCommand(data);
        }
        else {
            view.addConsoleText(data);
        }
    }

    handleControlBttn(value) {
        model.fsm(value);
    }

    handleCommand(command) {
        if (command == "#sync_ack")
            model.fsm(1);
        else if (command == "#sync_nak")
            model.fsm(-1);
        else if (command == "#init_ack")
            model.fsm(1);
        else if (command == "#init_nak")
            model.fsm(-1);
        else if (command == "#snap_ack")
            model.fsm(0);   // TODO
        else if (command == "#snap_nak")
            model.fsm(-1);
        else
            console.log("Unhandled command: " + command);
    }

    handleInitialiseSelection() {
        let colour_value = document.getElementById("colour_type").value;
        let resolution_value = document.getElementById("resolutions").value;
        view.initOptionSelection(colour_value, resolution_value);
    }

    handleSnapshotSelection() {
        let snapshot_checkbox = document.getElementById("get_snapshot_input");

        if (snapshot_checkbox.checked) {
            view.showSnapshotSkipFrames(true);
        }
        else {
            view.showSnapshotSkipFrames(false);
        }
    }

    validateSkipFramesInput() {
        const integer_regex = new RegExp("^(0|[1-9][0-9]*)$");
        
        let num_skip_frames = document.getElementById("snapshot_skip_frames").value;
        
        if (!integer_regex.test(num_skip_frames) || num_skip_frames > 0xFFFF) {
            alert("Invalid number of frames entered.");
            return false;
        }

        return true;
    }
    
    handleClearBttn() {
        view.clearElementValue("response_console");
    }


    handleExitImageBttn() {
        model.fsm(-5);
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