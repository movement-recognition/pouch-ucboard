import serial

def process(input_string):
    # Your custom processing logic here
    print("Received:", input_string)

# Open the serial connection
ser = serial.Serial('/dev/ttyUSB0', baudrate=500000)

# Send commands and process output
while True:
    # Get command from user
    command = input("Enter command ('q' to quit): ")

    # Check for quitting condition
    if command.lower() == 'q':
        break

    # Write command to serial console
    ser.write(command.encode() + b'\n')

    # Read output lines
    while True:
        line = ser.readline().decode().strip()
        if not line:
            break
        process(line)

# Close the serial connection
ser.close()
