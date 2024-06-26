// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(ImageInfo_validRowBytes, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(16, 8);
    for (size_t rowBytes = 60; rowBytes < 72; rowBytes += sizeof(SkPMColor)) {
        SkDebugf("validRowBytes(%zu): %s\n", rowBytes, info.validRowBytes(rowBytes) ?
                 "true" : "false");
    }
}
}  // END FIDDLE
