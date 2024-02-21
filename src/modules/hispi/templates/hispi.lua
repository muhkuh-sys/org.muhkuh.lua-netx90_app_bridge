local class = require 'pl.class'
local HiSpiSequence = class()

function HiSpiSequence:_init(tHiSpi, tLog)
  self.tHiSpi = tHiSpi
  self.tLog = tLog

  -- Get the LUA version number in the form major * 100 + minor .
  local strMaj, strMin = string.match(_VERSION, '^Lua (%d+)%.(%d+)$')
  if strMaj~=nil then
    self.LUA_VER_NUM = tonumber(strMaj) * 100 + tonumber(strMin)
  end

  if self.LUA_VER_NUM==501 then
    local vstruct = require "vstruct"
    self.tStructureReadRegister16 = vstruct.compile([[
      ucCommand:u1
      ucNetIolNode:u1
      usAddress:u2
    ]])

    self.tStructureWriteRegister16 = vstruct.compile([[
      ucCommand:u1
      ucNetIolNode:u1
      usAddress:u2
      usData:u2
    ]])
  end

  self.tSequence = { readsize = 0 }
end


function HiSpiSequence:readRegister16(ucNetIolNode, usAddress)
  local strBin
  if self.LUA_VER_NUM==501 then
    strBin = self.tStructureReadRegister16:write{
      ucCommand = self.tHiSpi.HISPI_COMMAND_ReadRegister16,
      ucNetIolNode = ucNetIolNode,
      usAddress = usAddress
    }
  else
    strBin = string.pack('<I1I1I2', self.tHiSpi.HISPI_COMMAND_ReadRegister16, ucNetIolNode, usAddress)
  end

  local tSequence = self.tSequence
  table.insert(tSequence, strBin)
  tSequence.readsize = tSequence.readsize + 2
end


function HiSpiSequence:writeRegister16(ucNetIolNode, usAddress, usData)
  local strBin
  if self.LUA_VER_NUM==501 then
    strBin = self.tStructureWriteRegister16:write{
      ucCommand = self.tHiSpi.HISPI_COMMAND_WriteRegister16,
      ucNetIolNode = ucNetIolNode,
      usAddress = usAddress,
      usData = usData
    }
  else
    strBin = string.pack('<I1I1I2I2', self.tHiSpi.HISPI_COMMAND_WriteRegister16, ucNetIolNode, usAddress, usData)
  end

  local tSequence = self.tSequence
  table.insert(tSequence, strBin)
end


function HiSpiSequence:run()
  local tSequence = self.tSequence

  -- Get the sequence data.
  local strSequence = table.concat(tSequence)
  -- Get the size of the result data.
  local sizResultData = tSequence.readsize

  return self.tHiSpi:__sequence_run(strSequence, sizResultData)
end



local AppBridgeModuleHiSpi = class()

function AppBridgeModuleHiSpi:_init(tAppBridge, tLog)
  self.pl = require'pl.import_into'()

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
  self.HISPI_COMMAND_RunSequence = ${HISPI_COMMAND_RunSequence}
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
    tLog.error('Failed to initialize the HiSPI: %s', tostring(ulValue))
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
  return (ulValue & 0xffff)
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


function AppBridgeModuleHiSpi:sequence_create()
  return HiSpiSequence(self, self.tLog)
end


function AppBridgeModuleHiSpi:__sequence_run(strSequence, sizResultData)
  local tLog = self.tLog
  local tAppBridge = self.tAppBridge
  local tResult

  local sizSequence = string.len(strSequence)
  if sizSequence==0 then
    tLog.debug('Ignoring empty sequence.')
    tResult = true
  else
    -- Copy the sequence to the APP buffer.
    tResult = tAppBridge:write_area(self.ulModuleBufferArea, strSequence)
    if tResult~=true then
      tLog.error('Failed to write the sequence data.')
      error('Failed to write the sequence data.')
    else
      -- Run the sequence.
      local ulResult = tAppBridge:call(self.ulModuleExecAddress, self.HISPI_COMMAND_RunSequence, sizSequence)
      if ulResult~=0 then
        tLog.error('Failed to execute the sequence : %d', ulResult)
        error('Failed to execute the sequence.')
      else
        if sizResultData==0 then
          tResult = true
        else
          tResult = tAppBridge:read_area(self.ulModuleBufferArea, sizResultData)
        end
      end
    end
  end

  return tResult
end


return AppBridgeModuleHiSpi
