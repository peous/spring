# Copyright (C) 2006  Tobi Vollebregt

# see rts/build/scons/*.py for the core of the build system

""" Available targets.
Each target has an equivalent install target. E.g. `CentralBuildAI' has
`install-CentralBuildAI' and the default target has `install'.

[default]

	spring
	unitsync
	GroupAI
		CentralBuildAI
		MetalMakerAI
		SimpleFormationAI
	GlobalAI
		JCAI
		NTAI
		OTAI
		TestGlobalAI
"""


import os, sys

# copy our build scripts to a tmp dir,
# so compiled files (*.pyc) do not end up in our source tree
import tempfile, hashlib, shutil
sourceDirHash = hashlib.md5(os.getcwd()).hexdigest()
tmpBuildScriptsDir = os.path.join(tempfile.gettempdir(), 'springSconsBuildScripts', sourceDirHash)
shutil.rmtree(tmpBuildScriptsDir, True)
print('SCons tools copied to tmp-dir: ' + tmpBuildScriptsDir)
shutil.copytree('rts/build/scons', tmpBuildScriptsDir)
sys.path.append(tmpBuildScriptsDir)

import filelist

filelist.setSourceRootDir(os.path.abspath(os.getcwd()))

if sys.platform == 'win32':
	# force to mingw, otherwise picks up msvc
	myTools = ['mingw']
else:
	myTools = ['default']
myTools += ['rts', 'gch']

env = Environment(tools = myTools, toolpath = ['.', tmpBuildScriptsDir])
#env = Environment(tools = myTools, toolpath = ['.', tmpBuildScriptsDir], ENV = {'PATH' : os.environ['PATH']})
if env['platform'] == 'windows':
        env['SHARED_OBJ_EXT'] = '.o'
else:
        env['SHARED_OBJ_EXT'] = '.os'

if env['use_gch']:
	env['Gch'] = env.Gch(source='rts/System/StdAfx.h', target=os.path.join(env['builddir'], 'rts/System/StdAfx.h.gch'), CPPDEFINES=env['CPPDEFINES']+env['spring_defines'])[0]
else:
	import os.path
	if os.path.exists('rts/System/StdAfx.h.gch'):
		os.unlink('rts/System/StdAfx.h.gch')

def createStaticExtLibraryBuilder(env):
	"""This is a utility function that creates the StaticExtLibrary
	Builder in an Environment if it is not there already.

	If it is already there, we return the existing one."""
	import SCons.Action

	try:
		static_extlib = env['BUILDERS']['StaticExtLibrary']
	except KeyError:
		action_list = [ SCons.Action.Action("$ARCOM", "$ARCOMSTR") ]
		if env.Detect('ranlib'):
			ranlib_action = SCons.Action.Action("$RANLIBCOM", "$RANLIBCOMSTR")
			action_list.append(ranlib_action)

	static_extlib = SCons.Builder.Builder(action = action_list,
											emitter = '$LIBEMITTER',
											prefix = '$LIBPREFIX',
											suffix = '$LIBSUFFIX',
											src_suffix = '$OBJSUFFIX',
											src_builder = 'SharedObject')

	env['BUILDERS']['StaticExtLibrary'] = static_extlib
	return static_extlib 


# stores shared objects so newer scons versions don't choke with
def create_static_objects(env, fileList, suffix, additionalCPPDEFINES = []):
	objsList = []
	myEnv = env.Clone()
	myEnv.AppendUnique(CPPDEFINES = additionalCPPDEFINES)
	for f in fileList:
		while isinstance(f, list):
			f = f[0]
		fpath, fbase = os.path.split(f)
		fname, fext = fbase.rsplit('.', 1)
		objsList.append(myEnv.StaticObject(os.path.join(fpath, fname + suffix), f))
	return objsList


################################################################################
### Build spring(.exe)
################################################################################
spring_files = filelist.get_spring_source(env)

# spring.exe icon
if env['platform'] == 'windows':
	# resource builder (the default one of mingw gcc 4.1.1 crashes because of a popen bug in msvcrt.dll)
	if sys.platform == 'win32':
		rcbld = Builder(action = 'windres --use-temp-file -i $SOURCE -o $TARGET', suffix = '.o', src_suffix = '.rc')
	else:
		# dirty hack assuming on all non-win32 systems windres is called i586-mingw32msvc-windres..
		rcbld = Builder(action = 'i586-mingw32msvc-windres --use-temp-file -i $SOURCE -o $TARGET', suffix = '.o', src_suffix = '.rc')
	env.Append(BUILDERS = {'RES': rcbld})
	spring_files += env.RES(env['builddir'] + '/rts/build/scons/icon.rc', CPPPATH=[])

# calculate datadir locations
if env['platform'] == 'windows':
	datadir = []
else:
	datadir = ['SPRING_DATADIR="\\"'+os.path.join(env['prefix'], env['datadir'])+'\\""',
			'SPRING_DATADIR_2="\\"'+os.path.join(env['prefix'], env['libdir'])+'\\""']

# Build DataDirLocater.cpp separately from the other sources.  This is to prevent recompilation of
# the entire source if one wants to change just the install installprefix (and hence the datadir).

ddlcpp = env.Object(os.path.join(env['builddir'], 'rts/System/FileSystem/DataDirLocater.cpp'), CPPDEFINES = env['CPPDEFINES']+env['spring_defines']+datadir)
spring_files += [ddlcpp]

springenv = env.Clone(CPPDEFINES=env['CPPDEFINES']+env['spring_defines'])
if env['platform'] == 'windows':
	# create import library and .def file on Windows
	springDef = os.path.join(env['builddir'], 'spring.def')
	springA = os.path.join(env['builddir'], 'spring.a')
	springenv['LINKFLAGS'] = springenv['LINKFLAGS'] + ['-Wl,--output-def,' + springDef]
	springenv['LINKFLAGS'] = springenv['LINKFLAGS'] + ['-Wl,--kill-at', '--add-stdcall-alias']
	springenv['LINKFLAGS'] = springenv['LINKFLAGS'] + ['-Wl,--out-implib,' + springA]
	instSpringSuppl = [env.Install(os.path.join(env['installprefix'], env['bindir']), springDef)]
	instSpringSuppl += [env.Install(os.path.join(env['installprefix'], env['bindir']), springA)]
	Alias('install', instSpringSuppl)
	Alias('install-spring', instSpringSuppl)
spring = springenv.Program(os.path.join(springenv['builddir'], 'spring'), spring_files)

Alias('spring', spring)
Default(spring)
inst = env.Install(os.path.join(env['installprefix'], env['bindir']), spring)
Alias('install', inst)
Alias('install-spring', inst)

# Strip the executable if rts.py said so.
if env['strip']:
	env.AddPostAction(spring, Action([['strip','$TARGET']]))


################################################################################
### Build unitsync shared object
################################################################################
# HACK   we should probably compile libraries from 7zip, hpiutil2 and minizip
# so we don't need so much bloat here.
# Need a new env otherwise scons chokes on equal targets built with different flags.
usync_builddir = os.path.join(env['builddir'], 'unitsync')
uenv = env.Clone(builddir=usync_builddir)
uenv.AppendUnique(CPPDEFINES=['UNITSYNC', 'BITMAP_NO_OPENGL'])

def remove_precompiled_header(env):
	while 'USE_PRECOMPILED_HEADER' in env['CPPDEFINES']:
		env['CPPDEFINES'].remove('USE_PRECOMPILED_HEADER')
	while '-DUSE_PRECOMPILED_HEADER' in env['CFLAGS']:
		env['CFLAGS'].remove('-DUSE_PRECOMPILED_HEADER')
	while '-DUSE_PRECOMPILED_HEADER' in env['CXXFLAGS']:
		env['CXXFLAGS'].remove('-DUSE_PRECOMPILED_HEADER')

remove_precompiled_header(uenv)

def usync_get_source(*args, **kwargs):
	return filelist.get_source(uenv, ignore_builddir=True, *args, **kwargs)

uenv.BuildDir(os.path.join(uenv['builddir'], 'tools/unitsync'), 'tools/unitsync', duplicate = False)
unitsync_files          = usync_get_source('tools/unitsync')
unitsync_fs_files       = usync_get_source('rts/System/FileSystem/', exclude_list=('rts/System/FileSystem/DataDirLocater.cpp'));
unitsync_lua_files      = usync_get_source('rts/lib/lua/src');
unitsync_7zip_files     = usync_get_source('rts/lib/7zip');
unitsync_minizip_files  = usync_get_source('rts/lib/minizip', 'rts/lib/minizip/iowin32.c');
unitsync_hpiutil2_files = usync_get_source('rts/lib/hpiutil2');
unitsync_extra_files = [
	'rts/Game/GameVersion.cpp',
	'rts/Lua/LuaUtils.cpp',
	'rts/Lua/LuaIO.cpp',
	'rts/Lua/LuaParser.cpp',
	'rts/Map/MapParser.cpp',
	'rts/Map/SMF/SmfMapFile.cpp',
	'rts/Rendering/Textures/Bitmap.cpp',
	'rts/Rendering/Textures/nv_dds.cpp',
	'rts/Sim/Misc/SideParser.cpp',
	'rts/System/Info.cpp',
	'rts/System/Option.cpp',
	'rts/System/ConfigHandler.cpp',
	'rts/System/LogOutput.cpp',
]
unitsync_files.extend(unitsync_fs_files)
unitsync_files.extend(unitsync_lua_files)
unitsync_files.extend(unitsync_7zip_files)
unitsync_files.extend(unitsync_minizip_files)
unitsync_files.extend(unitsync_hpiutil2_files)
unitsync_files.extend(unitsync_extra_files)

unitsync_objects = []
if env['platform'] == 'windows':
	# crosscompiles on buildbot need this, but native mingw builds fail
	# during linking
	if os.name != 'nt':
		unitsync_files.append('rts/lib/minizip/iowin32.c')
	unitsync_files.append('rts/System/FileSystem/DataDirLocater.cpp')
	# Need the -Wl,--kill-at --add-stdcall-alias because TASClient expects undecorated stdcall functions.
	uenv['LINKFLAGS'] += ['-Wl,--kill-at', '--add-stdcall-alias']
else:
	ddlcpp = uenv.SharedObject(os.path.join(env['builddir'], 'rts/System/FileSystem/DataDirLocater.cpp'), CPPDEFINES = uenv['CPPDEFINES']+datadir)
	unitsync_objects += [ ddlcpp ]
# some scons stupidity
unitsync_objects += [uenv.SharedObject(source=f, target=os.path.join(uenv['builddir'], f)+uenv['SHARED_OBJ_EXT']) for f in unitsync_files]
unitsync = uenv.SharedLibrary(os.path.join(uenv['builddir'], 'unitsync'), unitsync_objects)

Alias('unitsync', unitsync)
inst = env.Install(os.path.join(env['installprefix'], env['libdir']), unitsync)
Alias('install', inst)
Alias('install-unitsync', inst)

# Strip the DLL if rts.py said so.
if env['strip']:
	env.AddPostAction(unitsync, Action([['strip','$TARGET']]))

# Somehow unitsync fails to build with mingw:
#  "build\tools\unitsync\pybind.o(.text+0x129d): In function `initunitsync':
#   pybind.cpp:663: undefined reference to `_imp__Py_InitModule4TraceRefs'"
# Figured this out: this means we're attempting to build unitsync with debugging
# enabled against a python version with debugging disabled.
if env['platform'] != 'windows':
	Default(unitsync)


################################################################################
### AIs
################################################################################
# Make a copy of the build environment for the AIs, but remove libraries and add include path.
# TODO: make separate SConstructs for AIs
aienv = env.Clone()
aienv.AppendUnique(CPPPATH = ['rts/ExternalAI'])
aienv.AppendUnique(CPPDEFINES=['USING_CREG'])

# Use subst() to substitute $installprefix in datadir.
install_dir = os.path.join(aienv['installprefix'], aienv['libdir'], 'AI/Helper-libs')

# store shared ai objects so newer scons versions don't choke with
# *** Two environments with different actions were specified for the same target
aiobjs = []
for f in filelist.get_shared_AI_source(aienv):
        while isinstance(f, list):
                f = f[0]
        fpath, fbase = os.path.split(f)
        fname, fext = fbase.rsplit('.', 1)
        aiobjs.append(aienv.SharedObject(os.path.join(fpath, fname + '-ai'), f))

#Build GroupAIs
for f in filelist.list_groupAIs(aienv, exclude_list=['build']):
	lib = aienv.SharedLibrary(os.path.join('game/AI/Helper-libs', f), aiobjs + filelist.get_groupAI_source(aienv, f))
	Alias(f, lib)         # Allow e.g. `scons CentralBuildAI' to compile just an AI.
	Alias('GroupAI', lib) # Allow `scons GroupAI' to compile all groupAIs.
	Default(lib)
	inst = env.Install(install_dir, lib)
	Alias('install', inst)
	Alias('install-GroupAI', inst)
	Alias('install-'+f, inst)
	if aienv['strip']:
		aienv.AddPostAction(lib, Action([['strip','$TARGET']]))

install_dir = os.path.join(aienv['installprefix'], aienv.subst(aienv['libdir']), 'AI/Bot-libs')

#Build GlobalAIs
for f in filelist.list_globalAIs(aienv, exclude_list=['build', 'CSAI', 'TestABICAI','AbicWrappersTestAI']):
	lib = aienv.SharedLibrary(os.path.join('game/AI/Bot-libs', f), aiobjs + filelist.get_globalAI_source(aienv, f))
	Alias(f, lib)          # Allow e.g. `scons JCAI' to compile just a global AI.
	Alias('GlobalAI', lib) # Allow `scons GlobalAI' to compile all globalAIs.
	Default(lib)
	inst = env.Install(install_dir, lib)
	Alias('install', inst)
	Alias('install-GlobalAI', inst)
	Alias('install-'+f, inst)
	if aienv['strip']:
		aienv.AddPostAction(lib, Action([['strip','$TARGET']]))

# build TestABICAI
# lib = aienv.SharedLibrary(os.path.join('game/AI/Bot-libs','TestABICAI'), ['game/spring.a'], CPPDEFINES = env# ['CPPDEFINES'] + ['BUILDING_AI'] )
# Alias('TestABICAI', lib)
# Alias('install-TestABICAI', inst)
# if sys.platform == 'win32':
# 	Alias('GlobalAI', lib)
# 	Default(lib)
# 	inst = env.Install(install_dir, lib)
# 	Alias('install', inst)
# 	Alias('install-GlobalAI', inst)
# 	if env['strip']:
# 		env.AddPostAction(lib, Action([['strip','$TARGET']]))

################################################################################
### Build streflop (which has it's own Makefile-based build system)
################################################################################
if not 'configure' in sys.argv and not 'test' in sys.argv and not 'install' in sys.argv:
	cmd = "CC=" + env['CC'] + " CXX=" + env['CXX'] + " --no-print-directory -C rts/lib/streflop"
	if env.has_key('streflop_extra'):
		cmd += " " + env['streflop_extra']
	if env['fpmath'] == 'sse':
		cmd = "STREFLOP_SSE=1 " + cmd
	else:
		cmd = "STREFLOP_X87=1 " + cmd
	if env['platform'] == 'windows':
		cmd += " WIN32=1"
	if env.GetOption('clean'):
		cmd += " clean"
	print 'streflop options:', cmd
	if env['platform'] == 'freebsd':
		status = os.system("gmake " + cmd)
	else:
		status = os.system("make " + cmd)
	if status != 0:
		# try with mingw32-make
		status = os.system("mingw32-make " + cmd)
	if status != 0:
		print "Failed building streflop!"
		env.Exit(1)
	else:
		print "Success building streflop!"


################################################################################
### Run Tests
################################################################################

# Use this to avoid an error message 'how to make target test ?'
env.Alias('test', None)

# Simple unit testing framework. In all 'Test' subdirectories, if a file 'test'
# exists, it is run. This test script should then compile the test(s) and run them.
if 'test' in sys.argv and env['platform'] != 'windows':
	for dir in filelist.list_directories(env, 'rts'):
		if dir.endswith('/Test'):
			test = os.path.join(dir, 'test')
			if os.path.isfile(test):
				os.system(test)


################################################################################
### Build gamedata zip archives & misc.
################################################################################
# Can't use these, we can't set the working directory and putting a SConscript
# in the respective directories doesn't work either because then the SConstript
# ends up in the zip too... Bah. SCons sucks. Just like autoshit and everything else btw.
#env.Zip('game/base/springcontent.sdz', filelist.list_files(env, 'installer/builddata/springcontent'))
#env.Zip('game/base/spring/bitmaps.sdz', filelist.list_files(env, 'installer/builddata/bitmaps'))

if not 'configure' in sys.argv and not 'test' in sys.argv and not 'install' in sys.argv:
	if sys.platform != 'win32':
		if env.GetOption('clean'):
			if os.system("rm -f game/base/springcontent.sdz"):
				env.Exit(1)
			if os.system("rm -f game/base/spring/bitmaps.sdz"):
				env.Exit(1)
			if os.system("rm -f game/base/maphelper.sdz"):
				env.Exit(1)
			if os.system("rm -f game/base/cursors.sdz"):
				env.Exit(1)
		else:
			if os.system("installer/make_gamedata_arch.sh"):
				env.Exit(1)

inst = env.Install(os.path.join(env['installprefix'], env['datadir'], 'base'), 'game/base/springcontent.sdz')
Alias('install', inst)
inst = env.Install(os.path.join(env['installprefix'], env['datadir'], 'base'), 'game/base/maphelper.sdz')
Alias('install', inst)
inst = env.Install(os.path.join(env['installprefix'], env['datadir'], 'base'), 'game/base/cursors.sdz')
Alias('install', inst)
inst = env.Install(os.path.join(env['installprefix'], env['datadir'], 'base/spring'), 'game/base/spring/bitmaps.sdz')
Alias('install', inst)

# install fonts
for font in os.listdir('game/fonts'):
	if not os.path.isdir(os.path.join('game/fonts', font)):
		inst = env.Install(os.path.join(env['installprefix'], env['datadir'], 'fonts'), os.path.join('game/fonts', font))
		Alias('install', inst)

# install some files from root of datadir
for f in ['cmdcolors.txt', 'ctrlpanel.txt', 'selectkeys.txt', 'uikeys.txt', 'teamcolors.lua']:
	inst = env.Install(os.path.join(env['installprefix'], env['datadir']), os.path.join('game', f))
	Alias('install', inst)

# install menu entry & icon
inst = env.Install(os.path.join(env['installprefix'], 'share/pixmaps'), 'rts/spring.png')
Alias('install', inst)
inst = env.Install(os.path.join(env['installprefix'], 'share/applications'), 'installer/freedesktop/applications/spring.desktop')
Alias('install', inst)

# install AAI config files
aai_data=filelist.list_files_recursive(env, 'game/AI/AAI')
for f in aai_data:
	if not os.path.isdir(f):
		inst = env.Install(os.path.join(aienv['installprefix'], aienv['datadir'], os.path.dirname(f)[5:]), f)
		Alias('install', inst)

# install LuaUI files
for f in ['luaui.lua']:
	inst = env.Install(os.path.join(env['installprefix'], env['datadir']), os.path.join('game', f))
	Alias('install', inst)
luaui_files=filelist.list_files_recursive(env, 'game/LuaUI')
for f in luaui_files:
	if not os.path.isdir(f):
		inst = env.Install(os.path.join(env['installprefix'], env['datadir'], os.path.dirname(f)[5:]), f)
		Alias('install', inst)
