/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
import commonEvent from '@ohos.commonevent'

const injectRef = Object.getPrototypeOf(global) || global
injectRef.regeneratorRuntime = require('@babel/runtime/regenerator')

function publishCallBackOne() {
    console.debug("====>Publish CallBack ACTS_Particle_StartAbility_0100_CommonEvent====>");
}
function publishCallBackTwo() {
    console.debug("====>Publish CallBack ACTS_Particle_StartAbility_0200_CommonEvent====>");
}
function publishCallBackThree() {
    console.debug("====>Publish CallBack ACTS_Particle_StartAbility_0300_CommonEvent====>");
}
function publishCallBackFour() {
    console.debug("====>Publish CallBack ACTS_Particle_StartAbility_0400_CommonEvent====>");
}
function publishCallBackFive() {
    console.debug("====>Publish CallBack ACTS_Particle_StartAbility_0500_CommonEvent====>");
}
function publishCallBackSix() {
    console.debug("====>Publish CallBack ACTS_Particle_StartAbility_0600_CommonEvent====>");
}

export default {
    data: {
        title: "ParticleStartAbility"
    },
    onInit() {
        this.title = this.data.title;
    },
    onShow() {
        commonEvent.publish("ACTS_Particle_StartAbility_0100_CommonEvent", publishCallBackOne);
        commonEvent.publish("ACTS_Particle_StartAbility_0200_CommonEvent", publishCallBackTwo);
        commonEvent.publish("ACTS_Particle_StartAbility_0300_CommonEvent", publishCallBackThree);
        commonEvent.publish("ACTS_Particle_StartAbility_0400_CommonEvent", publishCallBackFour);
        commonEvent.publish("ACTS_Particle_StartAbility_0500_CommonEvent", publishCallBackFive);
        commonEvent.publish("ACTS_Particle_StartAbility_0600_CommonEvent", publishCallBackSix);
    },
    onReady() {
    },
}
