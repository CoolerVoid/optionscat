# optionscat

![alt tag](https://github.com/CoolerVoid/optionscat/blob/master/doc/optionscat.png)
European options tool, compound calc, finance manager for traders... 


# Screenshot

![alt tag](https://github.com/CoolerVoid/optionscat/blob/master/doc/screen.png)

# Install

So this project gonna tested at MacOs and Fedora Linux with Firefox browser.

$ git clone https://github.com/CoolerVoid/optionscat/
$ cd optionscat
$ make

Now before execute server need create cert to use TLS:

$ cd cert; openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:2048 -keyout privateKey.key -out certificate.crt
$ cat certificate.crt privateKey.key > certkey.pem
$ cd ..

$ bin/optionscat     (NOTE: note use cd bin; ./optionscat need real path)

Enter in your browser at:
https://127.0.0.1

Only address 127.0.0.1 can view the program.

# At the future

* Remote CSV download of google finance and yahoo fianance
* Options to digest CSV to black scholes table
* CRUD to trade risk manager
