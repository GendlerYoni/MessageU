"""
This module provides utility functions for handling low-level socket communication.
It includes functions for reading exact amounts of data from a connection
and extracting protocol fields from request headers.
"""
HEADER_SIZE = 23
PAYLOAD_SIZE_OFFSET = 19
CONTENT_SIZE = 4

"""
Reads exactly a given number of bytes from the socket.
@param: Receives a socket connection and the number of bytes to read
@return: Returns the received bytes
@raises: Raises ConnectionError if the client disconnects prematurely
"""
def receive_exact(conn, size):
    data = b''
    while len(data) < size:
        chunk = conn.recv(size - len(data))
        if not chunk:
            raise ConnectionError("Client disconnected")
        data += chunk
    return data

"""
Extracts the payload size from the request header.
@param: Receives the full header bytes of the request
@return: Returns the payload size as an integer (little-endian)
"""
def get_payload_size(header):
    return int.from_bytes(header[PAYLOAD_SIZE_OFFSET:PAYLOAD_SIZE_OFFSET + CONTENT_SIZE], "little")