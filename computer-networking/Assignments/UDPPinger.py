import random
import socket
from datetime import datetime

serverName = 'localhost'
serverPort = 12000

# Create client socket with a 1 second timeout
clientSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
clientSocket.settimeout(1)

for x in range(10):
    # Record start time
    time = datetime.now()
    try:
        # Send a ping message
        clientSocket.sendto("ping".encode(), (serverName, serverPort))
        pong, serverAddress = clientSocket.recvfrom(2048)
        # Record end time
        laterTime = datetime.now()
        print("Ping {} took {}".format(x+1, laterTime-time))
    except:
        # In case of timeout
        print("lost packet :(")

clientSocket.close()

