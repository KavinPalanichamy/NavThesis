from serial import Serial
import serial

# Specify the serial port and baud rate
serial_port = '/dev/ttyUSB0'  # Change this to your specific serial port
baud_rate = 115200  # Change this to the baud rate of your device

# Open the serial port
ser = serial.Serial(serial_port, baud_rate)

try:
    while True:
        # Read a line from the serial port
        serial_data = ser.readline().decode('utf-8').strip()

        # Check if the message starts with '$GNGGA'
        if serial_data.startswith('$GNGGA'):
            print("Received GGA sentence:", serial_data)

except KeyboardInterrupt:
    print("Serial reading stopped by user.")

finally:
    # Close the serial port when done
    ser.close()
