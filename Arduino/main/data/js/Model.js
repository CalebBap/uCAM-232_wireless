class Model{
    constructor(){
        this.socket = new WebSocket('ws://10.100.0.200:81');
        this.socket.onmessage = function(event){
            controller.handleWebSocketMessage(event.data);
        }
        this.fsm_state = 0;
        this.camera_synced = false;
    }

    closeWebSocket(){
        this.socket.close();
    }

    syncCmd(nextBttnPressed){
        if(nextBttnPressed){
            try{
                this.socket.send("#sync");
            }
            catch(error){
                console.error(error);
            }
        }
    }

    fsm(value){    
        this.fsm_state += value;
        const nextBttnPressed = value == 1;
    
        if(this.fsm_state < 0){
            this.fsm_state = 0;
        }
    
        switch(this.fsm_state){
            case 0:
                view.showSyncControls();
                break;
            case 1:
                if(!this.camera_synced){
                    this.syncCmd(nextBttnPressed);
                    view.waitForSync();
                }
                break;
            case 2:
                view.showInitialControls();
                break;
        }
    }
}