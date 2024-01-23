import socket
import json

def receive_udp_data(port):
    # Create a UDP socket
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.bind(('localhost', port))
    print(f"Listening for UDP data on port {port}...")
    while True:
        data, addr = udp_socket.recvfrom(1024)
        received_data = data.decode('utf-8')
        try:
            json_data = json.loads(received_data)
            print(json_data["acceleration"]["x"])
        except json.JSONDecodeError as e:
            print(f"Error decoding JSON: {e}")
            continue


udp_port = 5500
receive_udp_data(udp_port)
