# Spring 2020 Operating System
## Lab1
### Name: Yukun Jiang
### Net id: jy2363

# Hello~ Welcome to my Lab1 documentation.
### How to run my lab?
In the terminal, (if you are already in crackle and upload the main.c file) type in "gcc Linker.c" and then type in "./a.out", here you go! 
You can just copy and paste the input into the terminal and press "Enter".


# Some Important Assumption and Limits in the Lab
1. As documented in the promt, the address should not exceed 300 which is the maximum, so whenever this happens, we replace it with 299.
2. I set the maximum number of symbol and maximum number of instruction both to 300. This shoudl fair enough be a large number already. As long as in the program you do not define more than 300 symbols and input instructions, it would work fine.
3. As documented in the promt, I set the maximum length of the name of a symbol to be 8 character long. Any longer name would just be cut off and the first 8 characters will be retained as this symbol's name. But, the buffer I set to temporarily hold the input symbol name is 500 characters long, so I expect you not to enter any symbol's name longer than 500 chacters, otherwise the "Scanf" function might reads outside its bound. And for example, if you define two symbols whose have the same first-8-character, then they will be deemed as multiply defined. Be careful about this.
4. Symbol should not be called "None", because "None" is used for initialization, you can think of it as reserved word.

# Procedure of my program
{
1. update base address for each module
2. process definition { 1. >8 characters too long cut it down
									2. check if it's multiply defined }
3. process usage     { 1. if multiple symbols used in same instruction
                                   2. check if any definition exceed the size of that module }
4. process all instruction   { 1.find out absolute address for each symbols
                                             2. check if any symbol is used but never defined }
5. output                  {1. check if any absolute address exceed 299 machine limits 
                                2.  check if any symbol is defined not never used}
}

## Any confusion, just email me jy2363@nyu.edu. I will reply within 1/2 hour.
