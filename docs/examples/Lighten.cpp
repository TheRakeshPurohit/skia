// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Lighten, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    SkColor colors[] = { SK_ColorBLACK, SK_ColorWHITE };
    SkPoint horz[] = { { 0, 0 }, { 256, 0 } };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(horz, colors, nullptr, std::size(colors),
            SkTileMode::kClamp));
    paint.setBlendMode(SkBlendMode::kLighten);
    canvas->drawPaint(paint);
}
}  // END FIDDLE
