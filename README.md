# You Spin Me Round Robin

A C Program that simulates a round robin CPU scheduling algorithm, using a queue to manage the processes.

## Building
To build the program executable, run the following command in the terminal:
```shell
make
```

## Running
To run the program after building the executable, use the following command:
```shell
./rr <input_file> <quantum_length>
```
Where `<input_file>` is the name of the input file containing the processes to be scheduled, and `<quantum>` is the time quantum for the round robin algorithm.

## Results 
The program will output the results of the scheduling algorithm to the terminal. The results will print the average waiting time and average response time in the following format for the specified quantum:
```shell
Average waiting time: <waiting_time>
Average response time: <response_time>
```
Where `<waiting_time>` and `<response_time>` are the average waiting and response times for the processes in the input file, shown to 2 decimal places.

## Cleaning up
To clean up the executable and object files, run the following command in the terminal:
```shell
make clean
```
