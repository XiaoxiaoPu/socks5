# socks5 #

A simple SOCKS5 server

## Build ##

1. install libev (optional)

	```bash
	# Archlinux
	sudo pacman -S libev
	# CentOS
	sudo yum install libev-devel
	# Debian/Ubuntu
	sudo apt-get install libev-dev
	```

2. configure and make

	```bash
	autoreconf -if
	./configure --prefix=/usr
	make
	```

3. install

	```bash
	sudo make install
	```

## Cross compile ##

1. setup cross compile tool chain

2. build

	```bash
	autoreconf -if
	./configure --host=arm-unknown-linux-gnueabihf --prefix=/usr
	```

## License ##

Copyright (C) 2014 - 2015, Xiaoxiao <i@xiaoxiao.im>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
