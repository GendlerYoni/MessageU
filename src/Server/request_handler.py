"""
This module handles incoming client requests according to the MessageU server protocol.
It validates request headers and payloads, dispatches each request to the
appropriate handler function, interacts with the database when needed,
and builds protocol-compliant binary responses.
"""

import secrets
import struct
import sqlite3

SERVER_VERSION = 2
CLIENT_VERSION = 1
HEADER_SIZE = 23
CODE_OFFSET = 17
PAYLOAD_SIZE_OFFSET = 19
VERSION_OFFSET = 16
CODE_SIZE = 2
NAME_SIZE = 255
PUBLIC_KEY_SIZE = 160
CLIENT_ID_SIZE = 16

MESSAGE_TYPE_SIZE = 1
CONTENT_SIZE = 4
MESSAGE_ID = 4

SYMMETRIC_KEY_REQUEST = 1
SYMMETRIC_KEY_SEND = 2
TEXT_MESSAGE = 3

REQUEST_REGISTER = 700
REQUEST_CLIENTS_LIST = 701
REQUEST_PUBLIC_KEY = 702
REQUEST_SEND_MESSAGE = 703
REQUEST_WAITING_MESSAGES = 704

RESPONSE_REGISTER = 2100
RESPONSE_CLIENT_LIST = 2101
RESPONSE_PUBLIC_KEY = 2102
RESPONSE_MESSAGE_SENT = 2103
RESPONSE_WAITING_MESSAGES = 2104

ERROR_GENERAL = 9000

"""
Main dispatcher for all incoming client requests.
Validates the request header, checks the client version and request code,
and forwards the request to the matching handler function.

@param: Receives the full request bytes and a database object
@return: Returns the full response bytes according to the protocol
@raises: Does not raise database exceptions outward;
database-related errors are converted into a general error response
"""
def request_handler_main(data, db):
    if len(data) < HEADER_SIZE:
        return build_error_response()

    payload_size = int.from_bytes(data[PAYLOAD_SIZE_OFFSET: PAYLOAD_SIZE_OFFSET + CONTENT_SIZE], "little")
    if len(data) != HEADER_SIZE + payload_size:
        return build_error_response()

    version = data[VERSION_OFFSET]
    if version != CLIENT_VERSION:
        print("Unexpected version")
        return build_error_response()

    code = int.from_bytes(data[CODE_OFFSET: CODE_OFFSET + CODE_SIZE], "little")
    try:
        if code == REQUEST_REGISTER:
            return register_handler(data, db)

        requester_id = data[:CLIENT_ID_SIZE]
        if db.client_exists(requester_id) is None:
            print("Requester ID not found")
            return build_error_response()
        db.update_client_last_seen(requester_id)

        if code == REQUEST_CLIENTS_LIST:
            return clients_list_handler(data, db)

        elif code == REQUEST_PUBLIC_KEY:
            return public_key_handler(data, db)

        elif code == REQUEST_WAITING_MESSAGES:
            return waiting_messages_handler(data, db)

        elif code == REQUEST_SEND_MESSAGE:
            return send_message_handler(data, db)

        else:
            print("Unknown request code")
            return build_error_response()

    except sqlite3.Error as e:
        print("Database error:", e)
        return build_error_response()
    except Exception:
        return build_error_response()

"""
Handles a client registration request.
Extracts the username and public key from the request, validates them,
creates a new client ID, and stores the client in the database.

@param: Receives the full request bytes and a database object
@return: Returns a success response with the generated client ID, or an error response if validation fails
@raises: Does not raise exceptions outward; database errors (e.g., duplicate username)
are handled and converted into an error response
"""
def register_handler(data, db):
    print("Handling REGISTER")
    if len(data) != HEADER_SIZE+ NAME_SIZE + PUBLIC_KEY_SIZE:
        print("Error: Size isn't as expected.")
        return build_error_response()

    raw_name = data[HEADER_SIZE:HEADER_SIZE + NAME_SIZE]
    public_key = data[HEADER_SIZE + NAME_SIZE: HEADER_SIZE + NAME_SIZE + PUBLIC_KEY_SIZE]
    raw_name = raw_name.split(b'\x00', 1)[0]

    try:
        name = raw_name.decode("ascii")
    except UnicodeDecodeError:
        return build_error_response()

    if not name:
        print("Empty name")
        return build_error_response()

    client_id = secrets.token_bytes(CLIENT_ID_SIZE)
    try:
        db.add_client(client_id, name, public_key)
    except sqlite3.IntegrityError:
        print("User name already in database.")
        return build_error_response()
    except Exception:
        return build_error_response()

    return _build_response(RESPONSE_REGISTER, client_id)

"""
Handles a request for the list of registered clients.
Builds a response payload containing all clients except the requester,
with each entry including the client ID and username.

@param: Receives the full request bytes and a database object
@return: Returns a response containing the list of other registered clients,
or an error response if the request is invalid
"""
def clients_list_handler(data, db):
    print("Handling CLIENTS LIST")

    if len(data) != HEADER_SIZE:
        return build_error_response()

    requester_id = data[:CLIENT_ID_SIZE]

    clients = db.list_clients_except(requester_id)
    payload = bytearray()

    for client_id, username in clients:
        payload += client_id  # 16 bytes

        name_bytes = username.encode("ascii", errors="replace")[:NAME_SIZE - 1]
        name_bytes += b'\x00' * (NAME_SIZE - len(name_bytes))

        payload += name_bytes

    return _build_response(RESPONSE_CLIENT_LIST, payload)

"""
Handles a request for a client's public key.
Retrieves the public key of the requested client from the database
and builds a response containing the client ID and its public key.

@param: Receives the full request bytes and a database object
@return: Returns a response with the requested client ID and public key,
or an error response if the client is not found or the request is invalid
"""
def public_key_handler(data, db):
    print("Handling PUBLIC KEY")
    if len(data) != HEADER_SIZE + CLIENT_ID_SIZE:
        return build_error_response()

    requested_id = data[HEADER_SIZE: HEADER_SIZE + CLIENT_ID_SIZE]
    public_key = db.get_public_key_by_id(requested_id)

    if public_key is None or len(public_key) != PUBLIC_KEY_SIZE:
        print("Public key not found.")
        return build_error_response()

    payload = requested_id + public_key
    return _build_response(RESPONSE_PUBLIC_KEY, payload)

"""
Handles a request to retrieve all waiting messages for a client.
Fetches messages from the database, builds a response payload with all messages,
and deletes them from the database after successful retrieval.

@param: Receives the full request bytes and a database object
@return: Returns a response containing all waiting messages for the client,
or an error response if the request is invalid
"""
def waiting_messages_handler(data, db):
    print("Handling WAITING MESSAGES")

    if len(data) != HEADER_SIZE:
        return build_error_response()

    requester_id = data[:CLIENT_ID_SIZE]

    messages = db.get_waiting_messages(requester_id)

    payload = bytearray()
    ids = []

    for msg_id, from_client, msg_type, content in messages:
        payload += from_client
        payload += msg_id.to_bytes(MESSAGE_ID, "little")
        payload += int(msg_type).to_bytes(MESSAGE_TYPE_SIZE, "little")
        payload += len(content).to_bytes(CONTENT_SIZE, "little")
        payload += content

        ids.append(msg_id)

    db.delete_messages(ids)

    return _build_response(RESPONSE_WAITING_MESSAGES, payload)

"""
Handles a request to send a message to another client.
Validates the target client, message type, and content size,
stores the message in the database, and returns a confirmation response.

@param: Receives the full request bytes and a database object
@return: Returns a response containing the target client ID and the created message ID,
or an error response if the request is invalid
"""
def send_message_handler(data, db):
    print("Handling SEND MESSAGE (703)")

    min_len = HEADER_SIZE + CLIENT_ID_SIZE + MESSAGE_TYPE_SIZE + CONTENT_SIZE
    if len(data) < min_len:
        return build_error_response()

    requester_id = data[:CLIENT_ID_SIZE]
    requested_id = data[HEADER_SIZE: HEADER_SIZE + CLIENT_ID_SIZE]

    if db.client_exists(requested_id) is None:
        return build_error_response()

    msg_type = data[HEADER_SIZE + CLIENT_ID_SIZE]

    if msg_type not in (SYMMETRIC_KEY_REQUEST, SYMMETRIC_KEY_SEND, TEXT_MESSAGE):
        return build_error_response()

    content_size = int.from_bytes(
        data[HEADER_SIZE + CLIENT_ID_SIZE + MESSAGE_TYPE_SIZE:
             HEADER_SIZE + CLIENT_ID_SIZE + MESSAGE_TYPE_SIZE + CONTENT_SIZE], "little")

    if len(data) != min_len + content_size:
        return build_error_response()

    if msg_type == SYMMETRIC_KEY_REQUEST and content_size != 0:
        return build_error_response()

    content = data[min_len:min_len + content_size]

    msg_id = db.add_message(requested_id, requester_id, msg_type, content)
    payload = requested_id + msg_id.to_bytes(MESSAGE_ID, "little")

    return _build_response(RESPONSE_MESSAGE_SENT, payload)


"""
Builds a general error response according to the protocol (code 9000).
Used when a request is invalid or an internal error occurs.

@return: Returns a response with an error code and empty payload
"""
def build_error_response():
    print("Building general error response")
    return _build_response(ERROR_GENERAL)

"""
Builds a full protocol response with a header and optional payload.
Encodes the response using little-endian format.

@param: Receives a response code and an optional payload (bytes)
@return: Returns the complete response bytes (header + payload)
"""
def _build_response(code, payload = b""):
    header = struct.pack("<BHI", SERVER_VERSION, code, len(payload))
    return header + payload