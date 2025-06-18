local class = require 'pl.class'
local AppBridge = class()


AppBridge.__atErrorMessages = {
  [${APP_BRIDGE_RESULT_Ok}]  = 'OK',
  [${APP_BRIDGE_RESULT_InvalidDpmAddress}]  = 'Invalid DPM address',
  [${APP_BRIDGE_RESULT_DpmClocksMaskedOut}]  = 'The DPM clocks can not be enabled as they are masked out.',
  [${APP_BRIDGE_RESULT_AppClocksMaskedOut}]  = 'The APP CPU can not be enabled as the clock is masked out.',
  [${APP_BRIDGE_RESULT_AppFirmwareExceedsFirstSector}]  = 'The APP firmware exceeds the first flash sector.',
  [${APP_BRIDGE_RESULT_AppFirmwareUpdateNeededWhileAppRunning}]  = 'The APP firmware must be updated, but the APP CPU is running.',
  [${APP_BRIDGE_RESULT_AppFirmwareUpdateFlashFailed}]  = 'Failed to update the APP firmware in INTFLASH2.',
  [${APP_BRIDGE_RESULT_TransferTimeout}]  = 'Transfer timeout.',
  [${APP_BRIDGE_RESULT_AppStatusInvalidCommand}]  = 'Invalid command.',
  [${APP_BRIDGE_RESULT_AppStatusUnalignedAddress}]  = 'Unaligned address.',
  [${APP_BRIDGE_RESULT_AppStatusLengthTooLarge}] = 'The requested length exceeds the limits of the buffer.',
  [${APP_BRIDGE_RESULT_AppStatusCallRunning}] = 'A "call" command is running.',
  [${APP_BRIDGE_RESULT_AppStatusIdle}] = 'Idle.',
  [${APP_BRIDGE_RESULT_AppStatusUnknown}] = 'Unknown.'
}


function AppBridge:_init(tPlugin)
  self.BRIDGE_COMMAND_Initialize = ${BRIDGE_COMMAND_Initialize}
  self.BRIDGE_COMMAND_Identify = ${BRIDGE_COMMAND_Identify}
  self.BRIDGE_COMMAND_ReadRegister = ${BRIDGE_COMMAND_ReadRegister}
  self.BRIDGE_COMMAND_ReadArea = ${BRIDGE_COMMAND_ReadArea}
  self.BRIDGE_COMMAND_WriteRegister = ${BRIDGE_COMMAND_WriteRegister}
  self.BRIDGE_COMMAND_WriteRegisterUnlock = ${BRIDGE_COMMAND_WriteRegisterUnlock}
  self.BRIDGE_COMMAND_WriteArea = ${BRIDGE_COMMAND_WriteArea}
  self.BRIDGE_COMMAND_Call = ${BRIDGE_COMMAND_Call}

  self.__tPlugin = tPlugin
  self.__aAttr = nil
end



function AppBridge:__get_error_message(ulCode)
  local strErrorMsg = self.__atErrorMessages[ulCode]
  if strErrorMsg==nil then
    strErrorMsg = 'Unknown errorcode: ' .. tostring(ulCode)
  end
  return strErrorMsg
end



function AppBridge:initialize()
  local tResult
  local tPlugin = self.__tPlugin
  local tester = _G.tester
  local aAttr = tester:mbin_open('netx/netx90_app_bridge_com.bin', tPlugin)
  tester:mbin_debug(aAttr)
  tester:mbin_write(tPlugin, aAttr)

  -- Initialize the application.
  local aParameter = 0
  tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
  local ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
  if ulValue~=0 then
    print('Failed to initialize the bridge.')
  else
    -- Initialize the DPM.
    local aParameter = {
      self.BRIDGE_COMMAND_Initialize
    }
    tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
    ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print('Failed to initialize the bridge DPM: ' .. self:__get_error_message(ulValue))
    else
      self.__aAttr = aAttr
      tResult = true
    end
  end

  return tResult
end


function AppBridge:identify()
  local tResult
  local tester = _G.tester


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
    tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
    local ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print('Failed to identify the bridge DPM: ' .. self:__get_error_message(ulValue))
    else
      tResult = true
    end
  end

  return tResult
end


function AppBridge:read_register(ulAddress)
  local tResult
  local tester = _G.tester


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
    tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
    local ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print(
        string.format(
          'Failed to read the register 0x%08x: %s',
          ulAddress,
          self:__get_error_message(ulValue)
        )
      )
    else
      tResult = aParameter[3]
    end
  end

  return tResult
end


function AppBridge:read_area(ulAddress, ulSizeInBytes)
  local tResult
  local tester = _G.tester


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
    tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
    local ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print(
        string.format(
          'Failed to read the area 0x%08x-0x%08x : %s',
          ulAddress,
          ulAddress+ulSizeInBytes,
          self:__get_error_message(ulValue)
        )
      )
    else
      tResult = tPlugin:read_image(aAttr.ulParameterStartAddress+0x18, ulSizeInBytes, tester.callback_progress, ulSizeInBytes)
    end
  end

  return tResult
end


function AppBridge:write_register(ulAddress, ulData)
  local tResult
  local tester = _G.tester


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
    tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
    local ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print(
        string.format(
          'Failed to write the register 0x%08x: %s',
          ulAddress,
          self:__get_error_message(ulValue)
        )
      )
    else
      tResult = true
    end
  end

  return tResult
end


function AppBridge:write_register_unlock(ulAddress, ulData)
  local tResult
  local tester = _G.tester


  local aAttr = self.__aAttr
  local tPlugin = self.__tPlugin
  if aAttr==nil then
    print('Not Initialized.')
  elseif tPlugin==nil then
    print('No plugin.')
  else
    -- Read a register.
    local aParameter = {
      self.BRIDGE_COMMAND_WriteRegisterUnlock,
      ulAddress,
      ulData
    }
    tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
    local ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print(
        string.format(
          'Failed to write_unlock the register 0x%08x: %s',
          ulAddress,
          self:__get_error_message(ulValue)
        )
      )
    else
      tResult = true
    end
  end

  return tResult
end


function AppBridge:write_area(ulAddress, strData)
  local tResult
  local tester = _G.tester


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
    tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
    tPlugin:write_image(aAttr.ulParameterStartAddress+0x18, strData, tester.callback_progress, sizData)
    local ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print(
        string.format(
          'Failed to write the area 0x%08x-0x%08x : %s',
          ulAddress,
          ulAddress+sizData,
          self:__get_error_message(ulValue)
        )
      )
    else
      tResult = true
    end
  end

  return tResult
end


function AppBridge:call(ulAddress, ulR0, ulR1, ulR2, ulR3)
  ulR0 = ulR0 or 0
  ulR1 = ulR1 or 0
  ulR2 = ulR2 or 0
  ulR3 = ulR3 or 0
  local tResult
  local tester = _G.tester


  local aAttr = self.__aAttr
  local tPlugin = self.__tPlugin
  if aAttr==nil then
    print('Not Initialized.')
  elseif tPlugin==nil then
    print('No plugin.')
  else
    -- Read a register.
    local aParameter = {
      self.BRIDGE_COMMAND_Call,
      ulAddress,
      ulR0,
      ulR1,
      ulR2,
      ulR3,
      'OUTPUT'
    }
    tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
    local ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      print(
        string.format(
          'Failed to read the register 0x%08x : %s',
          ulAddress,
          self:__get_error_message(ulValue)
        )
      )
    else
      tResult = aParameter[7]
    end
  end

  return tResult
end

return AppBridge
