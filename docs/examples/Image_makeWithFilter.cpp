// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Image_makeWithFilter, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    sk_sp<SkImageFilter> shadowFilter = SkImageFilters::DropShadow(
                -10.0f * frame, 5.0f * frame, 3.0f, 3.0f, SK_ColorBLUE, nullptr);
    sk_sp<SkImageFilter> offsetFilter = SkImageFilters::Offset(40, 40, shadowFilter, nullptr);
    SkIRect subset = image->bounds();
    SkIRect clipBounds = image->bounds();
    clipBounds.outset(60, 60);
    SkIRect outSubset;
    SkIPoint offset;
    sk_sp<SkImage> filtered;

    if (auto rContext = canvas->recordingContext()) {
        filtered = SkImages::MakeWithFilter(rContext, image, offsetFilter.get(),
                                            subset, clipBounds, &outSubset, &offset);
    } else {
        filtered = SkImages::MakeWithFilter(image, offsetFilter.get(),
                                            subset, clipBounds, &outSubset, &offset);
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawLine(0, 0, offset.fX, offset.fY, paint);
    canvas->translate(offset.fX, offset.fY);
    canvas->drawImage(filtered, 0, 0);
    canvas->drawRect(SkRect::Make(outSubset), paint);
}
}  // END FIDDLE
