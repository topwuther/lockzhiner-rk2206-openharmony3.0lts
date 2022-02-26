import regeneratorRuntime from 'babel-runtime/regenerator'

import {
  mockAbilityFeatureAbility,
  mockAbilityParticleAbility,
  mockAbilityFormManager
} from './ability'

import { mockBattery } from './battery'
import { mockBrightness } from './brightness'
import { mockBluetooth } from './bluetooth'
import { mockCall } from './telephony'
import { mockCipher } from './cipher'
import { mockDataStorage } from './data'
import { mockDeviceInfo } from './deviceInfo'
import { mockDistributedSchedule } from './distributedSchedule'
import { mockDocument } from './document'
import { mockFetch } from './fetch'
import { mockFeatureAbility } from './featureAbility'
import { mockFile } from './file'
import { mockGeolocation } from './geolocation'
import { mockInputmethod } from './inputmethod'

import {
  mockMultimediaImage,
  mockMultimediaMedia,
  mockMultimediaMedialibrary
} from './multimedia'
import { mockNetwork } from './network'
import { mockNfcCardEmulation } from './nfc'
import { mockNotification } from './notification'
import { mockOhosBatteryinfo } from './batteryinfo'
import { mockOhosRequest } from './ohos/request'

import { mockPasteBoard } from './pasteboard'
import { mockReminderAgent } from './ohos/reminderAgent'
import { mockRequest } from './request'

import { mockSensor } from './sensor'
import { mockStorage } from './storage'
import { mockSystemPackage } from './systemPackage'

import { mockUserauth } from './userIAM'
import { mockVibrator } from './vibrator'

import { mockWebSocket, mockHttp } from './net'
import { mockWifi } from './wifi'
import { mockWindow } from './window'

import { mockRequireNapiFun } from './napi'
import { mockAccessibility } from './accessibility'
import { mockAI } from './ai'
import { mockDisplay } from './display'
import { mockDeviceManager } from './deviceManager'

export function mockSystemPlugin() {
  global.regeneratorRuntime = regeneratorRuntime

  global.systemplugin = {}
  global.ohosplugin = {}

  mockNotification()
  mockFetch()
  mockRequest()
  mockStorage()
  mockFile()
  mockVibrator()
  mockSensor()
  mockGeolocation()
  mockNetwork()
  mockBrightness()
  mockBattery()
  mockSystemPackage()
  mockFeatureAbility()
  mockBluetooth()
  mockCipher()
  mockOhosBatteryinfo()
  mockUserauth()
  mockDistributedSchedule()
  mockDocument()
  mockPasteBoard()
  mockInputmethod()
  mockOhosRequest()
  mockMultimediaImage()
  mockMultimediaMedia()
  mockMultimediaMedialibrary()
  mockCall()
  mockWebSocket()
  mockHttp()
  mockDeviceInfo()
  mockWifi()
  mockNfcCardEmulation()
  mockDataStorage()
  mockWindow()
  mockAbilityFeatureAbility()
  mockReminderAgent()
  mockRequireNapiFun()
  mockAccessibility()
  mockAbilityParticleAbility()
  mockAbilityFormManager()
  mockAI()
  mockDisplay()
  mockDeviceManager()
}
