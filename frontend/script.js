const ws = new WebSocket("wss://" + window.location.host);

const chat = document.getElementById("chat");

ws.onmessage = (event) => {
    const div = document.createElement("div");
    div.textContent = event.data;
    chat.appendChild(div);
};

function send() {
    const input = document.getElementById("msg");
    if (input.value.trim() === "") return;
    ws.send(input.value);
    input.value = "";
}
