# BulliBus

Simple serial communication library for Arduino and Friends

The purpose of this project is to provide an extremely simple to use but
nevertheless powerfull serial bus communiction library for Arduino and friends.

BulliBus aims to satisfy 95% of normal bus communication needs while staying
userfriendly.

BulliBus uses only plain old printable ascii characters to communicate. BulliBus Master
(just called BulliBus) can be easily simulated using an ascii terminal on your
PC using some cheapo rs485 adaptor.

BulliBus communication looks like:

master> tem1 get

tem1> 23.75

master> mot7 left 50

master> mot7 status

mot7> left 50

master> mot7 stop

master> mot7 status

mot7> halt
