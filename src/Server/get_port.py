"""
This module is responsible for retrieving the server port configuration.
It reads the port number from the 'myport.info.txt' file and validates it.
If the file is missing, invalid, or contains incorrect data, a default port is used.
"""
DEFAULT_PORT = 1357
PORT_FILE = "myport.info.txt"

"""
Retrieves the server port from a configuration file.

@return: Returns a valid port number from the file, or the default port if the file is missing or invalid
"""
def get_server_port():
    try:
        with open(PORT_FILE, "r") as f:
            lines = f.readlines()
            if len(lines) != 1:
                print("Warning: myport.info.txt contains more than 1 line.")
                return DEFAULT_PORT

            line = lines[0].strip()
            port = int(line)

            if 0 < port < 2 ** 16:
                return port
            else:
                print("Warning: invalid port in myport.info.txt.")
    except FileNotFoundError:
        print("Warning: myport.info.txt not found.")
    except Exception:
        print("Warning: error reading myport.info.txt.")

    return DEFAULT_PORT
