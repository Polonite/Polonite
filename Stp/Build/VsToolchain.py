#!/usr/bin/env python
# Copyright 2017 Polonite Authors. All rights reserved.
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import glob
import json
import os
import pipes
import platform
import re
import shutil
import stat
import subprocess
import sys


CURRENT_DEFAULT_TOOLCHAIN_VERSION = '2017'


def SetEnvironmentAndGetRuntimeDllDirs():
  """Sets up os.environ to use the depot_tools VS toolchain with GN, and
  returns the location of the VS runtime DLLs so they can be copied into
  the output directory after GN generation.

  Return value is [x64path, x86path] or None
  """
  vs_runtime_dll_dirs = None
  if sys.platform == 'win32':
    if not 'GN_MSVS_OVERRIDE_PATH' in os.environ:
      os.environ['GN_MSVS_OVERRIDE_PATH'] = DetectVisualStudioPath()
    if not 'GN_MSVS_VERSION' in os.environ:
      os.environ['GN_MSVS_VERSION'] = GetVisualStudioVersion()

    # When using an installed toolchain these files aren't needed in the output
    # directory in order to run binaries locally, but they are needed in order
    # to create isolates or the mini_installer. Copying them to the output
    # directory ensures that they are available when needed.
    bitness = platform.architecture()[0]
    # When running 64-bit python the x64 DLLs will be in System32
    x64_path = 'System32' if bitness == '64bit' else 'Sysnative'
    x64_path = os.path.join(r'C:\Windows', x64_path)
    vs_runtime_dll_dirs = [x64_path, r'C:\Windows\SysWOW64']

  return vs_runtime_dll_dirs


def _RegistryGetValueUsingWinReg(key, value):
  """Use the _winreg module to obtain the value of a registry key.

  Args:
    key: The registry key.
    value: The particular registry value to read.
  Return:
    contents of the registry key's value, or None on failure.  Throws
    ImportError if _winreg is unavailable.
  """
  import _winreg
  try:
    root, subkey = key.split('\\', 1)
    assert root == 'HKLM'  # Only need HKLM for now.
    with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, subkey) as hkey:
      return _winreg.QueryValueEx(hkey, value)[0]
  except WindowsError:
    return None


def _RegistryGetValue(key, value):
  try:
    return _RegistryGetValueUsingWinReg(key, value)
  except ImportError:
    raise Exception('The python library _winreg not found.')


def GetVisualStudioVersion():
  """Return GN_MSVS_VERSION of Visual Studio.
  """
  return os.environ.get('GN_MSVS_VERSION', CURRENT_DEFAULT_TOOLCHAIN_VERSION)


def DetectVisualStudioPath():
  """Return path to the GN_MSVS_VERSION of Visual Studio.
  """

  # Note that this code is used from
  # Build/Toolchain/Win/SetupToolchain.py as well.
  version_as_year = GetVisualStudioVersion()
  assert version_as_year == '2017'
  year_to_version = {
      '2017': '15.0',
  }
  # The VC++ 2017 install location needs to be located using COM instead of
  # the registry. For details see:
  # https://blogs.msdn.microsoft.com/heaths/2016/09/15/changes-to-visual-studio-15-setup/
  # For now we use a hardcoded default with an environment variable override.
  for path in (
    os.environ.get('vs2017_install'),
    r'C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional',
    r'C:\Program Files (x86)\Microsoft Visual Studio\2017\Community'):
    if path and os.path.exists(path):
      return path

  raise Exception(('Visual Studio Version %s (from GN_MSVS_VERSION)'
                   ' not found.') % (version_as_year))


def _CopyRuntimeImpl(target, source, verbose=True):
  """Copy |source| to |target| if it doesn't already exist or if it needs to be
  updated (comparing last modified time as an approximate float match as for
  some reason the values tend to differ by ~1e-07 despite being copies of the
  same file...).
  """
  if (os.path.isdir(os.path.dirname(target)) and
      (not os.path.isfile(target) or
       abs(os.stat(target).st_mtime - os.stat(source).st_mtime) >= 0.01)):
    if verbose:
      print 'Copying %s to %s...' % (source, target)
    if os.path.exists(target):
      # Make the file writable so that we can delete it now.
      os.chmod(target, stat.S_IWRITE)
      os.unlink(target)
    shutil.copy2(source, target)
    # Make the file writable so that we can overwrite or delete it later.
    os.chmod(target, stat.S_IWRITE)


def _CopyUCRTRuntime(target_dir, source_dir, target_cpu, dll_pattern, suffix):
  """Copy both the msvcp and vccorlib runtime DLLs, only if the target doesn't
  exist, but the target directory does exist."""
  for file_part in ('msvcp', 'vccorlib', 'vcruntime'):
    dll = dll_pattern % file_part
    target = os.path.join(target_dir, dll)
    source = os.path.join(source_dir, dll)
    _CopyRuntimeImpl(target, source)
  # Copy the UCRT files needed by VS 2015 from the Windows SDK. This location
  # includes the api-ms-win-crt-*.dll files that are not found in the Windows
  # directory. These files are needed for component builds.
  # If WINDOWSSDKDIR is not set use the default SDK path. This will be the case
  # when DEPOT_TOOLS_WIN_TOOLCHAIN=0 and vcvarsall.bat has not been run.
  win_sdk_dir = os.path.normpath(
      os.environ.get('WINDOWSSDKDIR',
                     'C:\\Program Files (x86)\\Windows Kits\\10'))
  ucrt_dll_dirs = os.path.join(win_sdk_dir, r'Redist\ucrt\DLLs', target_cpu)
  ucrt_files = glob.glob(os.path.join(ucrt_dll_dirs, 'api-ms-win-*.dll'))
  assert len(ucrt_files) > 0
  for ucrt_src_file in ucrt_files:
    file_part = os.path.basename(ucrt_src_file)
    ucrt_dst_file = os.path.join(target_dir, file_part)
    _CopyRuntimeImpl(ucrt_dst_file, ucrt_src_file, False)
  _CopyRuntimeImpl(os.path.join(target_dir, 'ucrtbase' + suffix),
                    os.path.join(source_dir, 'ucrtbase' + suffix))


def _CopyRuntime(target_dir, source_dir, target_cpu, debug):
  """In VS2017 the PGO runtime dependencies are located in
  {toolchain_root}/VC/Tools/MSVC/{x.y.z}/bin/Host{target_cpu}/{target_cpu}/, the
  {version_number} part is likely to change in case of a minor update of the
  toolchain so we don't hardcode this value here (except for the major number).

  This returns the '{toolchain_root}/VC/Tools/MSVC/{x.y.z}/bin/' path.

  This function should only be called when using VS2017.
  """
  assert GetVisualStudioVersion() == '2017'
  SetEnvironmentAndGetRuntimeDllDirs()
  assert ('GN_MSVS_OVERRIDE_PATH' in os.environ)
  vc_tools_msvc_root = os.path.join(os.environ['GN_MSVS_OVERRIDE_PATH'],
      'VC', 'Tools', 'MSVC')
  for directory in os.listdir(vc_tools_msvc_root):
    if not os.path.isdir(os.path.join(vc_tools_msvc_root, directory)):
      continue
    if re.match('14\.\d+\.\d+', directory):
      return os.path.join(vc_tools_msvc_root, directory, 'bin')
  raise Exception('Unable to find the VC tools directory.')


def CopyDlls(target_dir, configuration, target_cpu):
  """Copy the VS runtime DLLs into the requested directory as needed.

  configuration is one of 'Debug' or 'Release'.
  target_cpu is one of 'x86' or 'x64'.

  The debug configuration gets both the debug and release DLLs; the
  release config only the latter.
  """
  vs_runtime_dll_dirs = SetEnvironmentAndGetRuntimeDllDirs()
  if not vs_runtime_dll_dirs:
    return

  x64_runtime, x86_runtime = vs_runtime_dll_dirs
  runtime_dir = x64_runtime if target_cpu == 'x64' else x86_runtime
  _CopyRuntime(target_dir, runtime_dir, target_cpu, debug=False)
  if configuration == 'Debug':
    _CopyRuntime(target_dir, runtime_dir, target_cpu, debug=True)

  _CopyDebugger(target_dir, target_cpu)


def _CopyDebugger(target_dir, target_cpu):
  """Copy dbghelp.dll into the requested directory as needed.

  target_cpu is one of 'x86' or 'x64'.

  dbghelp.dll is used when application needs to symbolize stacks. Copying this file
  from the SDK directory avoids using the system copy of dbghelp.dll which then
  ensures compatibility with recent debug information formats, such as VS
  2017 /debug:fastlink PDBs.
  """
  win_sdk_dir = SetEnvironmentAndGetSDKDir()
  if not win_sdk_dir:
    return

  debug_file = 'dbghelp.dll'
  full_path = os.path.join(win_sdk_dir, 'Debuggers', target_cpu, debug_file)
  if not os.path.exists(full_path):
    raise Exception('dbghelp.dll not found in "%s"\r\nYou must install the '
                    '"Debugging Tools for Windows" feature from the Windows '
                    '10 SDK.' % full_path)
  target_path = os.path.join(target_dir, debug_file)
  _CopyRuntimeImpl(target_path, full_path)


def NormalizePath(path):
  while path.endswith("\\"):
    path = path[:-1]
  return path


def SetEnvironmentAndGetSDKDir():
  """Gets location information about the current sdk (must have been
  previously updated by 'update'). This is used for the GN build."""
  SetEnvironmentAndGetRuntimeDllDirs()

  # If WINDOWSSDKDIR is not set, search the default SDK path and set it.
  if not 'WINDOWSSDKDIR' in os.environ:
    default_sdk_path = 'C:\\Program Files (x86)\\Windows Kits\\10'
    if os.path.isdir(default_sdk_path):
      os.environ['WINDOWSSDKDIR'] = default_sdk_path

  return NormalizePath(os.environ['WINDOWSSDKDIR'])


def GetToolchainDir():
  """Gets location information about the current toolchain (must have been
  previously updated by 'update'). This is used for the GN build."""
  runtime_dll_dirs = SetEnvironmentAndGetRuntimeDllDirs()
  win_sdk_dir = SetEnvironmentAndGetSDKDir()

  print '''vs_path = "%s"
sdk_path = "%s"
vs_version = "%s"
wdk_dir = "%s"
runtime_dirs = "%s"
''' % (
      NormalizePath(os.environ['GN_MSVS_OVERRIDE_PATH']),
      win_sdk_dir,
      GetVisualStudioVersion(),
      NormalizePath(os.environ.get('WDK_DIR', '')),
      os.path.pathsep.join(runtime_dll_dirs or ['None']))


def main():
  commands = {
      'get_toolchain_dir': GetToolchainDir,
      'copy_dlls': CopyDlls,
  }
  if len(sys.argv) < 2 or sys.argv[1] not in commands:
    print >>sys.stderr, 'Expected one of: %s' % ', '.join(commands)
    return 1
  return commands[sys.argv[1]](*sys.argv[2:])


if __name__ == '__main__':
  sys.exit(main())
