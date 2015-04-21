all:
	$(MAKE) -i clean -C TCPchecklist/
	$(MAKE) -i clean -C stochalgo/
	$(MAKE) -i clean -C detalgo/
	$(MAKE) -i clean -C rlnc/
	$(MAKE) -i clean -C backpressure/
	$(MAKE) -i clean -C backpressureACK/
	$(MAKE) -i clean -C stability_region/
	$(MAKE) -i clean -C delay/
	$(MAKE) -i clean -C sim_main/
	$(MAKE) -C TCPchecklist/
	$(MAKE) -C detalgo/
	$(MAKE) -C stochalgo/
	$(MAKE) -C rlnc/
	$(MAKE) -C backpressure/
	$(MAKE) -C backpressureACK/
	$(MAKE) -C stability_region/
	$(MAKE) -C delay/
	$(MAKE) -C sim_main/
