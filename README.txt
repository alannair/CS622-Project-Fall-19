The source code for my project submission as a component of CS622 (Fall 
Semester 2019) at IIT Kanpur, taken by Prof. Mainak Chaudhary. Submitted
by Group 7 (Alan Nair, 170077).


First download the SPEC CPU traces:
 $cd scripts
 $./download_dpc3_traces.sh

To build baseline cache run
 $./build_cache.sh bimodal no no no no lru 1 

To build CMPSVR cache run
 $./fake_build_cache bimodal no no no no lru 1 [any-name]

The binaries will be stored in bin.

Run them as follows:

 $./run_champsim.sh [BINARY] [NUM_WARMUP] [NUM_SIM] [TRACE]

NUM_WARMUP is the number of instructions to be used for warmup (in millions)
NUM_SIM is the number of instructions to be used for simulation (in millions)
TRACE is the name of the SPEC 2017 trace in 


The results will be stored in a folder results_{NUM_WARMUP}

An example would be 

 $./fake_build_cache bimodal no no no no lru 1 cmpsvr
 $./run_champsim.sh fake-bimodal-no-no-no-no-lru-lcore-cmpsvr 50 200 600.perlbench_s-210B.champsimtrace.xz

The result would be stored in
 results_200M/600.perlbench_s-210B.champsimtrace.xz-fake-bimodal-no-no-no-no-lru-1core-cmpsvr.txt

which means results_{NUM_SIM}/{TRACE}-{BINARY}.txt
