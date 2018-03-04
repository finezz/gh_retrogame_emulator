# $Id: 3rdparty_download.py 11350 2010-03-23 01:18:06Z mthuurne $

from checksum import verifyFile
from components import requiredLibrariesFor
from configurations import getConfiguration
from download import downloadURL
from extract import TopLevelDirRenamer, extract
from libraries import allDependencies, librariesByName
from packages import getPackage
from patch import Diff, patch

from os import makedirs
from os.path import isdir, isfile, join as joinpath
from shutil import rmtree
import sys

# TODO: Make DirectX headers for MinGW a package and make the DirectX sound
#       driver a component.

def downloadPackage(package, tarballsDir):
	if not isdir(tarballsDir):
		makedirs(tarballsDir)
	filePath = joinpath(tarballsDir, package.getTarballName())
	if isfile(filePath):
		print '%s version %s - already downloaded' % (
			package.niceName, package.version
			)
	else:
		downloadURL(package.getURL(), tarballsDir)

def verifyPackage(package, tarballsDir):
	filePath = joinpath(tarballsDir, package.getTarballName())
	try:
		verifyFile(filePath, package.fileLength, package.checksums)
	except IOError, ex:
		print >> sys.stderr, '%s corrupt: %s' % (
			package.getTarballName(), ex
			)
		sys.exit(1)

def extractPackage(package, tarballsDir, sourcesDir, patchesDir):
	if not isdir(sourcesDir):
		makedirs(sourcesDir)
	sourceDirName = package.getSourceDirName()
	packageSrcDir = joinpath(sourcesDir, sourceDirName)
	if isdir(packageSrcDir):
		rmtree(packageSrcDir)
	extract(
		joinpath(tarballsDir, package.getTarballName()),
		sourcesDir,
		TopLevelDirRenamer(sourceDirName)
		)
	diffPath = joinpath(patchesDir, sourceDirName + '.diff')
	if isfile(diffPath):
		for diff in Diff.load(diffPath):
			patch(diff, sourcesDir)
			print 'Patched:', diff.getPath()

def main(platform, tarballsDir, sourcesDir, patchesDir):
	configuration = getConfiguration('3RD_STA')
	components = configuration.iterDesiredComponents()

	# Compute the set of all directly and indirectly required libraries,
	# then filter out system libraries.
	thirdPartyLibs = set(
		makeName
		for makeName in allDependencies(requiredLibrariesFor(components))
		if not librariesByName[makeName].isSystemLibrary(platform)
		)

	for makeName in sorted(thirdPartyLibs):
		package = getPackage(makeName)
		downloadPackage(package, tarballsDir)
		verifyPackage(package, tarballsDir)
		extractPackage(package, tarballsDir, sourcesDir, patchesDir)

if __name__ == '__main__':
	if len(sys.argv) == 2:
		main(
			sys.argv[1],
			'derived/3rdparty/download',
			'derived/3rdparty/src',
			'build/3rdparty'
			)
	else:
		print >> sys.stderr, (
			'Usage: python 3rdparty_download.py TARGET_OS'
			)
		sys.exit(2)
