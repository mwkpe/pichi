Running the program
---
Start the program with `sudo ./pichi`.<br>
The program works without sudo but won't be able to access the Raspberry Pi's 1MHz [system timer](src/timer.cpp#L14). This timer is only used for internal delay measurements and is not required for anything. Values are simply 0 without sudo.

Command line interface
---
It's also possible to start the program without the GUI by passing the `--nogui` option and one additional option specifying the mode you want to run, e.g.: `sudo ./pichi --nogui --transmit`.

| Mode          | What it does  |
| ------------- | ------------- |
| --transmit    | Read NMEA sentences from gnss_port and transmit the positional data to trans_ip/port |
| --receive     | Log positional data received on recv_ip/port |
| --log         | Read NMEA sentences from gnss_port and log positional data |
| --debug       | Print NMEA sentences read from gnss_port to terminal |

The port and IP variables must be set in the `config.json`. If the file doesn't exist start the program once.

Data format
---
The data packets are defined in the [packet.h](src/gnss/packet.h) file.

The UDP receive log file has the following columns.

| Mode             | Description   | Unit |
| ---------------- | ------------- | ---- |
| Device ID        | Identifies the sender |  |
| Packet type      | Defined in [packet.h](src/gnss/packet.h) |   |
| UDP receive time | Time the packet was received | ns |
| Transmit delay   | Transmission delay between sender and receiver<br>(devices must be synchronized) | ns |
| Sender System delay | Time the transmitter took to send the packet (since serial read) | µs |
| Packet counter   | Indicates transmit ordering and missing packets |   |
| UTC timestamp    | UTC timestamp from NMEA sentence |   |
| Latitude         | Position fix from NMEA sentence | decimal degrees [±90°] | 
| Longitude        | Position fix from NMEA sentence | decimal degrees [±180°] |

The serial read log file has the following columns.

| Mode             | Description   | Unit |
| ---------------- | ------------- | ---- |
| Serial read time | Time the NMEA sentence was read | ns |
| UTC timestamp    | UTC timestamp from NMEA sentence |   |
| Latitude         | Position from NMEA sentence | decimal degrees [±90°] | 
| Longitude        | Position from NMEA sentence | decimal degrees [±180°] |
