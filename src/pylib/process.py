import threading
import time
import serial

class SensorboxConnector:
    def __init__(self):
        self.serial_port = '/dev/ttyUSB0'
        self.baud_rate = 500000
        self.serial = None
        self.thread = None
        self.is_running = False

    def connect(self):
        self.serial = serial.Serial(self.serial_port, self.baud_rate)
        self.is_running = True
        self.thread = threading.Thread(target=self.__read_serial)
        self.thread.start()

    def disconnect(self):
        self.is_running = False
        if self.thread is not None:
            self.thread.join()
        if self.serial is not None:
            self.serial.close()

    def call_method(self, method):
        if self.serial is not None and self.serial.is_open:
            self.serial.write(method.encode())

    def __read_serial(self):
        while self.is_running:
            if self.serial is not None and self.serial.is_open:
                line = self.serial.readline().decode().strip()
                self.__process(line)

    def __process(self, line):
        # Add your processing logic here
        print(f"Received: {line}")


if __name__ == "__main__":
    connector = SensorboxConnector()
    connector.connect()


    connector.call_method("time")

    time.sleep(100)

    connector.disconnect()