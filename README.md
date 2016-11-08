# xel_miner 0.5

This is a prototype of a miner for solving XEL work packages.  This was put together as a tool to learn more about AST parsing & interpreting and does not include all the functionality and optimizations that will be needed...it is still in the earliest stages of developement.  This version includes the ElasticPL Crypto Functions; however, the funcational algos were not included as they aren't in openssl:

<ul>
<li>SEPC_192R1</li>
<li>SEPC_256R1</li>
<li>RIPEMD128</li>
<li>TIGER</li>
</ul>

Note:  The functions will need thorough testing.  Only minimal testing on them has been performed at this time.

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

To run the Miner

    sudo ./xel_miner -t <num_threads> -P <secret_phrase> -D -o http://127.0.0.1:6876/nxt

Use "sudo ./xel_miner -h" to see a full list of options.

#Credits
<ul>
<li>The core of the miner is based on cpuminer</li>
<li>The ElasticPL / Work Package logic is based on the tireless efforts of Evil-Knievel</li>
<li>The Elastic project can be found here: https://github.com/OrdinaryDude</li>
</ul>
