"""
This module is responsible for handling all database operations for the server.

It includes the Database class, which manages the SQLite database,
creates required tables, and provides methods for storing and retrieving
clients and messages.
"""

import sqlite3
from datetime import datetime

"""
Represents the database layer of the server.
Handles all operations related to clients and messages,
including creation, insertion, retrieval, and deletion.
"""
class Database:

    """Initializes the database connection and ensures required tables exist."""
    def __init__(self):
        self.conn = sqlite3.connect("defensive.db", timeout=2.0, check_same_thread=False)
        self.conn.execute("PRAGMA journal_mode=WAL;")
        self._create_tables()

    """
    Creates the required database tables if they do not already exist.
    Creates the 'clients' and 'messages' tables.
    """
    def _create_tables(self):
        self.conn.execute("""
        CREATE TABLE IF NOT EXISTS clients (
            ID BLOB PRIMARY KEY,
            UserName TEXT UNIQUE NOT NULL,
            PublicKey BLOB NOT NULL,
            LastSeen TEXT)""")

        self.conn.execute("""
        CREATE TABLE IF NOT EXISTS messages (
            ID INTEGER PRIMARY KEY AUTOINCREMENT,
            ToClient BLOB NOT NULL,
            FromClient BLOB NOT NULL,
            Type INTEGER NOT NULL,
            Content BLOB NOT NULL)""")
        self.conn.commit()

    """Closes the database connection."""
    def close(self):
        self.conn.close()

    """
    Adds a new client to the database.

    @param: Receives a client ID (bytes), username (string), and public key (bytes)
    @raises: May raise a database IntegrityError if the username already exists
    """
    def add_client(self, client_id, name, publicKey):
        self.conn.execute("INSERT INTO clients (ID, UserName, PublicKey, LastSeen) VALUES (?, ?, ?, ?)",
                          (client_id, name, publicKey, datetime.utcnow().isoformat()))
        self.conn.commit()

    """
    Updates the last seen timestamp of a client.

    @param: Receives a client ID (bytes)
    """
    def update_client_last_seen(self, client_id):
        self.conn.execute("UPDATE clients SET LastSeen = ? WHERE ID = ?", (datetime.utcnow().isoformat(), client_id))
        self.conn.commit()

    """
    Checks whether a client exists in the database.

    @param: Receives a client ID (bytes)
    @return: Returns the client record if found, otherwise None
    """
    def client_exists(self, client_id: bytes):
        cur = self.conn.execute("SELECT ID FROM clients WHERE ID = ?", (client_id,))
        return cur.fetchone()

    """
    Retrieves all clients except the requester.

    @param: Receives the requester client ID (bytes)
    @return: Returns a list of tuples (client_id, username) for all other clients
    """
    def list_clients_except(self, requester_id: bytes):
        cur = self.conn.execute("SELECT ID, UserName FROM clients WHERE ID != ?", (requester_id,))
        return cur.fetchall()

    """
    Retrieves the public key of a specific client from the database.

    @param: Receives a client ID (bytes)
    @return: Returns the public key (bytes) if found, otherwise None
    """
    def get_public_key_by_id(self, client_id: bytes):
        cur = self.conn.execute("SELECT PublicKey FROM clients WHERE ID = ?", (client_id,))
        pk = cur.fetchone()
        if pk is None:
            return None
        return pk[0]

    """
    Adds a new message to the database.

    @param: Receives target client ID, sender client ID, message type, and content (bytes)
    @return: Returns the generated message ID (integer)
    """
    def add_message(self, toClient, fromClient, message_type, content=b""):
        cur = self.conn.execute("INSERT INTO messages (ToClient, FromClient, Type, Content) VALUES (?, ?, ?, ?)",
                          (toClient, fromClient, message_type, content))
        self.conn.commit()
        return cur.lastrowid

    """
    Retrieves all waiting messages for a specific client.

    @param: Receives a client ID (bytes)
    @return: Returns a list of messages as tuples (message_id, from_client, type, content)
    """
    def get_waiting_messages(self, toClient):
        cur = self.conn.execute("SELECT ID, FromClient, Type, Content FROM messages WHERE ToClient = ? ORDER BY ID ASC",
                                (toClient,))
        return cur.fetchall()

    """
    Deletes messages from the database based on their IDs.

    @param: Receives a list of message IDs (integers)
    """
    def delete_messages(self, message_ids):
        if not message_ids:
            return

        placeholders = ",".join("?" for _ in message_ids)

        self.conn.execute(
            f"DELETE FROM messages WHERE ID IN ({placeholders})", tuple(message_ids))

        self.conn.commit()


#Debug functions.
    """Prints the current contents of the database for debugging purposes."""
    def print_db(self):
        print("=== CLIENTS ===")
        cur = self.conn.execute("SELECT ID, UserName, PublicKey, LastSeen FROM clients")
        for row in cur.fetchall():
            print(row)

        print("\n=== MESSAGES ===")
        cur = self.conn.execute("SELECT ID, ToClient, FromClient, Type, Content FROM messages")
        for row in cur.fetchall():
            print(row)

    """Prints the current contents of the database for debugging purposes."""
    def drop_tables(self):
        self.conn.execute("DROP TABLE IF EXISTS messages")
        self.conn.execute("DROP TABLE IF EXISTS clients")
        self.conn.commit()

