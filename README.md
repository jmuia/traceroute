# Traceroute

An implementation of traceroute.

# Running
`traceroute` should be portable, but it's only tested on Mac OS X 10.11.6 and Ubuntu 16.04.04.

Create binary
`$ make`

If your operating system supports capabilites, grant `traceroute` the abilty to open raw sockets. Otherwise, run with `sudo`.
`$ ./bin/traceroute example.com`

If you wan to run the tests
`$ make test`


## TODO
- Group responses by IP when printing
- Command-line parsing for basic options
- Support IPv6
- Support ICMP and TCP probes
- Support advanced options
- Test tools so network is not required

