Summary:
 
The capabalities of this program are gonna be the following:
 
* Based on a .yaml file used for configuration, we will be able to open sockets in both udp or tcp. You will be able to configure the type of conexion(udp or tcp), the protocol (custom udp or tcp), the endianess (big or little) of the data, the host, the localport, the remoteport, the frequency and the size of the incoming and outgoing buffers.
 
* There will be a custom protocol for both udp and tcp
 
* We will have a .idl file in which the data structures and available messages will be defined in a format that fits the custom udp and tcp protocols. There should only we one .idl file for both udp and tcp protocols.
 
* The program needs to work in both windows and linux, so we will use Standalone Asio to abstract the platform dependent code for managing sockets.
 
* We will use C++ 23 for this program, since we want the logging functionality present in there.