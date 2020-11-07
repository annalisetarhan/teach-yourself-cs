from socket import *
import os.path
import http.server

serverName = 'localhost'
serverPort = 12010
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('', serverPort))
serverSocket.listen(1)
print("Ready!")
while True:
	# Create a connection socket when contacted by client
	connectionSocket, addr = serverSocket.accept()

	# Receive HTTP request from connection and parse to determine file
	fileName = connectionSocket.recv(1024).decode()

	if os.path.isfile(fileName):
		# Get the requested file
		file = open(fileName, "r")
		splitFilename = fileName.split(".")
		fileType = splitFilename[-1]

		# Create an HTTP response message preceeded by header lines
		responseHeader = ("HTTP/1.1 200 OK\n"
					"Content-Type: {}\r\n\r\n".format(fileType))
		response = responseHeader + file.read()

		# Send the response over the TCP connection to the browser
		connectionSocket.send(response.encode())
	else:
		# If the file isn't present, return an error message
		response = ("HTTP/1.1 400 Not Found\r\n\r\n")
		connectionSocket.send(response.encode())

	connectionSocket.close()