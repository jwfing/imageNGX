sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install git
sudo apt-get install libjpeg62 libjpeg62-dev
sudo apt-get libpng-dev libpngc++-dev
sudo apt-get install liblog4cxx10-dev

# for openCV
if [ ! -e /usr/local/lib/libopencv_core.so ]; then
	echo "begin to install opencv..."
    sudo apt-get install cmake
    mkdir OpenCV/release
    cd OpenCV/release && cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D BUILD_PYTHON_SUPPORT=ON ../OpenCV-2.3.1/ && make && sudo make install
    cd ../..
else
	echo "opencv has been installed."
fi

# for imageMagicK
if [ ! -f /usr/local/lib/libMagickCore.a ]; then
	echo "begin to install imageMagick..."
    cd imageMagick/ImageMagick-6.7.5-0 && ./configure && make && sudo make install
    cd ../..
else
	echo "imageMagicK has been installed."
fi

# for cppunit
if [ ! -f /usr/lib/libcppunit.a ]; then
	echo "begin to install cppunit..."
	sudo apt-get install libcppunit-dev
else
	echo "cppunit has been installed."
fi

# for pion-net
if [ ! -f /usr/local/lib/libpion-net.a ]; then
	echo "begin to install packages dependencies by pion-net..."
        sudo apt-get install autoconf
        sudo apt-get install automake
	sudo apt-get install libboost-all-dev
	sudo apt-get install liblog4cpp5
	sudo apt-get install bzip2
	sudo apt-get install zlibc
	sudo apt-get install libbz2-dev
	sudo apt-get install libtool
    git clone git://github.com/cloudmeter/pion.git && cd pion/ && git checkout master && \
	./autogen.sh && ./configure --with-log4cxx --disable-doxygen-doc --disable-doxygen-html && make && sudo make install
    cd ..
else
	echo "pion-net has been installed."
fi

# for mongoClient
# mongoClient will be installed to /usr/local/lib/libmongoclient.so
if [ ! -f /usr/local/lib/libmongoclient.so ]; then
	echo "begin to install packages dependencies by mongodb driver..."
	sudo apt-get install autoconf
	sudo apt-get install pkg-config libssl-dev libsasl2-dev
        cd mongodbDriver/mongo-c-driver
        ./autogen.sh && make && sudo make install
        cd ../..
        cd mongocxx-driver-r3.1.1/build
cmake -DCMAKE_BUILD_TYPE=Release -DBSONCXX_POLY_USE_MNMLSTC=1 -DCMAKE_INSTALL_PREFIX=/usr/local -DLIBMONGOC_DIR=/usr/local -DLIBBSON_DIR=/usr/local ..
	sudo apt-get install scons
	sudo apt-get install libpcre3-dev
	cd mongodbDriver && tar xfz mongodb-linux-x86_64-v1.8-latest.tgz
    cd ..
	cd mongodbDriver/mongo-cxx-driver-v1.8 && sudo scons install -j4
    cd ../..
else
	echo "mongodb driver has been installed."
fi

# for libs3
if [ ! -f /usr/local/lib/libs3.so ]; then
    sudo apt-get install libxml2-dev
    sudo apt-get install libcurl3 libcurl4-openssl-dev
    cd libs3 && tar zxvf libs3-2.0.tar.gz
    cd libs3-2.0 && make DESTDIR=/usr/local install
else
    echo "libs3 has been installed."
fi    

sudo apt-get install lcov
