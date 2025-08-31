# ft_ping

## Description

`ft_ping` is a minimalist tool in C that sends ICMP Echo Request packets to an IP address or hostname to test network connectivity.

The ft_ping project is an excellent introduction to low-level networking and system programming in C. It teaches us how to interact directly with the ICMP (Internet Control Message Protocol) using raw sockets to send and receive network packets.

This project encourages us to:

Understand how the ICMP protocol works, used by the ping tool to test connectivity between two machines.


Manipulate low-level network structures (such as sockaddr_in, iphdr, icmphdr) and use system calls such as sendto, recvmsg, or gettimeofday.

Manage system signals, in particular to intercept SIGINT (Ctrl+C), in order to ensure that the program shuts down cleanly and releases resources.

Pay attention to memory management, avoiding leaks, especially in interrupt contexts.

## Features

- Host name resolution
- Sending ICMP Echo Request packets
- Displaying received responses
- Clean interrupt handling (Ctrl+C) with memory release