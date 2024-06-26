overlayaz
=======

Photo geolocation analysis software dedicated for long distance observations.

![Screenshot](/overlayaz.png?raw=true)

![Screenshot](/overlayaz-map.png?raw=true)

Copyright (C) 2020-2023  Konrad Kosmatka

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

# Build
In order to build overlayaz you will need:

- CMake
- C compiler
- GTK+ 3
- SQLite
- json-c
- osm-gps-map
- gexiv2

Once you have all the necessary dependencies, you can use scripts available in the `build` directory.

The gtk3-classic fork is recommended for Windows (MINGW) build.

# Installation
After a successful build, just use:
```sh
$ sudo make install
```
in the `build` directory.

# Releases
Windows: https://github.com/kkonradpl/overlayaz/releases

Arch Linux: https://aur.archlinux.org/packages/overlayaz-git

Other: See the build instructions above.

# Examples
An example photo with overlayaz profile is available in `example` directory.

![Screenshot](/example/dylewska-processed.jpg?raw=true)

The tutorial is available in application (see the Help tab).
