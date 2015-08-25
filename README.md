# example-mbedos-blinky
simple example program to blink an LED on an mbed board with mbed OS


## Your First Project
Ok, lets get to the knitty gritty and create your first Yotta project! For this project we are going to make an LED blink and print some commands to the terminal. We are going to use the mbed-classic yotta module.

#### Step 0: Install Yotta
Make sure you have an up to date version of python installed. On the command line run this command to install yotta.
```bash
$ pip install yotta
```
If you have any trouble installing yotta please see [docs.yottabuild.org](http://docs.yottabuild.org/#installing) for troubleshooting. 

#### Step 1: create executable module
We need to create an executable yotta module so that when we run `yotta build` an executable file will be created. 

First create an empty folder, then move into that folder
```bash
$ mkdir blinky
$ cd blinky
```

Next initialize the module with `yotta init` and fill out the details. Make sure you select executable as the module type at the end.
Note: you can fill in the repository and homepage URL's or leave them blank. You can also edit them later. 

```bash
$ mkdir example-mbedos-blinky
$ cd example-mbedos-blinky
$ yotta init
Enter the module name: <example-mbedos-blinky>
Enter the initial version: <0.0.0>
Short description: simple example program to blink an LED on an mbed board with mbed OS
Keywords:  <blinky, mbedOS>
Author: mbedAustin
Repository url:
Homepage:
What is the license for this project (Apache-2.0, ISC, MIT etc.)?  <Apache-2.0>
Is this module an executable? <no> yes
```

You should now have several folders and files in the directory.
```bash
$ ls 
example-mbedos-blinky  module.json  source  test
```
The `module.json` file contains all the settings for your project, everything you just entered can be found in that file, so if you want to add a repository URL later you can. The `./source` directorycontains all the source files, the `./test` directory contains all tests written to test your module, and the `./example-mbedos-blinky` directory contains all build files and dependency files.

#### Step 2: Select Target Platform!
Now that we have created a basic module lets set which target we are going to build for. For a full list of available targets available run the following `search` command.
```bash
$ yotta search target target
frdm-k64f-gcc 0.0.21: Official mbed build target for the mbed frdm-k64f development board.
st-nucleo-f401re-gcc 0.1.0: Official mbed build target for the mbed st-nucleo-f401re development board.
frdm-k64f-armcc 0.0.13: Official mbed build target for the mbed frdm-k64f development board, using the armcc toolchain.
stm32f429i-disco-gcc 0.0.4: Official mbed build target for the mbed st-nucleo-f429zi development board.
nordic-nrf51822-16k-gcc 0.0.5: Official mbed build target for the mbed nrf51822 development board, using the armgcc toolchain.
nordic-nrf51822-16k-armcc 0.0.5: Official mbed build target for the mbed nrf51822 development board, using the armcc toolchain.
bbc-microbit-classic-gcc 0.1.0: Official mbed build target for the mbed nrf51822 development board, using the armgcc toolchain.
st-stm32f439zi-gcc 0.0.3: Official mbed build target for the st stm32f439zi microcontroller.
st-stm32f429i-disco-gcc 0.0.2: Official mbed build target for the mbed st-nucleo-f429zi development board.
```
In this example we are going to use the Freescale FRDM K64F board, so we'll use the `frdm-k64f-gcc` target.

```bash
$ yotta target frdm-k64f-gcc
```
To check the target has been set correctly run the `target` command to see what yotta is currently targeting for its builds. 
```bash
$ yotta target
frdm-k64f-gcc,*
```

#### Step 3: Add Dependencies!
Now that we have created an executable file lets add the dependencies, in this case we are going to have `mbed-drivers` (aka mbed OS) as our dependency.
```bash
$ yotta install mbed-drivers
info: ... A bunch of messages about stuff being downloaded ...
```
You could at this point add other yotta modules, check out the `yotta search` function to search for other available modules.

#### Step 4: Add Source Files
Now that we have set up an executable module, and downloaded our dependencies, lets add some source code to use the module. In the `./source` folder create a file called `blinky.cpp` with the following contents.
```C
#include "mbed/mbed.h"

static void blinky(void) {
    static DigitalOut led(LED1);

    led = !led;
    printf("LED = %d \n\r",led.read());
}

void app_start(int, char**){
    minar::Scheduler::postCallback(blinky).period(minar::milliseconds(500));
}
```
This program will cause the LED on the board to flash and print out the status of the LED to the terminal. Default terminal speed is `9600baud` at `8-N-1`.
For more information about MINAR and the structure of mbed OS programs, please refer to the [MINAR documentation](https://github.com/ARMmbed/minar).

#### Step 5: Build
Run the yotta build command in the top level of the example directory (where the module.json file is located) to build the project.
```bash
$ yt build
info: generate for target: frdm-k64f-gcc 0.0.21 at ~\example-mbedos-blinky\yotta_targets\frdm-k64f-gcc
GCC version is: 4.9.3
-- The ASM compiler identification is GNU
-- Found assembler: GNU Tools ARM Embedded/4.9 2014q4/bin/arm-none-eabi-gcc.exe
-- Configuring done
-- Generating done
-- Build files have been written to: ~/example-mbedos-blinky/build/frdm-k64f-gcc
[135/135] Linking CXX executable source/example-mbedos-blinky
```
The compiled binary will be located in the build folder. Copy the binary from `.\build\frdm-k64f-gcc\source\example-mbedos-blinky.bin` to your mbed board and see the led blink. If you hook up a terminal to the board you can see the output being printed form the board that will look like the following.
```terminal
LED = 1 

LED = 0 

LED = 1 

LED = 0 
...
```

## Alternative Method
Instead of setting up your own project from scratch you could alternatively copy an existing executable module and modify it. We have uploaded the above  [blinky example project](www.github.com/armmbed/example-mbedos-blinky) to github where you can clone to repo and build it.

#### Step 1: Clone Repo
Clone the repository from github.
```bash
$ git clone https://github.com/ARMmbed/example-mbedos-blinky.git
```

#### Step 2: Select Target Platform
You can build this example for any target. To see all targets available run the following `search` command.
```bash
$ yotta search target target
frdm-k64f-gcc 0.0.21: Official mbed build target for the mbed frdm-k64f development board.
st-nucleo-f401re-gcc 0.1.0: Official mbed build target for the mbed st-nucleo-f401re development board.
frdm-k64f-armcc 0.0.13: Official mbed build target for the mbed frdm-k64f development board, using the armcc toolchain.
...
```
Any `target` will work, we're going to use the `frdm-k64f-gcc` target. To set the target run the following command.
```bash
$ yotta target frdm-k64f-gcc
```

#### Step 3: Build it!
Now that you have downloaded the project and selected the `target` to build for lets build the project! Run the following command at the top level of the project (the level as the module.json file).
```bash
$ yotta build
... bunch of build messages ...
[135/135] Linking CXX executable source/example-mbedos-blinky
```
The compiled binary will be located in the build folder. Copy the binary from `.\build\frdm-k64f-gcc\source\example-mbedos-blinky.bin` to your mbed board and see the led blink. If you hook up a terminal to the board you can see the output being printed form the board that will look like the following.
```terminal
LED = 1 

LED = 0 

LED = 1 

LED = 0 
...
```

