all:
	$(MAKE) -f Makefile.env

	$(MAKE) -C lib all
	$(MAKE) -C lib/usb
	$(MAKE) -C lib/input
	$(MAKE) -C lib/fs
	$(MAKE) -C lib/fs/fat
	$(MAKE) -C lib/sched
	$(MAKE) -C lib/net
	$(MAKE) -C addon/fatfs
#	$(MAKE) -C sample/01-gpiosimple
	$(MAKE) -C addon/linux
	
	$(MAKE) -C addon/vc4/vchiq
	#$(MAKE) -C addon/vc4/sound
	#$(MAKE) -C addon/vc4/sound/sample
	
	$(MAKE) -C addon/vc4/interface/vcos
	$(MAKE) -C addon/vc4/interface/vmcs_host
	$(MAKE) -C addon/vc4/interface/khronos
	$(MAKE) -C addon/vc4/interface/bcm_host
	#$(MAKE) -C addon/vc4/interface/sample/hello_dispmanx
	#$(MAKE) -C addon/vc4/interface/sample/hello_tiger
	#$(MAKE) -C addon/vc4/interface/sample/hello_triangle
	#$(MAKE) -C addon/vc4/interface/sample/hello_triangle2

	$(MAKE) -C addon/imgui
	$(MAKE) -C addon/imgui/sample

clean:
	$(MAKE) -C addon/imgui/sample clean
	$(MAKE) -C addon/imgui clean

	$(MAKE) -C addon/vc4/interface/sample/hello_triangle2 clean
	$(MAKE) -C addon/vc4/interface/sample/hello_triangle clean
	$(MAKE) -C addon/vc4/interface/sample/hello_tiger clean
	$(MAKE) -C addon/vc4/interface/sample/hello_dispmanx clean
	$(MAKE) -C addon/vc4/interface/bcm_host clean
	$(MAKE) -C addon/vc4/interface/khronos clean
	$(MAKE) -C addon/vc4/interface/vmcs_host clean
	$(MAKE) -C addon/vc4/interface/vcos clean

	$(MAKE) -C addon/vc4/sound/sample clean
	$(MAKE) -C addon/vc4/sound clean
	$(MAKE) -C addon/linux clean

	$(MAKE) -C sample/01-gpiosimple clean
	$(MAKE) -C lib/net clean
	$(MAKE) -C lib/sched clean
	$(MAKE) -C lib/fs/fat clean
	$(MAKE) -C lib/fs clean
	$(MAKE) -C lib/input clean
	$(MAKE) -C lib/usb clean
	$(MAKE) -C lib clean
