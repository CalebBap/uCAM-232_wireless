class Model{
    constructor(){
        this.socket = new WebSocket('ws://10.100.0.200:81');
        this.socket.onmessage = function(event){
            view.addConsoleText(event.data);
        }
        this.fsm_state = 0;
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
    
        switch(this.fsm_state){
            case 0:
                view.showSyncControls();
                break;
            case 1:
                this.syncCmd();
                view.showInitialControls();
                break;
        }
    }
}