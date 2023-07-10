import requests
from datetime import datetime, timedelta

# Configuration
alpha_host = "http://localhost:8086"
alpha_database = "master"

beta_host = "https://sensor:wd40@smartpouch.foobar.rocks/influx"
beta_database = "master"

# Utility functions
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
