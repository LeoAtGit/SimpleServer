Simple Server
-------------

"Simple Server" is a simple webserver for the most basic GET-Requests. It is
not completely compliant with the current HTTP specifications, so don't use it.
It was just thought as a learning experience and for a better understanding of
the HTTP protocol.

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

Installation:

The Makefile is included in the Root-Directory. You need to use a Linux Distri-
bution (only tested on Arch Linux and Ubuntu Linux). If you don't want debug
support for the app, simply put a `#` in front of the line that starts with
`debug` in the Makefile.
	To build the application, simply run `make`. The binary file will be
called `simpleserver` and it will stay there. If you want to invoke the program
from anywhere, you can symlink it to your preferred /bin directory.

Configuration:

The configuration file is searched for in the same directory where the exe-
cutable is located and is called `config.ini`.
	See the default config.ini that comes with the source so you know the
syntax of the file.

	Options:
		port - The port on which the application listens on.

		request_size - The number of bytes that are read from each in-
			coming request. The request will always be read comple-
			tely, this number just gives you the possibility to
			optimize the number of syscalls / time that's needed
			to read all of it in memory.

		max_request_size - This is the maximum memory that is read from
			one single request. If you don't put a limit there, the
			program will most definitely segfault.

		log - `1` enables the logfile and `0` disables it.

		logfile - The name of the logfile.

		html_template_start
		html_template_end - These two set the templates that are used
			when there is an error message to display.

		docroot - This specifies the location the application looks for
			the HTML files, that the requester can access. Make
			sure the permissions are correct on this directory.

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

Have fun!

