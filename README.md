# xel_miner 0.3

This is a prototype of a miner for solving XEL work packages.  This was put together as a tool to learn more about AST parsing & interpreting and is not a complete miner at this time...it is still in the earliest stages of developement and may have numerous bugs.  The funcational gaps are:

<ul>
<li>The only ElasticPL crypto function supported right now is SHA256 (however mangle_state needs to be fixed for this algo)</li>
<li>The blacklist work logic needs to be redone</li>
</ul>

Below are the steps I used to get the miner running on my Raspberry Pi.
<ul>
<li>sudo apt-get update</li>
<li>sudo apt-get install cmake libcurl4-openssl-dev libudev-dev screen libtool pkg-config libjansson-dev</li>
<li>git clone https://github.com/sprocket-fpga/xel_miner.git</li>
<li>cd xel_miner</li>
<li>mkdir build</li>
<li>cd build</li>
<li>cmake ..</li>
<li>make</li>
</ul>

To run the Miner

    sudo ./xel_miner -t <num_threads> -P <secret_phrase> -D -o http://127.0.0.1:6876/nxt -k <public_key>

Use "sudo ./xel_miner -h" to see a full list of options.

#Credits
<ul>
<li>The core of the miner is based on cpuminer</li>
<li>The ElasticPL / Work Package logic is based on the tireless efforts of Evil-Knievil</li>
<li>The Elastic project can be found here: https://github.com/OrdinaryDude</li>
</ul>
