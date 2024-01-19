import socket
import base64
import time
import sys
import serial


def read_serial_data(serial_port = '/dev/ttyUSB0' , baud_rate = 115200, keyword='$GNGGA'):
        # Open the serial port with error handling
        ser = serial.Serial(serial_port, baud_rate, timeout=1)
        while True:
            # Read a line from the serial port
            serial_data = ser.readline().decode('utf-8').strip()

            # Check if the message starts with the specified keyword
            if serial_data.startswith(keyword):
                ser.close()
                return(serial_data)


def send_serial(stream):
    # Opening the serial port using the with statement
    with serial.Serial("/dev/ttyUSB0", 115200) as ser:
        # Writing the data to the serial port
        ser.write(stream)
        ser.write(b"\r\n")
        ser.close()

def connect_to_caster(caster_ip = "91.198.76.2", caster_port = 8080, stream_identifier = "NAWGEO_POJ_3_1", username = "Kavin", password = "Kavin@4236", wait_time):

    stream_identifier = "NAWGEO_VRS_3_1"
    #stream_identifier = "JOZ2_RTCM_3_1"
    # Caster server credentials

    # Create a TCP/IP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect to the caster server
    caster_address = (caster_ip, caster_port)
    client_socket.connect(caster_address)
    print("\n>>>>>>>>>>Connected to the caster server<<<<<<<<<<<<<<\n")

    # Create an HTTP request with Basic Authentication
    auth_string = f"{username}:{password}"
    base64_auth = base64.b64encode(auth_string.encode('utf-8')).decode('utf-8')
    request_message = f"GET /{stream_identifier} HTTP/1.1\r\n"
    request_message += f"Authorization: Basic {base64_auth}\r\n\r\n"
    client_socket.send(request_message.encode('utf-8'))

    i = 0
    
    while True:
        if i != 0:
            print("\n\n--------- NEW ITERATION -------\n")
        gga_sentence = f'{read_serial_data()}\r\n'

        if i != 0:
            print("\n->Sending GGA sentence to the server : ", gga_sentence)
        # Send GGA sentence to the caster server
        client_socket.send(gga_sentence.encode('utf-8'))

        # Receive data from the caster server
        data = client_socket.recv(10240)
        if i != 0:
            print("->RTK correction received from the server")

        if not data:
            # No more data from the server, break the loop
            break
        if i != 0:
            send_serial(data)
            print("\n->Correction Data sent to the serial port")
            print("\n->Waiting 10s.....")
            time.sleep(wait_time)
        i = i + 1
        
        
        # Sleep for a while before sending the next GGA sentence
        

    client_socket.close()

connect_to_caster()
