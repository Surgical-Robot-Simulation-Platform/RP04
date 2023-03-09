import pickle
import socket
import sys

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to a specific IP and port (in this case local host) - port of 12345 because why not
IP = "127.0.0.1"
PORT = 12347
s.bind((IP, PORT))

while True:
    try:
        # Get data from sender - 4096 is buffer size
        data, address = s.recvfrom(4096)

        # Deserialize the data using pickle
        deserialized_data = pickle.loads(data)

        # Print the received data
        print(deserialized_data)
    except KeyboardInterrupt:
        s.close()
        sys.exit()
