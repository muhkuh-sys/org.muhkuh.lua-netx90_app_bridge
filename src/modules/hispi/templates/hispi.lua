local class = require 'pl.class'
local AppBridgeModuleHiSpi = class()

function AppBridgeModuleHiSpi:_init(tAppBridge, tLog)
  self.pl = require'pl.import_into'()
  self.bit = require 'bit'

  self.tAppBridge = tAppBridge
  self.tLog = tLog

  -- This is the path to the module binary.
  self.strModulePath = 'netx/netx90_module_hispi.bin'

  -- TODO: Get this from the binary.
  self.ulModuleLoadAddress = 0x000B8000
  self.ulModuleExecAddress = 0x000B8001
  self.ulModuleBufferArea = 0x000BC000
  
  self.HISPI_COMMAND_Initialize = ${HISPI_COMMAND_Initialize}
  self.HISPI_COMMAND_ReadRegister16 = ${HISPI_COMMAND_ReadRegister16}
  self.HISPI_COMMAND_WriteRegister16 = ${HISPI_COMMAND_WriteRegister16}
end


function AppBridgeModuleHiSpi:initialize(ulNumberOfDevices)
  local tAppBridge = self.tAppBridge
  local tLog = self.tLog

  -- Download the HiSPI module.
  local tModuleData, strError = self.pl.utils.readfile(self.strModulePath, true)
  if tModuleData==nil then
    tLog.error('Failed to load the module from "%s": %s', self.strModulePath, strError)
    error('Failed to load module.')
  end
  tAppBridge:write_area(self.ulModuleLoadAddress, tModuleData)

  -- Initialize the HiSPI bus.
  print('HiSPI initialize')
  local ulValue = tAppBridge:call(self.ulModuleExecAddress, self.HISPI_COMMAND_Initialize, ulNumberOfDevices)
  if ulValue~=0 then
    tLog.error('Failed to initialize the HiSPI: %d', ulValue)
    error('Failed to initialize the HiSPI.')
  end
end


function AppBridgeModuleHiSpi:readRegister16(ucNetIolNode, usAddress)
  local tAppBridge = self.tAppBridge
  local tLog = self.tLog

  local ulResult = tAppBridge:call(self.ulModuleExecAddress, self.HISPI_COMMAND_ReadRegister16, ucNetIolNode, usAddress, self.ulModuleBufferArea)
  if ulResult~=0 then
    tLog.error('Failed to read netIOL%d[0x%04x] : %d', ucNetIolNode, usAddress, ulResult)
    error('Failed to read.')
  end
  local ulValue = tAppBridge:read_register(self.ulModuleBufferArea)
  return self.bit.band(ulValue, 0xffff)
end


function AppBridgeModuleHiSpi:writeRegister16(ucNetIolNode, usAddress, usData)
  local tAppBridge = self.tAppBridge
  local tLog = self.tLog

  local ulResult = tAppBridge:call(self.ulModuleExecAddress, self.HISPI_COMMAND_WriteRegister16, ucNetIolNode, usAddress, usData)
  if ulResult~=0 then
    tLog.error('Failed to write netIOL%d[0x%04x]=0x%04x : %d', ucNetIolNode, usAddress, usData, ulResult)
    error('Failed to read.')
  end
end



return AppBridgeModuleHiSpi
