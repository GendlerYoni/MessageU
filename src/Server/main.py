"""
This module implements the main server logic for the MessageU system.

The server is responsible for accepting client connections, receiving requests,
and sending appropriate responses according to the protocol. It uses a
multi-threaded architecture where each connected client is handled in a separate thread.

The server reads incoming requests using a fixed-size header and a variable-size payload,
processes them through the request handler module, and interacts with the database
module to store and retrieve client and message data.

The server runs continuously, accepting new connections and handling multiple clients concurrently.
"""
import socket
import threading
import get_port
import request_handler as rh
import database
import connection as co

HOST = "0.0.0.0"
PORT = get_port.get_server_port()
HEADER_SIZE = 23

"""
Handles communication with a single connected client.
Receives requests from the client, processes them using the request handler,
and sends back responses. Each client is handled in a separate thread.

@param: Receives a socket connection object and the client address
@raises: Handles connection and general exceptions internally and terminates the connection if needed
"""
def handle_client(conn, addr):
    print("Connected by", addr)
    db = database.Database()  # DB per thread

    with conn:
        while True:
            try:
                data = co.receive_exact(conn, HEADER_SIZE)
                data += co.receive_exact(conn, co.get_payload_size(data))
                response = rh.request_handler_main(data, db)
                conn.sendall(response)

            except ConnectionError:
                print("Client disconnected", addr)
                break

            except Exception as e:
                print("Server error:", e)
                break

    db.close()

"""
Initializes and runs the server.
Creates a listening socket, binds it to the configured host and port,
and continuously accepts incoming client connections. Each new connection
is handled in a separate thread.
"""
def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind((HOST, PORT))
        sock.listen()
        print(f"Listening on {HOST}:{PORT}")

        while True:
            conn, addr = sock.accept()
            t = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
            t.start()

if __name__ == "__main__":
    main()
