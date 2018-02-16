#
#    Copyright (c) 2009-2015 Tom Keffer <tkeffer@gmail.com>
#
#    See the file LICENSE.txt for your full rights.
#
# StoicWS 2018-02-11

"""User extensions module

This module is imported from the main executable, so anything put here will be
executed before anything else happens. This makes it a good place to put user
extensions.
"""

# This caused all sorts of issues. 
import locale
# This will use the locale specified by the environment variable 'LANG'
# Other options are possible. See:
# http://docs.python.org/2/library/locale.html#locale.setlocale
#locale.setlocale(locale.LC_ALL, '')

# My code. Ugly but it works
locale.setlocale(locale.LC_ALL, 'en_US.utf8')
