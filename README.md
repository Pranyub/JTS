# JTS (Jank Trading System)

A (WIP) Gen 8 PokeGen that uses the in-game LAN Trading System based on XTranscieverPlus.
Requires 2 Switches / copies of Pokemon Sword or Shield to function. For single player support, look to XTranscieverPlus (also WIP)

Support for Pokemon BdSp / Legends Arceus coming soon

Require [PcapPlusPlus](https://pcapplusplus.github.io/) for packet injection and [OpenSSL](https://www.openssl.org/) for encryption / decryption

## Usage

The usage is moderately complicated (hence the name 'Jank'), but should become simpler with future releases. 

JTS must be executed from the command line. Use ``` ./XtransrecieverPlus [MAC1] [MAC2] ``` , where MAC1 and MAC2 correspond to computer MAC and Switch MAC addresses. The default injection file is ``` inject.ek8 ```, located in the same program directory.


### Requirements:
- [Npcap](https://nmap.org/npcap/#download)
- Two interfaces on different networks (e.g. an Ethernet connection & a Wi-Fi connection). An internet connection is not required.
- A .ek8 file to inject. This can be created with [PKHex](https://projectpokemon.org/home/files/file/1-pkhex/)
- You must specify device MAC Addresses, as JTS doesn't currently utilize ARP Spoofing (future implementation planned)
- 
## Build Instructions

Coming Soon

## Special Thanks
Special thanks to Yannik Marchand's (and others') Contributions to the [Wiki](https://github.com/kinnay/NintendoClients/wiki) on Nintendo Protocols. Without their documentation this project would have been much, much more difficult.

Also, a shoutout to [Slashcash](https://github.com/Slashcash) who worked on a very similar [project](https://github.com/Slashcash/PSD) 

## License
[MIT](https://choosealicense.com/licenses/mit/)
