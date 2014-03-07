Custom procfs that provides a cgroup aware `meminfo`. It is currently being
developed and tested against the kernel used on Ubuntu 13.10 (3.11.0), but it is
known to work on 3.14.0 as well.

## Building

This kernel module is built and distributed with [DKMS][dkms].

For a local build:

```
ln -s <path-to-procg> /usr/src/procg-0.0.1
sudo dkms add -m procg -v 0.0.1
sudo dkms build -m procg -v 0.0.1
sudo dkms install -m procg -v 0.0.1
```

### Usage

Once the module is installed (`/lib/modules/$(uname -r)/updates/dkms`):

```
sudo modprobe procg
sudo mkdir /procg
sudo mount -t procg none /procg
```

### License

    Copyright (C) 2014  Fabio Kung

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    A copy of the GNU General Public License is in the COPYING file, which
    can also be found online at <http://www.gnu.org/licenses/>.

[dkms]: http://linux.dell.com/dkms/
