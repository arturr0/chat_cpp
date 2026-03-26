const ws = new WebSocket("ws://chat_cpp.onrender.com");

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
