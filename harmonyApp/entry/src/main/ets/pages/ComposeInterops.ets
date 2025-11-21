/*
 * Tencent is pleased to support the open source community by making ovCompose available.
 * Copyright (C) 2025 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { promptAction } from '@kit.ArkUI';


interface ButtonArgs {
  text: string
  backgroundColor: string
}

@Builder
export function buttonBuilder(args: ButtonArgs) {
  button()
}

@Component
export struct button {
  @Consume compose_args: ButtonArgs

  build() {
    Column() {
      Button(this.compose_args.text).backgroundColor(this.compose_args.backgroundColor).width('100%').onClick(e => {
        console.log(`Button Clicked: ${this.compose_args.text}`)
      }).height('70%')

      Text(this.compose_args.text).backgroundColor(this.compose_args.backgroundColor).width('100%').onClick(e => {
        console.log(`Text Clicked: ${this.compose_args.text}`)
      }).height('20%')
      Stack().height('10%')
    }
  }
}

interface TextArgs {
  id: string
  text: string
  backgroundColor: string
}

@Builder
export function textBuilder(args: TextArgs) {
  text()
}

@Component
export struct text {
  @Consume compose_args: TextArgs

  build() {
    Text(this.compose_args.id + " " + this.compose_args.text)
      .backgroundColor(this.compose_args.backgroundColor)
      .width('100%')
      .height('100%')
      .borderRadius('5vp')
      .onClick(e => {
        console.log(`Text Clicked: ${JSON.stringify(this.compose_args)}`)
      })
  }
}

@Builder
export function labelBuilder(args: ButtonArgs) {
  label()
}

@Component
export struct label {
  @Consume compose_args: ButtonArgs

  build() {
    Column() {

      Button(`+ ${this.compose_args.text}`, { type: ButtonType.Normal })
        .backgroundColor(this.compose_args.backgroundColor)
        .width('100%')
        .height('50%')
        .borderRadius(2)
        .onClick(e => {
          promptAction.showToast({
            message: this.compose_args.text
          })
        })
      Text(this.compose_args.text).backgroundColor(Color.Orange).width('100%').height('50%')
    }
  }
}

@Builder
export function layerBuilder(args: ButtonArgs) {
  layer()
}

@Component
export struct layer {
  @Consume compose_args: ButtonArgs

  build() {
    Stack({ alignContent: Alignment.BottomEnd }) {
      Text(this.compose_args.text)
    }
    .width('100%')
    .height('100%')
    .backgroundColor(this.compose_args.backgroundColor)
    .onClick(() => {
      promptAction.showToast({ message: `${this.compose_args.text} Clicked` })
    })
    .onTouch(e => {
      console.log(`layer ${this.compose_args.text} onTouch`)
    })
  }
}

@Builder
export function buttonWrapContentBuilder(args: ButtonArgs) {
  buttonWrapContent()
}

@Component
export struct buttonWrapContent {
  @Consume compose_args: ButtonArgs

  build() {
    Column() {

      Button(this.compose_args.text).backgroundColor(this.compose_args.backgroundColor).width('100%').onClick(e => {
        console.log(`Button Clicked: ${this.compose_args.text}`)
      })
      Text(this.compose_args.text).backgroundColor(this.compose_args.backgroundColor).width('100%').onClick(e => {
        console.log(`Text Clicked: ${this.compose_args.text}`)
      })
    }
  }
}


interface InputArgs {
  text: string
}

@Builder
export function textInputBuilder(args: InputArgs) {
  textInput()
}


@Component
export struct textInput {
  @Consume compose_args: InputArgs

  build() {
    Column() {
      TextInput({ text: $$this.compose_args.text }).onChange(value => {
        console.log(`/// onChange: ${value}`)
      })

      Text("ArkUI组件更新:").fontColor(Color.Gray).margin({ top: 30 })

      Text(this.compose_args.text)
        .width('100%')
        .padding(10)
        .border({ width: 1, color: Color.Gray })
    }.alignItems(HorizontalAlign.Start)
  }
}

@Builder
export function verticalListBuilder(args?: object) {
  verticalList()
}

@Component
export struct verticalList {
  build() {
    Row() {
      List() {
        ForEach([1, 2, 3, 4, 5, 6, 7, 8, 9, 10], (item: number, index) => {
          ListItem() {
            Button(`Ark Button ${index}`, { type: ButtonType.Normal }).borderRadius(2).height('100vp').padding(10)
          }.padding(10)
        })
      }.listDirection(Axis.Vertical).height('100%').width('40%')

      Stack().width('20%').height('100%').borderWidth(1)

      Scroll() {
        Column() {
          ForEach([1, 2, 3, 4, 5, 6, 7, 8, 9, 10], (item: number, index) => {
            ListItem() {
              Button(`Ark Button ${index}`, { type: ButtonType.Normal }).borderRadius(2).height('100vp').padding(10)
            }.padding(10)
          })
        }
      }.scrollable(ScrollDirection.Vertical).height('100%').width('40%')
    }
  }
}

@Builder
export function horizontalListBuilder(args?: object) {
  horizontalList()
}

@Component
export struct horizontalList {
  build() {
    Column() {
      List() {
        ForEach([1, 2, 3, 4, 5, 6, 7, 8, 9, 10], (item: number, index) => {
          ListItem() {
            Button(`Ark Button ${index}`, { type: ButtonType.Normal }).borderRadius(2).height('100vp')
          }.padding(10)
        })
      }.listDirection(Axis.Horizontal).height('40%').width('100%')

      Stack().width('100%').height('20%').borderWidth(1)

      Scroll() {
        Row() {
          ForEach([1, 2, 3, 4, 5, 6, 7, 8, 9, 10], (item: number, index) => {
            ListItem() {
              Button(`Ark Button ${index}`, { type: ButtonType.Normal }).borderRadius(2).height('100vp')
            }.padding(10)
          })
        }
      }.scrollable(ScrollDirection.Horizontal).height('40%').width('100%')
    }
  }
}

@Builder
export function SimpleVideoBuilder(args?: object) {
  SimpleVideo()
}

@Component
export struct SimpleVideo {
  // $rawfile('video1.mp4')、$r('app.media.poster1')需要分别替换为开发者所需的视频、图片资源文件。
  @State videoSrc: Resource = $rawfile('trailer.mp4');
  //@State previewUri: Resource = $r('app.media.poster1');
  @State curRate: PlaybackSpeed = PlaybackSpeed.Speed_Forward_1_00_X;
  @State isAutoPlay: boolean = false;
  @State showControls: boolean = true;
  @State isShortcutKeyEnabled: boolean = false;
  @State showFirstFrame: boolean = false;
  controller: VideoController = new VideoController();
  build() {
    Column() {
      Video({
        src: this.videoSrc,
        //previewUri: this.previewUri, // 设置预览图。
        currentProgressRate: this.curRate, // 设置播放速度。
        controller: this.controller,
        posterOptions: { showFirstFrame: this.showFirstFrame } // 关闭首帧送显。
      })
        .width('100%')
        .height(600)
        .autoPlay(this.isAutoPlay)
        .controls(this.showControls)
        .enableShortcutKey(this.isShortcutKeyEnabled)
        .onStart(() => {
          console.info('onStart');
        })
        .onPause(() => {
          console.info('onPause');
        })
        .onFinish(() => {
          console.info('onFinish');
        })
        .onError(() => {
          console.info('onError');
        })
        .onStop(() => {
          console.info('onStop');
        })
        .onPrepared((e?: DurationObject) => {
          if (e != undefined) {
            console.info('onPrepared is ' + e.duration);
          }
        })
        .onSeeking((e?: TimeObject) => {
          if (e != undefined) {
            console.info('onSeeking is ' + e.time);
          }
        })
        .onSeeked((e?: TimeObject) => {
          if (e != undefined) {
            console.info('onSeeked is ' + e.time);
          }
        })
        .onUpdate((e?: TimeObject) => {
          if (e != undefined) {
            console.info('onUpdate is ' + e.time);
          }
        })
        .onFullscreenChange((e?: FullscreenObject) => {
          if (e != undefined) {
            console.info('onFullscreenChange is ' + e.fullscreen);
          }
        })
      Row() {
        // $rawfile('video2.mp4')、$r('app.media.poster2')需要分别替换为开发者所需的视频、图片资源文件。
        Button('src').onClick(() => {
          this.videoSrc = $rawfile('mother.mp4'); // 切换视频源。
        }).margin(5)
        Button('previewUri').onClick(() => {
          //this.previewUri = $r('app.media.poster2'); // 切换视频预览海报。
        }).margin(5)
        Button('controls').onClick(() => {
          this.showControls = !this.showControls; // 切换是否显示视频控制栏。
        }).margin(5)
      }
      Row() {
        Button('start').onClick(() => {
          this.controller.start(); // 开始播放。
        }).margin(2)
        Button('pause').onClick(() => {
          this.controller.pause(); // 暂停播放。
        }).margin(2)
        Button('stop').onClick(() => {
          this.controller.stop(); // 结束播放。
        }).margin(2)
        Button('reset').onClick(() => {
          this.controller.reset(); // 重置AVPlayer。
        }).margin(2)
        Button('setTime').onClick(() => {
          this.controller.setCurrentTime(10, SeekMode.Accurate); // 精准跳转到视频的10s位置。
        }).margin(2)
      }
      Row() {
        Button('rate 0.75').onClick(() => {
          this.curRate = PlaybackSpeed.Speed_Forward_0_75_X; // 0.75倍速播放。
        }).margin(5)
        Button('rate 1').onClick(() => {
          this.curRate = PlaybackSpeed.Speed_Forward_1_00_X; // 原倍速播放。
        }).margin(5)
        Button('rate 2').onClick(() => {
          this.curRate = PlaybackSpeed.Speed_Forward_2_00_X; // 2倍速播放。
        }).margin(5)
      }
    }
  }
}
interface DurationObject {
  duration: number;
}
interface TimeObject {
  time: number;
}
interface FullscreenObject {
  fullscreen: boolean;
}



