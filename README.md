## ping ipv4

### Usage 

```
gcc core.c log.c -o ping
sudo ./ping <IPv4>  [log dir path]  [num of requests | -t]
```
### Example

```
sudo ./ping 8.8.8.8 10    // pinging 8.8.8.8 for 10 times
```

```
sudo ./ping 8.8.4.4 -t   // cyclic ping for 8.8.4.4 
```
