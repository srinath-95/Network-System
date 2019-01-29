README file for Proxy server

In this program, I have built one proxy server program to handle multiple requests from Web Browser to access the HTTP servers. Following features have been implemented:

Multithreading:
Forking has been used to server multiple requests from browser to access the respective HTTP servers.

Caching:
1. All the requests coming from clients are served from HTTP servers and local copy of these are made.
2. Same requests coming from clients will be server from local rather than from HTTP server.
3. All the entries are saved in "cached.txt" along with timestamp.
4. If the timestamp and the request coming in has huge difference, then the request will be served from HTTP server.

IP Caching:
1. Domain name along with the IP address are recorded in "IP_cache.txt" for all the requests.
2. When request for the same domain name comes, there wont be any query with DNS server to fetch the IP address, which reduces the time of execution.

Blocked sites:
1. Requests coming for sites in "blocked.txt", would simply not process the requests.

Following are the Error conditions being handled:
1. "400 BAD REQUEST" - For requests other then "GET" command , wrong version and process only http requests
2. "403 Host name not resolved" - For bad URL
3. "404 Forbidden error" - for blocked sites

Link Prefetch(Extra credit):
Link Prefetch has been implemented which would hold the links of the page in cache so that the request can be handled in a faster manner.

How to RUN:
--> make server
--> ./server [portno] [time_out_for_cache]



