// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Matrix_mapPoints, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.reset();
    const int count = 4;
    SkPoint src[count];
    matrix.mapRectToQuad(src, {40, 70, 180, 220} );
    SkPaint paint;
    paint.setARGB(77, 23, 99, 154);
    for (int i = 0; i < 5; ++i) {
        SkPoint dst[count];
        matrix.mapPoints(dst, src);
        canvas->drawPoints(SkCanvas::kPolygon_PointMode, dst, paint);
        matrix.preRotate(35, 128, 128);
    }
}
}  // END FIDDLE
