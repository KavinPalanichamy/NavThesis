import socket
import base64
import time
from pyrtcm import RTCMReader

# Caster server information
caster_ip = "91.198.76.2"
caster_port = 8082

# RTN_MAC_3_1 stream identifier
stream_identifier = "NAWGEO_POJ_3_1"

# Caster server credentials
username = "Kavin"
password = "Kavin@4236"


# Create a TCP/IP socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to the caster server
caster_address = (caster_ip, caster_port)
client_socket.connect(caster_address)

try:
    print("Connected to the caster server.")

    # Create an HTTP request with Basic Authentication
    auth_string = f"{username}:{password}"
    base64_auth = base64.b64encode(auth_string.encode('utf-8')).decode('utf-8')
    request_message = f"GET /{stream_identifier} HTTP/1.1\r\n"
    request_message += f"Authorization: Basic {base64_auth}\r\n\r\n"

    # Send the request
    client_socket.send(request_message.encode('utf-8'))

    i = 0
    while True:
        # Simulate GGA sentence
        gga_sentence = '$GNGGA,103805.000,5211.6134400,N,02055.2583000,E,2,50,0.47,118.16,M,39.11,M,,*70\r\n'
        
        # Send GGA sentence to the caster server
        client_socket.send(gga_sentence.encode('utf-8'))

        # Receive data from the caster server
        data = client_socket.recv(10240)

        if not data:
            # No more data from the server, break the loop
            break

        # Process the received data as needed
        if i != 0:
            msg = RTCMReader.parse(data)
            print("Received data:", msg)
        i = i + 1

        # Sleep for a while before sending the next GGA sentence
        time.sleep(1)

except KeyboardInterrupt:
    print("Connection closed by user.")
finally:
    # Clean up the connection
    client_socket.close()
