class Controller{
    handleClearBttn(){
        view.clearTerminal();
    }

    handleControlBttn(value){
        model.fsm(value);
    }

    handleWebSocketMessage(data){
        if(data.charAt(0) == '#'){
            controller.handleCommand(data);
        }
        else{
            view.addConsoleText(data);
        }
    }

    handleCommand(command){
        switch(command){
            case "#sync_failed":
                model.fsm(-1); //TODO
                break;
            case "#synced":
                model.fsm(1);
                break;
        }
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