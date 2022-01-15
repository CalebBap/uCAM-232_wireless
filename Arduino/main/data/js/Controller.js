class Controller{
    handleClearBttn(){
        view.clearTerminal();
    }

    handleControlBttn(value){
        model.fsm(value);
    }
}

var controller = new Controller();
var view = new View();
var model = new Model();

window.addEventListener('load', function (e) {
    model.fsm(0);
});

window.addEventListener('beforeunload', function (e) {
    model.closeWebSocket();
});