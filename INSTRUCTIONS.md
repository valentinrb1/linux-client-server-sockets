Sistemas Operativos II - Laboratorio II IPC
###  Ingeniería en Computación - FCEFyN - UNC - 2023
# Laboratorio 2 - Interprocess Communication Advanced

## Introduction
Continuing the study of the IPC mechanisms that we saw in Laboratory I, now we will use the most used one: sockets.

## Objective
The objective of this practical work is for the student to be able to design and implement a software that makes use of the IPC mechanisms provided by the Operating System, implementing what was seen in the theoretical and practical sessions and making use of all the knowledge acquired in Software Engineering and Operating Systems I and what was developed in Laboratory I.

## Development
The student is asked to design, implement and test a set of applications (developed in C language) of a three-layer client-server architecture that implements and supports at least 3 protocols of the family described in <sys/socket.h>.

### Clients
Three types of clients must be implemented, which can be instantiated a number N times.
- Client A: Must have a CLI, where the user writes a string, in a format to be defined, that interacts with [journalctl][jctrl], and prints the result on the screen.
- Client B: Must have a CLI, the same as Client A, but receives a compressed file with the result.
- Client C: A report with:
- Free memory in the system
- Normalized system load

### Server
- Must meet the requirements of Laboratory I
- Must manage the connections and disconnections of the clients.
- It is in charge of communicating the messages of the users and responding accordingly.
- The logic and design of operation is the task of the students.

## Correction Criteria
- The code must be compiled with the compilation flags:
-Wall -Pedantic -Werror -Wextra -Wconversion -std=gnu11
- Correct memory management.
- Divide the code into modules judiciously.
- Code style.
- Error handling
- The code must not contain errors or warnings.
- The code must not contain cppcheck errors.
- The student must provide a script to validate the project requirements.

## Considerations
- Clients are _lightweight_. That is, all processing is done on the server side.
- Clients must validate the checksum of what is received.
- File download must work for any size/format.
- In case of a server-side failure during the processing of the request, the client must be informed before closing the connection.
- All messages between the server and the client must be in [json][jsonf] format.

## Delivery

Delivery is done through the GitHub repository and the completion of the same must be demonstrated through correct use of the [Git workflow][workflow]. The repository must provide the source files and any other files associated with the compilation, “Makefile” project files and the correctly documented code. It should not contain any files associated with IDE projects and it should be assumed that the project can be compiled and run by a `tty` on a Linux distribution with the typical development tools. A report must also be submitted (which can be in MD in the repo) explaining step by step the development, including graphics of the proposed solution design, justifying at all times what was implemented.

## Evaluation
This practical work is individual and must be submitted before 14:55ART on April 13, 2023, leaving it recorded in the LEV with the report file. It will be corrected and then a date will be coordinated for the oral defense of it.

[jctrl]: https://www.man7.org/linux/man-pages/man1/journalctl.1.html
[jsonf]: https://github.com/DaveGamble/cJSON
