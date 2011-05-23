--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

TOPDIR = os.getcwd()
print(TOPDIR)

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

PROJECT = {
  major_version = 2,
  minor_version = 99,
  revision      = 60,

  protocol      = '0118',

  build_type    = 'DEV',

  copyright     = 'Copyright (c) 1993-2010 Tim Riker',

  config_date   = os.date(),

  config = {}
}

print('os.get()', os.get())

print('premake.gcc.cxx', premake.gcc.cxx)
premake.gcc.cxx = 'ccache g++'
print('premake.gcc.cxx', premake.gcc.cxx)

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

if (1 > 0) then
  print()
  print(('-'):rep(80))
  for k, v in pairs(_G) do print('_G', k, tostring(v)) end
  for k, v in pairs(_MAKE) do print('_MAKE', k, tostring(v)) end
  for k, v in pairs(_OPTIONS) do print('_OPTIONS', k, tostring(v)) end
  for k, v in pairs(_VS) do print('_VS', k, tostring(v)) end
  for k, v in pairs(_ARGS) do print('_ARGS', k, tostring(v)) end
  print()
  print(('-'):rep(80))
  print()
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

include 'premake4_config'

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--[[FIXME
solution 'shit'
  configurations { 'shit2', 'shit1' }

configuration 'shit2'
  defines '_DEBUG'
  flags 'symbols'

project 'shit'
  kind 'ConsoleApp'
  language 'c++'
  files 'crap.c'
--]]

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

solution 'bz'
if (_OPTIONS['enable-debug']) then
  configurations { 'debug' }
else
  configurations { 'release', 'debug' }
end

if (_ACTION and (_ACTION ~= 'gmake')) then
  location('build_' .. _ACTION)
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

configuration 'release'
  defines { 'NDEBUG' }
  flags   { 'ExtraWarnings', 'OptimizeSpeed' }

configuration 'debug'
  defines { 'DEBUG', '_DEBUG', 'DEBUG_RENDERING' }
  flags   { 'ExtraWarnings', 'FatalWarnings', 'Symbols' }

configuration { 'debug', 'gmake', 'not windows'}
  buildoptions '-Wextra -Wundef -Wshadow -Wno-long-long -ansi -pedantic'


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

local function newopt(trigger, description)
  newoption { trigger = trigger, description = description }
end

newopt('disable-bzfs',     'disable the bzfs build')
newopt('disable-bzflag',   'disable the bzflag build')
newopt('disable-bzadmin',  'disable the bzadmin build')
newopt('disable-bzrobots', 'disable the bzrobots build')
newopt('disable-plugins',  'disable the bzfs plugins')
newopt('enable-debug',     'enable debugging')


--[[
if (_OPTIONS['disable-bzfs']) then
  _OPTIONS['disable-plugins'] = {
    value = 'true',
    trigger = 'trigger',
    description = 'desc',
  }
end
--]]


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

include 'other_src'
include 'src'
include 'man'

if (not _OPTIONS['disable-plugins']) then
  include 'plugins'
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--[[
newaction {
  trigger = 'install',
  description = 'install the shit',
  execute = function()
    os.execute('echo installing some shit')
  end
}
--]]

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

--[[
bots/
data/
debian/
doc/
m4/
man/
misc/
MSVC/
other_src/
package/
plugins/
src/
tools/
autom4te.cache/
BZFlag.xcodeproj/
--]]


print('22premake.gcc.cxx', premake.gcc.cxx)
premake.gcc.cxx = 'ccache g++'
print('22premake.gcc.cxx', premake.gcc.cxx)

print()
print(('-'):rep(80))
print(('-'):rep(80))
print()
