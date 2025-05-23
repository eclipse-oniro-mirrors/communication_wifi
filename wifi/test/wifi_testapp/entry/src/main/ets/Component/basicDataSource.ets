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

const TAG = "wifiTestApp [basicDataSource]"

/**
 * BasicDataSource page of WiFi test
 */

class BasicDataSource implements IDataSource {
  private listeners: DataChangeListener[] = []

  public totalCount(): number {
    return 0
  }

  public getData(index: number): any {
    return undefined
  }

  registerDataChangeListener(listener: DataChangeListener): void {
    if ( this.listeners.indexOf(listener) < 0 ) {
      console.log(TAG , 'add listener')
      this.listeners.push(listener)
    } else {
      console.log(TAG , 'indexOf(listener) >= 0')
    }
  }

  unregisterDataChangeListener(listener: DataChangeListener): void {
    const pos = this.listeners.indexOf(listener);
    if ( pos >= 0 ) {
      console.log(TAG , 'remove listener')
      this.listeners.splice(pos , 1)
    } else {
      console.log(TAG , 'pos < 0')
    }
  }

  notifyDataReload(): void {
    this.listeners.forEach(listener => {
      listener.onDataReloaded()
    })
  }

  notifyDataAdd(index: number): void {
    this.listeners.forEach(listener => {
      listener.onDataAdd(index)
    })
  }

  notifyDataChange(index: number): void {
    this.listeners.forEach(listener => {
      listener.onDataChange(index)
    })
  }

  notifyDataDelete(index: number): void {
    this.listeners.forEach(listener => {
      listener.onDataDelete(index)
    })
  }

  notifyDataMove(from: number , to: number): void {
    this.listeners.forEach(listener => {
      listener.onDataMove(from , to)
    })
  }
}

export default class WifiDataSource extends BasicDataSource {
  private dataArray: wifi.WifiScanInfo[] = []

  constructor(data: wifi.WifiScanInfo[]) {
    super()
    this.dataArray = data
  }

  public totalCount(): number {
    return this.dataArray.length
  }

  public getData(index: number): any {
    return this.dataArray[ index ]
  }

  public addData(index: number , data: wifi.WifiScanInfo): void {
    this.dataArray.splice(index , 0 , data)
    this.notifyDataAdd(index)
  }

  public pushData(data: wifi.WifiScanInfo): void {
    this.dataArray.push(data)
    this.notifyDataAdd(this.dataArray.length - 1)
  }
}