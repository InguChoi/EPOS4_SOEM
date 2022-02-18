# INFORMATION
Realtime EtherCAT master for EPOS4 motor control based on TCP/IP connection with GUI PC


**HARDWARE:**
+ Raspberry pi 4

**SOFTWARE:**
+ Raspberry pi OS
+ Preempt-rt patched kernel
+ Simple Open Source EtherCAT Master (SOEM)

**BUILD:**

      git clone https://github.com/shkwon98/EPOS4_SOEM.git
      cd EPOS4_SOEM
      chmod +x *.sh
      ./build.sh

**CLEAN:**

      ./clean.sh


**RUN:**
* Check information of all slaves on bus:

      cd test/slaveinfo/
      sudo ./slaveinfo [ifname]
	
* Check pdo mapping of all slaves on bus:

      cd test/slaveinfo/
      sudo ./slaveinfo [ifname] -map