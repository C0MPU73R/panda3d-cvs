########################################################################
##
## Caution: there are two separate, independent build systems:
## 'makepanda', and 'ppremake'.  Use one or the other, do not attempt
## to use both.  This file is part of the 'makepanda' system.
##
## This file, makepandacore, contains all the global state and
## global functions for the makepanda system.
##
########################################################################

import sys,os,time,stat,string,re,getopt,cPickle,fnmatch,threading,Queue,signal,shutil,platform

SUFFIX_INC=[".cxx",".c",".h",".I",".yxx",".lxx",".mm"]
SUFFIX_DLL=[".dll",".dlo",".dle",".dli",".dlm",".mll",".exe"]
SUFFIX_LIB=[".lib",".ilb"]
STARTTIME=time.time()
MAINTHREAD=threading.currentThread()
OUTPUTDIR="built"

########################################################################
##
## Maya and Max Version List (with registry keys)
##
########################################################################

MAYAVERSIONINFO=[("MAYA6",   "6.0"),
                 ("MAYA65",  "6.5"),
                 ("MAYA7",   "7.0"),
                 ("MAYA8",   "8.0"),
                 ("MAYA85",  "8.5"),
                 ("MAYA2008","2008"),
                 ("MAYA2009","2009"),
]

MAXVERSIONINFO = [("MAX6", "SOFTWARE\\Autodesk\\3DSMAX\\6.0", "installdir", "maxsdk\\cssdk\\include"),
                  ("MAX7", "SOFTWARE\\Autodesk\\3DSMAX\\7.0", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX8", "SOFTWARE\\Autodesk\\3DSMAX\\8.0", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX9", "SOFTWARE\\Autodesk\\3DSMAX\\9.0", "Installdir", "maxsdk\\include\\CS"),
                  ("MAX2009", "SOFTWARE\\Autodesk\\3DSMAX\\11.0", "Installdir", "maxsdk\\include\\CS"),
]

MAYAVERSIONS=[]
MAXVERSIONS=[]
DXVERSIONS=["DX8","DX9"]

for (ver,key) in MAYAVERSIONINFO:
    MAYAVERSIONS.append(ver)

for (ver,key1,key2,subdir) in MAXVERSIONINFO:
    MAXVERSIONS.append(ver)

########################################################################
##
## The exit routine will normally
##
## - print a message
## - save the dependency cache
## - exit
##
## However, if it is invoked inside a thread, it instead:
##
## - prints a message
## - raises the "initiate-exit" exception
##
## If you create a thread, you must be prepared to catch this
## exception, save the dependency cache, and exit.
##
########################################################################

WARNINGS=[]

def PrettyTime(t):
    t = int(t)
    hours = t/3600
    t -= hours*3600
    minutes = t/60
    t -= minutes*60
    seconds = t
    if (hours): return str(hours)+" hours "+str(minutes)+" min"
    if (minutes): return str(minutes)+" min "+str(seconds)+" sec"
    return str(seconds)+" sec"

def exit(msg):
    if (threading.currentThread() == MAINTHREAD):
        SaveDependencyCache()
        # Move any files we've moved away back.
        if os.path.isfile("dtool/src/dtoolutil/pandaVersion.h.moved"):
          os.rename("dtool/src/dtoolutil/pandaVersion.h.moved", "dtool/src/dtoolutil/pandaVersion.h")
        if os.path.isfile("dtool/src/dtoolutil/checkPandaVersion.h.moved"):
          os.rename("dtool/src/dtoolutil/checkPandaVersion.h.moved", "dtool/src/dtoolutil/checkPandaVersion.h")
        if os.path.isfile("dtool/src/dtoolutil/checkPandaVersion.cxx.moved"):
          os.rename("dtool/src/dtoolutil/checkPandaVersion.cxx.moved", "dtool/src/dtoolutil/checkPandaVersion.cxx")
        print "Elapsed Time: "+PrettyTime(time.time() - STARTTIME)
        print msg
        sys.stdout.flush()
        sys.stderr.flush()
        os._exit(1)
    else:
        print msg
        raise "initiate-exit"

########################################################################
##
## Run a command.
##
########################################################################

def oscmd(cmd, ignoreError = False):
    print cmd
    sys.stdout.flush()
    if sys.platform == "win32":
        exe = cmd.split()[0]+".exe"
        if os.path.isfile(exe)==0:
            for i in os.environ["PATH"].split(";"):
                if os.path.isfile(os.path.join(i, exe)):
                    exe = os.path.join(i, exe)
                    break
            if os.path.isfile(exe)==0:
                exit("Cannot find "+exe+" on search path")
        res = os.spawnl(os.P_WAIT, exe, cmd)
    else:
        res = os.system(cmd)
    if res != 0 and not ignoreError:
        exit("")

########################################################################
##
## GetDirectoryContents
##
## At times, makepanda will use a function like "os.listdir" to process
## all the files in a directory.  Unfortunately, that means that any
## accidental addition of a file to a directory could cause makepanda
## to misbehave without warning.
##
## To alleviate this weakness, we created GetDirectoryContents.  This
## uses "os.listdir" to fetch the directory contents, but then it
## compares the results to the appropriate CVS/Entries to see if
## they match.  If not, it prints a big warning message.
##
########################################################################

def GetDirectoryContents(dir, filters="*", skip=[]):
    if (type(filters)==str):
        filters = [filters]
    actual = {}
    files = os.listdir(dir)
    for filter in filters:
        for file in fnmatch.filter(files, filter):
            if (skip.count(file)==0) and (os.path.isfile(dir + "/" + file)):
                actual[file] = 1
    if (os.path.isfile(dir + "/CVS/Entries")):
        cvs = {}
        srchandle = open(dir+"/CVS/Entries", "r")
        files = []
        for line in srchandle:
            if (line[0]=="/"):
                s = line.split("/",2)
                if (len(s)==3):
                    files.append(s[1])
        srchandle.close()
        for filter in filters:
            for file in fnmatch.filter(files, filter):
                if (skip.count(file)==0):
                    cvs[file] = 1
        for file in actual.keys():
            if (cvs.has_key(file)==0):
                msg = "WARNING: %s is in %s, but not in CVS"%(file, dir)
                print msg
                WARNINGS.append(msg)
        for file in cvs.keys():
            if (actual.has_key(file)==0):
                msg = "WARNING: %s is not in %s, but is in CVS"%(file, dir)
                print msg
                WARNINGS.append(msg)
    results = actual.keys()
    results.sort()
    return results

########################################################################
##
## LocateBinary
##
## This function searches the system PATH for the binary. Returns its
## full path when it is found, or None when it was not found.
##
########################################################################

def LocateBinary(binary):
    if not os.environ.has_key("PATH") or os.environ["PATH"] == "":
        p = os.defpath
    else:
        p = os.environ["PATH"]
    
    for path in p.split(os.pathsep):
        if os.access(os.path.join(path, binary), os.X_OK):
            return os.path.abspath(os.path.realpath(os.path.join(path, binary)))
    return None

########################################################################
##
## The Timestamp Cache
##
## The make utility is constantly fetching the timestamps of files.
## This can represent the bulk of the file accesses during the make
## process.  The timestamp cache eliminates redundant checks.
##
########################################################################

TIMESTAMPCACHE = {}

def GetTimestamp(path):
    if TIMESTAMPCACHE.has_key(path):
        return TIMESTAMPCACHE[path]
    try: date = os.path.getmtime(path)
    except: date = 0
    TIMESTAMPCACHE[path] = date
    return date

def ClearTimestamp(path):
    del TIMESTAMPCACHE[path]

########################################################################
##
## The Dependency cache.
##
## Makepanda's strategy for file dependencies is different from most
## make-utilities.  Whenever a file is built, makepanda records
## that the file was built, and it records what the input files were,
## and what their dates were.  Whenever a file is about to be built,
## panda compares the current list of input files and their dates,
## to the previous list of input files and their dates.  If they match,
## there is no need to build the file.
##
########################################################################

BUILTFROMCACHE = {}

def JustBuilt(files,others):
    dates = []
    for file in files:
        del TIMESTAMPCACHE[file]
        dates.append(GetTimestamp(file))
    for file in others:
        dates.append(GetTimestamp(file))
    key = tuple(files)
    BUILTFROMCACHE[key] = [others,dates]

def NeedsBuild(files,others):
    dates = []
    for file in files:
        dates.append(GetTimestamp(file))
    for file in others:
        dates.append(GetTimestamp(file))
    key = tuple(files)
    if (BUILTFROMCACHE.has_key(key)):
        if (BUILTFROMCACHE[key] == [others,dates]):
            return 0
        else:
            oldothers = BUILTFROMCACHE[key][0]
            if (oldothers != others):
                print "CAUTION: file dependencies changed: "+str(files)
    return 1

########################################################################
##
## The CXX include cache:
##
## The following routine scans a CXX file and returns a list of
## the include-directives inside that file.  It's not recursive:
## it just returns the includes that are textually inside the 
## file.  If you need recursive dependencies, you need the higher-level
## routine CxxCalcDependencies, defined elsewhere.
##
## Since scanning a CXX file is slow, we cache the result.  It records
## the date of the source file and the list of includes that it
## contains.  It assumes that if the file date hasn't changed, that
## the list of include-statements inside the file has not changed
## either.  Once again, this particular routine does not return
## recursive dependencies --- it only returns an explicit list of
## include statements that are textually inside the file.  That
## is what the cache stores, as well.
##
########################################################################

CXXINCLUDECACHE = {}

global CxxIncludeRegex
CxxIncludeRegex = re.compile('^[ \t]*[#][ \t]*include[ \t]+"([^"]+)"[ \t\r\n]*$')

def CxxGetIncludes(path):
    date = GetTimestamp(path)
    if (CXXINCLUDECACHE.has_key(path)):
        cached = CXXINCLUDECACHE[path]
        if (cached[0]==date): return cached[1]
    try: sfile = open(path, 'rb')
    except:
        exit("Cannot open source file \""+path+"\" for reading.")
    include = []
    for line in sfile:
        match = CxxIncludeRegex.match(line,0)
        if (match):
            incname = match.group(1)
            include.append(incname)
    sfile.close()
    CXXINCLUDECACHE[path] = [date, include]
    return include

########################################################################
##
## SaveDependencyCache / LoadDependencyCache
##
## This actually saves both the dependency and cxx-include caches.
##
########################################################################

def SaveDependencyCache():
    try: icache = open(os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache"),'wb')
    except: icache = 0
    if (icache!=0):
        print "Storing dependency cache."
        cPickle.dump(CXXINCLUDECACHE, icache, 1)
        cPickle.dump(BUILTFROMCACHE, icache, 1)
        icache.close()

def LoadDependencyCache():
    global CXXINCLUDECACHE
    global BUILTFROMCACHE
    try: icache = open(os.path.join(OUTPUTDIR, "tmp", "makepanda-dcache"),'rb')
    except: icache = 0
    if (icache!=0):
        CXXINCLUDECACHE = cPickle.load(icache)
        BUILTFROMCACHE = cPickle.load(icache)
        icache.close()

########################################################################
##
## CxxFindSource: given a source file name and a directory list,
## searches the directory list for the given source file.  Returns
## the full pathname of the located file.
##
## CxxFindHeader: given a source file, an include directive, and a
## directory list, searches the directory list for the given header
## file.  Returns the full pathname of the located file.
##
## Of course, CxxFindSource and CxxFindHeader cannot find a source
## file that has not been created yet.  This can cause dependency
## problems.  So the function CreateStubHeader can be used to create
## a file that CxxFindSource or CxxFindHeader can subsequently find.
##
########################################################################

def CxxFindSource(name, ipath):
    for dir in ipath:
        if (dir == "."): full = name
        else: full = dir + "/" + name
        if GetTimestamp(full) > 0: return full
    exit("Could not find source file: "+name)

def CxxFindHeader(srcfile, incfile, ipath):
    if (incfile.startswith(".")):
        last = srcfile.rfind("/")
        if (last < 0): exit("CxxFindHeader cannot handle this case #1")
        srcdir = srcfile[:last+1]
        while (incfile[:1]=="."):
            if (incfile[:2]=="./"):
                incfile = incfile[2:]
            elif (incfile[:3]=="../"):
                incfile = incfile[3:]
                last = srcdir[:-1].rfind("/")
                if (last < 0): exit("CxxFindHeader cannot handle this case #2")
                srcdir = srcdir[:last+1]
            else: exit("CxxFindHeader cannot handle this case #3")
        full = srcdir + incfile
        if GetTimestamp(full) > 0: return full
        return 0
    else:
        for dir in ipath:
            full = dir + "/" + incfile
            if GetTimestamp(full) > 0: return full
        return 0

########################################################################
##
## CxxCalcDependencies(srcfile, ipath, ignore)
##
## Calculate the dependencies of a source file given a
## particular include-path.  Any file in the list of files to
## ignore is not considered.
##
########################################################################

global CxxIgnoreHeader
global CxxDependencyCache
CxxIgnoreHeader = {}
CxxDependencyCache = {}

def CxxCalcDependencies(srcfile, ipath, ignore):
    if (CxxDependencyCache.has_key(srcfile)):
        return CxxDependencyCache[srcfile]
    if (ignore.count(srcfile)): return []
    dep = {}
    dep[srcfile] = 1
    includes = CxxGetIncludes(srcfile)
    for include in includes:
        header = CxxFindHeader(srcfile, include, ipath)
        if (header!=0):
            if (ignore.count(header)==0):
                hdeps = CxxCalcDependencies(header, ipath, [srcfile]+ignore)
                for x in hdeps: dep[x] = 1
    result = dep.keys()
    CxxDependencyCache[srcfile] = result
    return result

########################################################################
##
## Registry Key Handling
##
## Of course, these routines will fail if you call them on a
## non win32 platform.  If you use them on a win64 platform, they
## will look in the win32 private hive first, then look in the
## win64 hive.
##
########################################################################

if sys.platform == "win32":
    import _winreg

def TryRegistryKey(path):
    try:
        key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, path, 0, _winreg.KEY_READ)
        return key
    except: pass
    try:
        key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, path, 0, _winreg.KEY_READ | 256)
        return key
    except: pass
    return 0
        
def ListRegistryKeys(path):
    result=[]
    index=0
    key = TryRegistryKey(path)
    if (key != 0):
        try:
            while (1):
                result.append(_winreg.EnumKey(key, index))
                index = index + 1
        except: pass
        _winreg.CloseKey(key)
    return result

def GetRegistryKey(path, subkey):
    k1=0
    key = TryRegistryKey(path)
    if (key != 0):
        try:
            k1, k2 = _winreg.QueryValueEx(key, subkey)
        except: pass
        _winreg.CloseKey(key)
    return k1

def GetProgramFiles():
    if (os.environ.has_key("PROGRAMFILES")):
        return os.environ["PROGRAMFILES"]
    elif (os.path.isdir("C:\\Program Files")):
        return "C:\\Program Files"
    elif (os.path.isdir("D:\\Program Files")):
        return "D:\\Program Files"
    elif (os.path.isdir("E:\\Program Files")):
        return "E:\\Program Files"
    return 0

########################################################################
##
## Parsing Compiler Option Lists
##
########################################################################

def GetListOption(opts, prefix):
    res=[]
    for x in opts:
        if (x.startswith(prefix)):
            res.append(x[len(prefix):])
    return res

def GetValueOption(opts, prefix):
    for x in opts:
        if (x.startswith(prefix)):
            return x[len(prefix):]
    return 0

def GetOptimizeOption(opts,defval):
    val = GetValueOption(opts, "OPT:")
    if (val == 0):
        return defval
    return val

########################################################################
##
## General File Manipulation
##
########################################################################

def MakeDirectory(path):
    if os.path.isdir(path): return 0
    os.mkdir(path)

def ReadFile(wfile):
    try:
        srchandle = open(wfile, "rb")
        data = srchandle.read()
        srchandle.close()
        return data
    except: exit("Cannot read "+wfile)

def WriteFile(wfile,data):
    try:
        dsthandle = open(wfile, "wb")
        dsthandle.write(data)
        dsthandle.close()
    except: exit("Cannot write "+wfile)

def ConditionalWriteFile(dest,desiredcontents):
    try:
        rfile = open(dest, 'rb')
        contents = rfile.read(-1)
        rfile.close()
    except:
        contents=0
    if contents != desiredcontents:
        sys.stdout.flush()
        WriteFile(dest,desiredcontents)

def DeleteCVS(dir):
    for entry in os.listdir(dir):
        if (entry != ".") and (entry != ".."):
            subdir = dir + "/" + entry
            if (os.path.isdir(subdir)):
                if (entry == "CVS"):
                    shutil.rmtree(subdir)
                else:
                    DeleteCVS(subdir)
            elif (os.path.isfile(subdir) and entry == ".cvsignore"):
                os.remove(subdir)

def DeleteCXX(dir):
    for entry in os.listdir(dir):
        if (entry != ".") and (entry != ".."):
            subdir = dir + "/" + entry
            if (os.path.isfile(subdir) and os.path.splitext(subdir)[-1] in [".h", ".I", ".c", ".cxx", ".cpp"]):
                os.remove(subdir)

def CreateFile(file):
    if (os.path.exists(file)==0):
        WriteFile(file,"")

########################################################################
#
# Create the panda build tree.
#
########################################################################

def MakeBuildTree():
    MakeDirectory(OUTPUTDIR)
    MakeDirectory(OUTPUTDIR+"/bin")
    MakeDirectory(OUTPUTDIR+"/lib")
    MakeDirectory(OUTPUTDIR+"/tmp")
    MakeDirectory(OUTPUTDIR+"/etc")
    MakeDirectory(OUTPUTDIR+"/plugins")
    MakeDirectory(OUTPUTDIR+"/modelcache")
    MakeDirectory(OUTPUTDIR+"/include")
    MakeDirectory(OUTPUTDIR+"/include/parser-inc")
    MakeDirectory(OUTPUTDIR+"/include/parser-inc/openssl")
    MakeDirectory(OUTPUTDIR+"/include/parser-inc/netinet")
    MakeDirectory(OUTPUTDIR+"/include/parser-inc/Cg")
    MakeDirectory(OUTPUTDIR+"/include/openssl")
    MakeDirectory(OUTPUTDIR+"/models")
    MakeDirectory(OUTPUTDIR+"/models/audio")
    MakeDirectory(OUTPUTDIR+"/models/audio/sfx")
    MakeDirectory(OUTPUTDIR+"/models/icons")
    MakeDirectory(OUTPUTDIR+"/models/maps")
    MakeDirectory(OUTPUTDIR+"/models/misc")
    MakeDirectory(OUTPUTDIR+"/models/gui")
    MakeDirectory(OUTPUTDIR+"/direct")
    MakeDirectory(OUTPUTDIR+"/pandac")
    MakeDirectory(OUTPUTDIR+"/pandac/input")

########################################################################
#
# Make sure that you are in the root of the panda tree.
#
########################################################################

def CheckPandaSourceTree():
    dir = os.getcwd()
    if ((os.path.exists(os.path.join(dir, "makepanda/makepanda.py"))==0) or
        (os.path.exists(os.path.join(dir, "dtool","src","dtoolbase","dtoolbase.h"))==0) or
        (os.path.exists(os.path.join(dir, "panda","src","pandabase","pandabase.h"))==0)):
        exit("Current directory is not the root of the panda tree.")

########################################################################
##
## Visual Studio Manifest Manipulation.
##
########################################################################

VC90CRTVERSIONRE=re.compile("name=['\"]Microsoft.VC90.CRT['\"]\\s+version=['\"]([0-9.]+)['\"]")

def GetVC90CRTVersion(fn):
    manifest = ReadFile(fn)
    version = VC90CRTVERSIONRE.search(manifest)
    if (version == None):
        exit("Cannot locate version number in "+fn)
    return version.group(1)

def SetVC90CRTVersion(fn, ver):
    manifest = ReadFile(fn)
    subst = " name='Microsoft.VC90.CRT' version='"+ver+"' "
    manifest = VC90CRTVERSIONRE.sub(subst, manifest)
    WriteFile(fn, manifest)

########################################################################
##
## Gets or sets the output directory, by default "built".
##
########################################################################

def GetOutputDir():
  return OUTPUTDIR

def SetOutputDir(outputdir):
  global OUTPUTDIR
  OUTPUTDIR=outputdir

########################################################################
##
## Package Selection
##
## This facility enables makepanda to keep a list of packages selected
## by the user for inclusion or omission.
##
########################################################################

PKG_LIST_ALL=0
PKG_LIST_OMIT=0

def PkgListSet(pkgs):
    global PKG_LIST_ALL
    global PKG_LIST_OMIT
    PKG_LIST_ALL=pkgs
    PKG_LIST_OMIT={}
    PkgDisableAll()

def PkgListGet():
    return PKG_LIST_ALL

def PkgEnableAll():
    for x in PKG_LIST_ALL:
        PKG_LIST_OMIT[x] = 0

def PkgDisableAll():
    for x in PKG_LIST_ALL:
        PKG_LIST_OMIT[x] = 1

def PkgEnable(pkg):
    PKG_LIST_OMIT[pkg] = 0

def PkgDisable(pkg):
    PKG_LIST_OMIT[pkg] = 1

def PkgSkip(pkg):
    return PKG_LIST_OMIT[pkg]

def PkgSelected(pkglist, pkg):
    if (pkglist.count(pkg)==0): return 0
    if (PKG_LIST_OMIT[pkg]): return 0
    return 1

########################################################################
##
## These functions are for libraries which use pkg-config.
##
########################################################################

def PkgConfigHavePkg(pkgname):
    """Returns a bool whether the pkg-config package is installed."""
    if (sys.platform == "win32" or not LocateBinary("pkg-config")):
        return False
    handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --modversion " + pkgname)
    result = handle.read().strip()
    handle.close()
    return bool(len(result) > 0)

def PkgConfigGetLibs(pkgname):
    """Returns a list of libs for the package, prefixed by -l."""
    if (sys.platform == "win32" or not LocateBinary("pkg-config")):
        return []
    handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --libs-only-l " + pkgname)
    result = handle.read().strip()
    handle.close()
    libs = []
    for l in result.split(" "):
        libs.append(l)
    return libs

def PkgConfigGetIncDirs(pkgname):
    """Returns a list of includes for the package, NOT prefixed by -I."""
    if (sys.platform == "win32" or not LocateBinary("pkg-config")):
        return []
    handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --cflags-only-I " + pkgname)
    result = handle.read().strip()
    handle.close()
    if len(result) == 0: return []
    libs = []
    for l in result.split(" "):
        libs.append(l.replace("-I", "").replace("\"", "").strip())
    return libs

def PkgConfigGetLibDirs(pkgname):
    """Returns a list of library paths for the package, NOT prefixed by -L."""
    if (sys.platform == "win32" or not LocateBinary("pkg-config")):
        return []
    handle = os.popen(LocateBinary("pkg-config") + " --silence-errors --libs-only-L " + pkgname)
    result = handle.read().strip()
    handle.close()
    if len(result) == 0: return []
    libs = []
    for l in result.split(" "):
        libs.append(l.replace("-L", "").replace("\"", "").strip())
    return libs

def PkgConfigEnable(opt, pkgname):
    """Adds the libraries and includes to IncDirectory, LibName and LibDirectory."""
    for i in PkgConfigGetIncDirs(pkgname):
        IncDirectory(opt, i)
    for i in PkgConfigGetLibDirs(pkgname):
        LibDirectory(opt, i)
    for i in PkgConfigGetLibs(pkgname):
        LibName(opt, i)

########################################################################
##
## SDK Location
##
## This section is concerned with locating the install directories
## for various third-party packages.  The results are stored in the
## SDK table.
##
## Microsoft keeps changing the &*#$*& registry key for the DirectX SDK.
## The only way to reliably find it is to search through the installer's
## uninstall-directories, look in each one, and see if it contains the
## relevant files.
##
########################################################################

SDK = {}

def SdkLocateDirectX():
    if (sys.platform != "win32"): return
    if (os.path.isdir("sdks/directx8")): SDK["DX8"]="sdks/directx8"
    if (os.path.isdir("sdks/directx9")): SDK["DX9"]="sdks/directx9"
    uninstaller = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    for subdir in ListRegistryKeys(uninstaller):
        if (subdir[0]=="{"):
            dir = GetRegistryKey(uninstaller+"\\"+subdir, "InstallLocation")
            if (dir != 0):
                if ((SDK.has_key("DX8")==0) and
                    (os.path.isfile(dir+"\\Include\\d3d8.h")) and
                    (os.path.isfile(dir+"\\Include\\d3dx8.h")) and
                    (os.path.isfile(dir+"\\Lib\\d3d8.lib")) and
                    (os.path.isfile(dir+"\\Lib\\d3dx8.lib"))):
                   SDK["DX8"] = dir.replace("\\", "/").rstrip("/")
                if ((SDK.has_key("DX9")==0) and
                    (os.path.isfile(dir+"\\Include\\d3d9.h")) and
                    (os.path.isfile(dir+"\\Include\\d3dx9.h")) and
                    (os.path.isfile(dir+"\\Include\\dxsdkver.h")) and
                    (os.path.isfile(dir+"\\Lib\\x86\\d3d9.lib")) and
                    (os.path.isfile(dir+"\\Lib\\x86\\d3dx9.lib"))):
                   SDK["DX9"] = dir.replace("\\", "/").rstrip("/")
    if (SDK.has_key("DX9")):
        SDK["DIRECTCAM"] = SDK["DX9"]

def SdkLocateMaya():
    for (ver,key) in MAYAVERSIONINFO:
        if (PkgSkip(ver)==0 and SDK.has_key(ver)==0):
            if (sys.platform == "win32"):
                ddir = "sdks/"+ver.lower().replace("x","")
                if (os.path.isdir(ddir)):
                    SDK[ver] = ddir
                else:
                    for dev in ["Alias|Wavefront","Alias","Autodesk"]:
                        fullkey="SOFTWARE\\"+dev+"\\Maya\\"+key+"\\Setup\\InstallPath"
                        res = GetRegistryKey(fullkey, "MAYA_INSTALL_LOCATION")
                        if (res != 0):
                            res = res.replace("\\", "/").rstrip("/")
                            SDK[ver] = res
            elif (sys.platform == "darwin"):
                ddir1 = "sdks/"+ver.lower().replace("x","")+"-osx"
                ddir2 = "/Applications/Autodesk/maya"+key+"/Maya.app/Contents"
                
                if (os.path.isdir(ddir1)):   SDK[ver] = ddir1
                elif (os.path.isdir(ddir2)): SDK[ver] = ddir2
            else:
                ddir1 = "sdks/"+ver.lower().replace("x","")+"-linux"+platform.architecture()[0].replace("bit","")
                if (platform.architecture()[0] == "64bit"):
                    ddir2 = "/usr/autodesk/maya"+key+"-x64"
                    ddir3 = "/usr/aw/maya"+key+"-x64"
                else:
                    ddir2 = "/usr/autodesk/maya"+key
                    ddir3 = "/usr/aw/maya"+key
                
                if (os.path.isdir(ddir1)):   SDK[ver] = ddir1
                elif (os.path.isdir(ddir2)): SDK[ver] = ddir2
                elif (os.path.isdir(ddir3)): SDK[ver] = ddir3

def SdkLocateMax():
    if (sys.platform != "win32"): return
    for version,key1,key2,subdir in MAXVERSIONINFO:
        if (PkgSkip(version)==0):
            if (SDK.has_key(version)==0):
                ddir = "sdks/maxsdk"+version.lower()[3:]
                if (os.path.isdir(ddir)):
                    SDK[version] = ddir
                    SDK[version+"CS"] = ddir
                else:
                    top = GetRegistryKey(key1,key2)
                    if (top != 0):
                        SDK[version] = top + "maxsdk"
                        if (os.path.isdir(top + "\\" + subdir)!=0):
                            SDK[version+"CS"] = top + subdir

def SdkLocatePython():
    if (PkgSkip("PYTHON")==0):
        if (sys.platform == "win32"):
            SDK["PYTHON"]="thirdparty/win-python"
            SDK["PYTHONVERSION"]="python2.5"
        elif (sys.platform == "darwin"):
            if not SDK.has_key("MACOSX"): SdkLocateMacOSX()
            if (os.path.isdir("%s/System/Library/Frameworks/Python.framework" % SDK["MACOSX"])):
                os.system("readlink %s/System/Library/Frameworks/Python.framework/Versions/Current > %s/tmp/pythonversion 2>&1" % (SDK["MACOSX"], OUTPUTDIR))
                pv = ReadFile(OUTPUTDIR+"/tmp/pythonversion")
                SDK["PYTHON"] = SDK["MACOSX"]+"/System/Library/Frameworks/Python.framework/Headers"
                SDK["PYTHONVERSION"] = "python"+pv
            else:
                exit("Could not find the python framework!")
        else:
            os.system("python -V > "+OUTPUTDIR+"/tmp/pythonversion 2>&1")
            pv=ReadFile(OUTPUTDIR+"/tmp/pythonversion")
            if (pv.startswith("Python ")==0):
                exit("python -V did not produce the expected output")
            pv = pv[7:10]
            if (os.path.isdir("/usr/include/python"+pv)==0):
                exit("Python reports version "+pv+" but /usr/include/python"+pv+" is not installed.")
            SDK["PYTHON"]="/usr/include/python"+pv
            SDK["PYTHONVERSION"]="python"+pv

def SdkLocateVisualStudio():
    if (sys.platform != "win32"): return
    vcdir = GetRegistryKey("SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7", "9.0")
    if (vcdir != 0) and (vcdir[-4:] == "\\VC\\"):
        vcdir = vcdir[:-3]
        SDK["VISUALSTUDIO"] = vcdir

def SdkLocateMSPlatform():
    if (sys.platform != "win32"): return
    platsdk=GetRegistryKey("SOFTWARE\\Microsoft\\MicrosoftSDK\\InstalledSDKs\\D2FF9F89-8AA2-4373-8A31-C838BF4DBBE1", "Install Dir")
    if (platsdk == 0):
        platsdk=GetRegistryKey("SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v6.1","InstallationFolder")
    
    if (platsdk == 0 and os.path.isdir(os.path.join(GetProgramFiles(), "Microsoft Platform SDK for Windows Server 2003 R2"))):
        platsdk = os.path.join(GetProgramFiles(), "Microsoft Platform SDK for Windows Server 2003 R2")
    
    # Doesn't work with the Express versions, so we're checking for the "atlmfc" dir, which is not in the Express 
    if (platsdk == 0 and os.path.isdir(os.path.join(GetProgramFiles(), "Microsoft Visual Studio 9\\VC\\atlmfc"))):
        platsdk = os.path.join(GetProgramFiles(), "Microsoft Visual Studio 9\\VC\\PlatformSDK")
    
    if (platsdk != 0):
        if (not platsdk.endswith("//")):
            platsdk += "//"
        SDK["MSPLATFORM"] = platsdk

def SdkLocateMacOSX():
    if (sys.platform != "darwin"): return
    if (os.path.exists("/Developer/SDKs/MacOSX10.5.sdk")):
        SDK["MACOSX"] = "/Developer/SDKs/MacOSX10.5.sdk"
    elif (os.path.exists("/Developer/SDKs/MacOSX10.4u.sdk")):
        SDK["MACOSX"] = "/Developer/SDKs/MacOSX10.4u.sdk"
    elif (os.path.exists("/Developer/SDKs/MacOSX10.4.0.sdk")):
        SDK["MACOSX"] = "/Developer/SDKs/MacOSX10.4.0.sdk"
    else:
        exit("Could not find any MacOSX SDK")

########################################################################
##
## SDK Auto-Disables
##
## Disable packages whose SDKs could not be found.
##
########################################################################

def SdkAutoDisableDirectX():
    for ver in ["DX8","DX9","DIRECTCAM"]:
        if (PkgSkip(ver)==0):
            if (SDK.has_key(ver)==0):
                if (sys.platform == "win32"):
                    WARNINGS.append("I cannot locate SDK for "+ver)
                else:
                    WARNINGS.append(ver+" only supported on windows yet")
                WARNINGS.append("I have automatically added this command-line option: --no-"+ver.lower())
                PkgDisable(ver)
            else:
                WARNINGS.append("Using "+ver+" sdk: "+SDK[ver])

def SdkAutoDisableMaya():
    for (ver,key) in MAYAVERSIONINFO:
        if (SDK.has_key(ver)==0) and (PkgSkip(ver)==0):
            if (sys.platform == "win32"):
                WARNINGS.append("The registry does not appear to contain a pointer to the "+ver+" SDK.")
            else:
                WARNINGS.append("I cannot locate SDK for "+ver)
            WARNINGS.append("I have automatically added this command-line option: --no-"+ver.lower())
            PkgDisable(ver)

def SdkAutoDisableMax():
    for version,key1,key2,subdir in MAXVERSIONINFO:
        if (PkgSkip(version)==0) and ((SDK.has_key(version)==0) or (SDK.has_key(version+"CS")==0)): 
            if (sys.platform == "win32"):
                if (SDK.has_key(version)):
                    WARNINGS.append("Your copy of "+version+" does not include the character studio SDK")
                else: 
                    WARNINGS.append("The registry does not appear to contain a pointer to "+version)
            else:
                WARNINGS.append(version+" only supported on windows yet")
            WARNINGS.append("I have automatically added this command-line option: --no-"+version.lower())
            PkgDisable(version)

########################################################################
##
## Visual Studio comes with a script called VSVARS32.BAT, which 
## you need to run before using visual studio command-line tools.
## The following python subroutine serves the same purpose.
##
########################################################################

def AddToPathEnv(path,add):
    if (os.environ.has_key(path)):
        os.environ[path] = add + ";" + os.environ[path]
    else:
        os.environ[path] = add

def SetupVisualStudioEnviron():
    if (SDK.has_key("VISUALSTUDIO")==0):
        exit("Could not find Visual Studio install directory")
    if (SDK.has_key("MSPLATFORM")==0):
        exit("Could not find the Microsoft Platform SDK")
    AddToPathEnv("PATH",    SDK["VISUALSTUDIO"] + "VC\\bin")
    AddToPathEnv("PATH",    SDK["VISUALSTUDIO"] + "Common7\\IDE")
    AddToPathEnv("INCLUDE", SDK["VISUALSTUDIO"] + "VC\\include")
    AddToPathEnv("INCLUDE", SDK["VISUALSTUDIO"] + "VC\\atlmfc\\include")
    AddToPathEnv("LIB",     SDK["VISUALSTUDIO"] + "VC\\lib")
    AddToPathEnv("PATH",    SDK["MSPLATFORM"] + "bin")
    AddToPathEnv("INCLUDE", SDK["MSPLATFORM"] + "include")
    AddToPathEnv("INCLUDE", SDK["MSPLATFORM"] + "include\\atl")
    AddToPathEnv("LIB",     SDK["MSPLATFORM"] + "lib")

########################################################################
#
# Include and Lib directories.
#
# These allow you to add include and lib directories to the
# compiler search paths.  These methods accept a "package"
# parameter, which specifies which package the directory is
# associated with.  The include/lib directory is not used
# if the package is not selected.  The package can be 'ALWAYS'.
#
########################################################################

INCDIRECTORIES = []
LIBDIRECTORIES = []
LIBNAMES = []
DEFSYMBOLS = []

def IncDirectory(opt, dir):
    INCDIRECTORIES.append((opt, dir))

def LibDirectory(opt, dir):
    LIBDIRECTORIES.append((opt, dir))

def LibName(opt, name):
    LIBNAMES.append((opt, name))

def DefSymbol(opt, sym, val):
    DEFSYMBOLS.append((opt, sym, val))

########################################################################
#
# On Linux, to run panda, the dynamic linker needs to know how to find
# the shared libraries.  This subroutine verifies that the dynamic
# linker is properly configured.  If not, it sets it up on a temporary
# basis and issues a warning.
#
########################################################################


def CheckLinkerLibraryPath():
    if (sys.platform == "win32"): return
    builtlib = os.path.abspath(os.path.join(OUTPUTDIR,"lib"))
    try:
        ldpath = []
        f = file("/etc/ld.so.conf","r")
        for line in f: ldpath.append(line.rstrip())
        f.close()
    except: ldpath = []
    if (os.environ.has_key("LD_LIBRARY_PATH")):
        ldpath = ldpath + os.environ["LD_LIBRARY_PATH"].split(":")
    if (ldpath.count(builtlib)==0):
        WARNINGS.append("Caution: the "+os.path.join(OUTPUTDIR,"lib")+" directory is not in LD_LIBRARY_PATH")
        WARNINGS.append("or /etc/ld.so.conf.  You must add it before using panda.")
        if (os.environ.has_key("LD_LIBRARY_PATH")):
            os.environ["LD_LIBRARY_PATH"] = builtlib + ":" + os.environ["LD_LIBRARY_PATH"]
        else:
            os.environ["LD_LIBRARY_PATH"] = builtlib

########################################################################
##
## Routines to copy files into the build tree
##
########################################################################

def CopyFile(dstfile,srcfile):
    if (dstfile[-1]=='/'):
        dstdir = dstfile
        fnl = srcfile.rfind("/")
        if (fnl < 0): fn = srcfile
        else: fn = srcfile[fnl+1:]
        dstfile = dstdir + fn
    if (NeedsBuild([dstfile],[srcfile])):
        WriteFile(dstfile,ReadFile(srcfile))
        JustBuilt([dstfile], [srcfile])

def CopyAllFiles(dstdir, srcdir, suffix=""):
    for x in GetDirectoryContents(srcdir, ["*"+suffix]):
        CopyFile(dstdir+x, srcdir+x)

def CopyAllHeaders(dir, skip=[]):
    for filename in GetDirectoryContents(dir, ["*.h", "*.I", "*.T"], skip):
        srcfile = dir + "/" + filename
        dstfile = OUTPUTDIR+"/include/" + filename
        if (NeedsBuild([dstfile],[srcfile])):
            WriteFile(dstfile,ReadFile(srcfile))
            JustBuilt([dstfile],[srcfile])

def CopyTree(dstdir,srcdir):
    if (os.path.isdir(dstdir)): return 0
    if (sys.platform == "win32"):
        cmd = 'xcopy /I/Y/E/Q "' + srcdir + '" "' + dstdir + '"'
    else:
        cmd = 'cp -R -f ' + srcdir + ' ' + dstdir
    oscmd(cmd)

########################################################################
##
## Parse PandaVersion.pp to extract the version number.
##
########################################################################

def ParsePandaVersion(fn):
    try:
        f = file(fn, "r")
        pattern = re.compile('^[ \t]*[#][ \t]*define[ \t]+PANDA_VERSION[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)')
        for line in f:
            match = pattern.match(line,0)
            if (match):
                version = match.group(1)+"."+match.group(2)+"."+match.group(3)
                break
        f.close()
    except: version="0.0.0"
    return version

########################################################################
##
## FindLocation
##
########################################################################

ORIG_EXT={}

def GetOrigExt(x):
    return ORIG_EXT[x]

def CalcLocation(fn, ipath):
    if (fn.count("/")): return fn
    
    if (fn.endswith(".cxx")): return CxxFindSource(fn, ipath)
    if (fn.endswith(".I")):   return CxxFindSource(fn, ipath)
    if (fn.endswith(".h")):   return CxxFindSource(fn, ipath)
    if (fn.endswith(".c")):   return CxxFindSource(fn, ipath)
    if (fn.endswith(".yxx")): return CxxFindSource(fn, ipath)
    if (fn.endswith(".lxx")): return CxxFindSource(fn, ipath)
    if (fn.endswith(".mll")): return OUTPUTDIR+"/plugins/"+fn
    if (sys.platform == "win32"):
        if (fn.endswith(".def")): return CxxFindSource(fn, ipath)
        if (fn.endswith(".obj")): return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".dll")): return OUTPUTDIR+"/bin/"+fn
        if (fn.endswith(".dlo")): return OUTPUTDIR+"/plugins/"+fn
        if (fn.endswith(".dli")): return OUTPUTDIR+"/plugins/"+fn
        if (fn.endswith(".dle")): return OUTPUTDIR+"/plugins/"+fn
        if (fn.endswith(".exe")): return OUTPUTDIR+"/bin/"+fn
        if (fn.endswith(".lib")): return OUTPUTDIR+"/lib/"+fn
        if (fn.endswith(".ilb")): return OUTPUTDIR+"/tmp/"+fn[:-4]+".lib"
        if (fn.endswith(".dat")): return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".in")):  return OUTPUTDIR+"/pandac/input/"+fn
    elif (sys.platform == "darwin"):
        if (fn.endswith(".mm")):  return CxxFindSource(fn, ipath)
        if (fn.endswith(".obj")): return OUTPUTDIR+"/tmp/"+fn[:-4]+".o"
        if (fn.endswith(".dll")): return OUTPUTDIR+"/lib/"+fn[:-4]+".dylib"
        if (fn.endswith(".exe")): return OUTPUTDIR+"/bin/"+fn[:-4]
        if (fn.endswith(".lib")): return OUTPUTDIR+"/lib/"+fn[:-4]+".a"
        if (fn.endswith(".ilb")): return OUTPUTDIR+"/tmp/"+fn[:-4]+".a"
        if (fn.endswith(".dat")): return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".in")):  return OUTPUTDIR+"/pandac/input/"+fn
    else:
        if (fn.endswith(".obj")): return OUTPUTDIR+"/tmp/"+fn[:-4]+".o"
        if (fn.endswith(".dll")): return OUTPUTDIR+"/lib/"+fn[:-4]+".so"
        if (fn.endswith(".exe")): return OUTPUTDIR+"/bin/"+fn[:-4]
        if (fn.endswith(".lib")): return OUTPUTDIR+"/lib/"+fn[:-4]+".a"
        if (fn.endswith(".ilb")): return OUTPUTDIR+"/tmp/"+fn[:-4]+".a"
        if (fn.endswith(".dat")): return OUTPUTDIR+"/tmp/"+fn
        if (fn.endswith(".in")):  return OUTPUTDIR+"/pandac/input/"+fn
    return fn


def FindLocation(fn, ipath):
    loc = CalcLocation(fn, ipath)
    (base,ext) = os.path.splitext(fn)
    ORIG_EXT[loc] = ext
    return loc

########################################################################
##
## TargetAdd
##
## Makepanda maintains a list of make-targets.  Each target has
## these attributes:
##
## name   - the name of the file being created.
## ext    - the original file extension, prior to OS-specific translation
## inputs - the names of the input files to the compiler
## deps   - other input files that the target also depends on
## opts   - compiler options, a catch-all category
##
## TargetAdd will create the target if it does not exist.  Then,
## depending on what options you pass, it will push data onto these
## various target attributes.  This is cumulative: for example, if
## you use TargetAdd to add compiler options, then use TargetAdd
## again with more compiler options, both sets of options will be
## included.
##
## TargetAdd does some automatic dependency generation on C++ files.
## It will scan these files for include-files and automatically push
## the include files onto the list of dependencies.  In order to do
## this, it needs an include-file search path.  So if you supply
## any C++ input, you also need to supply compiler options containing
## include-directories, or alternately, a separate ipath parameter.
## 
## The main body of 'makepanda' is a long list of TargetAdd
## directives building up a giant list of make targets.  Then, 
## finally, the targets are run and panda is built.
##
## Makepanda's dependency system does not understand multiple
## outputs from a single build step.  When a build step generates
## a primary output file and a secondary output file, it is
## necessary to trick the dependency system.  Insert a dummy
## build step that "generates" the secondary output file, using
## the primary output file as an input.  There is a special
## compiler option DEPENDENCYONLY that creates such a dummy
## build-step.  There are two cases where dummy build steps must
## be inserted: bison generates an OBJ and a secondary header
## file, interrogate generates an IN and a secondary IGATE.OBJ.
##
########################################################################

class Target:
    pass

TARGET_LIST=[]
TARGET_TABLE={}

def TargetAdd(target, dummy=0, opts=0, input=0, dep=0, ipath=0):
    if (dummy != 0):
        exit("Syntax error in TargetAdd "+target)
    if (ipath == 0): ipath = opts
    if (ipath == 0): ipath = []
    if (type(input) == str): input = [input]
    if (type(dep) == str): dep = [dep]
    full = FindLocation(target,[OUTPUTDIR+"/include"])
    if (TARGET_TABLE.has_key(full) == 0):
        t = Target()
        t.name = full
        t.inputs = []
        t.deps = {}
        t.opts = []
        TARGET_TABLE[full] = t
        TARGET_LIST.append(t)
    else:
        t = TARGET_TABLE[full]
    ipath = [OUTPUTDIR+"/tmp"] + GetListOption(ipath, "DIR:") + [OUTPUTDIR+"/include"]
    if (opts != 0):
        for x in opts:
            if (t.opts.count(x)==0):
                t.opts.append(x)
    if (input != 0):
        for x in input:
            fullinput = FindLocation(x, ipath)
            t.inputs.append(fullinput)
            t.deps[fullinput] = 1
            (base,suffix) = os.path.splitext(x)
            if (SUFFIX_INC.count(suffix)):
                for d in CxxCalcDependencies(fullinput, ipath, []):
                    t.deps[d] = 1
    if (dep != 0):                
        for x in dep:
            fulldep = FindLocation(x, ipath)
            t.deps[fulldep] = 1
    if (target.endswith(".in")):
        t.deps[FindLocation("interrogate.exe",[])] = 1
        t.deps[FindLocation("dtool_have_python.dat",[])] = 1

