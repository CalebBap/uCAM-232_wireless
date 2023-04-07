class Controller {
    image;
    #receiving_image;
    constructor() {
        this.image = new CameraImage();
        this.#receiving_image = false;
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
        else if (command.substring(0, 4) == "#img")
            this.handleImageData(command.substring(4));
        else
            console.log("Unhandled command: " + command);
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

    handleImageData(data) {
        if (data.charAt(0) == ',') {
            let delimiter_index = data.indexOf(',', 1);
            const colour_type = data.substring(1, delimiter_index);
            if (!this.image.setColourType(colour_type)) {
                console.log("Failed to set colour type, type = " + colour_type);
                // handle error
            }

            delimiter_index++;
            const x_index = data.indexOf('x', delimiter_index);
            const width = parseInt(data.substring(delimiter_index, x_index), 10);
            const height = parseInt(data.substring(x_index + 1), 10);
            if (!this.image.setResolution(width, height)) {
                console.log("Failed to set image resolution, width = " + width + ", height = " + height);
                // handle error
            }

            this.image.clearData();
            return;
        }

        let end_of_image = (data.slice(-4) == "#end");

        if (end_of_image) {
            data = data.replace("#end", "");

            // if there's an error receiving image from camera, #end is sent without any image data
            if (data == "") {
                this.#receiving_image = false;
                this.image.clearData();
                return;
            }
        }

        if (this.#receiving_image) {
            this.image.appendData(data);
        }
        else {
            this.#receiving_image = true;
            this.image.setData(data);
        }

        if (end_of_image) {
            if (this.image.validImageData() && this.image.processImageData())
                model.fsm(0);
            else
                console.log("Error processing image data");
        }
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