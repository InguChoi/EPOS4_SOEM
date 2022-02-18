# INFORMATION
Realtime EtherCAT master for EPOS4 motor control based on TCP/IP connection(String command) with GUI PC


**HARDWARE:**
+ Raspberry pi 4

**SOFTWARE:**
+ Raspberry pi OS
+ Preempt-rt patched kernel
+ Simple Open Source EtherCAT Master (SOEM)

**BUILD:**
+ Prerequisites: CMake 3.9 or later

      git clone -b ver.1.0 https://github.com/shkwon98/EPOS4_SOEM.git
      cd build
      cmake ..
      sudo make

**CLEAN:**
+ Clean binary files:

      ./clean.sh

**RUN:**
* Check information of all slaves on bus:

      cd test/slaveinfo/
      sudo ./slaveinfo [ifname]
	
* Check pdo mapping of all slaves on bus:

      cd test/slaveinfo/
      sudo ./slaveinfo [ifname] -map
