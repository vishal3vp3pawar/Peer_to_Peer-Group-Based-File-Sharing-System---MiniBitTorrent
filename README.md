# Group-based-file-sharing-system

## Commands to run the program : 

**To compile tracker** : g++ tracker.cpp -o tracker -pthread 

**To compile client**  : g++ client.cpp -o client -lssl -lcrypto -pthread

**To run tracker** : ./tracker tracker_info.txt 1

**To run client** : ./client 10.0.2.15:2000 tracker_info.txt
