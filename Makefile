all:

	g++  `pkg-config --cflags opencv` `pkg-config --libs opencv` -o lane LaneDetection.cpp SpeedRange.cpp -lwiringPi -lpthread 
	sudo ./lane
