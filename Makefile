all:
	$(MAKE) -f Makefile.env

	$(MAKE) -C lib all
	$(MAKE) -C lib/usb
	$(MAKE) -C lib/input
	$(MAKE) -C lib/fs
	$(MAKE) -C lib/fs/fat
	$(MAKE) -C lib/sched
	$(MAKE) -C lib/net
	$(MAKE) -C apps/tsc2046
	$(MAKE) -C apps/ili9325d
	$(MAKE) -C apps/ft6x06

clean:
	$(MAKE) -C apps/ft6x06 clean
	$(MAKE) -C apps/ili9325d clean
	$(MAKE) -C apps/tsc2046 clean
	$(MAKE) -C lib/net clean
	$(MAKE) -C lib/sched clean
	$(MAKE) -C lib/fs/fat clean
	$(MAKE) -C lib/fs clean
	$(MAKE) -C lib/input clean
	$(MAKE) -C lib/usb clean
	$(MAKE) -C lib clean
