# optionscat

![alt tag](https://github.com/CoolerVoid/optionscat/blob/master/doc/optionscat.png)
European options tool, compound calc, finance manager for traders... 

The Black-Scholes model for calculating the premium of an option was introduced in 1973 in a paper entitled, "The Pricing of Options and Corporate Liabilities" published in the Journal of Political Economy. The formula, developed by three economists – Fischer Black, Myron Scholes and Robert Merton – is perhaps the world's most well-known options pricing model. Black passed away two years before Scholes and Merton were awarded the 1997 Nobel Prize in Economics for their work in finding a new method to determine the value of derivatives (the Nobel Prize is not given posthumously; however, the Nobel committee acknowledged Black's role in the Black-Scholes model).

Read more: Options Pricing: Black-Scholes Model | Investopedia http://www.investopedia.com/university/options-pricing/black-scholes-model.asp#ixzz4T9rfuUyJ
Follow us: Investopedia on Facebook



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

https://127.0.0.1:1347

Only address 127.0.0.1 can view the program.

# NOTEs 

1- At second execution if return error, you need kill old process at port 1347:

$ fuser -k -n tcp 1347

2- If you want use at network, Whitelist to access server you can edit at "config/whitelist.conf"


# At the future

* Remote CSV download of google finance and yahoo fianance
* Options to digest CSV to black scholes table
* CRUD to trade risk manager
* Plot Gain and loss of all trades

Cheers !
