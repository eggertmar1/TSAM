# Scanner / Puzzlesolver 

## How to compile: 
Use 'make' command in terminal, in the directory Project1. 


## How to use: 
Run the scanner with the IP port \
``` ./scanner <remote-address> <port-low> <port-high> ```

Then run the client providing the IP address and the IP port. \
``` ./puzzlesolver <remote-address> <local-address>```

If this error occurs, ```not receiving in the oracle part: Resource temporarily unavailable```
 try running puzzlesolver again. 

Every port in the puzzlesolver is unfortunately hardcoded, except secret port two. The secret phrases are extracted. 

evil bit: line 180
checksum: line 244

