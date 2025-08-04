# UDP Chat Room System (Client-Server) in C

This project implements a simple chat system using UDP sockets, allowing multiple clients to communicate in real time via a central server.

---

## 🔧 Features

- UDP-based messaging (JOIN, CHAT, LEAVE)
- Server tracks active clients by IP/Port
- Clients operate on two threads (receive + send)
- Clean custom packet format with headers
- Graceful client exit

---

## 📁 Project Structure

```
UDPChatRoom/
├── client/
│   ├── UDPClient.c         # UDP client source
│   └── UDPClient           # Compiled binary
├── server/
│   ├── UDPServer.c         # UDP server source
│   ├── DieWithError.c      # Error handling utility
│   └── UDPServer           # Compiled binary
├── Makefile
├── .gitignore
└── README.md
```

---

## ⚙️ Compilation

```bash
make
```

- Compiles `client/UDPClient` and `server/UDPServer`

To clean up:

```bash
make clean
```

---

## 🚀 How to Run

### Run Server

```bash
./server/UDPServer
```

### Run Client

```bash
./client/UDPClient
```

The client prompts for a nickname, sends a JOIN packet, and starts sending/receiving messages.

---

## 📦 Message Format (UDP)

| Field   | Size  | Description                    |
|---------|-------|--------------------------------|
| Type    | 2     | 1 = JOIN, 2 = CHAT, 3 = LEAVE |
| Length  | 2     | Length of the message (<= 256) |
| Data    | ≤256  | Nickname or chat message       |

All integer values are sent in network byte order.

---

## 💬 Example Output

**Client:**
```
Enter nickname: Alice
[Alice]: Hello everyone!
```

**Server:**
```
Received JOIN from 192.168.0.12:50938 - Alice
Received CHAT from Alice:
Received LEAVE from Alice
```

---

## 📄 License

This project is open source under the [MIT License](LICENSE).

---

## 👨‍💻 Author

Developed by [Amena2026](https://github.com/Amena2026)
