```

███╗   ██╗██╗██╗  ██╗      ██████╗██╗  ██╗ █████╗ ████████╗
████╗  ██║██║╚██╗██╔╝     ██╔════╝██║  ██║██╔══██╗╚══██╔══╝
██╔██╗ ██║██║ ╚███╔╝      ██║     ███████║███████║   ██║   
██║╚██╗██║██║ ██╔██╗      ██║     ██╔══██║██╔══██║   ██║   
██║ ╚████║██║██╔╝ ██╗     ╚██████╗██║  ██║██║  ██║   ██║   
╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝      ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝

```

# NIX-chat

A minimal two-client chat app built with UNIX domain sockets in C. Written as a systems programming exercise covering sockets, `select()`, and signal handling.

## How it works

A server process binds a UNIX domain socket to `/tmp/chat.sock` and waits for exactly two clients to connect. Once both are connected, the server relays messages between them. Each client uses `select()` to simultaneously watch stdin and the socket, so it can send and receive without blocking.

```
client A  <---->  server  <---->  client B
```

## Project structure

```
.
├── server.c    # Binds, listens, accepts two clients, relays messages
├── client.c    # Connects to server, sends/receives messages
└── Makefile
```

## Building

```bash
make
```

## Running

Open three terminals:

```bash
# Terminal 1 — start the server first
./server

# Terminal 2 — first client
./client

# Terminal 3 — second client
./client
```

Once both clients connect, type in either terminal and the message will appear in the other.

## Example session

**Terminal 1 — server**
```
$ ./server
Server listening on /tmp/chat.sock
Client 1 connected.
Client 2 connected.
Both clients connected, starting relay.
Client 1 disconnected.
```

**Terminal 2 — client A**
```
$ ./client
Connected to server!
hello from A
Them: hey, this is B
how's it going?
Them: pretty good!
^C
Disconnecting...
```

**Terminal 3 — client B**
```
$ ./client
Connected to server!
Them: hello from A
hey, this is B
Them: how's it going?
pretty good!
Server disconnected.
```

## Stopping

`Ctrl+C` in any terminal shuts down gracefully. The server handles `SIGINT`, `SIGTERM`, and `SIGPIPE` — closing both client connections and unlinking the socket file on exit.

## Key concepts

- **UNIX domain sockets** — local-only sockets identified by a filesystem path instead of an IP/port
- **`SOCK_STREAM`** — reliable, ordered byte stream (analogous to TCP)
- **`select()`** — multiplexes multiple file descriptors so a single thread can watch stdin and a socket simultaneously
- **`SIGPIPE`** — suppressed so the server/client don't crash if the other end disconnects mid-send

## Limitations

- Exactly two clients — no more, no less
- Messages are plain text, newline-terminated
- No usernames or timestamps
- Sequential `accept()` — client A must connect before client B

---

Made by Alejandro Alvarado
