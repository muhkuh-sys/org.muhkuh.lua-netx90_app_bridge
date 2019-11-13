local class = require 'pl.class'
local AppBridge = class()


function AppBridge:_init(tPlugin)
  self.BRIDGE_COMMAND_Initialize = ${BRIDGE_COMMAND_Initialize}
  self.BRIDGE_COMMAND_Identify = ${BRIDGE_COMMAND_Identify}
  self.BRIDGE_COMMAND_ReadRegister = ${BRIDGE_COMMAND_ReadRegister}
  self.BRIDGE_COMMAND_ReadArea = ${BRIDGE_COMMAND_ReadArea}
  self.BRIDGE_COMMAND_WriteRegister = ${BRIDGE_COMMAND_WriteRegister}
  self.BRIDGE_COMMAND_WriteArea = ${BRIDGE_COMMAND_WriteArea}
  self.BRIDGE_COMMAND_Call = ${BRIDGE_COMMAND_Call}

  self.__tPlugin = tPlugin
  self.__aAttr = nil
end



function AppBridge:initialize()
  local tResult
  local tPlugin = self.__tPlugin
  local aAttr = tester.mbin_open('netx/netx90_app_bridge_com_demo.bin', tPlugin)
  tester.mbin_debug(aAttr)
  tester.mbin_write(nil, tPlugin, aAttr)

  -- Initialize the application.
  local aParameter = 0
  tester.mbin_set_parameter(tPlugin, aAttr, aParameter)
  local ulValue = tester.mbin_execute(nil, tPlugin, aAttr, aParameter)
  if ulValue~=0 then
    print('Failed to initialize the bridge.')
  else
    -- Initialize the DPM.
    local aParameter = {
      self.BRIDGE_COMMAND_Initialize
    }
    tester.mbin_set_parameter(tPlugin, aAttr, aParameter)
    ulValue = tester.mbin_execute(nil, tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print('Failed to initialize the bridge DPM.')
    else
      self.__aAttr = aAttr
      tResult = true
    end
  end

  return tResult
end


function AppBridge:identify()
  local tResult


  local aAttr = self.__aAttr
  local tPlugin = self.__tPlugin
  if aAttr==nil then
    print('Not Initialized.')
  elseif tPlugin==nil then
    print('No plugin.')
  else
    -- Identify the bridge.
    local aParameter = {
      self.BRIDGE_COMMAND_Identify
    }
    tester.mbin_set_parameter(tPlugin, aAttr, aParameter)
    ulValue = tester.mbin_execute(nil, tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print('Failed to identify the bridge DPM.')
    else
      tResult = true
    end
  end

  return tResult
end


function AppBridge:read_register(ulAddress)
  local tResult


  local aAttr = self.__aAttr
  local tPlugin = self.__tPlugin
  if aAttr==nil then
    print('Not Initialized.')
  elseif tPlugin==nil then
    print('No plugin.')
  else
    -- Read a register.
    local aParameter = {
      self.BRIDGE_COMMAND_ReadRegister,
      ulAddress,
      'OUTPUT'
    }
    tester.mbin_set_parameter(tPlugin, aAttr, aParameter)
    ulValue = tester.mbin_execute(nil, tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print(string.format('Failed to read the register 0x%08x.', ulAddress))
    else
      tResult = aParameter[3]
    end
  end

  return tResult
end


function AppBridge:read_area(ulAddress, ulSizeInBytes)
  local tResult


  local aAttr = self.__aAttr
  local tPlugin = self.__tPlugin
  if aAttr==nil then
    print('Not Initialized.')
  elseif tPlugin==nil then
    print('No plugin.')
  else
    -- Read a register.
    local aParameter = {
      self.BRIDGE_COMMAND_ReadArea,
      ulAddress,
      ulSizeInBytes
    }
    tester.mbin_set_parameter(tPlugin, aAttr, aParameter)
    ulValue = tester.mbin_execute(nil, tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print(string.format('Failed to read the area 0x%08x-0x%08x .', ulAddress, ulAddress+ulSizeInBytes))
    else
      tResult = tPlugin:read_image(aAttr.ulParameterStartAddress+0x18, ulSizeInBytes, tester.callback_progress, ulSizeInBytes)
    end
  end

  return tResult
end


function AppBridge:write_register(ulAddress, ulData)
  local tResult


  local aAttr = self.__aAttr
  local tPlugin = self.__tPlugin
  if aAttr==nil then
    print('Not Initialized.')
  elseif tPlugin==nil then
    print('No plugin.')
  else
    -- Read a register.
    local aParameter = {
      self.BRIDGE_COMMAND_WriteRegister,
      ulAddress,
      ulData
    }
    tester.mbin_set_parameter(tPlugin, aAttr, aParameter)
    ulValue = tester.mbin_execute(nil, tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print(string.format('Failed to write the register 0x%08x.', ulAddress))
    else
      tResult = aParameter[3]
    end
  end

  return tResult
end


function AppBridge:write_area(ulAddress, strData)
  local tResult


  local aAttr = self.__aAttr
  local tPlugin = self.__tPlugin
  if aAttr==nil then
    print('Not Initialized.')
  elseif tPlugin==nil then
    print('No plugin.')
  else
    -- Write an area.
    local sizData = string.len(strData)
    local aParameter = {
      self.BRIDGE_COMMAND_WriteArea,
      ulAddress,
      sizData
    }
    tester.mbin_set_parameter(tPlugin, aAttr, aParameter)
    tPlugin:write_image(aAttr.ulParameterStartAddress+0x18, strData, tester.callback_progress, sizData)
    ulValue = tester.mbin_execute(nil, tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print(string.format('Failed to write the area 0x%08x-0x%08x .', ulAddress, ulAddress+sizData))
    else
      tResult = true
    end
  end

  return tResult
end


return AppBridge
