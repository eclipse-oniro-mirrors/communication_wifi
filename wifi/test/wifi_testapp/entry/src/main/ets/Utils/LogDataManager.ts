/**
 * Copyright (c) 2022 Shenzhen Kaihong Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export class LogDataManager {
  TAG = 'WIFI_Manager_Test'
  //wifi
  testEnableWifiManager_StartLog = "testEnableWifiManager Start----------"
  testEnableWifiManager_EndLog = "testEnableWifi End----------"
  testDisableWifiManager_StartLog = "testDisableWifi Start----------"
  testDisableWifiManager_EndLog = "testDisableWifi End----------"
  testIsWifiActiveManager_StartLog = "testIsWifiActive Start----------"
  testIsWifiActiveManager_EndLog = "testDisableWifi End----------"
  testScanManager_StartLog = -"testScan Start----------"
  testScanManager_EndLog = "testScan End----------"
  testForcibleScanManager_StartLog = "testForcibleScan Start----------"
  testForcibleScanManager_EndLog = "testForcibleScan End----------"
  testGetScanInfoListManager_StartLog = "testGetScanInfoListManager start---------"
  testGetScanInfoListManager_EndLog = "testGetScanInfoListManager End----------"
  testGetScanInfosPromiseManager_StartLog = "testGetScanInfosPromise Start----------"
  testGetScanInfosPromiseManager_EndLog = "testGetScanInfosPromise End----------"
  testGetScanInfosPromiseManager_successfulLog = "getScanInfos promise successful"
  testGetScanInfosManager_infoLog = "wifi received scan info: "
  testGetScanInfosCallbackManager_StartLog = "testGetScanInfosCallback Start----------"
  testGetScanInfosCallbackManager_EndLog = "testGetScanInfosCallback End----------"
  testGetScanInfosCallbackManager_successfulLog = "getScanInfos callback successful"
  testAddDeviceConfigPromiseManager_StartLog = "testAddDeviceConfigPromise Start----------"
  testAddDeviceConfigPromiseManager_EndLog = "testAddDeviceConfigPromise End----------"
  testAddDeviceConfigPromiseManager_successfulLog = "addDeviceConfig promise successful"
  testAddDeviceConfigManagerLog = "addDeviceConfig: "
  testAddDeviceConfigCallbackManager_StartLog = "testAddDeviceConfigCallback Start----------"
  testAddDeviceConfigCallbackManager_EndLog = "testAddDeviceConfigCallback End----------"
  testAddDeviceConfigCallbackManager_successfulLog = "addDeviceConfig Callback successful"
  testAddCandidateConfigPromiseManager_StartLog = "testAddCandidateConfigPromise Start----------"
  testAddCandidateConfigPromiseManager_EndLog = "testAddCandidateConfigPromise End----------"
  testAddCandidateConfigPromiseManager_successfulLog = "addCandidateConfig promise successful"
  testAddCandidateConfigManagerLog = "addCandidateConfig: "
  testAddCandidateConfigCallbackManager_StartLog = "testAddCandidateConfigCallback Start----------"
  testAddCandidateConfigCallbackManager_EndLog = "testAddCandidateConfigCallback End----------"
  testAddCandidateConfigCallbackManager_successfulLog = "addCandidateConfig Callback successful"
  testRemoveCandidateConfigPromiseManager_StartLog = "testRemoveCandidateConfigPromise Start----------"
  testRemoveCandidateConfigPromiseManager_EndLog = "testRemoveCandidateConfigPromise End----------"
  testRemoveCandidateConfigPromiseManager_successfulLog = "removeCandidateConfig promise successful"
  testRemoveCandidateConfigManagerLog = "removeCandidateConfig: "
  testRemoveCandidateConfigCallbackManager_StartLog = "testRemoveCandidateConfigCallback Start----------"
  testRemoveCandidateConfigCallbackManager_EndLog = "testRemoveCandidateConfigCallback End----------"
  testRemoveCandidateConfigCallbackManager_successfulLog = "removeCandidateConfig Callback successful"
  testGetCandidateConfigsManager_StartLog = "testGetCandidateConfigsManager Start----------"
  testGetCandidateConfigsManager_EndLog = "testGetCandidateConfigsManager End----------"
  testConnectToCandidateConfigManager_StartLog = "testConnectToCandidateConfigManager Start----------"
  testConnectToCandidateConfigManager_EndLog = "testConnectToCandidateConfigManager End----------"
  testConnectToNetworkManager_StartLog = "testConnectToNetwork Start----------"
  testConnectToNetworkManager_EndLog = "testConnectToNetwork End----------"
  testConnectToDeviceManager_StartLog = "testConnectToDevice Start----------"
  testConnectToDeviceManager_EndLog = "testConnectToDevice End----------"
  testDisconnectManager_StartLog = "testDisconnect Start----------"
  testDisconnectManager_EndLog = "testDisconnect End----------"
  testGetSignalLevelManager_StartLog = "testGetSignalLevel Start----------"
  testGetSignalLevelManager_EndLog = "testGetSignalLevel End----------"
  testGetLinkedInfoPromiseManager_StartLog = "testGetLinkedInfoPromise Start----------"
  testGetLinkedInfoPromiseManager_EndLog = "testGetLinkedInfoPromise End----------"
  testGetLinkedInfoPromiseManager_successfulLog = "getLinkedInfoPromise test successful"
  testGetLinkedInfoCallbackManager_StartLog = "testGetLinkedInfoCallback Start----------"
  testGetLinkedInfoCallbackManager_EndLog = "testGetLinkedInfoCallback End----------"
  testGetLinkedInfoCallbackManager_successfulLog = "getLinkedInfoCallback test successful"
  testIsConnectedManager_StartLog = "testIsConnected Start----------"
  testIsConnectedManager_EndLog = "testIsConnected End----------"
  testGetSupportedFeaturesManager_StartLog = "testGetSupportedFeatures Start----------"
  testGetSupportedFeaturesManager_EndLog = "testGetSupportedFeatures End----------"
  testIsFeatureSupportedManager_StartLog = "testIsFeatureSupported Start----------"
  testIsFeatureSupportedManager_EndLog = "testIsFeatureSupported End----------"
  testGetDeviceMacAddressManager_StartLog = "testGetDeviceMacAddress Start----------"
  testGetDeviceMacAddressManager_EndLog = "testGetDeviceMacAddress End----------"
  testGetIpInfoManager_StartLog = "testGetIpInfo Start----------"
  testGetIpInfoManager_EndLog = "testGetIpInfo End----------"
  testGetCountryCodeManager_StartLog = "testGetCountryCode Start----------"
  testGetCountryCodeManager_EndLog = "testGetCountryCode End----------"
  testReassociateManager_StartLog = "testReassociate Start----------"
  testReassociateManager_EndLog = "testReassociate End----------"
  testReConnectManager_StartLog = "testReConnect Start----------"
  testReConnectManager_EndLog = "testReConnect End----------"
  testGetDeviceConfigsManager_StartLog = "testGetDeviceConfigs Start----------"
  testGetDeviceConfigsManager_EndLog = "testGetDeviceConfigs End----------"
  testUpdateNetworkManager_StartLog = "testUpdateNetwork Start----------"
  testUpdateNetworkManager_EndLog = "testUpdateNetwork End----------"
  testDisableNetworkManager_StartLog = "testDisableNetwork Start----------"
  testDisableNetworkManager_EndLog = "testDisableNetwork End----------"
  testRemoveAllNetworkManager_StartLog = "testRemoveAllNetwork Start----------"
  testRemoveAllNetworkManager_EndLog = "testRemoveAllNetwork End----------"
  testRemoveDeviceManager_StartLog = "testRemoveDevice Start----------"
  testRemoveDeviceManager_EndLog = "testRemoveDevice End----------"
  testOnWifiStateChangeManager_StartLog = "testOnWifiStateChange Start----------"
  testOnWifiStateChangeManager_EndLog = "testOnWifiStateChange End----------"
  testOnWifiConnectionChangeManager_StartLog = "testOnWifiConnectionChange Start----------"
  testOnWifiConnectionChangeManager_EndLog = "testOnWifiConnectionChange End----------"
  testOnWifiScanStateChangeManager_StartLog = "testOnWifiConnectionChange Start----------"
  testOnWifiScanStateChangeManager_EndLog = "testOnWifiConnectionChange End----------"
  testOnWifiRssiChangeManager_StartLog = "testOnWifiRssiChange Start----------"
  testOnWifiRssiChangeManager_EndLog = "testOnWifiRssiChange End----------"
  testOnStreamChangeManager_StartLog = "testOnStreamChange Start----------"
  testOnStreamChangeManager_EndLog = "testOnStreamChange End----------"
  testOnDeviceConfigChangeManager_StartLog = "testOnDeviceConfigChangeManager Start----------"
  testOnDeviceConfigChangeManager_EndLog = "testOnDeviceConfigChangeManager End----------"
  testEnableSemiWifi_StartLog = "testEnableSemiWifi Start----------"
  testEnableSemiWifi_EndLog = "testEnableSemiWifi End----------"
  testStartScanManager_StartLog = "testStartScan Start----------"
  testStartScanManager_EndLog = "testStartScan End----------"
  //hotspot
  testEnableHotspotManager_StartLog = "testEnableHotspot start--------------"
  testEnableHotspotManager_EndLog = "testEnableHotspot End--------------"
  testDisableHotspotManager_StartLog = "testDisableHotspot start--------------"
  testDisableHotspotManager_EndLog = "testDisableHotspot End--------------"
  testIsHotspotDualBandSupportedManager_StartLog = "testIsHotspotDualBandSupported start--------------"
  testIsHotspotDualBandSupportedManager_EndLog = "testIsHotspotDualBandSupported End--------------"
  testIsHostActiveManager_StartLog = "testIsHostActive start--------------"
  testIsHostActiveManager_EndLog = "testIsHostActive End--------------"
  testSetHotspotConfigManagerManager_StartLog = "testSetHotspotConfig start--------------"
  testSetHotspotConfigManager_EndLog = "testSetHotspotConfig End--------------"
  testGetHotspotConfigManager_StartLog = "testGetHotspotConfig start--------------"
  testGetHotspotConfigManager_EndLog = "testGetHotspotConfig End--------------"
  testGetStationsManager_StartLog = "testGetStations start--------------"
  testGetStationsManager_EndLog = "testGetStations End--------------"
  testOnHotspotStateChangeManager_StartLog = "testOnHotspotStateChange start--------------"
  testOnHotspotStateChangeManager_EndLog = "testOnHotspotStateChange End--------------"
  testOnHotspotStaJoinManager_StartLog = "testOnHotspotStaJoin start--------------"
  testOnHotspotStaJoinManager_EndLog = "testOnHotspotStaJoin End--------------"
  testOnHotspotStaLeaveManager_StartLog = "testOnHotspotStaLeave start--------------"
  testOnHotspotStaLeaveManager_EndLog = "testOnHotspotStaLeave End--------------"
  //p2p
  testGetP2pLinkedInfoPromiseManager_StartLog = "testGetP2pLinkedInfoPromise Start----------"
  testGetP2pLinkedInfoPromiseManager_EndLog = "testGetP2pLinkedInfoPromise End----------"
  getP2pLinkedInfoPromiseManager_successfulLog = "getP2pLinkedInfoPromise successful"
  testGetP2pLinkedInfoCallbackManager_StartLog = "testGetP2pLinkedInfoCallback Start----------"
  testGetP2pLinkedInfoCallbackManager_EndLog = "testGetP2pLinkedInfoCallback End----------"
  getP2pLinkedInfoCallbackManager_successfulLog = "getP2pLinkedInfoCallback successful "
  testGetCurrentGroupPromiseManager_StartLog = "testGetCurrentGroupPromise Start----------"
  testGetCurrentGroupPromiseManager_EndLog = "testGetCurrentGroupPromise End----------"
  testGetCurrentGroupPromiseManager_successfulLog = "getCurrentGroupPromise successful"
  testGetCurrentGroupCallbackManager_StartLog = "testGetCurrentGroupCallback Start----------"
  testGetCurrentGroupCallbackManager_EndLog = "testGetCurrentGroupCallback End----------"
  getCurrentGroupCallbackManager_successfulLog = "getCurrentGroupCallback successful"
  testGetP2pPeerDevicesPromiseManager_StartLog = "testGetP2pPeerDevicesPromise Start----------"
  testGetP2pPeerDevicesPromiseManager_EndLog = "testGetP2pPeerDevicesPromise End----------"
  getP2pPeerDevicesPromiseManager_successfulLog = "getP2pPeerDevicesPromise successful"
  testGetP2pPeerDevicesCallbackManager_StartLog = "testGetP2pPeerDevicesCallback Start----------"
  testGetP2pPeerDevicesCallbackManager_EndLog = "testGetP2pPeerDevicesCallback End----------"
  getP2pPeerDevicesCallbackManager_successfulLog = "getP2pPeerDevicesCallback successful"
  testCreateGroupManager_StartLog = "testCreateGroup Start----------"
  testCreateGroupManager_EndLog = "testCreateGroup End----------"
  testRemoveGroupManager_StartLog = "testRemoveGroup Start----------"
  testRemoveGroupManager_EndLog = "testRemoveGroup End----------"
  testP2pConnectManager_StartLog = "testP2pConnect Start----------"
  testP2pConnectManager_EndLog = "testP2pConnect End----------"
  testP2pCancelConnectManager_StartLog = "testP2pCancelConnect Start----------"
  testP2pCancelConnectManager_EndLog = "testP2pCancelConnect End----------"
  testStartDiscoverDevicesManager_StartLog = "testStartDiscoverDevices Start----------"
  testStartDiscoverDevicesManager_EndLog = "testStartDiscoverDevices End----------"
  testStopDiscoverDevicesManager_StartLog = "testStopDiscoverDevices Start----------"
  testStopDiscoverDevicesManager_EndLog = "testStopDiscoverDevices End----------"
  testDeletePersistentGroupManager_StartLog = "testDeletePersistentGroup Start----------"
  testDeletePersistentGroupManager_EndLog = "testDeletePersistentGroup End----------"
  testSetDeviceNameManager_StartLog = "testSetDeviceName Start----------"
  testSetDeviceNameManager_EndLog = "testSetDeviceName End----------"
  testOnP2pStateChangeManager_StartLog = "testOnP2pStateChange Start----------"
  testOnP2pStateChangeManager_EndLog = "testOnP2pStateChange End----------"
  testOnP2pConnectionChangeManager_StartLog = "testOnP2pConnectionChange Start----------"
  testOnP2pConnectionChangeManager_EndLog = "testOnP2pConnectionChange End----------"
  testOnP2pDeviceChangeManager_StartLog = "testOnP2pDeviceChange Start----------"
  testOnP2pDeviceChangeManager_EndLog = "testOnP2pDeviceChange End----------"
  testOnP2pPeerDeviceChangeManager_StartLog = "testOnP2pPeerDeviceChange Start----------"
  testOnP2pPeerDeviceChangeManager_EndLog = "testOnP2pPeerDeviceChange End----------"
  testOnP2pPersistentGroupChangeManager_StartLog = "testOnP2pPersistentGroupChange Start----------"
  testOnP2pPersistentGroupChangeManager_EndLog = "testOnP2pPersistentGroupChange End----------"
  testOnP2pDiscoveryChangeManager_StartLog = "testOnP2pDiscoveryChange Start----------"
  testOnP2pDiscoveryChangeManager_EndLog = "testOnP2pDiscoveryChange End----------"
}

let logDataManager = new LogDataManager();

export default logDataManager as LogDataManager;