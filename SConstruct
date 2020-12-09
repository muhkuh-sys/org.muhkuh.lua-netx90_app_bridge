# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------- #
#   Copyright (C) 2011 by Christoph Thelen                                #
#   doc_bacardi@users.sourceforge.net                                     #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
# ----------------------------------------------------------------------- #

import os

# -----------------------------------------------------------------------
#
# Set up the Muhkuh Build System.
#
SConscript('mbs/SConscript')
Import('atEnv')

# Create a build environment for the ARM9 based netX chips.
#env_arm9 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.7', 'asciidoc'])
#env_arm9.CreateCompilerEnv('NETX500', ['arch=armv5te'])
#env_arm9.CreateCompilerEnv('NETX56', ['arch=armv5te'])
#env_arm9.CreateCompilerEnv('NETX50', ['arch=armv5te'])
#env_arm9.CreateCompilerEnv('NETX10', ['arch=armv5te'])

# Create a build environment for the Cortex-R7 and Cortex-A9 based netX chips.
#env_cortexR7 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
#env_cortexR7.CreateCompilerEnv('NETX4000_RELAXED', ['arch=armv7', 'thumb'], ['arch=armv7-r', 'thumb'])

# Create a build environment for the Cortex-M4 based netX chips.
env_cortexM4 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
env_cortexM4.CreateCompilerEnv('NETX90', ['arch=armv7', 'thumb'], ['arch=armv7e-m', 'thumb'])
env_cortexM4.CreateCompilerEnv('NETX90_APP', ['arch=armv7', 'thumb'], ['arch=armv7e-m', 'thumb'])

# Build the platform libraries.
SConscript('platform/SConscript')


# -----------------------------------------------------------------------
#
# Get the source code version from the VCS.
#
atEnv.DEFAULT.Version('#targets/version/version.h', 'templates/version.h')


# -----------------------------------------------------------------------
# This is the list of sources. The elements must be separated with whitespace
# (i.e. spaces, tabs, newlines). The amount of whitespace does not matter.
sources_lib_com = """
    src/lib_com/app_bridge.c
"""

sources_com = """
    src/com/header.c
    src/com/init_muhkuh.S
    src/com/main.c
"""

sources_app = """
    src/app/app_hboot_header_iflash.c
    src/app/cm4_app_vector_table_iflash.c
    src/app/header.c
    src/app/init.S
    src/app/main.c
"""

sources_module_hispi = """
    src/modules/hispi/boot_drv_spi.c
    src/modules/hispi/boot_spi.c
    src/modules/hispi/crc.c
    src/modules/hispi/init_module.S
    src/modules/hispi/main_module.c
    src/modules/hispi/pad_control.c
"""

# -----------------------------------------------------------------------
#
# Build all files.
#

# The list of include folders. Here it is used for all files.
astrIncludePaths = ['src', '#platform/src', '#platform/src/lib', '#targets/version', 'targets/netx90_module_hispi/include']

# This is the bridge on the APP.
tEnvApp = atEnv.NETX90_APP.Clone()
tEnvApp.Append(CPPPATH = astrIncludePaths)
tEnvApp.Replace(LDFILE = 'src/app/netx90/netx90_app.ld')
tSrcApp = tEnvApp.SetBuildPath('targets/netx90_app', 'src', sources_app)
tElfApp = tEnvApp.Elf('targets/netx90_app/netx90_app_bridge.elf', tSrcApp + tEnvApp['PLATFORM_LIBRARY'])
tTxtApp = tEnvApp.ObjDump('targets/netx90_app/netx90_app_bridge.txt', tElfApp, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
tBinApp = tEnvApp.ObjCopy('targets/netx90_app/netx90_app_bridge.bin', tElfApp)
tImgApp = tEnvApp.IFlashImage('targets/netx90_app/netx90_app_bridge.img', tBinApp)

# This is an extension module for the bridge providing HiSPI routines.
tEnvModuleHispi = atEnv.NETX90_APP.Clone()
tEnvModuleHispi.Append(CPPPATH = astrIncludePaths)
tEnvModuleHispi.Replace(LDFILE = 'src/modules/hispi/netx90/netx90_app_module.ld')
tSrcModuleHispi = tEnvModuleHispi.SetBuildPath('targets/netx90_module_hispi', 'src', sources_module_hispi)
tElfModuleHispi = tEnvModuleHispi.Elf('targets/netx90_module_hispi/netx90_module_hispi.elf', tSrcModuleHispi + tEnvModuleHispi['PLATFORM_LIBRARY'])
tTxtModuleHispi = tEnvModuleHispi.ObjDump('targets/netx90_module_hispi/netx90_module_hispi.txt', tElfModuleHispi, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
tBinModuleHispi = tEnvModuleHispi.ObjCopy('targets/netx90_module_hispi/netx90_module_hispi.bin', tElfModuleHispi)
tIncModuleHispi = tEnvModuleHispi.GccSymbolTemplate('targets/netx90_module_hispi/include/hispi.h', tElfModuleHispi, GCCSYMBOLTEMPLATE_TEMPLATE=File('src/modules/hispi/templates/hispi.h'))

# Build the library for the COM side.
tEnvLibCom = atEnv.NETX90.Clone()
tEnvLibCom.Append(CPPPATH = astrIncludePaths)
tObjApp = tEnvLibCom.ObjImport('targets/netx90_app/netx90_app_bridge.obj', tImgApp)
tObjModuleHispi = tEnvLibCom.ObjImport('targets/netx90_module_hispi/netx90_module_hispi.obj', tBinModuleHispi)
tSrcLibCom = tEnvLibCom.SetBuildPath('targets/netx90_lib_com', 'src', sources_lib_com)
tLibCom = tEnvLibCom.StaticLibrary('targets/netx90_app_bridge_com.a', tSrcLibCom + tObjApp + tObjModuleHispi)

# This is the controller for the COM side.
tEnvCom = atEnv.NETX90.Clone()
tEnvCom.Append(CPPPATH = astrIncludePaths)
tEnvCom.Replace(LDFILE = 'src/com/netx90/netx90_com_intram.ld')
tSrcCom = tEnvCom.SetBuildPath('targets/netx90_com', 'src', sources_com)
tElfCom = tEnvCom.Elf('targets/netx90_app_bridge_com.elf', tSrcCom + tEnvCom['PLATFORM_LIBRARY'] + tLibCom)
tTxtCom = tEnvCom.ObjDump('targets/netx90_app_bridge_com.txt', tElfCom, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
BRIDGE_NETX90_COM = tEnvCom.ObjCopy('targets/netx90_app_bridge_com.bin', tElfCom)

BRIDGE_NETX90_LUA = atEnv.NETX90.GccSymbolTemplate('targets/lua/app_bridge.lua', tElfCom, GCCSYMBOLTEMPLATE_TEMPLATE=File('templates/app_bridge.lua'))
BRIDGE_MODULE_HISPI = atEnv.NETX90.GccSymbolTemplate('targets/lua/app_bridge/modules/hispi.lua', tElfModuleHispi, GCCSYMBOLTEMPLATE_TEMPLATE=File('src/modules/hispi/templates/hispi.lua'))


# -----------------------------------------------------------------------
#
# Build the documentation.
#
# Get the default attributes.
aAttribs = atEnv.DEFAULT['ASCIIDOC_ATTRIBUTES']
# Add some custom attributes.
aAttribs.update(dict({
    # Use ASCIIMath formulas.
    'asciimath': True,

    # Embed images into the HTML file as data URIs.
    'data-uri': True,

    # Use icons instead of text for markers and callouts.
    'icons': True,

    # Use numbers in the table of contents.
    'numbered': True,

    # Generate a scrollable table of contents on the left of the text.
    'toc2': True,

    # Use 4 levels in the table of contents.
    'toclevels': 4
}))

doc = atEnv.DEFAULT.Asciidoc('targets/doc/netx90_app_bridge.html', 'doc/netx90_app_bridge.asciidoc', ASCIIDOC_BACKEND='html5', ASCIIDOC_ATTRIBUTES=aAttribs)


# ---------------------------------------------------------------------------
#
# Build an archive.
#
strGroup = 'org.muhkuh.lua'
strModule = 'netx90_app_bridge'

# Split the group by dots.
aGroup = strGroup.split('.')
# Build the path for all artifacts.
strModulePath = 'targets/jonchki/repository/%s/%s/%s' % ('/'.join(aGroup), strModule, PROJECT_VERSION)

strArtifact = 'netx90_app_bridge'

tArcList = atEnv.DEFAULT.ArchiveList('zip')

tArcList.AddFiles('doc/',
                  doc)

tArcList.AddFiles('netx/',
                  BRIDGE_NETX90_COM,
                  tBinModuleHispi)

tArcList.AddFiles('lua/',
                  BRIDGE_NETX90_LUA)

tArcList.AddFiles('lua/app_bridge/modules/',
                  BRIDGE_MODULE_HISPI)

tArcList.AddFiles('dev/include/',
                  'src/lib_com/app_bridge.h')

tArcList.AddFiles('dev/lib/',
                  tLibCom)


tArcList.AddFiles('',
                  'installer/%s-%s/install.lua' % (strGroup, strModule))


strBasePath = os.path.join(strModulePath, '%s-%s' % (strArtifact, PROJECT_VERSION))
tArtifact = atEnv.DEFAULT.Archive('%s.zip' % strBasePath, None, ARCHIVE_CONTENTS = tArcList)
tArtifactHash = atEnv.DEFAULT.Hash('%s.hash' % tArtifact[0].get_path(), tArtifact[0].get_path(), HASH_ALGORITHM='md5,sha1,sha224,sha256,sha384,sha512', HASH_TEMPLATE='${ID_UC}:${HASH}\n')
tConfiguration = atEnv.DEFAULT.Version('%s.xml' % strBasePath, 'installer/%s-%s/%s.xml' % (strGroup, strModule, strArtifact))
tConfigurationHash = atEnv.DEFAULT.Hash('%s.hash' % tConfiguration[0].get_path(), tConfiguration[0].get_path(), HASH_ALGORITHM='md5,sha1,sha224,sha256,sha384,sha512', HASH_TEMPLATE='${ID_UC}:${HASH}\n')
tPom = atEnv.DEFAULT.ArtifactVersion('%s.pom' % strBasePath, 'installer/%s-%s/%s.pom' % (strGroup, strModule, strArtifact))


# -----------------------------------------------------------------------
#
# Install the files to the testbench.
#
atFiles = {
    'targets/testbench/netx/netx90_app_bridge_com.bin':           BRIDGE_NETX90_COM,
    'targets/testbench/lua/app_bridge.lua':                       BRIDGE_NETX90_LUA,
    'targets/testbench/lua/app_bridge/modules/hispi.lua':         BRIDGE_MODULE_HISPI,
    'targets/testbench/netx/netx90_module_hispi.bin':             tBinModuleHispi
}
for tDst, tSrc in atFiles.items():
    Command(tDst, tSrc, Copy("$TARGET", "$SOURCE"))
