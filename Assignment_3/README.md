# CS-224 Assignment-3 (Socket Programming)
In this Assignment, we have implemented the Stop-and-Wait Algorithm for Automatic Repeat Request (ARQ) using UDP (Datagram) Sockets in C. There are two C files ```sender.c``` and ```receiver.c``` for the sender and receiver respectively. These have been compiled into ```sender``` and ```receiver``` respectively.

## Sender Files
To run the compiled file ```sender```, use the command ```./sender <sender_port> <receiver_port> <timeout_value> <packet_count>``` (or ```sender <sender_port> <receiver_port> <timeout_value> <packet_count>``` on Windows). ```sender_port``` is the port number of the sender and ```receiver_port``` is that of the receiver. As always, make sure both of these are different. ```timeout_value``` is the time (in seconds) for which the sender will wait for an acknowledgement from the receiver. ```packet_count``` is the total number of packets that you want the sender to send. These packets contain the message ```Packet:<i>``` where ```i``` goes from 1 to ```packet_count```. Once a packet is sent, the sender will wait for ```timeout_value``` no. of seconds for an acknowledgement. If it doesn't receive the acknowledgement, it will retransmit the same packet or else it will transmit the next packet requested by the receiver.

## Receiver Files
To run the compiled file ```receiver```, use the command ```./receiver <receiver_port> <sender_port> <drop_probability>``` (or ```receiver <receiver_port> <sender_port> <drop_probability>``` for Windows). ```sender_port``` is the port number of the sender and ```receiver_port``` is that of the receiver. As always, make sure both of these are different. ```drop_probability``` is the probability that a packet gets dropped and is hard coded in ```receiver.c```. The receiver keeps track of the last packet it received and requests for the next one when it sends an acknowledgement.

## Additional Functionality
Apart from the functionalities mentioned in the Assignment, we have also added a global timeout which will wait upto 10 minutes (by default) if there is no activity. Activity here refers to getting a packet (either message or acknowledgement) from the other port. This is added in both ```sender.c``` as well ```receiver.c``` and can be changed by changing the macro ```MAXINACTIVETIME``` in these files. Note that after editing them, you need to recompile these files using the GCC compiler.

## Example Run
The ```sender.txt``` and ```receiver.txt``` files contain the outputs of ```sender``` and ```receiver``` respectively where we had set ```packet_count``` as 20 and ```drop_probability``` as 0.5.
