/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkYUVMath.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"

#include <cstring>

// in SkColorMatrix order (row-major)
// Created by running SkColorMatrix_DumpYUVMatrixTables()
const float JPEG_full_rgb_to_yuv[] = {
      0.299000f,  0.587000f,  0.114000f,  0.000000f,  0.000000f,
     -0.168736f, -0.331264f,  0.500000f,  0.000000f,  0.501961f,
      0.500000f, -0.418688f, -0.081312f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float JPEG_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.402000f,  0.000000f, -0.703749f,
      1.000000f, -0.344136f, -0.714136f,  0.000000f,  0.531211f,
      1.000000f,  1.772000f,  0.000000f,  0.000000f, -0.889475f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec601_limited_rgb_to_yuv[] = {
      0.256788f,  0.504129f,  0.097906f,  0.000000f,  0.062745f,
     -0.148223f, -0.290993f,  0.439216f,  0.000000f,  0.501961f,
      0.439216f, -0.367788f, -0.071427f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec601_limited_yuv_to_rgb[] = {
      1.164384f, -0.000000f,  1.596027f,  0.000000f, -0.874202f,
      1.164384f, -0.391762f, -0.812968f,  0.000000f,  0.531668f,
      1.164384f,  2.017232f,  0.000000f,  0.000000f, -1.085631f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec709_full_rgb_to_yuv[] = {
      0.212600f,  0.715200f,  0.072200f,  0.000000f,  0.000000f,
     -0.114572f, -0.385428f,  0.500000f,  0.000000f,  0.501961f,
      0.500000f, -0.454153f, -0.045847f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec709_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.574800f,  0.000000f, -0.790488f,
      1.000000f, -0.187324f, -0.468124f,  0.000000f,  0.329010f,
      1.000000f,  1.855600f, -0.000000f,  0.000000f, -0.931439f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec709_limited_rgb_to_yuv[] = {
      0.182586f,  0.614231f,  0.062007f,  0.000000f,  0.062745f,
     -0.100644f, -0.338572f,  0.439216f,  0.000000f,  0.501961f,
      0.439216f, -0.398942f, -0.040274f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float Rec709_limited_yuv_to_rgb[] = {
      1.164384f, -0.000000f,  1.792741f,  0.000000f, -0.972945f,
      1.164384f, -0.213249f, -0.532909f,  0.000000f,  0.301483f,
      1.164384f,  2.112402f, -0.000000f,  0.000000f, -1.133402f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_8bit_full_rgb_to_yuv[] = {
      0.262700f,  0.678000f,  0.059300f,  0.000000f,  0.000000f,
     -0.139630f, -0.360370f,  0.500000f,  0.000000f,  0.501961f,
      0.500000f, -0.459786f, -0.040214f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_8bit_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.474600f,  0.000000f, -0.740191f,
      1.000000f, -0.164553f, -0.571353f,  0.000000f,  0.369396f,
      1.000000f,  1.881400f, -0.000000f,  0.000000f, -0.944389f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_8bit_limited_rgb_to_yuv[] = {
      0.225613f,  0.582282f,  0.050928f,  0.000000f,  0.062745f,
     -0.122655f, -0.316560f,  0.439216f,  0.000000f,  0.501961f,
      0.439216f, -0.403890f, -0.035326f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_8bit_limited_yuv_to_rgb[] = {
      1.164384f, -0.000000f,  1.678674f,  0.000000f, -0.915688f,
      1.164384f, -0.187326f, -0.650424f,  0.000000f,  0.347458f,
      1.164384f,  2.141772f, -0.000000f,  0.000000f, -1.148145f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_10bit_full_rgb_to_yuv[] = {
      0.262700f,  0.678000f,  0.059300f,  0.000000f,  0.000000f,
     -0.139630f, -0.360370f,  0.500000f,  0.000000f,  0.500489f,
      0.500000f, -0.459786f, -0.040214f,  0.000000f,  0.500489f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_10bit_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.474600f,  0.000000f, -0.738021f,
      1.000000f, -0.164553f, -0.571353f,  0.000000f,  0.368313f,
      1.000000f,  1.881400f, -0.000000f,  0.000000f, -0.941620f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_10bit_limited_rgb_to_yuv[] = {
      0.224951f,  0.580575f,  0.050779f,  0.000000f,  0.062561f,
     -0.122296f, -0.315632f,  0.437928f,  0.000000f,  0.500489f,
      0.437928f, -0.402706f, -0.035222f,  0.000000f,  0.500489f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_10bit_limited_yuv_to_rgb[] = {
      1.167808f, -0.000000f,  1.683611f,  0.000000f, -0.915688f,
      1.167808f, -0.187877f, -0.652337f,  0.000000f,  0.347458f,
      1.167808f,  2.148072f, -0.000000f,  0.000000f, -1.148145f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_12bit_full_rgb_to_yuv[] = {
      0.262700f,  0.678000f,  0.059300f,  0.000000f,  0.000000f,
     -0.139630f, -0.360370f,  0.500000f,  0.000000f,  0.500122f,
      0.500000f, -0.459786f, -0.040214f,  0.000000f,  0.500122f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_12bit_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.474600f,  0.000000f, -0.737480f,
      1.000000f, -0.164553f, -0.571353f,  0.000000f,  0.368043f,
      1.000000f,  1.881400f, -0.000000f,  0.000000f, -0.940930f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_12bit_limited_rgb_to_yuv[] = {
      0.224787f,  0.580149f,  0.050742f,  0.000000f,  0.062515f,
     -0.122206f, -0.315401f,  0.437607f,  0.000000f,  0.500122f,
      0.437607f, -0.402411f, -0.035196f,  0.000000f,  0.500122f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_12bit_limited_yuv_to_rgb[] = {
      1.168664f, -0.000000f,  1.684846f,  0.000000f, -0.915688f,
      1.168664f, -0.188015f, -0.652816f,  0.000000f,  0.347458f,
      1.168664f,  2.149647f, -0.000000f,  0.000000f, -1.148145f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_16bit_full_rgb_to_yuv[] = {
      0.262700f,  0.678000f,  0.059300f,  0.000000f,  0.000000f,
     -0.139630f, -0.360370f,  0.500000f,  0.000000f,  0.500008f,
      0.500000f, -0.459786f, -0.040214f,  0.000000f,  0.500008f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_16bit_full_yuv_to_rgb[] = {
      1.000000f, -0.000000f,  1.474600f,  0.000000f, -0.737311f,
      1.000000f, -0.164553f, -0.571353f,  0.000000f,  0.367959f,
      1.000000f,  1.881400f, -0.000000f,  0.000000f, -0.940714f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_16bit_limited_rgb_to_yuv[] = {
      0.224735f,  0.580017f,  0.050730f,  0.000000f,  0.062501f,
     -0.122178f, -0.315329f,  0.437507f,  0.000000f,  0.500008f,
      0.437507f, -0.402319f, -0.035188f,  0.000000f,  0.500008f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float BT2020_16bit_limited_yuv_to_rgb[] = {
      1.168932f,  0.000000f,  1.685231f,  0.000000f, -0.915688f,
      1.168932f, -0.188058f, -0.652965f,  0.000000f,  0.347458f,
      1.168932f,  2.150139f, -0.000000f,  0.000000f, -1.148145f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float FCC_full_rgb_to_yuv[] = {
      0.300000f,  0.590000f,  0.110000f,  0.000000f,  0.000000f,
     -0.168539f, -0.331461f,  0.500000f,  0.000000f,  0.501961f,
      0.500000f, -0.421429f, -0.078571f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float FCC_full_yuv_to_rgb[] = {
      1.000000f,  0.000000f,  1.400000f,  0.000000f, -0.702745f,
      1.000000f, -0.331864f, -0.711864f,  0.000000f,  0.523911f,
      1.000000f,  1.780000f,  0.000000f,  0.000000f, -0.893490f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float FCC_limited_rgb_to_yuv[] = {
      0.257647f,  0.506706f,  0.094471f,  0.000000f,  0.062745f,
     -0.148050f, -0.291165f,  0.439216f,  0.000000f,  0.501961f,
      0.439216f, -0.370196f, -0.069020f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float FCC_limited_yuv_to_rgb[] = {
      1.164384f, -0.000000f,  1.593750f,  0.000000f, -0.873059f,
      1.164384f, -0.377792f, -0.810381f,  0.000000f,  0.523357f,
      1.164384f,  2.026339f,  0.000000f,  0.000000f, -1.090202f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float SMPTE240_full_rgb_to_yuv[] = {
      0.212000f,  0.701000f,  0.087000f,  0.000000f,  0.000000f,
     -0.116101f, -0.383899f,  0.500000f,  0.000000f,  0.501961f,
      0.500000f, -0.444797f, -0.055203f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float SMPTE240_full_yuv_to_rgb[] = {
      1.000000f,  0.000000f,  1.576000f,  0.000000f, -0.791090f,
      1.000000f, -0.226622f, -0.476622f,  0.000000f,  0.353001f,
      1.000000f,  1.826000f,  0.000000f,  0.000000f, -0.916580f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float SMPTE240_limited_rgb_to_yuv[] = {
      0.182071f,  0.602035f,  0.074718f,  0.000000f,  0.062745f,
     -0.101987f, -0.337229f,  0.439216f,  0.000000f,  0.501961f,
      0.439216f, -0.390724f, -0.048492f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float SMPTE240_limited_yuv_to_rgb[] = {
      1.164384f, -0.000000f,  1.794107f,  0.000000f, -0.973631f,
      1.164384f, -0.257985f, -0.542583f,  0.000000f,  0.328794f,
      1.164384f,  2.078705f,  0.000000f,  0.000000f, -1.116488f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YDZDX_full_rgb_to_yuv[] = {
      0.000000f,  1.000000f,  0.000000f,  0.000000f,  0.000000f,
      0.000000f, -0.500000f,  0.493283f,  0.000000f,  0.501961f,
      0.500000f, -0.495951f,  0.000000f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YDZDX_full_yuv_to_rgb[] = {
      0.991902f, -0.000000f,  2.000000f,  0.000000f, -1.003922f,
      1.000000f,  0.000000f,  0.000000f,  0.000000f,  0.000000f,
      1.013617f,  2.027234f,  0.000000f,  0.000000f, -1.017592f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YDZDX_limited_rgb_to_yuv[] = {
      0.000000f,  0.858824f,  0.000000f,  0.000000f,  0.062745f,
      0.000000f, -0.439216f,  0.433315f,  0.000000f,  0.501961f,
      0.439216f, -0.435659f,  0.000000f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YDZDX_limited_yuv_to_rgb[] = {
      1.154954f, -0.000000f,  2.276786f,  0.000000f, -1.215325f,
      1.164384f,  0.000000f,  0.000000f,  0.000000f, -0.073059f,
      1.180239f,  2.307788f,  0.000000f,  0.000000f, -1.232474f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float GBR_full_rgb_to_yuv[] = {
      0.000000f,  1.000000f,  0.000000f,  0.000000f,  0.000000f,
      0.000000f,  0.000000f,  1.000000f,  0.000000f,  0.000000f,
      1.000000f,  0.000000f,  0.000000f,  0.000000f,  0.000000f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float GBR_full_yuv_to_rgb[] = {
      0.000000f,  0.000000f,  1.000000f,  0.000000f,  0.000000f,
      1.000000f,  0.000000f,  0.000000f,  0.000000f,  0.000000f,
      0.000000f,  1.000000f,  0.000000f,  0.000000f, -0.000000f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float GBR_limited_rgb_to_yuv[] = {
      0.000000f,  0.858824f,  0.000000f,  0.000000f,  0.062745f,
      0.000000f,  0.000000f,  0.858824f,  0.000000f,  0.062745f,
      0.858824f,  0.000000f,  0.000000f,  0.000000f,  0.062745f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float GBR_limited_yuv_to_rgb[] = {
      0.000000f,  0.000000f,  1.164384f,  0.000000f, -0.073059f,
      1.164384f,  0.000000f,  0.000000f,  0.000000f, -0.073059f,
      0.000000f,  1.164384f,  0.000000f,  0.000000f, -0.073059f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_8bit_full_rgb_to_yuv[] = {
      0.250000f,  0.500000f,  0.250000f,  0.000000f,  0.000000f,
     -0.250000f,  0.500000f, -0.250000f,  0.000000f,  0.501961f,
      0.500000f,  0.000000f, -0.500000f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_8bit_full_yuv_to_rgb[] = {
      1.000000f, -1.000000f,  1.000000f,  0.000000f,  0.000000f,
      1.000000f,  1.000000f,  0.000000f,  0.000000f, -0.501961f,
      1.000000f, -1.000000f, -1.000000f,  0.000000f,  1.003922f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_8bit_limited_rgb_to_yuv[] = {
      0.214706f,  0.429412f,  0.214706f,  0.000000f,  0.062745f,
     -0.214706f,  0.429412f, -0.214706f,  0.000000f,  0.501961f,
      0.429412f,  0.000000f, -0.429412f,  0.000000f,  0.501961f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_8bit_limited_yuv_to_rgb[] = {
      1.164384f, -1.164384f,  1.164384f,  0.000000f, -0.073059f,
      1.164384f,  1.164384f,  0.000000f,  0.000000f, -0.657534f,
      1.164384f, -1.164384f, -1.164384f,  0.000000f,  1.095891f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_10bit_full_rgb_to_yuv[] = {
      0.250000f,  0.500000f,  0.250000f,  0.000000f,  0.000000f,
     -0.250000f,  0.500000f, -0.250000f,  0.000000f,  0.500489f,
      0.500000f,  0.000000f, -0.500000f,  0.000000f,  0.500489f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_10bit_full_yuv_to_rgb[] = {
      1.000000f, -1.000000f,  1.000000f,  0.000000f,  0.000000f,
      1.000000f,  1.000000f,  0.000000f,  0.000000f, -0.500489f,
      1.000000f, -1.000000f, -1.000000f,  0.000000f,  1.000978f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_10bit_limited_rgb_to_yuv[] = {
      0.214076f,  0.428153f,  0.214076f,  0.000000f,  0.062561f,
     -0.214076f,  0.428153f, -0.214076f,  0.000000f,  0.500489f,
      0.428153f,  0.000000f, -0.428153f,  0.000000f,  0.500489f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_10bit_limited_yuv_to_rgb[] = {
      1.167808f, -1.167808f,  1.167808f,  0.000000f, -0.073059f,
      1.167808f,  1.167808f,  0.000000f,  0.000000f, -0.657534f,
      1.167808f, -1.167808f, -1.167808f,  0.000000f,  1.095890f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_12bit_full_rgb_to_yuv[] = {
      0.250000f,  0.500000f,  0.250000f,  0.000000f,  0.000000f,
     -0.250000f,  0.500000f, -0.250000f,  0.000000f,  0.500122f,
      0.500000f,  0.000000f, -0.500000f,  0.000000f,  0.500122f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_12bit_full_yuv_to_rgb[] = {
      1.000000f, -1.000000f,  1.000000f,  0.000000f,  0.000000f,
      1.000000f,  1.000000f,  0.000000f,  0.000000f, -0.500122f,
      1.000000f, -1.000000f, -1.000000f,  0.000000f,  1.000244f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_12bit_limited_rgb_to_yuv[] = {
      0.213919f,  0.427839f,  0.213919f,  0.000000f,  0.062515f,
     -0.213919f,  0.427839f, -0.213919f,  0.000000f,  0.500122f,
      0.427839f,  0.000000f, -0.427839f,  0.000000f,  0.500122f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_12bit_limited_yuv_to_rgb[] = {
      1.168664f, -1.168664f,  1.168664f,  0.000000f, -0.073059f,
      1.168664f,  1.168664f,  0.000000f,  0.000000f, -0.657534f,
      1.168664f, -1.168664f, -1.168664f,  0.000000f,  1.095891f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_16bit_full_rgb_to_yuv[] = {
      0.250000f,  0.500000f,  0.250000f,  0.000000f,  0.000000f,
     -0.250000f,  0.500000f, -0.250000f,  0.000000f,  0.500008f,
      0.500000f,  0.000000f, -0.500000f,  0.000000f,  0.500008f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_16bit_full_yuv_to_rgb[] = {
      1.000000f, -1.000000f,  1.000000f,  0.000000f,  0.000000f,
      1.000000f,  1.000000f,  0.000000f,  0.000000f, -0.500008f,
      1.000000f, -1.000000f, -1.000000f,  0.000000f,  1.000015f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_16bit_limited_rgb_to_yuv[] = {
      0.213870f,  0.427741f,  0.213870f,  0.000000f,  0.062501f,
     -0.213870f,  0.427741f, -0.213870f,  0.000000f,  0.500008f,
      0.427741f,  0.000000f, -0.427741f,  0.000000f,  0.500008f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};
const float YCgCo_16bit_limited_yuv_to_rgb[] = {
      1.168932f, -1.168932f,  1.168932f,  0.000000f, -0.073059f,
      1.168932f,  1.168932f,  0.000000f,  0.000000f, -0.657534f,
      1.168932f, -1.168932f, -1.168932f,  0.000000f,  1.095890f,
      0.000000f,  0.000000f,  0.000000f,  1.000000f,  0.000000f,
};

static_assert(kJPEG_Full_SkYUVColorSpace            == 0, "");
static_assert(kRec601_Limited_SkYUVColorSpace       == 1, "");
static_assert(kRec709_Full_SkYUVColorSpace          == 2, "");
static_assert(kRec709_Limited_SkYUVColorSpace       == 3, "");
static_assert(kBT2020_8bit_Full_SkYUVColorSpace     == 4, "");
static_assert(kBT2020_8bit_Limited_SkYUVColorSpace  == 5, "");
static_assert(kBT2020_10bit_Full_SkYUVColorSpace    == 6, "");
static_assert(kBT2020_10bit_Limited_SkYUVColorSpace == 7, "");
static_assert(kBT2020_12bit_Full_SkYUVColorSpace    == 8, "");
static_assert(kBT2020_12bit_Limited_SkYUVColorSpace == 9, "");
static_assert(kBT2020_16bit_Full_SkYUVColorSpace    == 10, "");
static_assert(kBT2020_16bit_Limited_SkYUVColorSpace == 11, "");
static_assert(kFCC_Full_SkYUVColorSpace             == 12, "");
static_assert(kFCC_Limited_SkYUVColorSpace          == 13, "");
static_assert(kSMPTE240_Full_SkYUVColorSpace        == 14, "");
static_assert(kSMPTE240_Limited_SkYUVColorSpace     == 15, "");
static_assert(kYDZDX_Full_SkYUVColorSpace           == 16, "");
static_assert(kYDZDX_Limited_SkYUVColorSpace        == 17, "");
static_assert(kGBR_Full_SkYUVColorSpace             == 18, "");
static_assert(kGBR_Limited_SkYUVColorSpace          == 19, "");
static_assert(kYCgCo_8bit_Full_SkYUVColorSpace      == 20, "");
static_assert(kYCgCo_8bit_Limited_SkYUVColorSpace   == 21, "");
static_assert(kYCgCo_10bit_Full_SkYUVColorSpace     == 22, "");
static_assert(kYCgCo_10bit_Limited_SkYUVColorSpace  == 23, "");
static_assert(kYCgCo_12bit_Full_SkYUVColorSpace     == 24, "");
static_assert(kYCgCo_12bit_Limited_SkYUVColorSpace  == 25, "");
static_assert(kYCgCo_16bit_Full_SkYUVColorSpace     == 26, "");
static_assert(kYCgCo_16bit_Limited_SkYUVColorSpace  == 27, "");

const float* yuv_to_rgb_array[] = {
    JPEG_full_yuv_to_rgb,
    Rec601_limited_yuv_to_rgb,
    Rec709_full_yuv_to_rgb,
    Rec709_limited_yuv_to_rgb,
    BT2020_8bit_full_yuv_to_rgb,
    BT2020_8bit_limited_yuv_to_rgb,
    BT2020_10bit_full_yuv_to_rgb,
    BT2020_10bit_limited_yuv_to_rgb,
    BT2020_12bit_full_yuv_to_rgb,
    BT2020_12bit_limited_yuv_to_rgb,
    BT2020_16bit_full_yuv_to_rgb,
    BT2020_16bit_limited_yuv_to_rgb,
    FCC_full_yuv_to_rgb,
    FCC_limited_yuv_to_rgb,
    SMPTE240_full_yuv_to_rgb,
    SMPTE240_limited_yuv_to_rgb,
    YDZDX_full_yuv_to_rgb,
    YDZDX_limited_yuv_to_rgb,
    GBR_full_yuv_to_rgb,
    GBR_limited_yuv_to_rgb,
    YCgCo_8bit_full_yuv_to_rgb,
    YCgCo_8bit_limited_yuv_to_rgb,
    YCgCo_10bit_full_yuv_to_rgb,
    YCgCo_10bit_limited_yuv_to_rgb,
    YCgCo_12bit_full_yuv_to_rgb,
    YCgCo_12bit_limited_yuv_to_rgb,
    YCgCo_16bit_full_yuv_to_rgb,
    YCgCo_16bit_limited_yuv_to_rgb,
};

const float* rgb_to_yuv_array[] = {
    JPEG_full_rgb_to_yuv,
    Rec601_limited_rgb_to_yuv,
    Rec709_full_rgb_to_yuv,
    Rec709_limited_rgb_to_yuv,
    BT2020_8bit_full_rgb_to_yuv,
    BT2020_8bit_limited_rgb_to_yuv,
    BT2020_10bit_full_rgb_to_yuv,
    BT2020_10bit_limited_rgb_to_yuv,
    BT2020_12bit_full_rgb_to_yuv,
    BT2020_12bit_limited_rgb_to_yuv,
    BT2020_16bit_full_rgb_to_yuv,
    BT2020_16bit_limited_rgb_to_yuv,
    FCC_full_rgb_to_yuv,
    FCC_limited_rgb_to_yuv,
    SMPTE240_full_rgb_to_yuv,
    SMPTE240_limited_rgb_to_yuv,
    YDZDX_full_rgb_to_yuv,
    YDZDX_limited_rgb_to_yuv,
    GBR_full_rgb_to_yuv,
    GBR_limited_rgb_to_yuv,
    YCgCo_8bit_full_rgb_to_yuv,
    YCgCo_8bit_limited_rgb_to_yuv,
    YCgCo_10bit_full_rgb_to_yuv,
    YCgCo_10bit_limited_rgb_to_yuv,
    YCgCo_12bit_full_rgb_to_yuv,
    YCgCo_12bit_limited_rgb_to_yuv,
    YCgCo_16bit_full_rgb_to_yuv,
    YCgCo_16bit_limited_rgb_to_yuv,
};

constexpr size_t kSizeOfColorMatrix = 20 * sizeof(float);

void SkColorMatrix_RGB2YUV(SkYUVColorSpace cs, float m[20]) {
    if ((unsigned)cs < (unsigned)kIdentity_SkYUVColorSpace) {
        memcpy(m, rgb_to_yuv_array[(unsigned)cs], kSizeOfColorMatrix);
    } else {
        memset(m, 0, kSizeOfColorMatrix);
        m[0] = m[6] = m[12] = m[18] = 1;
    }
}

void SkColorMatrix_YUV2RGB(SkYUVColorSpace cs, float m[20]) {
    if ((unsigned)cs < (unsigned)kIdentity_SkYUVColorSpace) {
        memcpy(m, yuv_to_rgb_array[(unsigned)cs], kSizeOfColorMatrix);
    } else {
        memset(m, 0, kSizeOfColorMatrix);
        m[0] = m[6] = m[12] = m[18] = 1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// we just drop the alpha rol/col from the colormatrix
// output is |        tr |
//           |  3x3   tg |
//           |        tb |
//           | 0 0 0  1  |
static void colormatrix_to_matrix44(const float src[20], SkM44* dst) {
    *dst = SkM44(src[ 0], src[ 1], src[ 2], src[ 4],
                 src[ 5], src[ 6], src[ 7], src[ 9],
                 src[10], src[11], src[12], src[14],
                       0,       0,       0,       1);
}

// input: ignore the bottom row
// output: inject identity row/column for alpha
static void matrix44_to_colormatrix(const SkM44& src, float dst[20]) {
    dst[0] = src.rc(0,0);
    dst[1] = src.rc(0,1);
    dst[2] = src.rc(0,2);
    dst[3] = 0;
    dst[4] = src.rc(0,3);    // tx

    dst[5] = src.rc(1,0);
    dst[6] = src.rc(1,1);
    dst[7] = src.rc(1,2);
    dst[8] = 0;
    dst[9] = src.rc(1,3);    // ty

    dst[10] = src.rc(2,0);
    dst[11] = src.rc(2,1);
    dst[12] = src.rc(2,2);
    dst[13] = 0;
    dst[14] = src.rc(2,3);   // tz

    dst[15] = dst[16] = dst[17] = dst[19] = 0;
    dst[18] = 1;
}

static void scale3(float m[], float s) {
    for (int i = 0; i < 3; ++i) {
        m[i] *= s;
    }
}

namespace {
enum Range { kFull, kLimited };
struct YUVCoeff {
    float   Kr, Kb;
    int     bits;
    Range   range;
};

const YUVCoeff gCoeff[] = {
    { 0.2990f, 0.1140f,  8, kFull    }, // kJPEG_Full_SkYUVColorSpace
    { 0.2990f, 0.1140f,  8, kLimited }, // kRec601_Limited_SkYUVColorSpace
    { 0.2126f, 0.0722f,  8, kFull    }, // kRec709_Full_SkYUVColorSpace
    { 0.2126f, 0.0722f,  8, kLimited }, // kRec709_Limited_SkYUVColorSpace
    { 0.2627f, 0.0593f,  8, kFull    }, // kBT2020_8bit_Full_SkYUVColorSpace
    { 0.2627f, 0.0593f,  8, kLimited }, // kBT2020_8bit_Limited_SkYUVColorSpace
    { 0.2627f, 0.0593f, 10, kFull    }, // kBT2020_10bit_Full_SkYUVColorSpace
    { 0.2627f, 0.0593f, 10, kLimited }, // kBT2020_10bit_Limited_SkYUVColorSpace
    { 0.2627f, 0.0593f, 12, kFull    }, // kBT2020_12bit_Full_SkYUVColorSpace
    { 0.2627f, 0.0593f, 12, kLimited }, // kBT2020_12bit_Limited_SkYUVColorSpace
    { 0.2627f, 0.0593f, 16, kFull    }, // kBT2020_16bit_Full_SkYUVColorSpace
    { 0.2627f, 0.0593f, 16, kLimited }, // kBT2020_16bit_Limited_SkYUVColorSpace
    { 0.3000f, 0.1100f,  8, kFull    }, // kFCC_Full_SkYUVColorSpace
    { 0.3000f, 0.1100f,  8, kLimited }, // kFCC_Limited_SkYUVColorSpace
    { 0.2120f, 0.0870f,  8, kFull    }, // kSMPTE240_Full_SkYUVColorSpace
    { 0.2120f, 0.0870f,  8, kLimited }, // kSMPTE240_Limited_SkYUVColorSpace
};
}  // namespace

static void make_rgb_to_yuv_matrix_ycbcr(float mx[20], const YUVCoeff& c) {
    SkASSERT(c.bits >= 8);
    const float Kr = c.Kr;
    const float Kb = c.Kb;
    const float Kg = 1.0f - Kr - Kb;
    const float Cr = 0.5f / (1.0f - Kb);
    const float Cb = 0.5f / (1.0f - Kr);

    const int shift = c.bits - 8;

    const float denom = static_cast<float>((1 << c.bits) - 1);
    float scaleY  = 1.0f,
          addY    = 0.0f,
          scaleUV = 1.0f,
          addUV   = (128 << shift) / denom;

    if (c.range == kLimited) {
        scaleY  = (219 << shift) / denom;
        addY    = ( 16 << shift) / denom;
        scaleUV = (224 << shift) / denom;
    }

    float m[20] = {
          Kr,  Kg,   Kb,  0,  addY,
         -Kr, -Kg, 1-Kb,  0, addUV,
        1-Kr, -Kg,  -Kb,  0, addUV,
           0,   0,    0,  1,     0,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0,      scaleY );
    scale3(mx +  5, Cr * scaleUV);
    scale3(mx + 10, Cb * scaleUV);
}

static void make_rgb_to_yuv_matrix_ydzdx(float mx[20], Range range) {
    const int bits = 8;
    const float denom = static_cast<float>((1 << bits) - 1);
    float scaleY  = 1.0f,
          addY    = 0.0f,
          scaleUV = 1.0f,
          addUV   = 128 / denom;

    if (range == kLimited) {
        scaleY  = 219 / denom;
        addY    =  16 / denom;
        scaleUV = 224 / denom;
    }

    // YDZDX applies range correction to YUV values similar to YCbCr.
    float m[20] = {
          0.0f,              1.0f,             0.0f, 0.0f,  addY,  // Y
          0.0f,             -0.5f, 0.986566f / 2.0f, 0.0f, addUV,  // DX or DZ
          0.5f, -0.991902f / 2.0f,             0.0f, 0.0f, addUV,  // DZ or DX
          0.0f,              0.0f,             0.0f, 1.0f,  0.0f,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, scaleY );
    scale3(mx +  5, scaleUV);
    scale3(mx + 10, scaleUV);
}

static void make_rgb_to_yuv_matrix_gbr(float mx[20], Range range) {
    const int bits = 8;
    const float denom = static_cast<float>((1 << bits) - 1);
    float scaleY   = 1.0f,
          addY     = 0.0f;

    if (range == kLimited) {
        scaleY  = 219 / denom;
        addY    =  16 / denom;
    }

    // GBR applies range correction to RGB values similar to YCgCo.
    float m[20] = {
          0.0f, 1.0f, 0.0f, 0.0f,  addY, // G
          0.0f, 0.0f, 1.0f, 0.0f,  addY, // B
          1.0f, 0.0f, 0.0f, 0.0f,  addY, // R
          0.0f, 0.0f, 0.0f, 1.0f,  0.0f,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, scaleY);
    scale3(mx +  5, scaleY);
    scale3(mx + 10, scaleY);
}

static void make_rgb_to_yuv_matrix_ycgco(float mx[20], int bits, Range range) {
    SkASSERT(bits >= 8);
    const int shift = bits - 8;
    const float denom = static_cast<float>((1 << bits) - 1);
    float scaleY   = 1.0f,
          addY     = 0.0f,
          chroma05 = static_cast<float>(1 << (bits - 1)) / denom;

    if (range == kLimited) {
        scaleY  = (219 << shift) / denom;
        addY    = ( 16 << shift) / denom;
    }

    float m[20] = {
          0.25f,  0.5f,   0.25f,  0.0f,  addY,
         -0.25f,  0.5f,  -0.25f,  0.0f,  chroma05,
           0.5f,  0.0f,  -0.5f,   0.0f,  chroma05,
           0.0f,  0.0f,   0.0f,   1.0f,  0.0f,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, scaleY);
    scale3(mx +  5, scaleY);
    scale3(mx + 10, scaleY);
}

static void make_rgb_to_yuv_matrix(float mx[20], SkYUVColorSpace cs) {
    switch (cs) {
        case kJPEG_Full_SkYUVColorSpace:
        case kRec601_Limited_SkYUVColorSpace:
        case kRec709_Full_SkYUVColorSpace:
        case kRec709_Limited_SkYUVColorSpace:
        case kBT2020_8bit_Full_SkYUVColorSpace:
        case kBT2020_8bit_Limited_SkYUVColorSpace:
        case kBT2020_10bit_Full_SkYUVColorSpace:
        case kBT2020_10bit_Limited_SkYUVColorSpace:
        case kBT2020_12bit_Full_SkYUVColorSpace:
        case kBT2020_12bit_Limited_SkYUVColorSpace:
        case kBT2020_16bit_Full_SkYUVColorSpace:
        case kBT2020_16bit_Limited_SkYUVColorSpace:
        case kFCC_Full_SkYUVColorSpace:
        case kFCC_Limited_SkYUVColorSpace:
        case kSMPTE240_Full_SkYUVColorSpace:
        case kSMPTE240_Limited_SkYUVColorSpace:
        case kIdentity_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ycbcr(mx, gCoeff[(unsigned)cs]);
        case kYDZDX_Full_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ydzdx(mx, Range::kFull);
        case kYDZDX_Limited_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ydzdx(mx, Range::kLimited);
        case kGBR_Full_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_gbr(mx, Range::kFull);
        case kGBR_Limited_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_gbr(mx, Range::kLimited);
        case kYCgCo_8bit_Full_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ycgco(mx, /*bits=*/8, Range::kFull);
        case kYCgCo_8bit_Limited_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ycgco(mx, /*bits=*/8, Range::kLimited);
        case kYCgCo_10bit_Full_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ycgco(mx, /*bits=*/10, Range::kFull);
        case kYCgCo_10bit_Limited_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ycgco(mx, /*bits=*/10, Range::kLimited);
        case kYCgCo_12bit_Full_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ycgco(mx, /*bits=*/12, Range::kFull);
        case kYCgCo_12bit_Limited_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ycgco(mx, /*bits=*/12, Range::kLimited);
        case kYCgCo_16bit_Full_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ycgco(mx, /*bits=*/16, Range::kFull);
        case kYCgCo_16bit_Limited_SkYUVColorSpace:
            return make_rgb_to_yuv_matrix_ycgco(mx, /*bits=*/16, Range::kLimited);
    }
}

static void dump(const float m[20], SkYUVColorSpace cs, bool rgb2yuv) {
    const char* names[] = {
        "JPEG_full",
        "Rec601_limited",
        "Rec709_full",
        "Rec709_limited",
        "BT2020_8bit_full",
        "BT2020_8bit_limited",
        "BT2020_10bit_full",
        "BT2020_10bit_limited",
        "BT2020_12bit_full",
        "BT2020_12bit_limited",
        "BT2020_16bit_full",
        "BT2020_16bit_limited",
        "FCC_full",
        "FCC_limited",
        "SMPTE240_full",
        "SMPTE240_limited",
        "YDZDX_full",
        "YDZDX_limited",
        "GBR_full",
        "GBR_limited",
        "YCgCo_8bit_full",
        "YCgCo_8bit_limited",
        "YCgCo_10bit_full",
        "YCgCo_10bit_limited",
        "YCgCo_12bit_full",
        "YCgCo_12bit_limited",
        "YCgCo_16bit_full",
        "YCgCo_16bit_limited",
    };
    const char* dirnames[] = {
        "yuv_to_rgb", "rgb_to_yuv",
    };
    SkDebugf("const float %s_%s[] = {\n", names[cs], dirnames[rgb2yuv]);
    for (int i = 0; i < 4; ++i) {
        SkDebugf("    ");
        for (int j = 0; j < 5; ++j) {
            SkDebugf(" %9.6ff,", m[i * 5 + j]);
        }
        SkDebugf("\n");
    }
    SkDebugf("};\n");
}

// Used to create the prebuilt tables for each colorspace.
// Don't remove this function, in case we want to recompute those tables in the future.
void SkColorMatrix_DumpYUVMatrixTables() {
    for (int i = 0; i < kLastEnum_SkYUVColorSpace; ++i) {
        SkYUVColorSpace cs = static_cast<SkYUVColorSpace>(i);
        float m[20];
        make_rgb_to_yuv_matrix(m, cs);
        dump(m, cs, true);
        SkM44 m44, im44;
        colormatrix_to_matrix44(m, &m44);
        float im[20];
#ifdef SK_DEBUG
        // be sure our coversion between matrix44 and colormatrix is perfect
        matrix44_to_colormatrix(m44, im);
        SkASSERT(memcmp(m, im, sizeof(im)) == 0);
#endif
        SkAssertResult(m44.invert(&im44));
        matrix44_to_colormatrix(im44, im);
        dump(im, cs, false);
    }
}
