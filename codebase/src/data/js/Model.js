class Model {
    constructor() {
        this.socket = null;
        this.fsm_state = 0;
    }

    connectWebSocket() {
        this.socket = new WebSocket('ws://10.100.0.200:81');

        var timer = setTimeout(function() {
            view.loadError();
        }, 5000);

        this.socket.onerror = function(event) {
            clearTimeout(timer);
            view.loadError();
        }

        this.socket.onopen = function(event) {
            clearTimeout(timer);
            view.loadSuccess();
        }

        this.socket.onmessage = function(event) {
            controller.handleWebSocketMessage(event.data);
        }
    }

    closeWebSocket() {
        this.socket.close();
    }

    syncCmd() {
        try {
            this.socket.send("#sync");
        }
        catch (error) {
            console.error(error);
        }
    }

    initCmd() {
        let colour_type = document.getElementById("colour_type").value;
        let resolution = document.getElementById("resolutions").value;

        try {
            this.socket.send("#init:" + colour_type + "," + resolution);
        }
        catch (error) {
            console.error(error);
        }
    }

    snapshotCmd() {
        let num_skip_frames = document.getElementById("snapshot_skip_frames").value;
    
        try {
            this.socket.send("#snapshot:" + num_skip_frames);
        }
        catch (error) {
            console.error(error);
        }

        return true;
    }

    fsm(value) {
        this.fsm_state += value;
        if (this.fsm_state < 0) {
            this.fsm_state = 0;
        }

        const nextBttnPressed = value == 1;
    
        switch (this.fsm_state) {
            case 0:
                if (value == -5) {  // returning to controls from image display
                    view.loadSuccess();
                }
                view.showSyncControls();
                break;
            case 1:
                if (nextBttnPressed) {
                    this.syncCmd();
                    view.waitForSync();
                }
                else {
                    view.showSyncControls();
                }
                break;
            case 2:
                view.showInitialiseControls();
                view.initOptionSelection();
                break;
            case 3:
                if (nextBttnPressed) {
                    this.initCmd();
                    view.waitForInit();
                }
                else {
                    this.fsm(-1);
                }
                break;
            case 4:
                view.showSnapshotControls();
                break;
            case 5:
                if (value == 0) {
                    view.displayImage();
                    return;
                }

                let getSnapshot = document.getElementById("get_snapshot_input").checked;

                if (nextBttnPressed && getSnapshot) {
                    if (controller.validateSkipFramesInput()) {
                        this.snapshotCmd();
                        view.waitForSnapshotCmd();
                    }
                    else {
                        this.fsm(-1);
                    }
                }
                else if (nextBttnPressed) {
                    this.fsm(1);
                }
                else {
                    this.fsm(-1);
                }
                break;
            case 6:
                view.showGetPictureControls();
                break;
            default:
                console.log("Error: FSM in invalid state: " + this.fsm_state + " (transition value: " + value + ")");
        }
    }
}