include Makefile.env

all:
	make -C facedetect
	make -C thumbnail
	make -C montage
	make -C rainbow
	make -C webservice

.PHONY: test
test: 
	make -C facedetect test
	make -C thumbnail test
	make -C montage test
	make -C rainbow test
	make -C webservice test

.PHONY: clean
clean: 
	make -C facedetect clean
	make -C thumbnail clean
	make -C montage clean
	make -C rainbow clean
	make -C webservice clean

