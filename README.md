# xel_miner 0.8

<a href="https://scan.coverity.com/projects/ordinarydude-xel_miner">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/10948/badge.svg"/>
</a>  <img src="https://travis-ci.org/OrdinaryDude/xel_miner.svg?branch=master"/>


This is a prototype of a miner for solving XEL work packages.  This was put together as a tool for me to learn more about AST parsing & interpreting. I ultimately decided to convert it into an XEL miner; however, it is not optimized at all...it is a prototype that attempts demonstrate all the functionality of an XEL miner.

Please note, the following algos were not included in this miner as they aren't available directly in openssl:

<ul>
<li>SEPC_192R1</li>
<li>SEPC_256R1</li>
<li>RIPEMD128</li>
<li>TIGER</li>
</ul>

Also:  All crypto functions will need thorough testing.  Only minimal testing on them has been performed at this time.

The miner build has been tested using GCC in Linux as well as MinGW32.  It can also be built using Visual Studio but numerous additional project files are required to do so.

Below are the steps I used to get the miner running on my Raspberry Pi.
<ul>
<li>sudo apt-get update</li>
<li>sudo apt-get install cmake libcurl4-openssl-dev libudev-dev screen libtool pkg-config libjansson-dev</li>
<li>git clone https://github.com/sprocket-fpga/xel_miner.git</li>
<li>cd xel_miner</li>
<li>cd build</li>
<li>cmake ..</li>
<li>make install</li>
</ul>

<b>*** Don't forget to use "make install" and not just "make" ***</b>

To run the Miner

    sudo ./xel_miner -t <num_threads> -P <secret_phrase> -D -o http://127.0.0.1:6876/nxt

To run the Miner w/o a compiler installed

    sudo ./xel_miner -t <num_threads> -P <secret_phrase> -D -o http://127.0.0.1:6876/nxt --no-compile

Use "sudo ./xel_miner -h" to see a full list of options.

________________________________________________________________________________________________
<b>BTC:   1Kciogbv4DXR6A2N4ke5misuXcZH8rfTrt</b>
________________________________________________________________________________________________

#Credits
<ul>
<li>The core of the miner is based on cpuminer</li>
<li>The ElasticPL / Work Package logic is based on the tireless efforts of Evil-Knievel</li>
<li>The Elastic project can be found here: https://github.com/OrdinaryDude</li>
</ul>
