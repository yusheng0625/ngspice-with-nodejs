# Sahara Cloud Device Server 

_Devsrv_ is the Sahara Cloud _Device Server_ manages the _virtual_
electronic network communications between the pins on two or more
_devices_.  

## Pre Install
python

## Run on the linux
```bash
sudo apt install build-essential
sudo apt-get install libtool
npm run ngspice-install
npm install
npm test
```

## Run on the macOS
## pre requirement
install xcode
bison >= 3.7
brew install bison
export PATH="/usr/local/opt/bison/bin:$PATH"
brew install python

```bash
xcode-select --install
brew install libtool
npm run ngspice-install
npm install
npm test
```

## Test api

```bash
cd node/tests
./devices_update.sh
./devices_get.sh
./channels_get.sh
./channels_update.sh
./sdp_config.sh
./sdp_send.sh
```

## Setup build environment

```bash
sudo apt install cmake
sudo apt-get install build-essential
sudo apt install nlohmann-json3-dev
```


## Building the Software

Make sure _cmake_ has been installed on your system. Run _cmake_ as
follows to generate the Makefiles: 

```bash
% mkdir build
% cd build
% cmake ../src
% make -j
```

To build the images with debug symbols run cmake like this:

```bash
% cmake -DCMAKE_BUILD_TYPE=Debug ../src
```

Now _make_ the _Device Server_:

```bash
make
```

## Running the Software

```bash
devserv
```

## Configuration

The software can be started with two _configuration_ files:
_devices.json_ defines the static setup of the _devices_ attached to
a _Device_ server. The second file is the _network.json_ file
responsible for connecting _outgoing_ signals with _incoming_ pins of
_channels_.

Here is an example with two Raspberry PIs representing a _mock FPGAs_
and two Linux hosts acting as _Device Servers_. The logical network
diagram looks like:

<pre>

                  10.11.1.11                10.11.5.3

      +------+     +------+     +------+     +------+
      | rpi1 | --> |  ds1 | --> |  ds2 | --> | rpi2 |
      +------+     +------+     +------+     +------+
               sdp          snp          sdp

     10.11.3.3                 10.11.10.10 

</pre>

## Device and Device Server Configuration

The following _devices.json_ file provides the IP addresses of the
_Device Servers_ as well as _Devices_. This file also provides the
association of _Devices_ and the _Device Servers_ they are attached
to. 

```javascript
{
    "device-servers": [
        {
            "dsaddr": "10.11.10.10",
            "port": 3000,
            "devices": [
	        {
	            "uri": "rpi2",
	            "interface": "eth0",
	            "ipaddr": "10.11.5.3",
	            "port": 3005,
                    "ethaddr": "b8:27:eb:b7:7a:b0"
	        }
            ]
        },
        {
            "dsaddr": "10.11.11.11",
            "port": 3000,
            "devices": [
	        {
	            "uri": "rpi1",
	            "interface": "eth0",
	            "ipaddr": "10.11.3.3",
	            "port": 3005,
                    "ethaddr": "dc:a6:32:2c:38:c2"
	        }
            ]
        }
    ]
}
```

## Network Configuration

The _network.json_ configuration file provides the _channel_ that
defines how data gets from the _output_ pins of one device to _input_
pins of another _device_.

Channels are defined as output device(s) and pin(s) to input
devices(s) to and pin(s), (note currently only single input and output
pins are supported).

Our example network config could be two devices: **rpi1** and **rpi2**
each has one output pin connected to the others _input_ pin. 

```javascript
{
    "channels": [
        {
            "output": "rpi1.0",
            "input": "rpi2.0"
        },
        {
            "output": "rpi2.1",
            "input": "rpi1.1"
        }
}
```

A logical diagram of two devices sharing two _channels_ flowing in
opposite directions. An example could be a switch on **rpi1** sending data
to an LED on **rpi2**.

<pre>

      +------+     +------+
      |   p0 + --> + p0   |
      |      |     |      |
      | rpi1 |     | rpi2 |
      |      |     |      |
      |   p1 + <-- + p1   |
      |      |     |      |
      +------+     +------+

</pre>

## Forwarding Channels

These two pieces of information give the _Devsrv_ software sufficient
information to gather _SDP_ packets sent from an FPGA, translate the
data payloads into one or more _SNP_ packets each packet sent to the
respective DS(s) with a connected _input_ _Device_.

In otherwords: _Data_ transmitted from **rpi1** with be translated into an
SNP packet sent **ds2**, which in turn translates the data from the
_SNP_ message into an SDP message sent to the _intput device_.

## Running Device Servers

Running the _Device Server_ with the appropriate configuration is all
that is required: on the hosts that will act as **ds1** and **ds2**
simply start the _Device Server_ software **devsrv** like this:

```bash
ds1% ./devsrv
```

Two UDP sockets will be opened each listening for _SDP_ and _SNP_
packets respectively.

## Generating and Consuming SDP Packets

SDP packets will be produced by an actual FPGA. Likewise, production
will have FPGA packets consuming SDP packets. However, testing can
leverage the _sdptest_ and _devtest_ tools to mock an FPGA by acting
as a _source_ and _sink_ for _SDP_ packets.

### FGPA Mock Output

From **rpi1** we can run the _sdptest_ tool as a _client_ to produce one
or more SDP packets at various rates for example, to send an endless
stream of SDP packets with a 1sec. gap, use the _sdptest_ tool:

```bash
rpi1% sdptest -c 0 -d 1
```

### FPGA Mock Input

From **rpi2** we can also run the the _sdptest_ tool, this time in
_server_ (or recv) mode to read and decode SDP packets as they arrive
with the '-s' paramater like so:

```bash
rpi2% sdptest -s
```

The above will cause **sdptest** to run in an endless loop reading SDP
packets and optionally provide a hex dump for human consumption.


## Test Bed


1. ubuntu@andy.saharacloud.io (open ports => ssh: 22, other: 81)
password: joshpi4 (FPGA & Arduino)
2. pi@andy.saharacloud.io (open ports => ssh: 23, other: 82) password:
raspberry  
3. applepi@andy.saharacloud.io (open ports => ssh: 24, other: 83)
password: raspberry (FPGA) 
4. pecanpi@andy.saharacloud.io (open ports => ssh: 25, other: 84)
password: raspberry (FPGA) 
5. creampi@andy.saharacloud.io (open ports => ssh: 26, other: 85)
password: raspberry (FPGA) 

I believe that #1, 3, 4, 5 are connected to FPGAs and #1's FPGA is
connected to an arduino (the other ones' pins are not connected to
anything)

