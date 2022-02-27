# SOEM EPOS4 Motor Control Application
This is a real-time EtherCAT master application to control EPOS4 motor.

---

## PREREQUISITIES
+ This application was developed with Preempt-rt patched Raspberry Pi 4
	+ Developed Preempt-rt linux kernel version is 4.19.y-rt
	+ Developed OS debian version is 10(buster)
	+ For more details, go to the [link](https://github.com/shkwon98/RPI_PreemptRT)
+ This application requires extra Ethernet port on Raspberry Pi
	+ For example, WIZnet W5500 Ethernet module with SPI interface can be applied
	+ If you are going to use the same solution, you can find out details [here](https://github.com/shkwon98/RPI_PreemptRT)
+ [EPOS4_GUI](https://github.com/shkwon98/EPOS4_GUI) installed remote PC is required for remote connection
#### FRAMEWORK:

![framework](https://user-images.githubusercontent.com/86837107/155870476-c4409fc2-9101-4b08-91ce-bc8333312e19.jpg)

---

## HOW TO LAUNCH

#### BUILD:

      git clone https://github.com/shkwon98/EPOS4_SOEM.git
      cd EPOS4_SOEM
      chmod +x *.sh
      ./build.sh

#### CLEAN:

      ./clean.sh


#### RUN:
* Check information of all slaves on bus:

      cd test/slaveinfo/
      sudo ./slaveinfo [ifname]
	
* Check pdo mapping of all slaves on bus:

      cd test/slaveinfo/
      sudo ./slaveinfo [ifname] -map
      
* Activate motor in a single velocity:

      cd test/motor_test/
      sudo ./motor_test [ifname]
      
* Activate motor in a single velocity in Real-time:

      cd test/rt_motor_test/
      sudo ./rt_motor_test [ifname]
      
* Control the motor with the [EPOS4_GUI](https://github.com/shkwon98/EPOS4_GUI) Application:

      cd test/EPOS4/
      sudo ./main [ifname]
      
