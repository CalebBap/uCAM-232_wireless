class Controller {
    constructor() {
        this.receiving_image = false;
        this.image_data = "";
    }

    handleWebSocketMessage(data) {
        if (data.charAt(0) == '#') {
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
        switch (command) {
            case "#sync_ack":
                model.fsm(1);
                break;
            case "#sync_nak":
                model.fsm(-1);
                break;
            case "#init_ack":
                model.fsm(1);
                break;
            case "#init_nak":
                model.fsm(-1);
                break;
            case "#snap_ack":
                model.fsm(0);   // TODO
                break;
            case "#snap_nak":
                model.fsm(-1);
                break;
            default:
                if (command.substring(0, 4) == "#img")
                    this.processImageData(command.substring(4));
                else
                    console.log("Unhandled command: " + command);
                break;
        }
    }

    handleInitialiseSelection() {
        let colour_value = document.getElementById("colour_type").value;
        let raw_res_value = document.getElementById("raw_resolutions").value;
        let jpeg_res_value = document.getElementById("jpeg_resolutions").value;
        model.initialiseOptionSelection(colour_value, raw_res_value, jpeg_res_value);
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

    processImageData(data) {
        let end_of_image = (data.slice(-4) == "#end");

        if (end_of_image) {
            data = data.replace("#end", "");

            // if there's an error receiving image from camera, #end is sent without any image data
            if (data == "") {
                this.receiving_image = false;
                this.image_data = "";
                return;
            }
        }

        if (this.receiving_image) {
            this.image_data += data;
        }
        else {
            this.receiving_image = true;
            this.image_data = data;
        }

        if (end_of_image) {
            if (this.validImageData())
                model.fsm(0);
            this.receiving_image = false;
            this.image_data = "";
        }
    }

    validImageData() {
        let invalid_chars = this.image_data.match(/[^A-Fa-f0-9]/g);
        if (invalid_chars) {
            view.addConsoleText("Invalid characters in image data: " + invalid_chars);
            return false;
        }
        return true;
    }

    getImageData() {
        return this.image_data;
    }

    handleExitImageBttn() {
        model.fsm(-1);
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