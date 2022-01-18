class Model{
    constructor(){
        this.socket = null;
        this.fsm_state = 0;
        this.camera_synced = false;
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
                if(!this.camera_synced && nextBttnPressed){
                    this.syncCmd();
                    view.waitForSync();
                }
                break;
            case 2:
                this.camera_synced = true;
                view.showInitialControls();
                break;
        }
    }
}