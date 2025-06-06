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

import ConfigData from '../utils/ConfigData';
import { TestScenario } from '../entryability/model/testData'
import promptAction from '@ohos.promptAction'

const TAG = "wifiTestApp [scenarioContentTable]"
/**
 *  Scenario Test ContentTable Component Page Of Wifi test
 */

@Component
export struct ScenarioContentTable {
  @State scenarioItems: TestScenario[] = [];
  private scenarioItemsX!: TestScenario[]
  @Prop changeIndex: number;
  @State localName: string = 'DaYuBlue'

  @Builder IngredientItem(scenarioItem: TestScenario , index: number) {
    Stack() {
      Flex() {
        Flex({ direction : FlexDirection.Column , alignItems : ItemAlign.Start }) {
          Row() {
            Text(scenarioItem.detail)
              .fontSize("17vp")
              .margin({ top : "3vp" , bottom : "3vp" , left : "10vp" })
              .textAlign(TextAlign.Start)
              .height(40)
              .width(30)

            Text(scenarioItem.method)
              .fontSize("17vp")
              .margin({ top : "3vp" , bottom : "3vp" , left : "10vp" })
              .textAlign(TextAlign.Start)
              .height(40)
              .width(240)
          }
          .height(30)
          .width(ConfigData.WH_70_100)
        }
        .height(30)
        .layoutWeight(6)

        Flex({ alignItems : ItemAlign.Center }) {
          Text(this.changeIndex >= 0 ? this.getCurrentState(index) : scenarioItem.result)
            .margin({ left : "10vp" })
            .fontSize("17vp")
            .flexGrow(1)
        }
        .layoutWeight(3)
        .padding({ left : 10 })
        .margin({ left : 10 })
      }
    }
    .height(40)
    .borderRadius(10)
    .margin({ top : 3 , bottom : 3 })
    .backgroundColor($r("app.color.white"))
  }

  getCurrentState(index: number) {
    if ( index <= this.changeIndex ) {
      this.scenarioItemsX[ index ].result = 'pass';
    } else {
      console.log(TAG , "index > this.changeIndex")
    }
    return this.scenarioItemsX[ index ].result
  }

  aboutToAppear(): void {
    console.log(TAG , "scenario组件构建完毕")
    //初始化私有变量
    this.scenarioItemsX = this.scenarioItems
  }

  build() {
    Column() {
      Scroll() {
        Flex({
          direction : FlexDirection.Column ,
          justifyContent : FlexAlign.SpaceBetween ,
          alignItems : ItemAlign.Start
        }) {
          ForEach(this.scenarioItems , (item: TestScenario , index: number) => {
            this.IngredientItem(item , index)
          })
        }
      }
      .scrollBarWidth(20)
    }
    .height(ConfigData.WH_80_100)
    .padding({ top : 5 , right : 10 , left : 10 })
  }
}