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

import wifi from '@ohos.wifi'
import wifiManager from '@ohos.wifiManager'
import { AvailableP2p } from '../../../Component/availableP2p'
import { P2pTitleBar } from '../../../Component/p2pTitleBar'
import { P2pModel } from '../../../entryability/model/p2pModel'
import ConfigData from '../../../Utils/ConfigData'

const TAG = 'wifiTestApp [p2pFound]'
const PERMISSION_P2P_NAME: string = 'ohos.permission.LOCATION'
/**
 * p2p Found Test Page Of Wifi test
 */
@Entry
@Component
struct P2pFound {
  private p2pModel: P2pModel = new P2pModel()
  private testItem: string = "p2p扫描"
  @State p2pList: Array<wifiManager.WifiP2pDevice> = []
  @State p2pIsLinked: boolean = false
  @State p2pIsSwitchOn: boolean = false
  @State p2pLinkedInfo: wifiManager.WifiP2pLinkedInfo = {
    connectState : wifiManager.P2pConnectState.DISCONNECTED ,
    isGroupOwner : false ,
    groupOwnerAddr : '' ,
  }

  scan() {
    this.p2pModel.getP2pPeerDevices((result: wifiManager.WifiP2pDevice[]) => {
      if ( this.p2pIsSwitchOn ) {
        this.p2pList = result
        console.log('scan p2pList:' + JSON.stringify(this.p2pList))
        setTimeout(() => {
          this.scan()
        } , 2000)
      }
    })
  }

  getP2pLinkedInfo() {
    wifiManager.getP2pLinkedInfo((err , p2pLinkedInfo) => {
      if ( err ) {
        console.log(TAG , `getLinkedInfo err: ${ JSON.stringify(err) }`)
        return
      }
      console.log(TAG , `p2pLinkedInfo: ${ JSON.stringify(p2pLinkedInfo) }`)
      if ( p2pLinkedInfo === null || p2pLinkedInfo.connectState === 0 ) {
        this.p2pIsLinked = false
        this.p2pLinkedInfo = {
          connectState : 0 ,
          isGroupOwner : false ,
          groupOwnerAddr : ''
        }
        return
      }
      this.p2pIsLinked = true
      this.p2pLinkedInfo = p2pLinkedInfo
    })
  }

  addP2pListener() {
    wifiManager.on('p2pConnectionChange' , wifiP2pLinkedInfo => {
      let state = wifiP2pLinkedInfo.connectState
      console.log(TAG , `p2pConnectionChange: ${ state }`)
      this.getP2pLinkedInfo()
    })
    wifiManager.on('p2pStateChange' , state => {
      console.log(TAG , `p2pStateLisener state: ${ state }`)
      this.p2pList = []
      if ( state === 1 ) { // 1: wifi is enable, 0:wifi is disable
        wifiManager.startDiscoverDevices()
        this.scan()
      }
    })
  }

  aboutToAppear() {
    if ( wifiManager.isWifiActive() ) {
      console.log(TAG , 'wifi is active')
      this.p2pIsSwitchOn = true
      wifiManager.startDiscoverDevices()
      this.scan()
      this.getP2pLinkedInfo()
    }
    this.addP2pListener()
  }

  build() {
    Column() {
      Stack({ alignContent : Alignment.TopStart }) {
        P2pTitleBar({ detail : this.testItem })
      }

      Row() {
        Text($r('app.string.wlan'))
          .fontSize(22)
          .fontWeight(FontWeight.Bold)
          .layoutWeight(1)
          .align(Alignment.TopStart)
        Column() {
          Toggle({ type : ToggleType.Switch , isOn : this.p2pIsSwitchOn })
            .onChange((isOn: boolean) => {
              console.log(TAG , `wifi swtich is: ${ isOn }`)
              this.p2pList = []
              if ( isOn ) {
                this.p2pIsSwitchOn = true
                wifiManager.enableWifi()
                wifiManager.startDiscoverDevices()
                this.scan()
                return
              }
              this.p2pIsSwitchOn = false
              this.p2pIsLinked = false
              wifiManager.disconnect()
              wifiManager.disableWifi()
            })
        }
      }
      .width(ConfigData.WH_95_100)

      AvailableP2p({ p2pList : $p2pList , p2pLinkedInfo : $p2pLinkedInfo })
    }
    .size({ width : ConfigData.WH_100_100 , height : ConfigData.WH_100_100 })
    .backgroundColor('#F5F5F5')
  }

  aboutToDisappear() {
    wifiManager.off('p2pConnectionChange')
    wifiManager.off('p2pStateChange')
  }
}