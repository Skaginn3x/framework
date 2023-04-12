webSocket = new WebSocket("ws://127.0.0.1:4096?connect=ec_example_run_context.test.easyecat.1.uint.in.0", ["tfc-ipc"]);
//webSocket = new WebSocket("ws://127.0.0.1:4096?connect=ec_example_run_context.test.atv320.string.command", ["tfc-ipc"]);

webSocket.onmessage = (event) => {
    console.log("Event recived " + event.data);

    //webSocket.send(event.data + "HiHi");
}

webSocket.onopen = (event) => {
    console.log("Socket opened");
    //webSocket.send("HiHi");
}

webSocket.onclose = (event) => {
    console.log("Connection closed: " + event.reason + "  code: " + event.code);
}

webSocket.onerror = (event) => {
    console.log("Error: " + event.data + " " + event.type + " ");
}