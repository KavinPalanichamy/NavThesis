import socket
import json

def receive_udp_data(port):
    # Create a UDP socket
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Bind the socket to a specific address and port
    udp_socket.bind(('localhost', port))
    
    print(f"Listening for UDP data on port {port}...")

    while True:
        # Receive data and the sender's address
        data, addr = udp_socket.recvfrom(1024)
        
        # Decode the received data as UTF-8
        received_data = data.decode('utf-8')
        
        try:
            # Parse the received JSON data
            json_data = json.loads(received_data)
            
            # Process the JSON data as needed
            print(json_data["acceleration"]["x"])
            
        except json.JSONDecodeError as e:
            print(f"Error decoding JSON: {e}")
            continue

if __name__ == "__main__":
    # Specify the UDP port to listen on
    udp_port = 5500
    
    # Call the function to receive UDP data
    receive_udp_data(udp_port)
