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

import image from '@ohos.multimedia.image'
import fileio from '@ohos.fileio'
import fs from '@ohos.file.fs';
import promptAction from '@ohos.promptAction'
import photoAccessHelper from '@ohos.file.photoAccessHelper'
import DateTimeUtil from './DateTimeUtil'


/**
 * mediaUtils of wifi test
 */

const TAG = "wifiTestApp [MediaUtils]"

class MediaUtils {
  async createAndGetFile(context: any) {
    let mediaTest = photoAccessHelper.getPhotoAccessHelper(context)
    let info = {
      prefix: 'IMG_', suffix: '.jpg', directory: photoAccessHelper.PhotoType.IMAGE
    }
    let dateTimeUtil = new DateTimeUtil()
    let name = `${dateTimeUtil.getDate()}_${dateTimeUtil.getTime()}`
    let displayName = `${info.prefix}${name}${info.suffix}`
    let photoType: photoAccessHelper.PhotoType = photoAccessHelper.PhotoType.IMAGE;
    let options: photoAccessHelper.CreateOptions = {
      title: displayName
    }
    return await mediaTest.createAsset(photoType, '.jpg', options)
  }

  async savePicture(data: image.PixelMap, context: any) {
    console.log(TAG, `savePicture`)
    let packOpts: image.PackingOption = {
      format: "image/jpeg", quality: 100
    }
    let imagePackerApi = image.createImagePacker()
    let arrayBuffer = await imagePackerApi.packing(data, packOpts)
    let fileAsset = await this.createAndGetFile(context)
    let file = await fs.open(fileAsset,  fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
    imagePackerApi.release()
    try {
      await fs.write(file.fd, arrayBuffer);
    } catch (err) {
      console.log(`write failed, code is ${err.code}, message is ${err.message}`)
    }
    await fs.close(file.fd);
    console.log(TAG, `write done`)
    promptAction.showToast({
      message: '图片保存成功', duration: 1000
    })
  }
}

export default new MediaUtils()