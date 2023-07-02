## concept for the data-flow between smart pouch and a cloud-server for visualization

![Foo](docs/data-flow-macroscopic-www.svg)


### setup of the cloud server

1. install `influxdb` (please note, because of the line-protocol the legacy-version 1.8 of influxdb is used). See the manual [here](https://docs.influxdata.com/influxdb/v1.8/introduction/get-started/)
2. add the database `master` to the influxdb. Therefor, start the database-shell via `influx` in a terminal and enter `CREATE DATABASE master`.

3. install `grafana` and add the influx-server as a data-source. See the manual [here](https://grafana.com/tutorials/install-grafana-on-raspberry-pi/). The manual is a little bit out of date but still works fine.

4. install `apache2` and setup `certbot` afterwards for https. install the packages `certbot` and `python3-certbot-apache`. Run `certbot --apache` afterwards. follow the instructions there.

5. modify the `apache2`-configuration files:
- `/influx/`-namespace is redirected to the internal port 8086. For authentification, instead of the influx-functionality a simple implementation based on `.htaccess` files stored in `/etc/apache2` is used.
- all other requests are redirected to the grafana instance. note, that the apache-server must also be configured for websockets.

