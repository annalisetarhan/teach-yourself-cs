from socket import *
import threading
import sys
import os

"""
    I'm not completely happy with this, but it's time to move on. Why do my requests
    have to use messy timeouts instead of just returning something with len() == 0
    like all the example code I found? Why can't I figure out how to close the
    clientSocket? (After all the images to load, of course.) Why is google's homepage
    so fussy?
"""

serverPort = 12000
currentBaseURL = '-'
    
def serve_object(clientSocket):
    # Need to save base URL for subsequent requests
    global currentBaseURL
    
    # Receive request from client
    request = clientSocket.recv(1024)
            
    # Extract URL from request
    URL = request.decode().split()[1].partition("/")[2]
    
    # Detect if request is for the base page or a subsequent object request
    if ('.com' in URL or '.net' in URL or '.edu' in URL or '.org' in URL):
        currentBaseURL = URL.partition("/")[0]
    # If the request is for an object used by the page, append base URL
    else:
        URL = currentBaseURL + "/" + URL
    
    print("URL is {}".format(URL))
    
    # Determine path
    path = URL.split("/", 1)[-1]
    
    print("currentBaseURL is {}".format(currentBaseURL))
    print("Path is {}".format(path))

                        
    # Create a socket for the proxy server
    proxyClientSocket = socket(AF_INET, SOCK_STREAM)
    proxyClientSocket.connect((currentBaseURL, 80))
    proxyClientSocket.settimeout(2)
     
    # Forward request to server
    newRequest = ""
    newRequest += "GET /" + path + " HTTP/1.1\r\n"
    newRequest += "Host: " + currentBaseURL + "\r\n"
    newRequest += "\r\n"
    proxyClientSocket.sendall(newRequest.encode())
    
    print("newRequest is {}".format(newRequest))

    # Forward results to client
    while True:
        try:
            result = proxyClientSocket.recv(2048)
            clientSocket.send(result)
        except timeout:
            break
    
    # Close proxy connection
    proxyClientSocket.close()
    
""" MAIN """
# Create a TCP server socket, bind it to a port, and start listening
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
serverSocket.bind(('', serverPort))
serverSocket.listen(1)
print('Server ready')

while True:
    # Start receiving data from client
    clientSocket, addr = serverSocket.accept()
    thread = threading.Thread(name = addr, target = serve_object, args=(clientSocket,))
    thread.setDaemon(True)
    thread.start()
