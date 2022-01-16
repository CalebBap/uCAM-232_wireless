class Controller{
    handleClearBttn(){
        view.clearTerminal();
    }

    handleControlBttn(value){
        model.fsm(value);
    }

    handleWebSocketMessage(data){
        view.addConsoleText(data);
    }
}

let controller = new Controller();
let view = new View();
let model = new Model();

window.addEventListener('load', function (e) {
    model.fsm(0);
});

window.addEventListener('beforeunload', function (e) {
    model.closeWebSocket();
});