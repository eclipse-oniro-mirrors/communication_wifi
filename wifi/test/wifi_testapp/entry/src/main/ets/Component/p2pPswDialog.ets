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
import wifiManager from '@ohos.wifiManager';
import ConfigData from '../utils/ConfigData';
/**
 * PswDialog page of WiFi test
 */

@CustomDialog
export struct P2pPswDialog {
  private p2pScanInfo!: wifiManager.WifiP2pDevice
  private psw: string = ''
  private controller: CustomDialogController
  private action!: (p2pScanInfo: wifiManager.WifiP2pDevice , psw: string) => void

  build() {
    Column() {
      Text(this.p2pScanInfo.deviceName)
        .fontSize(20)
        .width(ConfigData.WH_95_100)
      TextInput({ placeholder : 'input password' })
        .key('input')
        .type(InputType.Password)
        .placeholderColor(Color.Gray)
        .fontSize(19)
        .margin({ top : 15 })
        .width(ConfigData.WH_95_100)
        .onChange((value: string) => {
          this.psw = value
        })
      Row() {
        Button() {
          Text($r('app.string.sure'))
            .fontColor(Color.Blue)
            .fontSize(17)
        }
        .layoutWeight(7)
        .backgroundColor(Color.White)
        .margin(5)
        .onClick(() => {
          this.controller.close()
          this.action(this.p2pScanInfo , this.psw)
        })

        Text()
          .width(1).height(35)
          .backgroundColor('#8F8F8F')
        Button() {
          Text($r('app.string.cancel'))
            .fontColor(Color.Red)
            .fontSize(17)
        }
        .layoutWeight(7)
        .backgroundColor(Color.White)
        .margin(5)
        .onClick(() => {
          this.controller.close()
        })
      }
      .width(ConfigData.WH_100_100)
      .margin({ top : ConfigData.WH_3_100 })
    }
    .padding(15)
  }
}