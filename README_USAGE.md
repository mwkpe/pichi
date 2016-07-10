Running the program
---
Start the program with `sudo ./pichi`.<br>
The program works without sudo but won't be able to access the Raspberry Pi's 1MHz [system timer](src/timer.cpp#L14). This timer is only used for internal delay measurements and is not required for anything. Values are simply 0 without sudo.

The transmitted data is defined in the [packet.h](src/gnss/packet.h) file.

Command line interface
---
It's also possible to start the program without the GUI by passing the `--nogui` option and one additional option defining the mode you want to run, e.g.: `sudo ./pichi --nogui --transmit``.

| Mode          | What it does  |
| ------------- | ------------- |
| --transmit    | Read NMEA sentences from gnss_port and transmit the positional data to trans_ip:trans_port |
| --receive     | Log positional data received on recv_ip:recv_port |
| --log         | Read NMEA sentences from gnss_port and log positional data |
| --debug       | Print NMEA sentences read from gnss_port to terminal |

The port and IP variables must be set in the `config.json`. If the file doesn't exist start the program once.
