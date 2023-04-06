class Controller {
    constructor() {
        this.image = new CameraImage();
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
            this.image.type = data.substring(1, delimiter_index);
            this.image.type += '_';
            this.image.type += data.substring(delimiter_index + 1);
            this.image.data = "";
            return;
        }

        let end_of_image = (data.slice(-4) == "#end");

        if (end_of_image) {
            data = data.replace("#end", "");

            // if there's an error receiving image from camera, #end is sent without any image data
            if (data == "") {
                this.receiving_image = false;
                this.image.data = "";
                return;
            }
        }

        if (this.receiving_image) {
            this.image.data += data;
        }
        else {
            this.receiving_image = true;
            this.image.data = data;
        }

        if (end_of_image) {
            if (this.validImageData() && this.processImageData())
                model.fsm(0);
            else
                console.log("Error processing image data");
        }
    }

    // move below method to CameraImage class?
    validImageData() {
        let invalid_chars = this.image.data.match(/[^A-Fa-f0-9]/g);
        if (invalid_chars) {
            view.addConsoleText("Invalid characters in image data: " + invalid_chars);
            return false;
        }
        return true;
    }

    // move below method to CameraImage class?
    processImageData() {
        const underscore_index = this.image.type.indexOf('_');
        const colour_type = this.image.type.substring(0, underscore_index);
        const bit_depth = parseInt(colour_type, 10);

        let regex = /[a-f0-9]{1,2}/gi;  // for JPEG or 8-bit images
        if ((bit_depth == 2) || (bit_depth == 4))
            regex = /[a-f0-9]/gi;
        else if (bit_depth == 12)
            regex = /[a-f0-9]{1,3}/gi;
        else if (bit_depth == 16)
            regex = /[a-f0-9]{1,4}/gi;

        const img_vals = Uint8Array.from(this.image.data.match(regex), x => parseInt(x, 16));

        if (colour_type == "J") {
            this.image.blob = new Blob([img_vals], { type: 'application/octet-stream' });
            this.image.image_uri = window.URL.createObjectURL(this.image.blob);
        }
        else {
            const resolution = this.image.type.substring(underscore_index + 1);
            const x_index = resolution.indexOf('x');
            const width = resolution.substring(0, x_index);
            const height = resolution.substring(x_index + 1);

            let buffer = new Uint8ClampedArray(width * height * 4);

            let count = 0;
            if (colour_type == "2GS" || colour_type == "4GS" || colour_type == "8GS") {
                for (let y = 0; y < height; y++) {
                    for (let x = 0; x < width; x++) {
                        let val = img_vals[count];
                        if (bit_depth == 2)
                            val = val << 6;
                        else if (bit_depth == 4)
                            val = val << 4;

                            let pos = (y * width + x) * 4;
                        buffer[pos] = val;
                        buffer[pos+1] = val;
                        buffer[pos+2] = val;
                        buffer[pos+3] = 255;
                        if ( (colour_type != "2GS") || (count % 2 == 0) )
                            count += 1;
                    }
                }
            }
            else {
                // for 8-bit colour images
                let red_mask = 0b11100000;
                let red_shift = 0;
                let green_mask = 0b00011100;
                let green_shift = 3;
                let blue_mask = 0b00000011;
                let blue_shift = 6;

                if (colour_type == "12C") {
                    red_mask = 0b111100000000;
                    red_shift = 4;
                    green_mask = 0b000011110000;
                    green_shift = 0;
                    blue_mask = 0b000000001111;
                    blue_shift = 4;
                }
                else if (colour_type == "16C") {
                    red_mask = 0b1111100000000000;
                    red_shift = 8;
                    green_mask = 0b0000011111100000;
                    green_shift = 3;
                    blue_mask = 0b0000000000011111;
                    blue_shift = 3;
                }

                for (let y = 0; y < height; y++) {
                    for (let x = 0; x < width; x++) {
                        let pos = (y * width + x) * 4;

                        buffer[pos] = (img_vals[count] & red_mask) >> red_shift;
                        const g = img_vals[count] & green_mask;
                        buffer[pos+1] = (colour_type == "16C") ? (g >> green_shift) : (g << green_shift);
                        buffer[pos+2] = (img_vals[count] & blue_mask) << blue_shift;
                        buffer[pos+3] = 255;

                        count += 1;
                    }
                }
            }

            let canvas = document.createElement('canvas');
            canvas.width = width;
            canvas.height = height;

            let canvas_context = canvas.getContext('2d');
            let image_data = canvas_context.createImageData(width, height);
            image_data.data.set(buffer);
            canvas_context.putImageData(image_data, 0, 0);

            this.image.image_uri = canvas.toDataURL();
        }

        return true;
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