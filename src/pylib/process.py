import threading
import time
import serial

class SensorboxConnector:
    def __init__(self, event_hook=lambda x: 0):
        self.serial_port = '/dev/ttyUSB1'
        self.baud_rate = 500000
        self.serial = None
        self.thread = None
        self.is_running = False
        self.event_hook = event_hook
        self.state_lookup = ["standstill", "mvmnt_planar", "accel_planar", "mvmnt_downward", "mvmnt_upward", "free_fall"]

        self.µc_time = 0
        self.raspi_time = time.time()

        self.events = []

    def connect(self):
        self.serial = serial.Serial(self.serial_port, self.baud_rate)
        self.is_running = True
        self.thread = threading.Thread(target=self.__read_serial)
        self.thread.start()
        time.sleep(0.3)
        self.call_thread = threading.Thread(target=self.__execute_periodic)
        self.call_thread.start()
        print("dumping all old events after boot.")
        self.__call_method("dump")
        self.__call_method("clear")

    def disconnect(self):
        self.is_running = False
        if self.thread is not None:
            self.thread.join()
        if self.serial is not None:
            self.serial.close()

    def __call_method(self, method):
        if self.serial is not None and self.serial.is_open:
            self.serial.write((method+"\n").encode())

    def __read_serial(self):
        while self.is_running:
            if self.serial is not None and self.serial.is_open:
                line = self.serial.readline().decode().strip()
                self.__process(line)

    def __execute_periodic(self):
        while self.is_running:
            self.__call_method("time")
            time.sleep(15)
            self.__call_method("clear")
            time.sleep(15)

    def __process(self, line):
        data = line.split(";")
        if len(data) >= 2 and data[0] == ">":
            if data[1] == "time":
                self.µc_time = int(data[2]) / 1000
                self.raspi_time = time.time()
            elif data[1] == "anal":
                event = {
                    "time": (int(data[2])/1000 - self.µc_time) + self.raspi_time,
                    "state": self.state_lookup[int(data[3])],
                    "accel": {
                        "x": int(data[3]),
                        "y": int(data[4]),
                        "z": int(data[5])
                    },
                    "vibration": int(data[6]),
                    # "temperature": int(data[7])
                }
                self.event_hook(event)
                self.events.append(event)
            elif data[2] == "meas":
                print(line)


def event_hook(event):
    print(event["state"], "happened", time.time() - event["time"], "seconds ago!")
    pass


if __name__ == "__main__":
    connector = SensorboxConnector(event_hook)
    connector.connect()

    time.sleep(100)
    
    # connector.disconnect()