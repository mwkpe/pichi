Running the program
---
Start the program with `sudo ./pichi`.

The program works without sudo but won't be able to access the Raspberry Pi's 1MHz [system timer](src/timer.cpp#L14). This timer is only used for some delay measurements (see [sender system delay](README_USAGE.md#data-format)) and is not required. Values are simply 0 without sudo.

Note: Logs will be put into a `logs` directory which must be manually created next to the executable.

Command line interface
---
It's also possible to start the program without the GUI by passing the `--nogui` option and one additional option specifying the mode you want to run, e.g.: `sudo ./pichi --nogui --transmit`. Any further terminal input will stop the program again.

| Mode          | Description  |
| ------------- | ------------ |
| --transmit    | Read NMEA sentences from gnss_port and transmit the positional data to trans_ip/port |
| --receive     | Log positional data received on recv_ip/port |
| --log         | Read NMEA sentences from gnss_port and log positional data |
| --debug       | Print NMEA sentences read from gnss_port to terminal |

The port, IP and other variables must be set in the `config.json`. If the file doesn't exist start the program once.

Data format
---
The data packets are defined in the [packet.h](src/gnss/packet.h) file.

The UDP receive CSV file has the following columns.

| #   | What             | Description   | Unit | Short |
| ---:| ---------------- | ------------- | ---- |:-----:|
| 1 | Device ID        | Identifies the sender |  | ✓
| 2 | Packet type      | Defined in [packet.h](src/gnss/packet.h) |   |
| 3 | UDP receive time | Time the packet was received | ns | ✓
| 4 | Transmit delay   | Transmission delay between sender and receiver<br>(devices must be synchronized) | ns |
| 5 | Sender system delay | Time the transmitter took to send the packet<br>(since serial read) | µs |
| 6 | Packet counter   | Indicates transmit ordering and missing packets |   |
| 7 | UTC timestamp    | UTC timestamp from NMEA sentence |   | ✓
| 8 | Latitude         | Position from NMEA sentence | deg [±90°] | ✓
| 9 | Longitude        | Position from NMEA sentence | deg [±180°] | ✓

The serial read CSV file has the following columns.

| #   | What             | Description   | Unit |
| ---:| ---------------- | ------------- | ---- |
| 1 | Serial read time | Time the NMEA sentence was read | ns |
| 2 | UTC timestamp    | UTC timestamp from NMEA sentence |   |
| 3 | Latitude         | Position from NMEA sentence | deg [±90°] | 
| 4 | Longitude        | Position from NMEA sentence | deg [±180°] |
