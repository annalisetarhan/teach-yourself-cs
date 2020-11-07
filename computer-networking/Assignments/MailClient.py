from socket import *
import ssl

# To encode username and password: 'echo -ne "\0username\0password"|base64'
credentials = '\0sender@gmail.com\0password|base64'

mailserver = "smtp.gmail.com"
fromEmail = 'sender@gmail.com'
toEmail = 'receiver@gmail.com'
msg = "\r\n Guess what I just did?"
endmsg = "\r\n.\r\n"

# Create socket and establish TCP connection with mailserver using SSL
clientSocket = socket(AF_INET, SOCK_STREAM)
wrappedSocket = ssl.create_default_context().wrap_socket(clientSocket, server_hostname="smtp.gmail.com")
wrappedSocket.connect((mailserver, 465))

recv = wrappedSocket.recv(1024)
print('recv: {}'.format(recv.decode()))

if recv[:3].decode()!= '220':
    print('220 reply not received from server on recv.')

# Send EHLO command and print server response
ehloCommand = 'EHLO gmail.com\r\n'.encode()
wrappedSocket.send(ehloCommand)
recv1 = wrappedSocket.recv(1024)
print('recv1: {}'.format(recv1.decode()))

if recv1[:3].decode() != '250':
    print('250 reply not received from server on rcv1.')

# Authenticate and print server response
authCommand = 'AUTH PLAIN\r\n'.encode()
wrappedSocket.send(authCommand)
recv2 = wrappedSocket.recv(1024)
print('recv2: {}'.format(recv2.decode()))

if recv2[:3].decode() != '334':
    print('334 reply not received from server on rcv2.')
    
wrappedSocket.send(credentials.encode())
recv3 = wrappedSocket.recv(1024)
print('recv3: {}'.format(recv3.decode()))

if recv3[:3].decode() != '235':
    print('235 reply not received from server on rcv3.')
    
# Send MAIL FROM command and print server response
mailFromCommand = 'MAIL FROM: <{}>\r\n'.format(fromEmail).encode()
wrappedSocket.send(mailFromCommand)
recv4 = wrappedSocket.recv(1024)
print('recv4: {}'.format(recv4.decode()))

if recv4[:3].decode() != '250':
    print('250 reply not received from server on rcv4')
    
# Send RCPT TO command and print server response
rcptToCommand = 'RCPT TO: <{}>\r\n'.format(toEmail).encode()
wrappedSocket.send(rcptToCommand)
recv5 = wrappedSocket.recv(1024)
print('recv5: {}'.format(recv5.decode()))

if recv5[:3].decode() != '250':
    print('250 reply not received from server on rcv5')
    
# Send DATA command and print server response
dataCommand = 'DATA\r\n'
wrappedSocket.send(dataCommand.encode())
recv6 = wrappedSocket.recv(1024)
print('recv6: {}'.format(recv5.decode()))

if recv6[:3].decode() != '354':
    print('354 reply not received from server on rcv6')
    
# Send message data and print server response
wrappedSocket.send(msg.encode())
wrappedSocket.send(endmsg.encode())
recv7 = wrappedSocket.recv(1024)
print('recv7: {}'.format(recv7.decode()))

if recv7[:3].decode() != '250':
    print('250 reply not received from server on rcv7')

# Quit and print server response
quitCommand = 'QUIT\r\n'
wrappedSocket.send(quitCommand.encode())
recv8 = wrappedSocket.recv(1024)
print('recv8: {}'.format(recv8.decode()))

if recv8[:3].decode() != '221':
    print('221 reply not received from server on rcv8')

wrappedSocket.close()
