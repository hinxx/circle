all:
	make -f Makefile.env

	make -C lib all
	make -C lib/usb
	make -C lib/input
	make -C lib/fs
	make -C lib/fs/fat
	make -C lib/sched
	make -C lib/net
	make -C sample/01-gpiosimple
	make -C addon/linux
	
	make -C addon/vc4/vchiq
	#make -C addon/vc4/sound
	#make -C addon/vc4/sound/sample
	
	make -C addon/vc4/interface/vcos
	make -C addon/vc4/interface/vmcs_host
	make -C addon/vc4/interface/khronos
	make -C addon/vc4/interface/bcm_host
	make -C addon/vc4/interface/sample/hello_dispmanx
	make -C addon/vc4/interface/sample/hello_tiger
	make -C addon/vc4/interface/sample/hello_triangle
	make -C addon/vc4/interface/sample/hello_triangle2

clean:
	make -C addon/vc4/interface/sample/hello_triangle2 clean
	make -C addon/vc4/interface/sample/hello_triangle clean
	make -C addon/vc4/interface/sample/hello_tiger clean
	make -C addon/vc4/interface/sample/hello_dispmanx clean
	make -C addon/vc4/interface/bcm_host clean
	make -C addon/vc4/interface/khronos clean
	make -C addon/vc4/interface/vmcs_host clean
	make -C addon/vc4/interface/vcos clean

	make -C addon/vc4/sound/sample clean
	make -C addon/vc4/sound clean
	make -C addon/linux clean

	make -C sample/01-gpiosimple clean
	make -C net clean
	make -C sched clean
	make -C fs/fat clean
	make -C fs clean
	make -C input clean
	make -C usb clean
	make -C lib clean
