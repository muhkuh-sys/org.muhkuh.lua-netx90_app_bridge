# -*- coding: utf-8 -*-
#-------------------------------------------------------------------------#
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
#-------------------------------------------------------------------------#


#----------------------------------------------------------------------------
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


#----------------------------------------------------------------------------
#
# Get the source code version from the VCS.
#
atEnv.DEFAULT.Version('#targets/version/version.h', 'templates/version.h')


#----------------------------------------------------------------------------
# This is the list of sources. The elements must be separated with whitespace
# (i.e. spaces, tabs, newlines). The amount of whitespace does not matter.
sources_com = """
	src/com/app_bridge.c
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

#----------------------------------------------------------------------------
#
# Build all files.
#

# The list of include folders. Here it is used for all files.
astrIncludePaths = ['src', '#platform/src', '#platform/src/lib', '#targets/version']

# This is the demo for the COM side.
tEnvCom = atEnv.NETX90.Clone()
tEnvCom.Append(CPPPATH = astrIncludePaths)
tEnvCom.Replace(LDFILE = 'src/com/netx90/netx90_com_intram.ld')
tSrcCom = tEnvCom.SetBuildPath('targets/netx90_com', 'src', sources_com)
tElfCom = tEnvCom.Elf('targets/netx90_app_bridge_com_demo.elf', tSrcCom + tEnvCom['PLATFORM_LIBRARY'])
tTxtCom = tEnvCom.ObjDump('targets/netx90_app_bridge_com_demo.txt', tElfCom, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
BRIDGE_NETX90_COM = tEnvCom.ObjCopy('targets/netx90_app_bridge_com_demo.bin', tElfCom)

# This is the bridge on the APP.
tEnvApp = atEnv.NETX90_APP.Clone()
tEnvApp.Append(CPPPATH = astrIncludePaths)
tEnvApp.Replace(LDFILE = 'src/app/netx90/netx90_app.ld')
tSrcApp = tEnvApp.SetBuildPath('targets/netx90_app', 'src', sources_app)
tElfApp = tEnvApp.Elf('targets/netx90_app/netx90_app_bridge.elf', tSrcApp + tEnvApp['PLATFORM_LIBRARY'])
tTxtApp = tEnvApp.ObjDump('targets/netx90_app/netx90_app_bridge.txt', tElfApp, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
tBinApp = tEnvApp.ObjCopy('targets/netx90_app/netx90_app_bridge.bin', tElfApp)
tImgApp = tEnvApp.IFlashImage('targets/netx90_app_bridge.img', tBinApp)

# This is an extension module for the bridge providing HiSPI routines.
tEnvModuleHispi = atEnv.NETX90_APP.Clone()
tEnvModuleHispi.Append(CPPPATH = astrIncludePaths)
tEnvModuleHispi.Replace(LDFILE = 'src/modules/hispi/netx90/netx90_app_module.ld')
tSrcModuleHispi = tEnvModuleHispi.SetBuildPath('targets/netx90_module_hispi', 'src', sources_module_hispi)
tElfModuleHispi = tEnvModuleHispi.Elf('targets/netx90_module_hispi/netx90_module_hispi.elf', tSrcModuleHispi + tEnvModuleHispi['PLATFORM_LIBRARY'])
tTxtModuleHispi = tEnvModuleHispi.ObjDump('targets/netx90_module_hispi/netx90_module_hispi.txt', tElfModuleHispi, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
tBinModuleHispi = tEnvModuleHispi.ObjCopy('targets/netx90_module_hispi/netx90_module_hispi.bin', tElfModuleHispi)

BRIDGE_NETX90_LUA = atEnv.NETX90.GccSymbolTemplate('targets/lua/app_bridge.lua', tElfCom, GCCSYMBOLTEMPLATE_TEMPLATE=File('templates/app_bridge.lua'))

# Install the files to the testbench.
atFiles = {
    'targets/testbench/netx/netx90_app_bridge_com_demo.bin':      BRIDGE_NETX90_COM,
    'targets/testbench/lua/app_bridge.lua':                       BRIDGE_NETX90_LUA
}
for tDst, tSrc in atFiles.iteritems():
    Command(tDst, tSrc, Copy("$TARGET", "$SOURCE"))


