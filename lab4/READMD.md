# Spring 2020 Operating System
### Lab4: Paging
### Name: Yukun Jiang
### Net id: jy2363
### Email: jy2363@nyu.edu

## Hello, welcome to my documentation. It's gonna be brief, please do read them.


### 1. How to run my code?
After you have uploaded the code file “paging.cpp", "random-numbers.txt", all input files to the nyu crackle system. Type in the command window:
```
g++ -std=c++11 -o paging paging.cpp
```
then Type in:
```
./banker 10 10 20 1 10 lru 0
```
Here you go!
The Last number could be 0 (no-debug print) or 1(with debug info), it's up to you~I implement them both.

### 2. Input specification & Important assumptions
1. The assumption mostly comes from the input specification. For example, we assume all process have the same number of reference to make. And the way we access random number generator is also specified by the promt

### 3. Design Pattern
I adopt OOP sysle programming using C++ in this lab. But I am being a little lazy there as you can see I didn't implement so many getter() and setter(), instead I will put all the class member and class method in the public scope so that we can easily access it. Admittedly this is not ideal and safest design, but given the scale of this lab this is sufficient already.

After initialization of variable and data structures, the most important part lies in a double foo-loop:

```
for (int rank = 0; rank < process_num; rank++) {
	for (int ref = 0; ref < Quantum; ref++) {
		if (process[rank] not finish yet) {
			simulate next reference for this process
			calculate next reference for this process
		}
	}
}
```


### 4. Contact Info
if you run into any trouble understanding my design or compiling the code, please contact me by jy2363@nyu.edu. Reply is guaranteed within 1/2 hour.

