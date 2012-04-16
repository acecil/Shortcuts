#	
#	(c) 2012 by Andrew Gascoyne-Cecil.
#	
#	This file is part of Shortcuts.
#
#	Shortcuts is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#	
#	Shortcuts is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#	
#	You should have received a copy of the GNU General Public License
#	along with Shortcuts.  If not, see <http://www.gnu.org/licenses/>.
#

# This is a script to create the release package for Shortcuts.

import os
import shutil
import zipfile
import re

# Function for zipping tree of files
def zipdir(basedir, archivename):
	assert os.path.isdir(basedir)
	with zipfile.ZipFile(archivename, "w", zipfile.ZIP_DEFLATED) as z:
		for root, dirs, files in os.walk(basedir):
			for fn in files:
				absfn = os.path.join(root, fn)
				zfn = os.path.join(basedir, absfn[len(basedir)+len(os.sep):])
				z.write(absfn, zfn)

# Read version from VERSION file.
versionLines = ""
with open("VERSION", "r") as f:
	for line in f:
		versionLines += line
majorMatch = re.search("MAJOR_VERSION ([0-9])", versionLines)
minorMatch = re.search("MINOR_VERSION ([0-9])", versionLines)
microMatch = re.search("MICRO_VERSION ([0-9])", versionLines)
version = majorMatch.group(1) + "." + minorMatch.group(1) + "." + microMatch.group(1)

# Make sure directory for release exists and is empty
relDir = "Shortcuts-V" + version
if os.path.exists(relDir):
	shutil.rmtree(relDir)
os.mkdir(relDir)

# Copy config files to release directory
configDir = os.path.join(relDir, "Config")
os.mkdir(configDir)
configFiles = os.listdir("Config")
for fileName in configFiles:
	fullFileName = os.path.join("Config", fileName)
	if fullFileName.endswith(".txt") and (os.path.isfile(fullFileName)):
		shutil.copy(fullFileName, configDir)

# Copy README, CHANGELOG and GPL to release directory
shutil.copy("README", relDir)
shutil.copy("CHANGELOG", relDir)
shutil.copy("GPL", relDir)

# Copy executable to release directory
exePath = os.path.join("Release", "shortcuts.exe")
shutil.copy(exePath, relDir);

# Zip release directory
relZipName = relDir + ".zip"
if os.path.exists(relZipName):
	os.remove(relZipName)
zipdir(relDir, relZipName)

