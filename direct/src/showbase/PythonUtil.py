"""Undocumented Module"""

__all__ = ['enumerate', 'unique', 'indent', 'nonRepeatingRandomList',
'writeFsmTree', 'StackTrace', 'traceFunctionCall', 'traceParentCall',
'printThisCall', 'tron', 'trace', 'troff', 'getClassLineage', 'pdir',
'_pdir', '_is_variadic', '_has_keywordargs', '_varnames', '_getcode',
'Signature', 'doc', 'adjust', 'difference', 'intersection', 'union',
'sameElements', 'makeList', 'makeTuple', 'list2dict', 'invertDict',
'invertDictLossless', 'uniqueElements', 'disjoint', 'contains',
'replace', 'reduceAngle', 'fitSrcAngle2Dest', 'fitDestAngle2Src',
'closestDestAngle2', 'closestDestAngle', 'binaryRepr', 'profile',
'profiled', 'startProfile', 'printProfile', 'getSetterName',
'getSetter', 'Functor', 'Stack', 'Queue', 'ParamObj', 
'POD', 'bound', 'lerp', 'average', 'addListsByValue',
'boolEqual', 'lineupPos', 'formatElapsedSeconds', 'solveQuadratic',
'stackEntryInfo', 'lineInfo', 'callerInfo', 'lineTag',
'findPythonModule', 'describeException', 'mostDerivedLast',
'clampScalar', 'weightedChoice', 'randFloat', 'normalDistrib',
'weightedRand', 'randUint31', 'randInt32', 'randUint32',
'SerialNumGen', 'serialNum', 'uniqueName', 'Enum', 'Singleton',
'SingletonError', 'printListEnum', 'gcDebugOn', 'safeRepr',
'fastRepr', 'tagRepr', 'tagWithCaller', 'isDefaultValue', 'setTrace',
'_equal', '_notEqual', '_isNone', '_notNone', '_contains', '_notIn',
'ScratchPad', 'Sync', 'RefCounter', 'itype', 'getNumberedTypedString',
'printNumberedTyped', 'DelayedCall', 'DelayedFunctor',
'FrameDelayedCall', 'ArgumentEater', 'ClassTree', 'getBase',
'superFlattenShip','HotkeyBreaker','logMethodCalls','GoldenRatio',
'GoldenRectangle', 'pivotScalar', 'rad90', 'rad180', 'rad270', 'rad360']

import types
import string
import re
import math
import operator
import inspect
import os
import sys
import random
import time
import new
#if __debug__:
import traceback



from direct.directutil import Verify

ScalarTypes = (types.FloatType, types.IntType, types.LongType)

def enumerate(L):
    """Returns (0, L[0]), (1, L[1]), etc., allowing this syntax:
    for i, item in enumerate(L):
       ...

    enumerate is a built-in feature in Python 2.3, which implements it
    using an iterator. For now, we can use this quick & dirty
    implementation that returns a list of tuples that is completely
    constructed every time enumerate() is called.
    """
    return zip(xrange(len(L)), L)

import __builtin__
if not hasattr(__builtin__, 'enumerate'):
    __builtin__.enumerate = enumerate

def unique(L1, L2):
    """Return a list containing all items in 'L1' that are not in 'L2'"""
    L2 = dict([(k, None) for k in L2])
    return [item for item in L1 if item not in L2]

def indent(stream, numIndents, str):
    """
    Write str to stream with numIndents in front of it
    """
    # To match emacs, instead of a tab character we will use 4 spaces
    stream.write('    ' * numIndents + str)


def nonRepeatingRandomList(vals, max):
    random.seed(time.time())
    #first generate a set of random values
    valueList=range(max)
    finalVals=[]
    for i in range(vals):
        index=int(random.random()*len(valueList))
        finalVals.append(valueList[index])
        valueList.remove(valueList[index])
    return finalVals



def writeFsmTree(instance, indent = 0):
    if hasattr(instance, 'parentFSM'):
        writeFsmTree(instance.parentFSM, indent-2)
    elif hasattr(instance, 'fsm'):
        name = ''
        if hasattr(instance.fsm, 'state'):
            name = instance.fsm.state.name
        print "%s: %s"%(instance.fsm.name, name)




#if __debug__: #RAU accdg to Darren its's ok that StackTrace is not protected by __debug__
# DCR: if somebody ends up using StackTrace in production, either
# A) it will be OK because it hardly ever gets called, or
# B) it will be easy to track it down (grep for StackTrace)
class StackTrace:
    def __init__(self, label="", start=0, limit=None):
        """
        label is a string (or anything that be be a string)
        that is printed as part of the trace back.
        This is just to make it easier to tell what the
        stack trace is referring to.
        start is an integer number of stack frames back
        from the most recent.  (This is automatically
        bumped up by one to skip the __init__ call
        to the StackTrace).
        limit is an integer number of stack frames
        to record (or None for unlimited).
        """
        self.label = label
        if limit is not None:
            self.trace = traceback.extract_stack(sys._getframe(1+start),
                                                 limit=limit)
        else:
            self.trace = traceback.extract_stack(sys._getframe(1+start))
            
    def compact(self):
        r = ''
        comma = ','
        for filename, lineNum, funcName, text in self.trace:
            r += '%s.%s:%s%s' % (filename[:filename.rfind('.py')][filename.rfind('\\')+1:], funcName, lineNum, comma)
        if len(r):
            r = r[:-len(comma)]
        return r

    def __str__(self):
        r = "Debug stack trace of %s (back %s frames):\n"%(
            self.label, len(self.trace),)
        for i in traceback.format_list(self.trace):
            r+=i
        r+="***** NOTE: This is not a crash. This is a debug stack trace. *****"
        return r

def printStack():
    print StackTrace(start=1).compact()
    return True
def printVerboseStack():
    print StackTrace(start=1)
    return True

#-----------------------------------------------------------------------------

def traceFunctionCall(frame):
    """
    return a string that shows the call frame with calling arguments.
    e.g.
    foo(x=234, y=135)
    """
    f = frame
    co = f.f_code
    dict = f.f_locals
    n = co.co_argcount
    if co.co_flags & 4: n = n+1
    if co.co_flags & 8: n = n+1
    r=''
    if dict.has_key('self'):
        r = '%s.'%(dict['self'].__class__.__name__,)
    r+="%s("%(f.f_code.co_name,)
    comma=0 # formatting, whether we should type a comma.
    for i in range(n):
        name = co.co_varnames[i]
        if name=='self':
            continue
        if comma:
            r+=', '
        else:
            # ok, we skipped the first one, the rest get commas:
            comma=1
        r+=name
        r+='='
        if dict.has_key(name):
            v=str(dict[name])
            if len(v)>2000:
                # r+="<too big for debug>"
                r += (str(dict[name])[:2000] + "...")
            else:
                r+=str(dict[name])
        else: r+="*** undefined ***"
    return r+')'

def traceParentCall():
    return traceFunctionCall(sys._getframe(2))

def printThisCall():
    print traceFunctionCall(sys._getframe(1))
    return 1 # to allow "assert printThisCall()"


if __debug__:
    def lineage(obj, verbose=0, indent=0):
        """
        return instance or class name in as a multiline string.

        Usage: print lineage(foo)

        (Based on getClassLineage())
        """
        r=""
        if type(obj) == types.ListType:
            r+=(" "*indent)+"python list\n"
        elif type(obj) == types.DictionaryType:
            r+=(" "*indent)+"python dictionary\n"
        elif type(obj) == types.ModuleType:
            r+=(" "*indent)+str(obj)+"\n"
        elif type(obj) == types.InstanceType:
            r+=lineage(obj.__class__, verbose, indent)
        elif type(obj) == types.ClassType:
            r+=(" "*indent)
            if verbose:
                r+=obj.__module__+"."
            r+=obj.__name__+"\n"
            for c in obj.__bases__:
                r+=lineage(c, verbose, indent+2)
        return r

def tron():
    sys.settrace(trace)
def trace(frame, event, arg):
    if event == 'line':
        pass
    elif event == 'call':
        print traceFunctionCall(sys._getframe(1))
    elif event == 'return':
        print "returning"
    elif event == 'exception':
        print "exception"
    return trace
def troff():
    sys.settrace(None)

#-----------------------------------------------------------------------------

def getClassLineage(obj):
    """
    print object inheritance list
    """
    if type(obj) == types.DictionaryType:
        # Just a dictionary, return dictionary
        return [obj]
    elif (type(obj) == types.InstanceType):
        # Instance, make a list with the instance and its class interitance
        return [obj] + getClassLineage(obj.__class__)
    elif ((type(obj) == types.ClassType) or
          (type(obj) == types.TypeType)):
        # Class or type, see what it derives from
        lineage = [obj]
        for c in obj.__bases__:
            lineage = lineage + getClassLineage(c)
        return lineage
    # New FFI objects are types that are not defined.
    # but they still act like classes
    elif hasattr(obj, '__class__'):
        # Instance, make a list with the instance and its class interitance
        return [obj] + getClassLineage(obj.__class__)
    else:
        # Not what I'm looking for
        return []

def pdir(obj, str = None, width = None,
            fTruncate = 1, lineWidth = 75, wantPrivate = 0):
    # Remove redundant class entries
    uniqueLineage = []
    for l in getClassLineage(obj):
        if type(l) == types.ClassType:
            if l in uniqueLineage:
                break
        uniqueLineage.append(l)
    # Pretty print out directory info
    uniqueLineage.reverse()
    for obj in uniqueLineage:
        _pdir(obj, str, width, fTruncate, lineWidth, wantPrivate)
        print

def _pdir(obj, str = None, width = None,
            fTruncate = 1, lineWidth = 75, wantPrivate = 0):
    """
    Print out a formatted list of members and methods of an instance or class
    """
    def printHeader(name):
        name = ' ' + name + ' '
        length = len(name)
        if length < 70:
            padBefore = int((70 - length)/2.0)
            padAfter = max(0, 70 - length - padBefore)
            header = '*' * padBefore + name + '*' * padAfter
        print header
        print
    def printInstanceHeader(i, printHeader = printHeader):
        printHeader(i.__class__.__name__ + ' INSTANCE INFO')
    def printClassHeader(c, printHeader = printHeader):
        printHeader(c.__name__ + ' CLASS INFO')
    def printDictionaryHeader(d, printHeader = printHeader):
        printHeader('DICTIONARY INFO')
    # Print Header
    if type(obj) == types.InstanceType:
        printInstanceHeader(obj)
    elif type(obj) == types.ClassType:
        printClassHeader(obj)
    elif type (obj) == types.DictionaryType:
        printDictionaryHeader(obj)
    # Get dict
    if type(obj) == types.DictionaryType:
        dict = obj
    # FFI objects are builtin types, they have no __dict__
    elif not hasattr(obj, '__dict__'):
        dict = {}
    else:
        dict = obj.__dict__
    # Adjust width
    if width:
        maxWidth = width
    else:
        maxWidth = 10
    keyWidth = 0
    aproposKeys = []
    privateKeys = []
    remainingKeys = []
    for key in dict.keys():
        if not width:
            keyWidth = len(key)
        if str:
            if re.search(str, key, re.I):
                aproposKeys.append(key)
                if (not width) and (keyWidth > maxWidth):
                    maxWidth = keyWidth
        else:
            if key[:1] == '_':
                if wantPrivate:
                    privateKeys.append(key)
                    if (not width) and (keyWidth > maxWidth):
                        maxWidth = keyWidth
            else:
                remainingKeys.append(key)
                if (not width) and (keyWidth > maxWidth):
                    maxWidth = keyWidth
    # Sort appropriate keys
    if str:
        aproposKeys.sort()
    else:
        privateKeys.sort()
        remainingKeys.sort()
    # Print out results
    if wantPrivate:
        keys = aproposKeys + privateKeys + remainingKeys
    else:
        keys = aproposKeys + remainingKeys
    format = '%-' + `maxWidth` + 's'
    for key in keys:
        value = dict[key]
        if callable(value):
            strvalue = `Signature(value)`
        else:
            strvalue = `value`
        if fTruncate:
            # Cut off line (keeping at least 1 char)
            strvalue = strvalue[:max(1, lineWidth - maxWidth)]
        print (format % key)[:maxWidth] + '\t' + strvalue

# Magic numbers: These are the bit masks in func_code.co_flags that
# reveal whether or not the function has a *arg or **kw argument.
_POS_LIST = 4
_KEY_DICT = 8

def _is_variadic(function):
    return function.func_code.co_flags & _POS_LIST

def _has_keywordargs(function):
    return function.func_code.co_flags & _KEY_DICT

def _varnames(function):
    return function.func_code.co_varnames

def _getcode(f):
    """
    _getcode(f)
    This function returns the name and function object of a callable
    object.
    """
    def method_get(f):
        return f.__name__, f.im_func
    def function_get(f):
        return f.__name__, f
    def instance_get(f):
        if hasattr(f, '__call__'):
            method = f.__call__
            if (type(method) == types.MethodType):
                func = method.im_func
            else:
                func = method
            return ("%s%s" % (f.__class__.__name__, '__call__'), func)
        else:
            s = ("Instance %s of class %s does not have a __call__ method" %
                 (f, f.__class__.__name__))
            raise TypeError, s
    def class_get(f):
        if hasattr(f, '__init__'):
            return f.__name__, f.__init__.im_func
        else:
            return f.__name__, lambda: None
    codedict = { types.UnboundMethodType: method_get,
                 types.MethodType:        method_get,
                 types.FunctionType:      function_get,
                 types.InstanceType:      instance_get,
                 types.ClassType:         class_get,
                 }
    try:
        return codedict[type(f)](f)
    except KeyError:
        if callable(f): # eg, built-in functions and methods
            # raise ValueError, "type %s not supported yet." % type(f)
            return f.__name__, None
        else:
            raise TypeError, ("object %s of type %s is not callable." %
                                     (f, type(f)))

class Signature:
    def __init__(self, func):
        self.type = type(func)
        self.name, self.func = _getcode(func)
    def ordinary_args(self):
        n = self.func.func_code.co_argcount
        return _varnames(self.func)[0:n]
    def special_args(self):
        n = self.func.func_code.co_argcount
        x = {}
        #
        if _is_variadic(self.func):
            x['positional'] = _varnames(self.func)[n]
            if _has_keywordargs(self.func):
                x['keyword'] = _varnames(self.func)[n+1]
        elif _has_keywordargs(self.func):
            x['keyword'] = _varnames(self.func)[n]
        else:
            pass
        return x
    def full_arglist(self):
        base = list(self.ordinary_args())
        x = self.special_args()
        if x.has_key('positional'):
            base.append(x['positional'])
        if x.has_key('keyword'):
            base.append(x['keyword'])
        return base
    def defaults(self):
        defargs = self.func.func_defaults
        args = self.ordinary_args()
        mapping = {}
        if defargs is not None:
            for i in range(-1, -(len(defargs)+1), -1):
                mapping[args[i]] = defargs[i]
        else:
            pass
        return mapping
    def __repr__(self):
        if self.func:
            defaults = self.defaults()
            specials = self.special_args()
            l = []
            for arg in self.ordinary_args():
                if defaults.has_key(arg):
                    l.append(arg + '=' + str(defaults[arg]))
                else:
                    l.append(arg)
            if specials.has_key('positional'):
                l.append('*' + specials['positional'])
            if specials.has_key('keyword'):
                l.append('**' + specials['keyword'])
            return "%s(%s)" % (self.name, string.join(l, ', '))
        else:
            return "%s(?)" % self.name


def doc(obj):
    if (isinstance(obj, types.MethodType)) or \
       (isinstance(obj, types.FunctionType)):
        print obj.__doc__

def adjust(command = None, dim = 1, parent = None, **kw):
    """
    adjust(command = None, parent = None, **kw)
    Popup and entry scale to adjust a parameter

    Accepts any Slider keyword argument.  Typical arguments include:
    command: The one argument command to execute
    min: The min value of the slider
    max: The max value of the slider
    resolution: The resolution of the slider
    text: The label on the slider

    These values can be accessed and/or changed after the fact
    >>> vg = adjust()
    >>> vg['min']
    0.0
    >>> vg['min'] = 10.0
    >>> vg['min']
    10.0
    """
    # Make sure we enable Tk
    from direct.tkwidgets import Valuator
    # Set command if specified
    if command:
        kw['command'] = lambda x: apply(command, x)
        if parent is None:
            kw['title'] = command.__name__
    kw['dim'] = dim
    # Create toplevel if needed
    if not parent:
        vg = apply(Valuator.ValuatorGroupPanel, (parent,), kw)
    else:
        vg = apply(Valuator.ValuatorGroup, (parent,), kw)
        vg.pack(expand = 1, fill = 'x')
    return vg

def difference(a, b):
    """
    difference(list, list):
    """
    if not a: return b
    if not b: return a
    d = []
    for i in a:
        if (i not in b) and (i not in d):
            d.append(i)
    for i in b:
        if (i not in a) and (i not in d):
            d.append(i)
    return d

def intersection(a, b):
    """
    intersection(list, list):
    """
    if not a: return []
    if not b: return []
    d = []
    for i in a:
        if (i in b) and (i not in d):
            d.append(i)
    for i in b:
        if (i in a) and (i not in d):
            d.append(i)
    return d

def union(a, b):
    """
    union(list, list):
    """
    # Copy a
    c = a[:]
    for i in b:
        if (i not in c):
            c.append(i)
    return c

def sameElements(a, b):
    if len(a) != len(b):
        return 0
    for elem in a:
        if elem not in b:
            return 0
    for elem in b:
        if elem not in a:
            return 0
    return 1

def makeList(x):
    """returns x, converted to a list"""
    if type(x) is types.ListType:
        return x
    elif type(x) is types.TupleType:
        return list(x)
    else:
        return [x,]

def makeTuple(x):
    """returns x, converted to a tuple"""
    if type(x) is types.ListType:
        return tuple(x)
    elif type(x) is types.TupleType:
        return x
    else:
        return (x,)

def list2dict(L, value=None):
    """creates dict using elements of list, all assigned to same value"""
    return dict([(k, value) for k in L])

def invertDict(D, lossy=False):
    """creates a dictionary by 'inverting' D; keys are placed in the new
    dictionary under their corresponding value in the old dictionary.
    It is an error if D contains any duplicate values.

    >>> old = {'key1':1, 'key2':2}
    >>> invertDict(old)
    {1: 'key1', 2: 'key2'}
    """
    n = {}
    for key, value in D.items():
        if not lossy and value in n:
            raise 'duplicate key in invertDict: %s' % value
        n[value] = key
    return n

def invertDictLossless(D):
    """similar to invertDict, but values of new dict are lists of keys from
    old dict. No information is lost.

    >>> old = {'key1':1, 'key2':2, 'keyA':2}
    >>> invertDictLossless(old)
    {1: ['key1'], 2: ['key2', 'keyA']}
    """
    n = {}
    for key, value in D.items():
        n.setdefault(value, [])
        n[value].append(key)
    return n

def uniqueElements(L):
    """are all elements of list unique?"""
    return len(L) == len(list2dict(L))

def disjoint(L1, L2):
    """returns non-zero if L1 and L2 have no common elements"""
    used = dict([(k, None) for k in L1])
    for k in L2:
        if k in used:
            return 0
    return 1

def contains(whole, sub):
    """
    Return 1 if whole contains sub, 0 otherwise
    """
    if (whole == sub):
        return 1
    for elem in sub:
        # The first item you find not in whole, return 0
        if elem not in whole:
            return 0
    # If you got here, whole must contain sub
    return 1

def replace(list, old, new, all=0):
    """
    replace 'old' with 'new' in 'list'
    if all == 0, replace first occurrence
    otherwise replace all occurrences
    returns the number of items replaced
    """
    if old not in list:
        return 0

    if not all:
        i = list.index(old)
        list[i] = new
        return 1
    else:
        numReplaced = 0
        for i in xrange(len(list)):
            if list[i] == old:
                numReplaced += 1
                list[i] = new
        return numReplaced

rad90 = math.pi / 2.
rad180 = math.pi
rad270 = 1.5 * math.pi
rad360 = 2. * math.pi

def reduceAngle(deg):
    """
    Reduces an angle (in degrees) to a value in [-180..180)
    """
    return (((deg + 180.) % 360.) - 180.)

def fitSrcAngle2Dest(src, dest):
    """
    given a src and destination angle, returns an equivalent src angle
    that is within [-180..180) of dest
    examples:
    fitSrcAngle2Dest(30, 60) == 30
    fitSrcAngle2Dest(60, 30) == 60
    fitSrcAngle2Dest(0, 180) == 0
    fitSrcAngle2Dest(-1, 180) == 359
    fitSrcAngle2Dest(-180, 180) == 180
    """
    return dest + reduceAngle(src - dest)

def fitDestAngle2Src(src, dest):
    """
    given a src and destination angle, returns an equivalent dest angle
    that is within [-180..180) of src
    examples:
    fitDestAngle2Src(30, 60) == 60
    fitDestAngle2Src(60, 30) == 30
    fitDestAngle2Src(0, 180) == -180
    fitDestAngle2Src(1, 180) == 180
    """
    return src + (reduceAngle(dest - src))

def closestDestAngle2(src, dest):
    # The function above didn't seem to do what I wanted. So I hacked
    # this one together. I can't really say I understand it. It's more
    # from impirical observation... GRW
    diff = src - dest
    if diff > 180:
        # if the difference is greater that 180 it's shorter to go the other way
        return dest - 360
    elif diff < -180:
        # or perhaps the OTHER other way...
        return dest + 360
    else:
        # otherwise just go to the original destination
        return dest

def closestDestAngle(src, dest):
    # The function above didn't seem to do what I wanted. So I hacked
    # this one together. I can't really say I understand it. It's more
    # from impirical observation... GRW
    diff = src - dest
    if diff > 180:
        # if the difference is greater that 180 it's shorter to go the other way
        return src - (diff - 360)
    elif diff < -180:
        # or perhaps the OTHER other way...
        return src - (360 + diff)
    else:
        # otherwise just go to the original destination
        return dest


def binaryRepr(number, max_length = 32):
    # This will only work reliably for relatively small numbers.
    # Increase the value of max_length if you think you're going
    # to use long integers
    assert number < 2L << max_length
    shifts = map (operator.rshift, max_length * [number], \
                  range (max_length - 1, -1, -1))
    digits = map (operator.mod, shifts, max_length * [2])
    if not digits.count (1): return 0
    digits = digits [digits.index (1):]
    return string.join (map (repr, digits), '')

# constant profile defaults
PyUtilProfileDefaultFilename = 'profiledata'
PyUtilProfileDefaultLines = 80
PyUtilProfileDefaultSorts = ['cumulative', 'time', 'calls']

def profile(callback, name, terse):
    import __builtin__
    if 'globalProfileFunc' in __builtin__.__dict__:
        # rats. Python profiler is not re-entrant...
        base.notify.warning(
            'PythonUtil.profileStart(%s): aborted, already profiling %s'
            #'\nStack Trace:\n%s'
            % (name, __builtin__.globalProfileFunc,
            #StackTrace()
            ))
        return
    __builtin__.globalProfileFunc = callback
    print '***** START PROFILE: %s *****' % name
    startProfile(cmd='globalProfileFunc()', callInfo=(not terse))
    print '***** END PROFILE: %s *****' % name
    del __builtin__.__dict__['globalProfileFunc']

def profiled(category=None, terse=False):
    """ decorator for profiling functions
    turn categories on and off via "want-profile-categoryName 1"
    
    e.g.

    @profiled('particles')
    def loadParticles():
        ...

    want-profile-particles 1
    """
    assert type(category) in (types.StringType, types.NoneType), "must provide a category name for @profiled"

    try:
        null = not __dev__
    except:
        null = not __debug__
    if null:
        # if we're not in __dev__, just return the function itself. This
        # results in zero runtime overhead, since decorators are evaluated
        # at module-load.
        def nullDecorator(f):
            return f
        return nullDecorator

    def profileDecorator(f):
        def _profiled(*args, **kArgs):
            #import pdb;pdb.set_trace()
            # must do this in here because we don't have base/simbase
            # at the time that PythonUtil is loaded
            name = '(%s) %s from %s' % (category, f.func_name, f.__module__)
            try:
                _base = base
            except:
                _base = simbase
            if (category is None) or _base.config.GetBool('want-profile-%s' % category, 0):
                return profile(Functor(f, *args, **kArgs), name, terse)
            else:
                return f(*args, **kArgs)
        #import pdb;pdb.set_trace()
        _profiled.__doc__ = f.__doc__
        return _profiled
    return profileDecorator

# call this from the prompt, and break back out to the prompt
# to stop profiling
#
# OR to do inline profiling, you must make a globally-visible
# function to be profiled, i.e. to profile 'self.load()', do
# something like this:
#
#        def func(self=self):
#            self.load()
#        import __builtin__
#        __builtin__.func = func
#        PythonUtil.startProfile(cmd='func()', filename='profileData')
#        del __builtin__.func
#
def startProfile(filename=PyUtilProfileDefaultFilename,
                 lines=PyUtilProfileDefaultLines,
                 sorts=PyUtilProfileDefaultSorts,
                 silent=0,
                 callInfo=1,
                 cmd='run()'):
    # uniquify the filename to allow multiple processes to profile simultaneously
    filename = '%s.%s' % (filename, randUint31())
    import profile
    profile.run(cmd, filename)
    if not silent:
        printProfile(filename, lines, sorts, callInfo)
    import os
    os.remove(filename)

# call this to see the results again
def printProfile(filename=PyUtilProfileDefaultFilename,
                 lines=PyUtilProfileDefaultLines,
                 sorts=PyUtilProfileDefaultSorts,
                 callInfo=1):
    import pstats
    s = pstats.Stats(filename)
    s.strip_dirs()
    for sort in sorts:
        s.sort_stats(sort)
        s.print_stats(lines)
        if callInfo:
            s.print_callees(lines)
            s.print_callers(lines)

def getSetterName(valueName, prefix='set'):
    # getSetterName('color') -> 'setColor'
    # getSetterName('color', 'get') -> 'getColor'
    return '%s%s%s' % (prefix, string.upper(valueName[0]), valueName[1:])
def getSetter(targetObj, valueName, prefix='set'):
    # getSetter(smiley, 'pos') -> smiley.setPos
    return getattr(targetObj, getSetterName(valueName, prefix))

class Functor:
    def __init__(self, function, *args, **kargs):
        assert callable(function), "function should be a callable obj"
        self._function = function
        self._args = args
        self._kargs = kargs
        self.__name__ = 'Functor: %s' % self._function.__name__
        self.__doc__ = self._function.__doc__

    def destroy(self):
        del self._function
        del self._args
        del self._kargs
        del self.__name__
        del self.__doc__
    
    def __call__(self, *args, **kargs):
        """call function"""
        _args = list(self._args)
        _args.extend(args)
        _kargs = self._kargs.copy()
        _kargs.update(kargs)
        return apply(self._function, _args, _kargs)

    def __repr__(self):
        s = 'Functor(%s' % self._function.__name__
        for arg in self._args:
            try:
                argStr = repr(arg)
            except:
                argStr = 'bad repr: %s' % arg.__class__
            s += ', %s' % argStr
        for karg, value in self._kargs.items():
            s += ', %s=%s' % (karg, repr(value))
        s += ')'
        return s

class Stack:
    def __init__(self):
        self.__list = []
    def push(self, item):
        self.__list.append(item)
    def top(self):
        # return the item on the top of the stack without popping it off
        return self.__list[-1]
    def pop(self):
        return self.__list.pop()
    def clear(self):
        self.__list = []
    def isEmpty(self):
        return len(self.__list) == 0
    def __len__(self):
        return len(self.__list)

class Queue:
    # FIFO queue
    # interface is intentionally identical to Stack (LIFO)
    def __init__(self):
        self.__list = []
    def push(self, item):
        self.__list.append(item)
    def top(self):
        # return the next item at the front of the queue without popping it off
        return self.__list[0]
    def front(self):
        return self.__list[0]
    def back(self):
        return self.__list[-1]
    def pop(self):
        return self.__list.pop(0)
    def clear(self):
        self.__list = []
    def isEmpty(self):
        return len(self.__list) == 0
    def __len__(self):
        return len(self.__list)

if __debug__:
    q = Queue()
    assert q.isEmpty()
    q.clear()
    assert q.isEmpty()
    q.push(10)
    assert not q.isEmpty()
    q.push(20)
    assert not q.isEmpty()
    assert len(q) == 2
    assert q.front() == 10
    assert q.back() == 20
    assert q.top() == 10
    assert q.top() == 10
    assert q.pop() == 10
    assert len(q) == 1
    assert not q.isEmpty()
    assert q.pop() == 20
    assert len(q) == 0
    assert q.isEmpty()

def mostDerivedLast(classList):
    """pass in list of classes. sorts list in-place, with derived classes
    appearing after their bases"""
    def compare(a, b):
        if issubclass(a, b):
            result=1
        elif issubclass(b, a):
            result=-1
        else:
            result=0
        #print a, b, result
        return result
    classList.sort(compare)

"""
ParamObj/ParamSet
=================

These two classes support you in the definition of a formal set of
parameters for an object type. The parameters may be safely queried/set on
an object instance at any time, and the object will react to newly-set
values immediately.

ParamSet & ParamObj also provide a mechanism for atomically setting
multiple parameter values before allowing the object to react to any of the
new values--useful when two or more parameters are interdependent and there
is risk of setting an illegal combination in the process of applying a new
set of values.

To make use of these classes, derive your object from ParamObj. Then define
a 'ParamSet' subclass that derives from the parent class' 'ParamSet' class,
and define the object's parameters within its ParamSet class. (see examples
below)

The ParamObj base class provides 'get' and 'set' functions for each
parameter if they are not defined. These default implementations
respectively set the parameter value directly on the object, and expect the
value to be available in that location for retrieval.

Classes that derive from ParamObj can optionally declare a 'get' and 'set'
function for each parameter. The setter should simply store the value in a
location where the getter can find it; it should not do any further
processing based on the new parameter value. Further processing should be
implemented in an 'apply' function. The applier function is optional, and
there is no default implementation.

NOTE: the previous value of a parameter is available inside an apply
function as 'self.getPriorValue()'

The ParamSet class declaration lists the parameters and defines a default
value for each. ParamSet instances represent a complete set of parameter
values. A ParamSet instance created with no constructor arguments will
contain the default values for each parameter. The defaults may be
overriden by passing keyword arguments to the ParamSet's constructor. If a
ParamObj instance is passed to the constructor, the ParamSet will extract
the object's current parameter values.

ParamSet.applyTo(obj) sets all of its parameter values on 'obj'.

SETTERS AND APPLIERS
====================
Under normal conditions, a call to a setter function, i.e.

 cam.setFov(90)

will actually result in the following calls being made:

 cam.setFov(90)
 cam.applyFov()

Calls to several setter functions, i.e.

 cam.setFov(90)
 cam.setViewType('cutscene')

will result in this call sequence:

 cam.setFov(90)
 cam.applyFov()
 cam.setViewType('cutscene')
 cam.applyViewType()

Suppose that you desire the view type to already be set to 'cutscene' at
the time when applyFov() is called. You could reverse the order of the set
calls, but suppose that you also want the fov to be set properly at the
time when applyViewType() is called.

In this case, you can 'lock' the params, i.e.

 cam.lockParams()
 cam.setFov(90)
 cam.setViewType('cutscene')
 cam.unlockParams()

This will result in the following call sequence:

 cam.setFov(90)
 cam.setViewType('cutscene')
 cam.applyFov()
 cam.applyViewType()

NOTE: Currently the order of the apply calls following an unlock is not
guaranteed.

EXAMPLE CLASSES
===============
Here is an example of a class that uses ParamSet/ParamObj to manage its
parameters:

class Camera(ParamObj):
    class ParamSet(ParamObj.ParamSet):
        Params = {
            'viewType': 'normal',
            'fov': 60,
            }
    ...

    def getViewType(self):
        return self.viewType
    def setViewType(self, viewType):
        self.viewType = viewType
    def applyViewType(self):
        if self.viewType == 'normal':
            ...

    def getFov(self):
        return self.fov
    def setFov(self, fov):
        self.fov = fov
    def applyFov(self):
        base.camera.setFov(self.fov)
    ...


EXAMPLE USAGE
=============

cam = Camera()
...

# set up for the cutscene
savedSettings = cam.ParamSet(cam)
cam.setViewType('closeup')
cam.setFov(90)
...

# cutscene is over, set the camera back
savedSettings.applyTo(cam)
del savedSettings

"""

class ParamObj:
    # abstract base for classes that want to support a formal parameter
    # set whose values may be queried, changed, 'bulk' changed, and
    # extracted/stored/applied all at once (see documentation above)

    # ParamSet subclass: container of parameter values. Derived class must
    # derive a new ParamSet class if they wish to define new params. See
    # documentation above.
    class ParamSet:
        Params = {
            # base class does not define any parameters, but they would
            # appear here as 'name': defaultValue,
            #
            # WARNING: default values of mutable types that do not copy by
            # value (dicts, lists etc.) will be shared by all class instances
            # if default value is callable, it will be called to get actual
            # default value
            #
            # for example:
            #
            # class MapArea(ParamObj):
            #     class ParamSet(ParamObj.ParamSet):
            #         Params = {
            #             'spawnIndices': Functor(list, [1,5,22]),
            #         }
            #
            }

        def __init__(self, *args, **kwArgs):
            self.__class__._compileDefaultParams()
            if len(args) == 1 and len(kwArgs) == 0:
                # extract our params from an existing ParamObj instance
                obj = args[0]
                self.paramVals = {}
                for param in self.getParams():
                    self.paramVals[param] = getSetter(obj, param, 'get')()
            else:
                assert len(args) == 0
                if __debug__:
                    for arg in kwArgs.keys():
                        assert arg in self.getParams()
                self.paramVals = dict(kwArgs)
        def getValue(self, param):
            if param in self.paramVals:
                return self.paramVals[param]
            return self._Params[param]
        def applyTo(self, obj):
            # Apply our entire set of params to a ParamObj
            obj.lockParams()
            for param in self.getParams():
                getSetter(obj, param)(self.getValue(param))
            obj.unlockParams()
        def extractFrom(self, obj):
            # Extract our entire set of params from a ParamObj
            obj.lockParams()
            for param in self.getParams():
                self.paramVals[param] = getSetter(obj, param, 'get')()
            obj.unlockParams()
        # CLASS METHODS
        def getParams(cls):
            # returns safely-mutable list of param names
            cls._compileDefaultParams()
            return cls._Params.keys()
        getParams = classmethod(getParams)
        def getDefaultValue(cls, param):
            cls._compileDefaultParams()
            dv = cls._Params[param]
            if callable(dv):
                dv = dv()
            return dv
        getDefaultValue = classmethod(getDefaultValue)
        def _compileDefaultParams(cls):
            if cls.__dict__.has_key('_Params'):
                # we've already compiled the defaults for this class
                return
            bases = list(cls.__bases__)
            # bring less-derived classes to the front
            mostDerivedLast(bases)
            cls._Params = {}
            for c in (bases + [cls]):
                # make sure this base has its dict of param defaults
                c._compileDefaultParams()
                if c.__dict__.has_key('Params'):
                    # apply this class' default param values to our dict
                    cls._Params.update(c.Params)
        _compileDefaultParams = classmethod(_compileDefaultParams)
        def __repr__(self):
            argStr = ''
            for param in self.getParams():
                argStr += '%s=%s,' % (param,
                                      repr(self.getValue(param)))
            return '%s.%s(%s)' % (
                self.__class__.__module__, self.__class__.__name__, argStr)
    # END PARAMSET SUBCLASS

    def __init__(self, *args, **kwArgs):
        assert issubclass(self.ParamSet, ParamObj.ParamSet)
        # If you pass in a ParamSet obj, its values will be applied to this
        # object in the constructor.
        params = None
        if len(args) == 1 and len(kwArgs) == 0:
            # if there's one argument, assume that it's a ParamSet
            params = args[0]
        elif len(kwArgs) > 0:
            assert len(args) == 0
            # if we've got keyword arguments, make a ParamSet out of them
            params = self.ParamSet(**kwArgs)

        self._paramLockRefCount = 0
        # this holds dictionaries of parameter values prior to the set that we
        # are performing
        self._priorValuesStack = Stack()
        # this holds the name of the parameter that we are currently modifying
        # at the top of the stack
        self._curParamStack = Stack()

        def setterStub(param, setterFunc, self,
                       value):
            # should we apply the value now or should we wait?
            # if this obj's params are locked, we track which values have
            # been set, and on unlock, we'll call the applyers for those
            # values
            if self._paramLockRefCount > 0:
                setterFunc(value)
                priorValues = self._priorValuesStack.top()
                if param not in priorValues:
                    try:
                        priorValue = getSetter(self, param, 'get')()
                    except:
                        priorValue = None
                    priorValues[param] = priorValue
                self._paramsSet[param] = None
            else:
                # prepare for call to getPriorValue
                self._priorValuesStack.push({
                    param: getSetter(self, param, 'get')()
                    })
                setterFunc(value)
                # call the applier, if there is one
                applier = getattr(self, getSetterName(param, 'apply'), None)
                if applier is not None:
                    self._curParamStack.push(param)
                    applier()
                    self._curParamStack.pop()
                self._priorValuesStack.pop()
                if hasattr(self, 'handleParamChange'):
                    self.handleParamChange((param,))

        # insert stub funcs for param setters
        for param in self.ParamSet.getParams():
            setterName = getSetterName(param)
            getterName = getSetterName(param, 'get')

            # is there a setter defined?
            if not hasattr(self, setterName):
                # no; provide the default
                def defaultSetter(self, value, param=param):
                    setattr(self, param, value)
                self.__class__.__dict__[setterName] = defaultSetter

            # is there a getter defined?
            if not hasattr(self, getterName):
                # no; provide the default. If there is no value set, return
                # the default
                def defaultGetter(self, param=param,
                                  default=self.ParamSet.getDefaultValue(param)):
                    return getattr(self, param, default)
                self.__class__.__dict__[getterName] = defaultGetter

            # grab a reference to the setter
            setterFunc = getattr(self, setterName)
            # if the setter is a direct member of this instance, move the setter
            # aside
            if setterName in self.__dict__:
                self.__dict__[setterName + '_MOVED'] = self.__dict__[setterName]
                setterFunc = self.__dict__[setterName]
            # install a setter stub that will a) call the real setter and
            # then the applier, or b) call the setter and queue the
            # applier, depending on whether our params are locked
            self.__dict__[setterName] = Functor(setterStub, param,
                                                setterFunc, self)

        if params is not None:
            params.applyTo(self)

    def destroy(self):
        for param in self.ParamSet.getParams():
            setterName = getSetterName(param)
            self.__dict__[setterName].destroy()
            del self.__dict__[setterName]
    
    def setDefaultParams(self):
        # set all the default parameters on ourself
        self.ParamSet().applyTo(self)

    def getCurrentParams(self):
        params = self.ParamSet()
        params.extractFrom(self)
        return params

    def lockParams(self):
        self._paramLockRefCount += 1
        if self._paramLockRefCount == 1:
            self._handleLockParams()
    def unlockParams(self):
        if self._paramLockRefCount > 0:
            self._paramLockRefCount -= 1
            if self._paramLockRefCount == 0:
                self._handleUnlockParams()
    def _handleLockParams(self):
        # this will store the names of the parameters that are modified
        self._paramsSet = {}
        # this will store the values of modified params (from prior to
        # the lock).
        self._priorValuesStack.push({})
    def _handleUnlockParams(self):
        for param in self._paramsSet:
            # call the applier, if there is one
            applier = getattr(self, getSetterName(param, 'apply'), None)
            if applier is not None:
                self._curParamStack.push(param)
                applier()
                self._curParamStack.pop()
        self._priorValuesStack.pop()
        if hasattr(self, 'handleParamChange'):
            self.handleParamChange(tuple(self._paramsSet.keys()))
        del self._paramsSet
    def paramsLocked(self):
        return self._paramLockRefCount > 0
    def getPriorValue(self):
        # call this within an apply function to find out what the prior value
        # of the param was
        return self._priorValuesStack.top()[self._curParamStack.top()]

    def __repr__(self):
        argStr = ''
        for param in self.ParamSet.getParams():
            argStr += '%s=%s,' % (param,
                                  repr(getSetter(self, param, 'get')()))
        return '%s(%s)' % (self.__class__.__name__, argStr)

"""
POD (Plain Ol' Data)

Like ParamObj/ParamSet, but without lock/unlock/getPriorValue and without
appliers. Similar to a C++ struct, but with auto-generated setters and
getters.

Use POD when you want the generated getters and setters of ParamObj, but
efficiency is a concern and you don't need the bells and whistles provided
by ParamObj.

POD.__init__ *MUST* be called. You should NOT define your own data getters
and setters. Data values may be read, set, and modified directly. You will
see no errors if you define your own getters/setters, but there is no
guarantee that they will be called--and they will certainly be bypassed by
POD internally.

EXAMPLE CLASSES
===============
Here is an example of a class heirarchy that uses POD to manage its data:

class Enemy(POD):
  DataSet = {
    'faction': 'navy',
    }

class Sailor(Enemy):
  DataSet = {
    'build': HUSKY,
    'weapon': Cutlass(scale=.9),
    }

EXAMPLE USAGE
=============
s = Sailor(faction='undead', build=SKINNY)

# make two copies of s
s2 = s.makeCopy()
s3 = Sailor(s)

# example sets
s2.setWeapon(Musket())
s3.build = TALL

# example gets
faction2 = s2.getFaction()
faction3 = s3.faction
"""
class POD:
    DataSet = {
        # base class does not define any data items, but they would
        # appear here as 'name': defaultValue,
        #
        # WARNING: default values of mutable types that do not copy by
        # value (dicts, lists etc.) will be shared by all class instances
        # if default value is callable, it will be called to get actual
        # default value
        #
        # for example:
        #
        # class MapData(POD):
        #     DataSet = {
        #         'spawnIndices': Functor(list, [1,5,22]),
        #         }
        }
    def __init__(self, **kwArgs):
        self.__class__._compileDefaultDataSet()
        if __debug__:
            for arg in kwArgs.keys():
                assert arg in self.getDataNames(), (
                    "unknown argument for %s: '%s'" % (
                    self.__class__, arg))
        for name in self.getDataNames():
            if name in kwArgs:
                getSetter(self, name)(kwArgs[name])
            else:
                getSetter(self, name)(self.getDefaultValue(name))

    def setDefaultValues(self):
        # set all the default data values on ourself
        for name in self.getDataNames():
            getSetter(self, name)(self.getDefaultValue(name))
    # this functionality used to be in the constructor, triggered by a single
    # positional argument; that was conflicting with POD subclasses that wanted
    # to define different behavior for themselves when given a positional
    # constructor argument
    def copyFrom(self, other, strict=False):
        # if 'strict' is true, other must have a value for all of our data items
        # otherwise we'll use the defaults
        for name in self.getDataNames():
            if hasattr(other, getSetterName(name, 'get')):
                setattr(self, name, getSetter(other, name, 'get')())
            else:
                if strict:
                    raise "object '%s' doesn't have value '%s'" % (other, name)
                else:
                    setattr(self, name, self.getDefaultValue(name))
        # support 'p = POD.POD().copyFrom(other)' syntax
        return self
    def makeCopy(self):
        # returns a duplicate of this object
        return self.__class__().copyFrom(self)
    def applyTo(self, obj):
        # Apply our entire set of data to another POD
        for name in self.getDataNames():
            getSetter(obj, name)(getSetter(self, name, 'get')())
    def getValue(self, name):
        return getSetter(self, name, 'get')()

    # CLASS METHODS
    def getDataNames(cls):
        # returns safely-mutable list of datum names
        cls._compileDefaultDataSet()
        return cls._DataSet.keys()
    getDataNames = classmethod(getDataNames)
    def getDefaultValue(cls, name):
        cls._compileDefaultDataSet()
        dv = cls._DataSet[name]
        if callable(dv):
            dv = dv()
        return dv
    getDefaultValue = classmethod(getDefaultValue)
    def _compileDefaultDataSet(cls):
        if cls.__dict__.has_key('_DataSet'):
            # we've already compiled the defaults for this class
            return
        # create setters & getters for this class
        if cls.__dict__.has_key('DataSet'):
            for name in cls.DataSet:
                setterName = getSetterName(name)
                if not hasattr(cls, setterName):
                    def defaultSetter(self, value, name=name):
                        setattr(self, name, value)
                    cls.__dict__[setterName] = defaultSetter
                getterName = getSetterName(name, 'get')
                if not hasattr(cls, getterName):
                    def defaultGetter(self, name=name,
                                      default=cls.DataSet[name]):
                        return getattr(self, name, default)
                    cls.__dict__[getterName] = defaultGetter
        # this dict will hold all of the aggregated default data values for
        # this particular class, including values from its base classes
        cls._DataSet = {}
        bases = list(cls.__bases__)
        # bring less-derived classes to the front
        mostDerivedLast(bases)
        for c in (bases + [cls]):
            # skip multiple-inheritance base classes that do not derive from POD
            if issubclass(c, POD):
                # make sure this base has its dict of data defaults
                c._compileDefaultDataSet()
                if c.__dict__.has_key('DataSet'):
                    # apply this class' default data values to our dict
                    cls._DataSet.update(c.DataSet)
    _compileDefaultDataSet = classmethod(_compileDefaultDataSet)
    # END CLASS METHODS

    def __repr__(self):
        argStr = ''
        for name in self.getDataNames():
            argStr += '%s=%s,' % (name, repr(getSetter(self, name, 'get')()))
        return '%s(%s)' % (self.__class__.__name__, argStr)

if __debug__:
    class PODtest(POD):
        DataSet = {
            'foo': dict,
            }
    p1 = PODtest()
    p2 = PODtest()
    p1.foo[1] = 2
    assert len(p2.foo) == 0

def bound(value, bound1, bound2):
    """
    returns value if value is between bound1 and bound2
    otherwise returns bound that is closer to value
    """
    if bound1 > bound2:
        return min(max(value, bound2), bound1)
    else:
        return min(max(value, bound1), bound2)

def lerp(v0, v1, t):
    """
    returns a value lerped between v0 and v1, according to t
    t == 0 maps to v0, t == 1 maps to v1
    """
    return v0 + ((v1 - v0) * t)

def average(*args):
    """ returns simple average of list of values """
    val = 0.
    for arg in args:
        val += arg
    return val / len(args)

def addListsByValue(a, b):
    """
    returns a new array containing the sums of the two array arguments
    (c[0] = a[0 + b[0], etc.)
    """
    c = []
    for x, y in zip(a, b):
        c.append(x + y)
    return c

def boolEqual(a, b):
    """
    returns true if a and b are both true or both false.
    returns false otherwise
    (a.k.a. xnor -- eXclusive Not OR).
    """
    return (a and b) or not (a or b)

def lineupPos(i, num, spacing):
    """
    use to line up a series of 'num' objects, in one dimension,
    centered around zero
    'i' is the index of the object in the lineup
    'spacing' is the amount of space between objects in the lineup
    """
    assert num >= 1
    assert i >= 0 and i < num
    pos = float(i) * spacing
    return pos - ((float(spacing) * (num-1))/2.)

def formatElapsedSeconds(seconds):
    """
    Returns a string of the form "mm:ss" or "hh:mm:ss" or "n days",
    representing the indicated elapsed time in seconds.
    """
    sign = ''
    if seconds < 0:
        seconds = -seconds
        sign = '-'

    # We use math.floor() instead of casting to an int, so we avoid
    # problems with numbers that are too large to represent as
    # type int.
    seconds = math.floor(seconds)
    hours = math.floor(seconds / (60 * 60))
    if hours > 36:
        days = math.floor((hours + 12) / 24)
        return "%s%d days" % (sign, days)

    seconds -= hours * (60 * 60)
    minutes = (int)(seconds / 60)
    seconds -= minutes * 60
    if hours != 0:
        return "%s%d:%02d:%02d" % (sign, hours, minutes, seconds)
    else:
        return "%s%d:%02d" % (sign, minutes, seconds)

def solveQuadratic(a, b, c):
    # quadratic equation: ax^2 + bx + c = 0
    # quadratic formula:  x = [-b +/- sqrt(b^2 - 4ac)] / 2a
    # returns None, root, or [root1, root2]

    # a cannot be zero.
    if a == 0.:
        return None

    # calculate the determinant (b^2 - 4ac)
    D = (b * b) - (4. * a * c)

    if D < 0:
        # there are no solutions (sqrt(negative number) is undefined)
        return None
    elif D == 0:
        # only one root
        return (-b) / (2. * a)
    else:
        # OK, there are two roots
        sqrtD = math.sqrt(D)
        twoA = 2. * a
        root1 = ((-b) - sqrtD) / twoA
        root2 = ((-b) + sqrtD) / twoA
        return [root1, root2]

def stackEntryInfo(depth=0, baseFileName=1):
    """
    returns the sourcefilename, line number, and function name of
    an entry in the stack.
    'depth' is how far back to go in the stack; 0 is the caller of this
    function, 1 is the function that called the caller of this function, etc.
    by default, strips off the path of the filename; override with baseFileName
    returns (fileName, lineNum, funcName) --> (string, int, string)
    returns (None, None, None) on error
    """
    try:
        stack = None
        frame = None
        try:
            stack = inspect.stack()
            # add one to skip the frame associated with this function
            frame = stack[depth+1]
            filename = frame[1]
            if baseFileName:
                filename = os.path.basename(filename)
            lineNum = frame[2]
            funcName = frame[3]
            result = (filename, lineNum, funcName)
        finally:
            del stack
            del frame
    except:
        result = (None, None, None)

    return result

def lineInfo(baseFileName=1):
    """
    returns the sourcefilename, line number, and function name of the
    code that called this function
    (answers the question: 'hey lineInfo, where am I in the codebase?')
    see stackEntryInfo, above, for info on 'baseFileName' and return types
    """
    return stackEntryInfo(1, baseFileName)

def callerInfo(baseFileName=1, howFarBack=0):
    """
    returns the sourcefilename, line number, and function name of the
    caller of the function that called this function
    (answers the question: 'hey callerInfo, who called me?')
    see stackEntryInfo, above, for info on 'baseFileName' and return types
    """
    return stackEntryInfo(2+howFarBack, baseFileName)

def lineTag(baseFileName=1, verbose=0, separator=':'):
    """
    returns a string containing the sourcefilename and line number
    of the code that called this function
    (equivalent to lineInfo, above, with different return type)
    see stackEntryInfo, above, for info on 'baseFileName'

    if 'verbose' is false, returns a compact string of the form
    'fileName:lineNum:funcName'
    if 'verbose' is true, returns a longer string that matches the
    format of Python stack trace dumps

    returns empty string on error
    """
    fileName, lineNum, funcName = callerInfo(baseFileName)
    if fileName is None:
        return ''
    if verbose:
        return 'File "%s", line %s, in %s' % (fileName, lineNum, funcName)
    else:
        return '%s%s%s%s%s' % (fileName, separator, lineNum, separator,
                               funcName)

def findPythonModule(module):
    # Look along the python load path for the indicated filename.
    # Returns the located pathname, or None if the filename is not
    # found.
    filename = module + '.py'
    for dir in sys.path:
        pathname = os.path.join(dir, filename)
        if os.path.exists(pathname):
            return pathname

    return None

def describeException(backTrace = 4):
    # When called in an exception handler, returns a string describing
    # the current exception.

    def byteOffsetToLineno(code, byte):
        # Returns the source line number corresponding to the given byte
        # offset into the indicated Python code module.

        import array
        lnotab = array.array('B', code.co_lnotab)

        line   = code.co_firstlineno
        for i in range(0, len(lnotab), 2):
            byte -= lnotab[i]
            if byte <= 0:
                return line
            line += lnotab[i+1]

        return line

    infoArr = sys.exc_info()
    exception = infoArr[0]
    exceptionName = getattr(exception, '__name__', None)
    extraInfo = infoArr[1]
    trace = infoArr[2]

    stack = []
    while trace.tb_next:
        # We need to call byteOffsetToLineno to determine the true
        # line number at which the exception occurred, even though we
        # have both trace.tb_lineno and frame.f_lineno, which return
        # the correct line number only in non-optimized mode.
        frame = trace.tb_frame
        module = frame.f_globals.get('__name__', None)
        lineno = byteOffsetToLineno(frame.f_code, frame.f_lasti)
        stack.append("%s:%s, " % (module, lineno))
        trace = trace.tb_next

    frame = trace.tb_frame
    module = frame.f_globals.get('__name__', None)
    lineno = byteOffsetToLineno(frame.f_code, frame.f_lasti)
    stack.append("%s:%s, " % (module, lineno))

    description = ""
    for i in range(len(stack) - 1, max(len(stack) - backTrace, 0) - 1, -1):
        description += stack[i]

    description += "%s: %s" % (exceptionName, extraInfo)
    return description

def clampScalar(value, a, b):
    # calling this ought to be faster than calling both min and max
    if a < b:
        if value < a:
            return a
        elif value > b:
            return b
        else:
            return value
    else:
        if value < b:
            return b
        elif value > a:
            return a
        else:
            return value

def pivotScalar(scalar, pivot):
    # reflect scalar about pivot; see tests below
    return pivot + (pivot - scalar)

if __debug__:
    assert pivotScalar(1, 0) == -1
    assert pivotScalar(-1, 0) == 1
    assert pivotScalar(3, 5) == 7
    assert pivotScalar(10, 1) == -8

def weightedChoice(choiceList, rng=random.random, sum=None):
    """given a list of (weight, item) pairs, chooses an item based on the
    weights. rng must return 0..1. if you happen to have the sum of the
    weights, pass it in 'sum'."""
    # TODO: add support for dicts
    if sum is None:
        sum = 0.
        for weight, item in choiceList:
            sum += weight

    rand = rng()
    accum = rand * sum
    for weight, item in choiceList:
        accum -= weight
        if accum <= 0.:
            return item
    # rand is ~1., and floating-point error prevented accum from hitting 0.
    # Or you passed in a 'sum' that was was too large.
    # Return the last item.
    return item

def randFloat(a, b=0., rng=random.random):
    """returns a random float in [a, b]
    call with single argument to generate random float between arg and zero
    """
    return lerp(a, b, rng())

def normalDistrib(a, b, gauss=random.gauss):
    """
    NOTE: assumes a < b

    Returns random number between a and b, using gaussian distribution, with
    mean=avg(a, b), and a standard deviation that fits ~99.7% of the curve
    between a and b. Outlying results are clipped to a and b.

    ------------------------------------------------------------------------
    http://www-stat.stanford.edu/~naras/jsm/NormalDensity/NormalDensity.html

    The 68-95-99.7% Rule
    ====================
    All normal density curves satisfy the following property which is often
      referred to as the Empirical Rule:
    68% of the observations fall within 1 standard deviation of the mean.
    95% of the observations fall within 2 standard deviations of the mean.
    99.7% of the observations fall within 3 standard deviations of the mean.

    Thus, for a normal distribution, almost all values lie within 3 standard
      deviations of the mean.
    ------------------------------------------------------------------------

    In calculating our standard deviation, we divide (b-a) by 6, since the
    99.7% figure includes 3 standard deviations _on_either_side_ of the mean.
    """
    return max(a, min(b, gauss((a+b)*.5, (b-a)/6.)))

def weightedRand(valDict, rng=random.random):
    """
    pass in a dictionary with a selection -> weight mapping.  Eg.
    {"Choice 1": 10,
     "Choice 2": 30,
     "bear":     100}

    -Weights need not add up to any particular value.
    -The actual selection will be returned.
    """
    selections = valDict.keys()
    weights = valDict.values()

    totalWeight = 0
    for weight in weights:
        totalWeight += weight

    # get a random value between 0 and the total of the weights
    randomWeight = rng() * totalWeight

    # find the index that corresponds with this weight
    for i in range(len(weights)):
        totalWeight -= weights[i]
        if totalWeight <= randomWeight:
            return selections[i]

    assert True, "Should never get here"
    return selections[-1]

def randUint31(rng=random.random):
    """returns a random integer in [0..2^31).
    rng must return float in [0..1]"""
    return int(rng() * 0x7FFFFFFF)

def randInt32(rng=random.random):
    """returns a random integer in [-2147483648..2147483647].
    rng must return float in [0..1]
    """
    i = int(rng() * 0x7FFFFFFF)
    if rng() < .5:
        i *= -1
    return i

def randUint32(rng=random.random):
    """returns a random integer in [0..2^32).
    rng must return float in [0..1]"""
    return long(rng() * 0xFFFFFFFFL)

class SerialNumGen:
    """generates serial numbers"""
    def __init__(self, start=None):
        if start is None:
            start = 0
        self.__counter = start-1
    def next(self):
        self.__counter += 1
        return self.__counter

_serialGen = SerialNumGen()
def serialNum():
    global _serialGen
    return _serialGen.next()
def uniqueName(name):
    global _serialGen
    return '%s-%s' % (name, _serialGen.next())

class EnumIter:
    def __init__(self, enum):
        self._values = enum._stringTable.keys()
        self._index = 0
    def __iter__(self):
        return self
    def next(self):
        if self._index >= len(self._values):
            raise StopIteration
        self._index += 1
        return self._values[self._index-1]

class Enum:
    """Pass in list of strings or string of comma-separated strings.
    Items are accessible as instance.item, and are assigned unique,
    increasing integer values. Pass in integer for 'start' to override
    starting value.

    Example:

    >>> colors = Enum('red, green, blue')
    >>> colors.red
    0
    >>> colors.green
    1
    >>> colors.blue
    2
    >>> colors.getString(colors.red)
    'red'
    """

    if __debug__:
        # chars that cannot appear within an item string.
        InvalidChars = string.whitespace
        def _checkValidIdentifier(item):
            invalidChars = string.whitespace+string.punctuation
            invalidChars = invalidChars.replace('_','')
            invalidFirstChars = invalidChars+string.digits
            if item[0] in invalidFirstChars:
                raise SyntaxError, ("Enum '%s' contains invalid first char" %
                                    item)
            if not disjoint(item, invalidChars):
                for char in item:
                    if char in invalidChars:
                        raise SyntaxError, (
                            "Enum\n'%s'\ncontains illegal char '%s'" %
                            (item, char))
            return 1
        _checkValidIdentifier = staticmethod(_checkValidIdentifier)

    def __init__(self, items, start=0):
        if type(items) == types.StringType:
            items = items.split(',')

        self._stringTable = {}

        # make sure we don't overwrite an existing element of the class
        assert self._checkExistingMembers(items)
        assert uniqueElements(items)

        i = start
        for item in items:
            # remove leading/trailing whitespace
            item = string.strip(item)
            # is there anything left?
            if len(item) == 0:
                continue
            # make sure there are no invalid characters
            assert Enum._checkValidIdentifier(item)
            self.__dict__[item] = i
            self._stringTable[i] = item
            i += 1

    def __iter__(self):
        return EnumIter(self)

    def getString(self, value):
        return self._stringTable[value]

    def __contains__(self, value):
        return value in self._stringTable

    def __len__(self):
        return len(self._stringTable)

    def copyTo(self, obj):
        # copies all members onto obj
        for name, value in self._stringTable:
            setattr(obj, name, value)

    if __debug__:
        def _checkExistingMembers(self, items):
            for item in items:
                if hasattr(self, item):
                    return 0
            return 1

############################################################
# class: Singleton
# Purpose: This provides a base metaclass for all classes
#          that require one and only one instance.
#
# Example: class mySingleton:
#              __metaclass__ = PythonUtil.Singleton
#              def __init__(self, ...):
#                  ...
#
# Note: This class is based on Python's New-Style Class
#       design. An error will occur if a defined class
#       attemps to inherit from a Classic-Style Class only,
#       ie: class myClassX:
#               def __init__(self, ...):
#                   ...
#
#           class myNewClassX(myClassX):
#               __metaclass__ = PythonUtil.Singleton
#               def __init__(self, ...):
#                   myClassX.__init__(self, ...)
#                   ...
#
#           This causes problems because myNewClassX is a
#           New-Style class that inherits from only a
#           Classic-Style base class. There are two ways
#           simple ways to resolve this issue.
#
#           First, if possible, make myClassX a
#           New-Style class by inheriting from object
#           object. IE:  class myClassX(object):
#
#           If for some reason that is not an option, make
#           myNewClassX inherit from object and myClassX.
#           IE: class myNewClassX(object, myClassX):
############################################################
class Singleton(type):
    def __init__(cls, name, bases, dic):
        super(Singleton, cls).__init__(name, bases, dic)
        cls.instance=None
    def __call__(cls, *args, **kw):
        if cls.instance is None:
            cls.instance=super(Singleton, cls).__call__(*args, **kw)
        return cls.instance

class SingletonError(ValueError):
    """ Used to indicate an inappropriate value for a Singleton."""

def printListEnum(l):
    # log each individual item with a number in front of it
    digits = 0
    n = len(l)
    while n > 0:
        digits += 1
        n /= 10
    format = '%0' + '%s' % digits + 'i:%s'
    for i in range(len(l)):
        print format % (i, l[i])

def gcDebugOn():
    import gc
    return (gc.get_debug() & gc.DEBUG_SAVEALL) == gc.DEBUG_SAVEALL

def safeRepr(obj):
    try:
        return repr(obj)
    except:
        return '<** FAILED REPR OF %s **>' % obj.__class__.__name__

def fastRepr(obj, maxLen=200, strFactor=10, _visitedIds=None):
    """ caps the length of iterable types """
    if _visitedIds is None:
        _visitedIds = set()
    if id(obj) in _visitedIds:
        return '<ALREADY-VISITED %s>' % itype(obj)
    if type(obj) in (types.TupleType, types.ListType):
        s = ''
        s += {types.TupleType: '(',
              types.ListType:  '[',}[type(obj)]
        if len(obj) > maxLen:
            o = obj[:maxLen]
            ellips = '...'
        else:
            o = obj
            ellips = ''
        _visitedIds.add(id(obj))
        for item in o:
            s += fastRepr(item, maxLen, _visitedIds=_visitedIds)
            s += ', '
        _visitedIds.remove(id(obj))
        s += ellips
        s += {types.TupleType: ')',
              types.ListType:  ']',}[type(obj)]
        return s
    elif type(obj) is types.DictType:
        s = '{'
        if len(obj) > maxLen:
            o = obj.keys()[:maxLen]
            ellips = '...'
        else:
            o = obj.keys()
            ellips = ''
        _visitedIds.add(id(obj))
        for key in o:
            value = obj[key]
            s += '%s: %s, ' % (fastRepr(key, maxLen, _visitedIds=_visitedIds),
                               fastRepr(value, maxLen, _visitedIds=_visitedIds))
        _visitedIds.remove(id(obj))
        s += ellips
        s += '}'
        return s
    elif type(obj) is types.StringType:
        maxLen *= strFactor
        if len(obj) > maxLen:
            return repr(obj[:maxLen])
        else:
            return repr(obj)
    else:
        return safeRepr(obj)

def tagRepr(obj, tag):
    """adds a string onto the repr output of an instance"""
    def reprWithTag(oldRepr, tag, self):
        return oldRepr() + '::<TAG=' + tag + '>'
    oldRepr = getattr(obj, '__repr__', None)
    if oldRepr is None:
        def stringer(s):
            return s
        oldRepr = Functor(stringer, repr(obj))
        stringer = None
    obj.__repr__ = new.instancemethod(Functor(reprWithTag, oldRepr, tag), obj, obj.__class__)
    reprWithTag = None
    return obj

def tagWithCaller(obj):
    """add info about the caller of the caller"""
    tagRepr(obj, str(callerInfo(howFarBack=1)))

def isDefaultValue(x):
    return x == type(x)()

def notNone(A, B):
    # returns A if not None, B otherwise
    if A is None:
        return B
    return A

def appendStr(obj, st):
    """adds a string onto the __str__ output of an instance"""
    def appendedStr(oldStr, st, self):
        return oldStr() + st
    oldStr = getattr(obj, '__str__', None)
    if oldStr is None:
        def stringer(s):
            return s
        oldStr = Functor(stringer, str(obj))
        stringer = None
    obj.__str__ = new.instancemethod(Functor(appendedStr, oldStr, st), obj, obj.__class__)
    appendedStr = None
    return obj

# debugging functions that conditionally bring up the debugger in __dev__
# all can be used with assert, as in 'assert _equal(a,b)'
def setTrace():
    if __dev__:
        print StackTrace()
        import pdb;pdb.set_trace()
        # setTrace
    return True
def _is(a,b):
    if a is not b:
        if __dev__:
            print StackTrace()
            import pdb;pdb.set_trace()
            # setTrace
        else:
            assert a is b
    return True
def _equal(a,b):
    if a != b:
        if __dev__:
            print StackTrace()
            import pdb;pdb.set_trace()
            # setTrace
        else:
            assert a == b
    return True
def _notEqual(a,b):
    if a == b:
        if __dev__:
            print StackTrace()
            import pdb;pdb.set_trace()
            # setTrace
        else:
            assert a != b
    return True
def _isNone(a):
    if a is not None:
        if __dev__:
            print StackTrace()
            import pdb;pdb.set_trace()
            # setTrace
        else:
            assert a is None
    return True
def _notNone(a):
    if a is None:
        if __dev__:
            print StackTrace()
            import pdb;pdb.set_trace()
            pass # import pdb;pdb.set_trace()
        else:
            assert a is not None
    return True
def _contains(container,item):
    if item not in container:
        if __dev__:
            print StackTrace()
            import pdb;pdb.set_trace()
            # setTrace
        else:
            assert item in container
    return True
def _notIn(container,item):
    if item in container:
        if __dev__:
            print StackTrace()
            import pdb;pdb.set_trace()
            # setTrace
        else:
            assert item not in container
    return True

class ScratchPad:
    """empty class to stick values onto"""
    def __init__(self, **kArgs):
        for key, value in kArgs.items():
            setattr(self, key, value)
        self._keys = kArgs.keys()
    def destroy(self):
        for key in self._keys:
            delattr(self, key)

class Sync:
    _SeriesGen = SerialNumGen()
    def __init__(self, name, other=None):
        self._name = name
        if other is None:
            self._series = self._SeriesGen.next()
            self._value = 0
        else:
            self._series = other._series
            self._value = other._value
    def invalidate(self):
        self._value = None
    def change(self):
        self._value += 1
    def sync(self, other):
        if (self._series != other._series) or (self._value != other._value):
            self._series = other._series
            self._value = other._value
            return True
        else:
            return False
    def isSynced(self, other):
        return ((self._series == other._series) and
                (self._value == other._value))
    def __repr__(self):
        return '%s(%s)<family=%s,value=%s>' % (self.__class__.__name__,
                              self._name, self._series, self._value)

class RefCounter:
    def __init__(self, byId=False):
        self._byId = byId
        self._refCounts = {}
    def _getKey(self, item):
        if self._byId:
            key = id(item)
        else:
            key = item
    def inc(self, item):
        key = self._getKey(item)
        self._refCounts.setdefault(key, 0)
        self._refCounts[key] += 1
    def dec(self, item):
        """returns True if ref count has hit zero"""
        key = self._getKey(item)
        self._refCounts[key] -= 1
        result = False
        if self._refCounts[key] == 0:
            result = True
            del self._refCounts[key]
        return result

def itype(obj):
    t = type(obj)
    if t is types.InstanceType:
        return '%s of <class %s>' % (repr(types.InstanceType),
                                     str(obj.__class__))
    else:
        return t

def getNumberedTypedString(items, maxLen=5000, numPrefix=''):
    """get a string that has each item of the list on its own line,
    and each item is numbered on the left from zero"""
    digits = 0
    n = len(items)
    while n > 0:
        digits += 1
        n /= 10
    digits = digits
    format = numPrefix + '%0' + '%s' % digits + 'i:%s \t%s'
    first = True
    s = ''
    for i in xrange(len(items)):
        if not first:
            s += '\n'
        first = False
        objStr = fastRepr(items[i])
        if len(objStr) > maxLen:
            snip = '<SNIP>'
            objStr = '%s%s' % (objStr[:(maxLen-len(snip))], snip)
        s += format % (i, itype(items[i]), objStr)
    return s

def printNumberedTyped(items, maxLen=5000):
    """print out each item of the list on its own line,
    with each item numbered on the left from zero"""
    digits = 0
    n = len(items)
    while n > 0:
        digits += 1
        n /= 10
    digits = digits
    format = '%0' + '%s' % digits + 'i:%s \t%s'
    first = True
    for i in xrange(len(items)):
        first = False
        objStr = fastRepr(items[i])
        if len(objStr) > maxLen:
            snip = '<SNIP>'
            objStr = '%s%s' % (objStr[:(maxLen-len(snip))], snip)
        print format % (i, itype(items[i]), objStr)

class DelayedCall:
    """ calls a func after a specified delay """
    def __init__(self, func, name=None, delay=None):
        if name is None:
            name = 'anonymous'
        if delay is None:
            delay = .01
        self._func = func
        taskMgr.doMethodLater(delay, self._doCallback, 'DelayedCallback-%s' % name)
    def _doCallback(self, task):
        func = self._func
        del self._func
        func()

class FrameDelayedCall:
    """ calls a func after N frames """
    def __init__(self, name, callback, frames=None, cancelFunc=None):
        # checkFunc is optional; called every frame, if returns True, FrameDelay is cancelled
        # and callback is not called
        if frames is None:
            frames = 1
        self._name = name
        self._frames = frames
        self._callback = callback
        self._cancelFunc = cancelFunc
        self._taskName = uniqueName('%s-%s' % (self.__class__.__name__, self._name))
        self._startTask()
    def destroy(self):
        self._stopTask()
    def finish(self):
        self._callback()
        self.destroy()
    def _startTask(self):
        taskMgr.add(self._frameTask, self._taskName)
        self._counter = 0
    def _stopTask(self):
        taskMgr.remove(self._taskName)
    def _frameTask(self, task):
        if self._cancelFunc and self._cancelFunc():
            self.destroy()
            return task.done
        self._counter += 1
        if self._counter >= self._frames:
            self.finish()
            return task.done
        return task.cont

class DelayedFunctor:
    """ Waits for this object to be called, then calls supplied functor after a delay.
    Effectively inserts a time delay between the caller and the functor. """
    def __init__(self, functor, name=None, delay=None):
        self._functor = functor
        self._name = name
        # FunctionInterval requires __name__
        self.__name__ = self._name
        self._delay = delay
    def _callFunctor(self):
        cb = Functor(self._functor, *self._args, **self._kwArgs)
        del self._functor
        del self._name
        del self._delay
        del self._args
        del self._kwArgs
        del self._delayedCall
        del self.__name__
        cb()
    def __call__(self, *args, **kwArgs):
        self._args = args
        self._kwArgs = kwArgs
        self._delayedCall = DelayedCall(self._callFunctor, self._name, self._delay)

class ArgumentEater:
    def __init__(self, numToEat, func):
        self._numToEat = numToEat
        self._func = func
    def __call__(self, *args, **kwArgs):
        self._func(*args[self._numToEat:], **kwArgs)

class ClassTree:
    def __init__(self, instanceOrClass):
        if type(instanceOrClass) in (types.ClassType, types.TypeType):
            cls = instanceOrClass
        else:
            cls = instanceOrClass.__class__
        self._cls = cls
        self._bases = []
        for base in self._cls.__bases__:
            if base not in (types.ObjectType, types.TypeType):
                self._bases.append(ClassTree(base))
    def getAllClasses(self):
        # returns set of this class and all base classes
        classes = set()
        classes.add(self._cls)
        for base in self._bases:
            classes.update(base.getAllClasses())
        return classes
    def _getStr(self, indent=None, clsLeftAtIndent=None):
        # indent is how far to the right to indent (i.e. how many levels
        # deep in the hierarchy from the most-derived)
        #
        # clsLeftAtIndent is an array of # of classes left to be
        # printed at each level of the hierarchy; most-derived is
        # at index 0
        if indent is None:
            indent = 0
            clsLeftAtIndent = [1]
        s = ''
        if (indent > 1):
            for i in range(1, indent):
                # if we have not printed all base classes at
                # this indent level, keep printing the vertical
                # column
                if clsLeftAtIndent[i] > 0:
                    s += ' |'
                else:
                    s += '  '
        if (indent > 0):
            s += ' +'
        s += self._cls.__name__
        clsLeftAtIndent[indent] -= 1
        """
        ### show the module to the right of the class name
        moduleIndent = 48
        if len(s) >= moduleIndent:
            moduleIndent = (len(s) % 4) + 4
        padding = moduleIndent - len(s)
        s += padding * ' '
        s += self._cls.__module__
        ###
        """
        if len(self._bases):
            newList = list(clsLeftAtIndent)
            newList.append(len(self._bases))
            bases = self._bases
            # print classes with fewer bases first
            bases.sort(lambda x,y: len(x._bases)-len(y._bases))
            for base in bases:
                s += '\n%s' % base._getStr(indent+1, newList)
        return s
    def __repr__(self):
        return self._getStr()


def report(types = [], prefix = '', notifyFunc = None, dConfigParam = []):
    """
    This is a decorator generating function.  Use is similar to
    a @decorator, except you must be sure to call it as a function.
    It actually returns the decorator which is then used to transform
    your decorated function. Confusing at first, I know.

    Decoration occurs at function definition time.

    If __dev__ is not defined, or resolves to False, this function
    has no effect and no wrapping/transform occurs.  So in production,
    it's as if the report has been asserted out.
    
    Parameters::
    types : A subset list of ['timeStamp', 'frameCount', 'avLocation']
            This allows you to specify certain common bits of info.

            args: Prints the arguments as they were passed to this
                  function.
            timeStamp:  Adds the current frame time to the output.
            
            frameCount: Adds the current frame count to the output.
                        Usually cleaner than the timeStamp output.

            avLocation: Adds the localAvatar's network location
                        to the output.  Useful for interest debugging.

    prefix: Optional string to prepend to output, just before the function.
            Allows for easy grepping.
    notifyFunc: A notify function such as info, debug, warning, etc.
                By default the report will be printed to stdout. This 
                will allow you send the report to a designated 'notify'
                output.

    dConfigParams: A list of Config.prc string variables.
                   By default the report will always print.  If you
                   specify this param, it will only print if one of the
                   specified config strings resolve to True.
    """
    def decorator(f):
        return f
    try:
        if not __dev__:
            return decorator            
    except NameError,e:
        return decorator

    from direct.distributed.ClockDelta import globalClockDelta

    def decorator(f):
        def wrap(*args,**kwargs):
            if args:
                rArgs = [args[0].__class__.__name__ + ', ']
            else:
                rArgs = []

            if 'args' in types:
                rArgs += [`x`+', ' for x in args[1:]] + \
                         [ x + ' = ' + '%s, ' % `y` for x,y in kwargs.items()]
            
            if not rArgs:
                rArgs = '()'
            else:
                rArgs = '(' + reduce(str.__add__,rArgs)[:-2] + ')'
                
            outStr = '%s%s' % (f.func_name, rArgs)

            preStr = ''
            
            if 'frameCount' in types:
                outStr = '%8d : %s' % (globalClock.getFrameCount(), outStr)
                
            if 'timeStamp' in types:
                outStr = '%8.3f : %s' % (globalClock.getFrameTime(), outStr)

            if 'deltaStamp' in types:
                outStr = '%8.2f : %s' % (globalClock.getRealTime() - \
                                         globalClockDelta.delta, outStr)                
            if 'avLocation' in types:
                outStr = '%s : %s' % (outStr, str(localAvatar.getLocation()))

            if prefix:
                outStr = '%s %s' % (prefix, outStr)

            # determine whether we should print
            doPrint = False
            if not dConfigParam:
                doPrint = True
            else:
                if isinstance(dConfigParam,str):
                    if(config.GetBool(dConfigParam, 0)):
                        doPrint = True
                elif isinstance(dConfigParam, (list,tuple)):
                    for param in dConfigParam:
                        if(config.GetBool(dConfigParam, 0)):
                            doPrint = True
                            break

            if doPrint:
                if notifyFunc:
                    notifyFunc(outStr)
                else:
                    print outStr

                if 'printInterests' in types:
                    base.cr.printInterestSets()
                    
                if 'stackTrace' in types:
                    print StackTrace()

            return f(*args,**kwargs)

        wrap.func_name = f.func_name
        wrap.func_dict = f.func_dict
        wrap.func_doc = f.func_doc
        return wrap
    
    return decorator

def getBase():
    try:
        return base
    except:
        return simbase

def superFlattenShip(ship):

    #PHASE 1: remove characters
    characterNodes=ship.findAllMatches("**/+Character")
    for i in range(characterNodes.getNumPaths()):
        character=characterNodes.getPath(i)
        children=character.getChildrenAsList()
        for child in children:
            child.copyTo(character.getParent())
        character.reparentTo(hidden)

    #PHASE 2: remove textures
    tex=ship.findAllTextures().getTexture(0)
    ship.setTexture(tex,1000)

    #PHASE 3: stop rocking task
    taskMgr.remove("shipRocking-%d"%(ship.getDoId()))

    #PHASE 4: kill lamp effects
    from pirates.shipparts.DistributedShipDecor import DistributedShipDecor
    for DO in base.cr.doId2do.values():
        if(type(DO) == DistributedShipDecor):
            if (hasattr(DO.prop, 'lanternGlowEffect')):
                DO.prop.lanternGlowEffect.destroy()
            
    #PHASE 5: flatten strong!
    return ship.flattenStrong()

def exceptionLogged(append=True):
    """decorator that outputs the function name and all arguments
    if an exception passes back through the stack frame
    if append is true, string is appended to the __str__ output of
    the exception. if append is false, string is printed to the log
    directly. If the output will take up many lines, it's recommended
    to set append to False so that the exception stack is not hidden
    by the output of this decorator.
    """
    def _decoratorFunc(f, append=append):
        def _exceptionLogged(*args, **kArgs):
            try:
                return f(*args, **kArgs)
            except Exception, e:
                try:
                    s = 'STACK UNWIND: %s(' % f.func_name
                    for arg in args:
                        s += '%s, ' % arg
                    for key, value in kArgs.items():
                        s += '%s=%s, ' % (key, value)
                    if len(args) or len(kArgs):
                        s = s[:-2]
                    s += ')'
                    if append:
                        appendStr(e, '\n%s' % s)
                    else:
                        print s
                except:
                    print 'exceptionLogged(%s): ERROR IN PRINTING' % f.func_name
                raise
        _exceptionLogged.__doc__ = f.__doc__
        return _exceptionLogged
    return _decoratorFunc

# class 'decorator' that records the stack at the time of creation
# be careful with this, it creates a StackTrace, and that can take a
# lot of CPU
def recordCreationStack(cls):
    if not hasattr(cls, '__init__'):
        raise 'recordCreationStack: class \'%s\' must define __init__' % cls.__name__
    cls.__moved_init__ = cls.__init__
    def __recordCreationStack_init__(self, *args, **kArgs):
        self._creationStackTrace = StackTrace(start=1)
        return self.__moved_init__(*args, **kArgs)
    def getCreationStackTrace(self):
        return self._creationStackTrace
    cls.__init__ = __recordCreationStack_init__
    cls.getCreationStackTrace = getCreationStackTrace
    return cls

# class 'decorator' that logs all method calls for a particular class
def logMethodCalls(cls):
    if not hasattr(cls, 'notify'):
        raise 'logMethodCalls: class \'%s\' must have a notify' % cls.__name__
    for name in dir(cls):
        method = getattr(cls, name)
        if callable(method):
            def getLoggedMethodCall(method):
                def __logMethodCall__(obj, *args, **kArgs):
                    s = '%s(' % method.__name__
                    for arg in args:
                        try:
                            argStr = repr(arg)
                        except:
                            argStr = 'bad repr: %s' % arg.__class__
                        s += '%s, ' % argStr
                    for karg, value in kArgs.items():
                        s += '%s=%s, ' % (karg, repr(value))
                    if len(args) or len(kArgs):
                        s = s[:-2]
                    s += ')'
                    obj.notify.info(s)
                    return method(obj, *args, **kArgs)
                return __logMethodCall__
            setattr(cls, name, getLoggedMethodCall(method))
    __logMethodCall__ = None
    return cls

# http://en.wikipedia.org/wiki/Golden_ratio
GoldenRatio = (1. + math.sqrt(5.)) / 2.
class GoldenRectangle:
    @staticmethod
    def getLongerEdge(shorter):
        return shorter * GoldenRatio
    @staticmethod
    def getShorterEdge(longer):
        return longer / GoldenRatio

class HotkeyBreaker:
    def __init__(self,breakKeys = []):
        from direct.showbase.DirectObject import DirectObject
        self.do = DirectObject()
        self.breakKeys = {}
        for key in breakKeys:
            self.addBreakKey(key)
        
    def addBreakKey(self,breakKey):
        if __dev__:
            self.do.accept(breakKey,self.breakFunc,extraArgs = [breakKey])
        
    def removeBreakKey(self,breakKey):
        if __dev__:
            self.do.ignore(breakKey)

    def breakFunc(self,breakKey):
        if __dev__:
            self.breakKeys[breakKey] = True

    def setBreakPt(self,breakKey = None):
        if __dev__:
            if breakKey == None:
                import pdb;pdb.set_trace()
            if self.breakKeys.pop(breakKey,False):
                import pdb;pdb.set_trace()


import __builtin__
__builtin__.Functor = Functor
__builtin__.Stack = Stack
__builtin__.Queue = Queue
__builtin__.Enum = Enum
__builtin__.SerialNumGen = SerialNumGen
__builtin__.ScratchPad = ScratchPad
__builtin__.uniqueName = uniqueName
__builtin__.serialNum = serialNum
__builtin__.profiled = profiled
__builtin__.setTrace = setTrace
__builtin__._is = _is
__builtin__._equal = _equal
__builtin__._notEqual = _notEqual
__builtin__._isNone = _isNone
__builtin__._notNone = _notNone
__builtin__._contains = _contains
__builtin__._notIn = _notIn
__builtin__.itype = itype
__builtin__.exceptionLogged = exceptionLogged
__builtin__.appendStr = appendStr
__builtin__.bound = bound
__builtin__.lerp = lerp
__builtin__.notNone = notNone
__builtin__.clampScalar = clampScalar
__builtin__.makeList = makeList
__builtin__.makeTuple = makeTuple
__builtin__.rad90 = rad90
__builtin__.rad180 = rad180
__builtin__.rad270 = rad270
__builtin__.rad360 = rad360
__builtin__.printStack = printStack
__builtin__.printVerboseStack = printVerboseStack
__builtin__.DelayedCall = DelayedCall
__builtin__.DelayedFunctor = DelayedFunctor
__builtin__.FrameDelayedCall = FrameDelayedCall
__builtin__.ArgumentEater = ArgumentEater
__builtin__.ClassTree = ClassTree
__builtin__.invertDict = invertDict
__builtin__.invertDictLossless = invertDictLossless
