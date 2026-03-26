const ws = new WebSocket("ws://localhost:9001");

const chat = document.getElementById("chat");

ws.onmessage = (event) => {
    const div = document.createElement("div");
    div.textContent = event.data;
    chat.appendChild(div);
};

function send() {
    const input = document.getElementById("msg");
    ws.send(input.value);
    input.value = "";
}