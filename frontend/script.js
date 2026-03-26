// 🔥 automatyczne dopasowanie ws/wss
const ws = new WebSocket("wss://chat-cpp.onrender.com");

const chat = document.getElementById("chat");
const input = document.getElementById("msg");

// 🔥 odbieranie wiadomości
ws.onmessage = (event) => {
    addMessage(event.data);
};

// 🔥 wysyłanie wiadomości
function send() {
    if (input.value.trim() === "") return;

    ws.send(input.value);
    input.value = "";
}

// 🔥 enter = send
input.addEventListener("keypress", (e) => {
    if (e.key === "Enter") {
        send();
    }
});

// 🔥 dodanie wiadomości do UI
function addMessage(text) {
    const div = document.createElement("div");
    div.className = "message";
    div.textContent = text;

    chat.appendChild(div);

    // auto scroll
    chat.scrollTop = chat.scrollHeight;
}

// 🔥 debug
ws.onopen = () => console.log("Connected");
ws.onclose = () => console.log("Disconnected");
ws.onerror = (e) => console.log("Error:", e);
