Description:

The purpose of this simulation is to showcase the throughput region and mean packet delad of a number of polices that control packet transmissions in a telecommunications system described in:

http://ieeexplore.ieee.org/xpl/login.jsp?tp=&arnumber=6991509&url=http%3A%2F%2Fieeexplore.ieee.org%2Fiel7%2F49%2F7055202%2F06991509.pdf%3Farnumber%3D6991509

I will briefly describe the system here, for more information, please refer to the paper.

Consider a one-hop broadcast channel with two receivers. The receivers have side information obtained by overhearing wireless channels. Two packet flows, each destined for a single receiver, arrive at the relay. The relay takes control decisions by coding transmissions based on its knowledge of side information in the receivers. There are two control mechanisms. In the ACK system, the relay has definite knowledge of side information announced via overhearing reports. In the NACK system, the relay has statistical knowledge of side information and receives feedback after every decoding failure. 


Implementation: 

Because of the random nature of packet arrivals, transmission policies, and correct reception, the simulation must run for a large number of timeslots to derive the mean throughouput and packet delay (see law of large numbers). Due to this demand, a fast language along with optimized data structures had to be used. The C programming language was chosen and the libraries used are gsl ( gnu scientific library for random number generation and other utilities) and glib ( library used for the construction of GNOME desktop environment), which contains data structures like hash tables and queues.

The policies tested are the two policies described in the paper, Random Linear Network Coding, Backpressure algorithm and the COPE policy, described here:

http://nms.csail.mit.edu/~sachin/papers/copesc.pdf

Compile: 

To compile the code, open a terminal at the directory you installed the code and execute make. This was tested on the Linux Mint 16 operating system. The gsl and glib libraries must be installed as well as gnuplot for plotting the results.

Run:

To run the program:
1) Go to sim_main
2) You may choose between three modes of operation, delay, stability and instance.

	*** stability: This runs multiple instances of arrivals so that a throughput region is formed. The throughput region is the set of all sustainable arrival 			       rates such that the packet queues do not overflow with packets.
	1) edit the file input_stability.txt
	2) at the first line, the string "stability" must be present
	3) at the second line, input the number of timeslots the simulation will run
	4) at the third line input the probabilites for overhearing for receivers 1 and 2 respectively
	5) at the fourth line input the threshold number of packets above which a queue is considered to overflow.
	6) at the fifth line input the number of points to compute for the throughput region line. This line is the boundary of the throughput region. The more
	   points, the more detailed the line will be, but the simulation will be slower.
	7) run the command sim < input_stability.txt
	8) the output is given in a single file with four columns, plot_stability.out. 
		- The first column represents the arrival rate of flow 1 (x coordinate for the throughput region line) for the ACK system 
		- The second column represents the arrival rate of flow 2 (y coordinate for the throughput region line) for the ACK system 
		- The third column represents the arrival rate of flow 1 (x coordinate for the throughput region line) for the NACK system 
		- The fourth column represents the arrival rate of flow 2 (y coordinate for the throughput region line) for the NACK system 
	9) A plot of the throughput region line will also be presented with gnuplot and saved at stability_region.svg

	*** delay: This runs multiple instances of arrivals so that an arrival rate - mean delay plot is formed. This arrival rate is also called load scaling
		   coefficient. The arrival rate of flow 1 and 2 are computed with respect to this coefficient. For example, if arrival rate λ is tested, then
		    λ1 =  aλ and λ2 = bλ, where a and b are constants defined in the input_delay.txt file
	1) edit the file input_delay.txt
	2) at the first line, the string "delay" must be present
	3) at the second line, input the number of timeslots the simulation will run
	4) at the third line input the probabilites for overhearing for receivers 1 and 2 respectively
	5) at the fourth line input the number of points for the arrival rate - mean delay plot.
	6) at the fifth line input the mean delay threshold for the plot (more delay than this and the point is not represented)
	7) at the sixth line input the largest tested arrival rate (the simulation runs for arrival rates from zero to this threshold)
	8) at the seventh line input the ratio of the flow 1 and flow 2 arrival rates to the tested arrival rate (coefficients a and b as described above)
	9) run the command sim < input_delay.txt
	10) the output is given in a single file with four columns, plot_delay.out. 
		- The first pair of columns represents the arrival rate - mean delay points of our deterministic policy (described in the paper) for the ACK system 
		- The second pair of columns represents the arrival rate - mean delay points of our stochastic policy (described in the paper) for the NACK system 
		- The third pair of columns represents the arrival rate - mean delay points of the RLNC policy for both systems
		- The fourth pair of columns represents the arrival rate - mean delay points of the backpressure policy for the NACK system 
		- The fifth pair of columns represents the arrival rate - mean delay points of the backpressure policy for the ACK system 
	11) A plot of the arrival rate - mean delay lines will also be presented with gnuplot and saved at delay.svg

	*** Instance: This runs a simple instance of arrivals and the response of the system to it. Its purpose is to measure the difference between delay and queue 			      backlogs of the system.
	1) edit the file input_instance.txt
	2) at the first line, the string "instance" must be present
	3) at the second line, input the number of timeslots the simulation will run
	4) at the third line input the probabilites for overhearing for receivers 1 and 2 respectively
	5) at the fourth line input the poisson arrival rate of each flow (flow 1 and 2 respectively)
	6) run the command sim < input_instance.txt
	7) the output is given in four files, plot_det_backlog.out and plot_stoch_backlog.out, plot_det.out and plot_stoch.out. They represent:
		- The queue backlog of the two flows at each timeslot for the ACK and NACK system respectively
		- The packet delay of the two flows at each timeslot for the ACK and NACK system respectively
	8) A plot of the backlog and delay lines will also be presented and saved at instance_det.svg and instance_stoch.svg for the ACK and NACK systems respectively


Notes:

Code and Libraries need some more comments.
