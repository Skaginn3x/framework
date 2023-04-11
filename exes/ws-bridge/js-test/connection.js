
console.log("Hello world");

webSocket = new WebSocket("ws://localhost:4096/ipc/connect/ec_example_run_context.test.atv320.bool.in.0");

webSocket.onmessage = (event) => {
    console.log("Event recived " + event.data);

    //webSocket.send(event.data + "HiHi");
}

webSocket.onopen = (event) => {
    console.log("Socket opened");
    webSocket.send("HiHi");
}

webSocket.onclose = (event) => {
    console.log("Connection closed");
}