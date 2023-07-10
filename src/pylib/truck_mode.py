import threading
import time
import serial
import requests
import logging
from datetime import datetime, timedelta

# Configuration for yellow
alpha_host = "http://localhost:8086"
alpha_database = "master"

beta_host = "https://sensor:wd40@smartpouch.foobar.rocks/influx"
beta_database = "master"

class SensorboxConnector:
    def __init__(self, event_hook=lambda x: 0):
        self.serial_port = '/dev/ttyUSB0'
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
        print("dump completed.")

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
        try:
            if len(data) >= 2 and data[0] == ">":
                if data[1] == "time":
                    self.µc_time = int(data[2]) / 1000
                    self.raspi_time = time.time()
                elif data[1] == "anal":
                    event = {
                        "type": "anal",
                        "time": (int(data[2])/1000 - self.µc_time) + self.raspi_time,
                        "state_str": self.state_lookup[int(data[3])],
                        "state": int(data[3]), 
                        "accel": {
                            "x": int(data[3]),
                            "y": int(data[4]),
                            "z": int(data[5])
                        },
                        "vibration": int(data[6]),
                        "temperature": int(data[7])
                    }
                    self.event_hook(event)
                    self.events.append(event)
                elif data[1] == "tris":
                    event = {
                        "type": "tris",
                        "time": (int(data[2])/1000 - self.µc_time) + self.raspi_time,
                        "temperature": int(data[9]), 
                        "mean": {
                            "magnitude": data[3],
                            "phi": data[4],
                            "theta": data[5],
                            },
                        "stdev": {
                            "magnitude": data[6],
                            "phi": data[7],
                            "theta": data[8],
                            },
                    }
                    self.event_hook(event)
                    self.events.append(event)
                elif data[2] == "meas":
                    print("MEAS", line)
                elif data[2] == "stat":
                    print("STAT", line)
        except BaseException: 
            pass
        
    def set_parameter(param_name, value):
        if param_name == "treshold_planar":
            self.__call_method(f"trsh_plnr {value}")
        elif param_name == "treshold_vibration":
            self.__call_method(f"trsh_vibr {value}")
        elif param_name == "treshold_z-axis":
            self.__call_method(f"trsh_zaxs {value}")
        



def event_hook(event):
    url = "http://localhost:8086"
    time_str = f"{round(event['time']*1e6):100.0f}".strip()
    if event["type"] == "anal":
        print(event["state"], "happened", time.time() - event["time"], "seconds ago!")
        payload = f"schnieboard_events,sensor=pouch01,type=anal state={event['state']},x={event['accel']['x']},y={event['accel']['y']},z={event['accel']['z']},vib={event['vibration']} {time_str}"
    elif event["type"] == "tris":
        print(event)
        payload = f"schnieboard_events,sensor=pouch01,type=tris magni_mean={event['mean']['magnitude']},phi_mean={event['mean']['phi']},theta_mean={event['mean']['theta']},magni_stdev={event['stdev']['magnitude']},phi_stdev={event['stdev']['phi']},theta_stdev={event['stdev']['theta']},temperature={event['temperature']} {time_str}"
    # try:
    if True: 
        r = requests.post(url+"/write?db=master&precision=u", payload, timeout=1)
        print(r, r.text)
    # except BaseException as e:
        # pass


# Utility functions for yellow
def get_influxdb_url(host, database):
    return f"{host}/query?db={database}&format=line"

def query_influxdb(host, database, query):
    url = get_influxdb_url(host, database)
    params = {
        "q": query
    }
    response = requests.get(url, params=params)
    response.raise_for_status()
    return response.json()

def write_to_influxdb(host, database, data):
    url = host + "/write?precision=u&db=" + database
    # headers = {
    #     "Content-Type": "application/octet-stream"
    # }
    response = requests.post(url, data=data)
    # response.raise_for_status()
    print(response.text)


message_queue = []

def sendTelegramMessage(message):
    message_queue.append(message)

def telegramSendqueue():
    while True:
        if len(message_queue) >= 1:
            token = "1007402523:AAFSfoh9NhYe-hDh6bt4dboLnaPdCdZ_Q9U"
            url = f"https://api.telegram.org/bot{token}/sendMessage"
            try:
                response = requests.post(url, json={"chat_id": -1001917168271, "text": message_queue[0]}, timeout=5)
                if response.status_code == 200:
                    message_queue.pop(0)
            except BaseException as e:
                time.sleep(5)
        time.sleep(0.1)



    
# setup logging
stream_handler = logging.StreamHandler()
stream_handler.setLevel(logging.DEBUG)
logging.basicConfig(level=logging.DEBUG,
        format='[%(asctime)s] %(message)s',
        handlers=[logging.FileHandler("truck_mode.log", mode='a'),
        stream_handler])
logging.getLogger("requests").setLevel(logging.WARNING)

TGthread = threading.Thread(target=telegramSendqueue)
TGthread.start()

# logmessage = 'just booted'
# logging.debug(logmessage)
# sendTelegramMessage(logmessage)

connector = SensorboxConnector(event_hook)
connector.connect()
time.sleep(100)
# connector.disconnect()

logmessage = 'ran orange successfully, now trying yellow'
logging.debug(logmessage)
sendTelegramMessage(logmessage)


# YELLOW

alpha_url = get_influxdb_url(alpha_host, alpha_database)

# Get the current time and the time 10 minutes ago
end_time = datetime.utcnow()
start_time = end_time - timedelta(minutes=11)

# Query the tables in "master" database on "alpha"
query = f'SHOW MEASUREMENTS ON "{alpha_database}"'
response = query_influxdb(alpha_host, alpha_database, query)
print(response)
tables = [table[0] for table in response['results'][0]['series'][0]['values']]

# Read data from each table within the last 10 minutes
points_to_write = []
for table in tables:
    if table != "___________________________*":
        query = f'SHOW FIELD KEYS FROM "{table}";'
        query += f'SELECT * FROM "{table}" WHERE time >= \'{start_time.strftime("%Y-%m-%dT%H:%M:%SZ")}\' AND time <= \'{end_time.strftime("%Y-%m-%dT%H:%M:%SZ")}\''
        response = query_influxdb(alpha_host, alpha_database, query)
        if "series" in response["results"][0]:
            value_fields = [_[0] for _ in response["results"][0]["series"][0]["values"]]

            for series in response["results"][1].get("series", []):
                measurement = series["name"]
                columns = series["columns"]
                values = series["values"]

                for value_set in values:
                    fields = dict(zip(columns, value_set))
                    tim = int(datetime.strptime(fields["time"].split(".")[0], "%Y-%m-%dT%H:%M:%S").timestamp()) + float("0." + fields["time"].split(".")[1].split("Z")[0])
                    tim = f"{(tim*1e6):200.0f}".strip()
                    del fields["time"]
                    tags_str = ",".join([f'{field}={value}' for field, value in fields.items() if value != None and field not in value_fields])
                    fields_str = ",".join([f'{field}={value}' for field, value in fields.items() if value != None and field in value_fields])
                    line = f'{measurement}{"," + tags_str if tags_str else ""} {fields_str} {tim}'
                    points_to_write.append(line)


write_to_influxdb(beta_host, beta_database, "\n".join(points_to_write))


logmessage = 'finished yellow'
logging.debug(logmessage)
sendTelegramMessage(logmessage)
    
    
    
    
    
    
    
    
    
    
    
    
