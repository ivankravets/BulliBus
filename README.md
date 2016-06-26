# BulliBus

=== NOT READY, YET! ===

Simple serial communication library for Arduino and Friends

"It seems that perfection is attained, not when there is nothing more to add,
but when there is nothing more to take away."
                                                    (Antoine de Saint Exupéry)

The purpose of this project is to provide an extremely simple to use but
nevertheless powerfull serial bus communiction library for Arduino and
friends.

BulliBus aims to satisfy 95% of normal bus communication needs while staying
userfriendly.

BulliBus uses only plain old printable ascii characters to communicate.
BulliBus Master (just called Bulli) can be easily simulated using an ascii
terminal on your PC using some cheapo rs485 adaptor.


BulliBus communication looks like:

master> tem1 get
tem1> 23.75

master> mot7 left 50
master> mot7 status
mot7> left 50

master> mot7 stop
master> mot7 status
mot7> halt


BulliBus Messages can be accompanied by a crc16 checksum in order to ensure
message integrity. Automated BulliBus clients do use this. But as humans we
are bad at calculating checksums in our head so BulliBus allowes us to leave
them out.

Including checksums BulliBus Messages look like:

master> tem1 get~CAFE

tem1> 23.75~F00B

BulliBus only supports a subset of ascii. And by ASCII we do mean 0x00-0x7F.
So no latin-x or windows-y extension.


More formally:

Message format: (all plain ascii)

Request: ADDRESS<SP>TEXT[~RC16]<CR/LF>
Response: ADDRESS><SP>TEXT[~RC16]<CR/LF>

with:
    ADDRESS: 4 characters ascii in range A..Z, a..z, 0..9, _, -. Case *does* matter.
    TEXT: Any printable ascii text excluding <NUL>, <CR> and <LF>, but including <SP>
    <NUL>: Null byte: 0x00
    <SP>: Space character: 0x20
    <CR>: Carriage return: 0x0D
    <LF>: Linefeed: 0x0A
    <CR/LF>: **ANY** combination of <CR> and <LF>: <CR>, <LF>, <CR><LF> *and* <LF><CR>
         (this is in order to simply use terminals and libraries and don't care what
          their makers thought was the *right* character to use.
    [~RC16]:
          Tilde character plus CRC16 hex encoded over the complete address and text excluding starting <SP> and excluding ending <CR/LF> optional when sending to devices. mandatory for devices to return.
          If supplied devices *must* check it and return an error if not matches

For broadcasts masking may be used:
    questionmarks can replay *any* address character and mean: can be any character. so
        1??4 does mean any device in range 1004 .. 1zz4

So this protocoll has some drawbacks:
* It's not fast (It has hugh delays, ascii is only 0..0x7f)
* It only transports text which most probably needs parsing
* It doesn't make guarantees on throughput at all.
* NO device discovery of any kind (at leat not now)
* NO two-way broadcasts. (But one-way)

But it has some major advantages, too:
* It just works
* It can be implemented on nearly any device as long as it has a serial interface
* It is bus capable and avoids collisions
* It is dead simple. You can even communicate with your clients using just a simple terminal
* It can be implemented in just a few LOCs

***Don't use this* if you:***
* Want to send much data
* Have realtime needs.
* Don't like text protocols

But use it if you:
* Just want to send messages between a few devices
* Want to be able to read what your devices are talking
* Are fed up searching for a bus protocol which runs on all your devices
