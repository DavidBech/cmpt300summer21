# Assignment 3 Branch

Due: 2021 - Jul - 23

By: David Bechert

## Makefile Targets

- `make` or `make all` to build the default binary file
- `make run` for defult run
- `make valgrind` to run with valgrind
- `make clean` to clean the directory

## Modules

- kernel\_sim
  - Initializes the environment
  - Reads user input for commands thier arguments
  - calls other modules shutdown methods
- executioner
  - Implementation of the commands
- queue\_manager
  - Implements ready queues and IPC Blocking queues
- sem (semaphore)
  - Semaphore implementation
- process\_cb
  - Implements process control block structure as well as api for modifying the fields

## Process printout

1. PID: [Process ID]
    - the process id given to the process at its creation
2. Prio: [low, norm, high, init]
    - priority of the process given at its creation
3. State: [running, ready, blocked]
    - the processes current state
4. Queue: [None, Init, Ready (low, norm, high), Blocked (send, rec), Sem(i)]
    - None is used for current running process
    - Init is used for init process
    - Ready is for processes on one of the 3 ready queues
    - Blocked is for processes blocked on IPC
    - Sem(i) is for processes block on a semaphore
5. Message:
    - No Message when message is not being sent or received
    - To/From PID as well sas the message contents

## Assumptions Made

- RR Arbiter Priority
  - A process may continually win arbitration when they are the only process of that priority even if there are other processes of a lower priority
- Exemption in the Creation Command
  - When a new process is created through the create command and the current process is init it exempts the init process
  - When a new process is created with a higher priority than current process it **does not** exempt the current process
- Command Case
  - A command can be both upper and lower case
- Send restrictions
  - The recipient of the send command must exist at the time of the call to send
  - A message is only sent once, a second call to receive will block the process

## How the Init process is handled

- pid is always 0
- info messages will state that the init is located on the "Init" queue regardless of its state
- the total info printout will display it separately when it isn't the current process
- attempting to fork will result in error
- killing or exiting will cause termination when it is the only process
- will only recieve arbitration if every other process is blocked
- may be "blocked" on multiple semaphores
- may be "blocked" multiple times on the same semaphore
- the init process may recieve at most once at a time
  - ie. a call to receive will fail if the init process already on the receive queue
- the init process may send at most one message at a time
  - ie. send calls will fail if the init process is already on the send queue
