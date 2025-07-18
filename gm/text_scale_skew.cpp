/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/utils/SkTextUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <initializer_list>

class SkCanvas;

static void draw_the_text(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    SkFont font = ToolUtils::DefaultPortableFont();
    font.setSize(18.0f);
    float y = 10.0f;
    for (float scale : { 0.5f, 0.71f, 1.0f, 1.41f, 2.0f }) {
        font.setScaleX(scale);
        y += font.getSpacing();
        float x = 50.0f;
        for (float skew : { -0.5f, 0.0f, 0.5f }) {
            font.setSkewX(skew);
            SkTextUtils::DrawString(canvas, "Skia", x, y, font, p, SkTextUtils::kCenter_Align);
            x += 78.0f;
        }
    }
}

// skbug.com/40038559
DEF_SIMPLE_GM(text_scale_skew, canvas, 256, 128) {
    draw_the_text(canvas);
}

// make sure we apply matrices in the correct order inside scalercontext
DEF_SIMPLE_GM(text_scale_skew_rotate, canvas, 256, 128) {
    canvas->rotate(30, 128, 64);

    draw_the_text(canvas);
}
