# Copyright (C) 2007,2008,2009,2010,2011,2012 Olaf Lenz, Axel Arnold
#
# This file is part of ESPResSo.
#
# ESPResSo is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ESPResSo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
SUBDIRS = logo doxygen dg ug tutorials

.PHONY: FORCE doc ug dg tutorials doxygen
doc: ug dg tutorials doxygen
dg: FORCE
	cd dg; $(MAKE) --print-directory $@
doxygen: FORCE
	cd doxygen; $(MAKE) --print-directory $@
ug: latexit.sh FORCE
	cd ug; $(MAKE) --print-directory $@
tutorial: latexit.sh FORCE
	cd tutorials; $(MAKE) --print-directory $@
FORCE:

