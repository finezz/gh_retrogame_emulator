// $Id: FileOperations.cc 12867 2012-09-18 20:09:09Z m9710797 $

#ifdef	_WIN32
#ifndef _WIN32_IE
#define _WIN32_IE 0x0500	// For SHGetSpecialFolderPathW with MinGW
#endif
#include "utf8_checked.hh"
#include "vla.hh"
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include <direct.h>
#include <ctype.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#else
#include <sys/types.h>
#include <pwd.h>
#include <climits>
#endif

#if defined(PATH_MAX)
#define MAXPATHLEN PATH_MAX
#elif defined(MAX_PATH)
#define MAXPATHLEN MAX_PATH
#else
#define MAXPATHLEN 4096
#endif


#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#include "ReadDir.hh"
#include "FileOperations.hh"
#include "FileException.hh"
#include "StringOp.hh"
#include "statp.hh"
#include "unistdp.hh"
#include "countof.hh"
#include "build-info.hh"
#include <sstream>
#include <cerrno>
#include <cstdlib>
#include <cassert>

#ifndef _MSC_VER
#include <dirent.h>
#endif

using std::string;

#ifdef _WIN32
using namespace utf8;
#endif

namespace openmsx {

namespace FileOperations {

#ifdef __APPLE__

static std::string findShareDir()
{
	// Find bundle location:
	// for an app folder, this is the outer directory,
	// for an unbundled executable, it is the executable itself.
	ProcessSerialNumber psn;
	if (GetCurrentProcess(&psn) != noErr) {
		throw FatalError("Failed to get process serial number");
	}
	FSRef location;
	if (GetProcessBundleLocation(&psn, &location) != noErr) {
		throw FatalError("Failed to get process bundle location");
	}
	// Get info about the location.
	FSCatalogInfo catalogInfo;
	FSRef parentRef;
	if (FSGetCatalogInfo(
		&location, kFSCatInfoVolume | kFSCatInfoNodeFlags,
		&catalogInfo, NULL, NULL, &parentRef
		) != noErr) {
		throw FatalError("Failed to get info about bundle path");
	}
	// Get reference to root directory of the volume we are searching.
	// We will need this later to know when to give up.
	FSRef root;
	if (FSGetVolumeInfo(
		catalogInfo.volume, 0, NULL, kFSVolInfoNone, NULL, NULL, &root
		) != noErr) {
		throw FatalError("Failed to get reference to root directory");
	}
	// Make sure we are looking at a directory.
	if (~catalogInfo.nodeFlags & kFSNodeIsDirectoryMask) {
		// Location is not a directory, so it is the path to the executable.
		location = parentRef;
	}
	while (true) {
		// Iterate through the files in the directory.
		FSIterator iterator;
		if (FSOpenIterator(&location, kFSIterateFlat, &iterator) != noErr) {
			throw FatalError("Failed to open iterator");
		}
		bool filesLeft = true; // iterator has files left for next call
		while (filesLeft) {
			// Get info about several files at a time.
			const int MAX_SCANNED_FILES = 100;
			ItemCount actualObjects;
			FSRef refs[MAX_SCANNED_FILES];
			FSCatalogInfo catalogInfos[MAX_SCANNED_FILES];
			HFSUniStr255 names[MAX_SCANNED_FILES];
			OSErr err = FSGetCatalogInfoBulk(
				iterator,
				MAX_SCANNED_FILES,
				&actualObjects,
				NULL /*containerChanged*/,
				kFSCatInfoNodeFlags,
				catalogInfos,
				refs,
				NULL /*specs*/,
				names
				);
			if (err == errFSNoMoreItems) {
				filesLeft = false;
			} else if (err != noErr) {
				throw FatalError("Catalog get failed");
			}
			for (ItemCount i = 0; i < actualObjects; i++) {
				// We're only interested in subdirectories.
				if (catalogInfos[i].nodeFlags & kFSNodeIsDirectoryMask) {
					// Convert the name to a CFString.
					CFStringRef name = CFStringCreateWithCharactersNoCopy(
						kCFAllocatorDefault,
						names[i].unicode,
						names[i].length,
						kCFAllocatorNull // do not deallocate character buffer
						);
					// Is this the directory we are looking for?
					static const CFStringRef SHARE = CFSTR("share");
					CFComparisonResult cmp = CFStringCompare(SHARE, name, 0);
					CFRelease(name);
					if (cmp == kCFCompareEqualTo) {
						// Clean up.
						OSErr closeErr = FSCloseIterator(iterator);
						assert(closeErr == noErr); (void)closeErr;
						// Get full path of directory.
						UInt8 path[256];
						if (FSRefMakePath(
							&refs[i], path, sizeof(path)) != noErr
							) {
							throw FatalError("Path too long");
						}
						return std::string(reinterpret_cast<char*>(path));
					}
				}
			}
		}
		OSErr closeErr = FSCloseIterator(iterator);
		assert(closeErr == noErr); (void)closeErr;
		// Are we in the root yet?
		if (FSCompareFSRefs(&location, &root) == noErr) {
			throw FatalError("Could not find \"share\" directory anywhere");
		}
		// Go up one level.
		if (FSGetCatalogInfo(
			&location, kFSCatInfoNone, NULL, NULL, NULL, &parentRef
			) != noErr
		) {
			throw FatalError("Failed to get parent directory");
		}
		location = parentRef;
	}
}

#endif // __APPLE__

string expandTilde(string_ref path)
{
	if (path.empty() || path[0] != '~') {
		return path.str();
	}
	string_ref::size_type pos = path.find_first_of('/');
	string_ref user = ((path.size() == 1) || (pos == 1))
	                ? ""
	                : path.substr(1, (pos == string_ref::npos) ? pos : pos - 1);
	string result = getUserHomeDir(user);
	if (result.empty()) {
		// failed to find homedir, return the path unchanged
		return path.str();
	}
	if (pos == string_ref::npos) {
		return result;
	}
	if (*result.rbegin() != '/') {
		result += '/';
	}
	string_ref last = path.substr(pos + 1);
	result.append(last.data(), last.size());
	return result;
}

void mkdir(const string& path, mode_t mode)
{
#ifdef _WIN32
	(void)&mode; // Suppress C4100 VC++ warning
	if ((path == "/") ||
		StringOp::endsWith(path, ':') ||
		StringOp::endsWith(path, ":/")) {
		return;
	}
	int result = _wmkdir(utf8to16(getNativePath(path)).c_str());
#else
	int result = ::mkdir(path.c_str(), mode);
#endif
	if (result && (errno != EEXIST)) {
		throw FileException("Error creating dir " + path);
	}
}

void mkdirp(string_ref path_)
{
	if (path_.empty()) {
		return;
	}
	string path = expandTilde(path_);

	string::size_type pos = 0;
	do {
		pos = path.find_first_of('/', pos + 1);
		mkdir(path.substr(0, pos), 0755);
	} while (pos != string::npos);

	if (!isDirectory(path)) {
		throw FileException("Error creating dir " + path);
	}
}

int unlink(const std::string& path)
{
#ifdef _WIN32
	return _wunlink(utf8to16(path).c_str());
#else
	return ::unlink(path.c_str());
#endif
}

int rmdir(const std::string& path)
{
#ifdef _WIN32
	return _wrmdir(utf8to16(path).c_str());
#else
	return ::rmdir(path.c_str());
#endif
}

FILE* openFile(const std::string& filename, const std::string& mode)
{
	// Mode must contain a 'b' character. On unix this doesn't make any
	// difference. But on windows this is required to open the file
	// in binary mode.
	assert(mode.find('b') != std::string::npos);
#ifdef _WIN32
	return _wfopen(
		utf8to16(filename).c_str(),
		utf8to16(mode).c_str());
#else
	return fopen(filename.c_str(), mode.c_str());
#endif
}

void openofstream(std::ofstream& stream, const std::string& filename)
{
#if defined _WIN32 && defined _MSC_VER
	// MinGW 3.x doesn't support ofstream.open(wchar_t*)
	// TODO - this means that unicode text may not work right here
	stream.open(utf8to16(filename).c_str());
#else
	stream.open(filename.c_str());
#endif
}

void openofstream(std::ofstream& stream, const std::string& filename,
				  std::ios_base::openmode mode)
{
#if defined _WIN32 && defined _MSC_VER
	// MinGW 3.x doesn't support ofstream.open(wchar_t*)
	// TODO - this means that unicode text may not work right here
	stream.open(utf8to16(filename).c_str(), mode);
#else
	stream.open(filename.c_str(), mode);
#endif
}

string_ref getFilename(string_ref path)
{
	string_ref::size_type pos = path.rfind('/');
	if (pos == string_ref::npos) {
		return path;
	} else {
		return path.substr(pos + 1);
	}
}

string_ref getBaseName(string_ref path)
{
	string_ref::size_type pos = path.rfind('/');
	if (pos == string_ref::npos) {
		return "";
	} else {
		return path.substr(0, pos + 1);
	}
}

string_ref getExtension(string_ref path)
{
	string_ref filename = getFilename(path);
	string_ref::size_type pos = filename.rfind('.');
	if (pos == string_ref::npos) {
		return "";
	} else {
		return filename.substr(pos + 1);
	}
}

string_ref stripExtension(string_ref path)
{
	string_ref::size_type pos = path.rfind('.');
	if (pos == string_ref::npos) {
		return path;
	} else {
		return path.substr(0, pos);
	}
}

string join(string_ref part1, string_ref part2)
{
	if (part1.empty() || isAbsolutePath(part2)) {
		return part2.str();
	}
	if (part1.back() == '/') {
		return part1 + part2;
	}
	return part1 + '/' + part2;
}
string join(string_ref part1, string_ref part2, string_ref part3)
{
	return join(part1, join(part2, part3));
}

string join(string_ref part1, string_ref part2,
            string_ref part3, string_ref part4)
{
	return join(part1, join(part2, join(part3, part4)));
}

string getNativePath(string_ref path)
{
	string result = path.str();
#ifdef _WIN32
	replace(result.begin(), result.end(), '/', '\\');
#endif
	return result;
}

string getConventionalPath(string_ref path)
{
	string result = path.str();
#ifdef _WIN32
	replace(result.begin(), result.end(), '\\', '/');
#endif
	return result;
}

string getCurrentWorkingDirectory()
{
#ifdef _WIN32
	wchar_t bufW[MAXPATHLEN];
	wchar_t* result = _wgetcwd(bufW, MAXPATHLEN);
	string buf;
	if (result) {
		buf = utf16to8(result);
	}
#else
	char buf[MAXPATHLEN];
	char* result = getcwd(buf, MAXPATHLEN);
#endif
	if (!result) {
		throw FileException("Couldn't get current working directory.");
	}
	return buf;
}

string getAbsolutePath(string_ref path)
{
	// In rare cases getCurrentWorkingDirectory() can throw,
	// so only call it when really necessary.
	if (isAbsolutePath(path)) {
		return path.str();
	}
	string currentDir = getCurrentWorkingDirectory();
	return join(currentDir, path);
}

bool isAbsolutePath(string_ref path)
{
#ifdef _WIN32
	if ((path.size() >= 3) && (path[1] == ':') && (path[2] == '/')) {
		char drive = tolower(path[0]);
		if (('a' <= drive) && (drive <= 'z')) {
			return true;
		}
	}
#endif
	return !path.empty() && (path[0] == '/');
}

string getUserHomeDir(string_ref username)
{
#ifdef _WIN32
	(void)(&username); // ignore parameter, avoid warning

	wchar_t bufW[MAXPATHLEN + 1];
	if (!SHGetSpecialFolderPathW(NULL, bufW, CSIDL_PERSONAL, TRUE)) {
		throw FatalError(StringOp::Builder() <<
			"SHGetSpecialFolderPathW failed: " << GetLastError());
	}

	return getConventionalPath(utf16to8(bufW));

#elif PLATFORM_GP2X
	return ""; // TODO figure out why stuff below doesn't work
	// We cannot use generic implementation below, because for some
	// reason getpwuid() and getpwnam() cannot be used in statically
	// linked applications.
	const char* dir = getenv("HOME");
	if (!dir) {
		dir = "/root";
	}
	return dir;
#else
	const char* dir = NULL;
	struct passwd* pw = NULL;
	if (username.empty()) {
		dir = getenv("HOME");
		if (!dir) {
			pw = getpwuid(getuid());
		}
	} else {
		pw = getpwnam(username.str().c_str());
	}
	if (pw) {
		dir = pw->pw_dir;
	}
	return dir ? dir : "";
#endif
}

const string& getUserOpenMSXDir()
{
#ifdef _WIN32
	static const string OPENMSX_DIR = expandTilde("~/openMSX");
#else
	static const string OPENMSX_DIR = expandTilde("~/.openMSX");
#endif
	return OPENMSX_DIR;
}

string getUserDataDir()
{
	const char* const NAME = "OPENMSX_USER_DATA";
	char* value = getenv(NAME);
	return value ? value : getUserOpenMSXDir() + "/share";
}

string getSystemDataDir()
{
	const char* const NAME = "OPENMSX_SYSTEM_DATA";
	if (char* value = getenv(NAME)) {
		return value;
	}

	string newValue;
#ifdef _WIN32
	wchar_t bufW[MAXPATHLEN + 1];
	int res = GetModuleFileNameW(NULL, bufW, countof(bufW));
	if (!res) {
		throw FatalError(StringOp::Builder() <<
			"Cannot detect openMSX directory. GetModuleFileNameW failed: " <<
			GetLastError());
	}

	string filename = utf16to8(bufW);
	string::size_type pos = filename.find_last_of('\\');
	if (pos == string::npos) {
		throw FatalError("openMSX is not in directory!?");
	}
	newValue = getConventionalPath(filename.substr(0, pos)) + "/share";
#elif defined(__APPLE__)
	newValue = findShareDir();
#else
	// defined in build-info.hh (default /opt/openMSX/share)
	newValue = DATADIR;
#endif
	return newValue;
}

#ifdef _WIN32
bool driveExists(char driveLetter)
{
	char buf[] = { driveLetter, ':', 0 };
	return GetFileAttributesA(buf) != INVALID_FILE_ATTRIBUTES;
}
#endif

string expandCurrentDirFromDrive(string_ref path)
{
	string result = path.str();
#ifdef _WIN32
	if (((path.size() == 2) && (path[1] == ':')) ||
		((path.size() >= 3) && (path[1] == ':') && (path[2] != '/'))) {
		// get current directory for this drive
		unsigned char drive = tolower(path[0]);
		if (('a' <= drive) && (drive <= 'z')) {
			wchar_t bufW[MAXPATHLEN + 1];
			if (driveExists(drive) &&
				_wgetdcwd(drive - 'a' + 1, bufW, MAXPATHLEN) != NULL) {
				result = getConventionalPath(utf16to8(bufW));
				if (*result.rbegin() != '/') {
					result += '/';
				}
				if (path.size() > 2) {
					string_ref tmp = path.substr(2);
					result.append(tmp.data(), tmp.size());
				}
			}
		}
	}
#endif
	return result;
}

bool getStat(const string& filename_, Stat& st)
{
	string filename = expandTilde(filename_);
	// workaround for VC++: strip trailing slashes (but keep it if it's the
	// only character in the path)
	string::size_type pos = filename.find_last_not_of('/');
	if (pos == string::npos) {
		// string was either empty or a (sequence of) '/' character(s)
		filename = filename.empty() ? "" : "/";
	} else {
		filename.resize(pos + 1);
	}
#ifdef _WIN32
	return _wstat(utf8to16(filename).c_str(), &st) == 0;
#else
	return stat(filename.c_str(), &st) == 0;
#endif
}

bool isRegularFile(const Stat& st)
{
	return S_ISREG(st.st_mode);
}
bool isRegularFile(const string& filename)
{
	Stat st;
	return getStat(filename, st) && isRegularFile(st);
}

bool isDirectory(const Stat& st)
{
	return S_ISDIR(st.st_mode);
}

bool isDirectory(const string& directory)
{
	Stat st;
	return getStat(directory, st) && isDirectory(st);
}

bool exists(const string& filename)
{
	Stat st; // dummy
	return getStat(filename, st);
}

time_t getModificationDate(const Stat& st)
{
	return st.st_mtime;
}

static int getNextNum(dirent* d, string_ref prefix, string_ref extension,
                      unsigned nofdigits)
{
	string_ref::size_type extensionLen = extension.size();
	string_ref::size_type prefixLen = prefix.size();
	string_ref name(d->d_name);

	if ((name.size() != (prefixLen + nofdigits + extensionLen)) ||
	    (name.substr(0, prefixLen) != prefix) ||
	    (name.substr(prefixLen + nofdigits, extensionLen) != extension)) {
		return 0;
	}
	string_ref num = name.substr(prefixLen, nofdigits);
	string_ref::size_type idx;
	unsigned long n = stoul(num, &idx, 10);
	return (idx == num.size()) ? n : 0;
}

string getNextNumberedFileName(
	string_ref directory, string_ref prefix, string_ref extension)
{
	const unsigned nofdigits = 4;

	int max_num = 0;

	string dirName = getUserOpenMSXDir() + '/' + directory;
	try {
		mkdirp(dirName);
	} catch (FileException&) {
		// ignore
	}

	ReadDir dir(dirName);
	while (dirent* d = dir.getEntry()) {
		max_num = std::max(max_num, getNextNum(d, prefix, extension, nofdigits));
	}

	std::ostringstream os;
	os << dirName << '/' << prefix;
	os.width(nofdigits);
	os.fill('0');
	os << (max_num + 1) << extension;
	return os.str();
}

string parseCommandFileArgument(
	string_ref argument, string_ref directory,
	string_ref prefix,   string_ref extension)
{
	if (argument.empty()) {
		// directory is also created when needed
		return getNextNumberedFileName(directory, prefix, extension);
	}

	string filename = argument.str();
	if (getBaseName(filename).empty()) {
		// no dir given, use standard dir (and create it)
		string dir = getUserOpenMSXDir() + '/' + directory;
		mkdirp(dir);
		filename = dir + '/' + filename;
	} else {
		filename = expandTilde(filename);
	}

	if (!StringOp::endsWith(filename, extension) &&
	    !exists(filename)) {
		// Expected extension not already given, append it. But only
		// when the filename without extension doesn't already exist.
		// Without this exception stuff like 'soundlog start /dev/null'
		// reports an error " ... error opening file /dev/null.wav."
		filename.append(extension.data(), extension.size());
	}
	return filename;
}

string getTempDir()
{
#ifdef _WIN32
	DWORD len = GetTempPathW(0, NULL);
	if (len) {
		VLA(wchar_t, bufW, (len+1));
		len = GetTempPathW(len, bufW);
		if (len) {
			// Strip last backslash
			if (bufW[len-1] == L'\\') {
				bufW[len-1] = L'\0';
			}
			return utf16to8(bufW);
		}
	}
	throw FatalError(StringOp::Builder() <<
		"GetTempPathW failed: " << GetLastError());
#else
	const char* result = NULL;
	if (!result) result = getenv("TMPDIR");
	if (!result) result = getenv("TMP");
	if (!result) result = getenv("TEMP");
	if (!result) {
		result = "/tmp";
	}
	return result;
#endif
}

FILE* openUniqueFile(const std::string& directory, std::string& filename)
{
#ifdef _WIN32
	std::wstring directoryW = utf8to16(directory);
	wchar_t filenameW[MAX_PATH];
	if (!GetTempFileNameW(directoryW.c_str(), L"msx", 0, filenameW)) {
		throw FileException(StringOp::Builder() <<
			"GetTempFileNameW failed: " << GetLastError());
	}
	filename = utf16to8(filenameW);
	FILE* fp = _wfopen(filenameW, L"wb");
#else
	filename = directory + "/XXXXXX";
	int fd = mkstemp(const_cast<char*>(filename.c_str()));
	if (fd == -1) {
		throw FileException("Coundn't get temp file name");
	}
	FILE* fp = fdopen(fd, "wb");
#endif
	return fp;
}

} // namespace FileOperations

} // namespace openmsx
