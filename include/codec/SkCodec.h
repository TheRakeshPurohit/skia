/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_DEFINED
#define SkCodec_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkNoncopyable.h"
#include "modules/skcms/skcms.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <vector>

class SkData;
class SkFrameHolder;
class SkImage;
class SkPngChunkReader;
class SkSampler;
class SkStream;
struct SkGainmapInfo;
enum SkAlphaType : int;
enum class SkEncodedImageFormat;

namespace SkCodecAnimation {
enum class Blend;
enum class DisposalMethod;
}

namespace DM {
class CodecSrc;
} // namespace DM

namespace SkCodecs {
struct Decoder;
}

/**
 *  Abstraction layer directly on top of an image codec.
 */
class SK_API SkCodec : SkNoncopyable {
public:
    /**
     *  Minimum number of bytes that must be buffered in SkStream input.
     *
     *  An SkStream passed to NewFromStream must be able to use this many
     *  bytes to determine the image type. Then the same SkStream must be
     *  passed to the correct decoder to read from the beginning.
     *
     *  This can be accomplished by implementing peek() to support peeking
     *  this many bytes, or by implementing rewind() to be able to rewind()
     *  after reading this many bytes.
     */
    static constexpr size_t MinBufferedBytesNeeded() { return 32; }

    /**
     *  Error codes for various SkCodec methods.
     */
    enum Result {
        /**
         *  General return value for success.
         */
        kSuccess,
        /**
         *  The input is incomplete. A partial image was generated.
         */
        kIncompleteInput,
        /**
         *  Like kIncompleteInput, except the input had an error.
         *
         *  If returned from an incremental decode, decoding cannot continue,
         *  even with more data.
         */
        kErrorInInput,
        /**
         *  The generator cannot convert to match the request, ignoring
         *  dimensions.
         */
        kInvalidConversion,
        /**
         *  The generator cannot scale to requested size.
         */
        kInvalidScale,
        /**
         *  Parameters (besides info) are invalid. e.g. NULL pixels, rowBytes
         *  too small, etc.
         */
        kInvalidParameters,
        /**
         *  The input did not contain a valid image.
         */
        kInvalidInput,
        /**
         *  Fulfilling this request requires rewinding the input, which is not
         *  supported for this input.
         */
        kCouldNotRewind,
        /**
         *  An internal error, such as OOM.
         */
        kInternalError,
        /**
         *  This method is not implemented by this codec.
         *  FIXME: Perhaps this should be kUnsupported?
         */
        kUnimplemented,
    };

    /**
     *  Readable string representing the error code.
     */
    static const char* ResultToString(Result);

    /**
     * For container formats that contain both still images and image sequences,
     * instruct the decoder how the output should be selected. (Refer to comments
     * for each value for more details.)
     */
    enum class SelectionPolicy {
        /**
         *  If the container format contains both still images and image sequences,
         *  SkCodec should choose one of the still images. This is the default.
         *  Note that kPreferStillImage may prevent use of the animation features
         *  if the input is not rewindable.
         */
        kPreferStillImage,
        /**
         *  If the container format contains both still images and image sequences,
         *  SkCodec should choose one of the image sequences for animation.
         */
        kPreferAnimation,
    };

    /**
     *  If this stream represents an encoded image that we know how to decode,
     *  return an SkCodec that can decode it. Otherwise return NULL.
     *
     *  As stated above, this call must be able to peek or read
     *  MinBufferedBytesNeeded to determine the correct format, and then start
     *  reading from the beginning. First it will attempt to peek, and it
     *  assumes that if less than MinBufferedBytesNeeded bytes (but more than
     *  zero) are returned, this is because the stream is shorter than this,
     *  so falling back to reading would not provide more data. If peek()
     *  returns zero bytes, this call will instead attempt to read(). This
     *  will require that the stream can be rewind()ed.
     *
     *  If Result is not NULL, it will be set to either kSuccess if an SkCodec
     *  is returned or a reason for the failure if NULL is returned.
     *
     *  If SkPngChunkReader is not NULL, take a ref and pass it to libpng if
     *  the image is a png.
     *
     *  If the SkPngChunkReader is not NULL then:
     *      If the image is not a PNG, the SkPngChunkReader will be ignored.
     *      If the image is a PNG, the SkPngChunkReader will be reffed.
     *      If the PNG has unknown chunks, the SkPngChunkReader will be used
     *      to handle these chunks.  SkPngChunkReader will be called to read
     *      any unknown chunk at any point during the creation of the codec
     *      or the decode.  Note that if SkPngChunkReader fails to read a
     *      chunk, this could result in a failure to create the codec or a
     *      failure to decode the image.
     *      If the PNG does not contain unknown chunks, the SkPngChunkReader
     *      will not be used or modified.
     *
     *  If NULL is returned, the stream is deleted immediately. Otherwise, the
     *  SkCodec takes ownership of it, and will delete it when done with it.
     */
    static std::unique_ptr<SkCodec> MakeFromStream(
            std::unique_ptr<SkStream>,
            SkSpan<const SkCodecs::Decoder> decoders,
            Result* = nullptr,
            SkPngChunkReader* = nullptr,
            SelectionPolicy selectionPolicy = SelectionPolicy::kPreferStillImage);
    // deprecated
    static std::unique_ptr<SkCodec> MakeFromStream(
            std::unique_ptr<SkStream>,
            Result* = nullptr,
            SkPngChunkReader* = nullptr,
            SelectionPolicy selectionPolicy = SelectionPolicy::kPreferStillImage);

    /**
     *  If this data represents an encoded image that we know how to decode,
     *  return an SkCodec that can decode it. Otherwise return NULL.
     *
     *  If the SkPngChunkReader is not NULL then:
     *      If the image is not a PNG, the SkPngChunkReader will be ignored.
     *      If the image is a PNG, the SkPngChunkReader will be reffed.
     *      If the PNG has unknown chunks, the SkPngChunkReader will be used
     *      to handle these chunks.  SkPngChunkReader will be called to read
     *      any unknown chunk at any point during the creation of the codec
     *      or the decode.  Note that if SkPngChunkReader fails to read a
     *      chunk, this could result in a failure to create the codec or a
     *      failure to decode the image.
     *      If the PNG does not contain unknown chunks, the SkPngChunkReader
     *      will not be used or modified.
     */
    static std::unique_ptr<SkCodec> MakeFromData(sk_sp<SkData>,
                                                 SkSpan<const SkCodecs::Decoder> decoders,
                                                 SkPngChunkReader* = nullptr);
    // deprecated
    static std::unique_ptr<SkCodec> MakeFromData(sk_sp<SkData>, SkPngChunkReader* = nullptr);

    virtual ~SkCodec();

    /**
     *  Return a reasonable SkImageInfo to decode into.
     *
     *  If the image has an ICC profile that does not map to an SkColorSpace,
     *  the returned SkImageInfo will use SRGB.
     */
    SkImageInfo getInfo() const { return fEncodedInfo.makeImageInfo(); }

    SkISize dimensions() const { return {fEncodedInfo.width(), fEncodedInfo.height()}; }
    SkIRect bounds() const {
        return SkIRect::MakeWH(fEncodedInfo.width(), fEncodedInfo.height());
    }

    /**
     * Return the ICC profile of the encoded data.
     */
    const skcms_ICCProfile* getICCProfile() const {
        return this->getEncodedInfo().profile();
    }

    /**
     * Whether the encoded input uses 16 or more bits per component.
     */
    bool hasHighBitDepthEncodedData() const {
        // API design note: We don't return `bitsPerComponent` because it may be
        // misleading in some cases - see https://crbug.com/359350061#comment4
        // for more details.
        return this->getEncodedInfo().bitsPerComponent() >= 16;
    }

    /**
     *  Returns the image orientation stored in the EXIF data.
     *  If there is no EXIF data, or if we cannot read the EXIF data, returns kTopLeft.
     */
    SkEncodedOrigin getOrigin() const { return fOrigin; }

    /**
     *  Return a size that approximately supports the desired scale factor.
     *  The codec may not be able to scale efficiently to the exact scale
     *  factor requested, so return a size that approximates that scale.
     *  The returned value is the codec's suggestion for the closest valid
     *  scale that it can natively support
     */
    SkISize getScaledDimensions(float desiredScale) const {
        // Negative and zero scales are errors.
        SkASSERT(desiredScale > 0.0f);
        if (desiredScale <= 0.0f) {
            return SkISize::Make(0, 0);
        }

        // Upscaling is not supported. Return the original size if the client
        // requests an upscale.
        if (desiredScale >= 1.0f) {
            return this->dimensions();
        }
        return this->onGetScaledDimensions(desiredScale);
    }

    /**
     *  Return (via desiredSubset) a subset which can decoded from this codec,
     *  or false if this codec cannot decode subsets or anything similar to
     *  desiredSubset.
     *
     *  @param desiredSubset In/out parameter. As input, a desired subset of
     *      the original bounds (as specified by getInfo). If true is returned,
     *      desiredSubset may have been modified to a subset which is
     *      supported. Although a particular change may have been made to
     *      desiredSubset to create something supported, it is possible other
     *      changes could result in a valid subset.
     *      If false is returned, desiredSubset's value is undefined.
     *  @return true if this codec supports decoding desiredSubset (as
     *      returned, potentially modified)
     */
    bool getValidSubset(SkIRect* desiredSubset) const {
        return this->onGetValidSubset(desiredSubset);
    }

    /**
     *  Format of the encoded data.
     */
    SkEncodedImageFormat getEncodedFormat() const { return this->onGetEncodedFormat(); }

    /**
     *  Return the underlying encoded data stream. This may be nullptr if the original
     *  stream could not be duplicated.
     */
    virtual std::unique_ptr<SkStream> getEncodedData() const;

    /**
     *  Whether or not the memory passed to getPixels is zero initialized.
     */
    enum ZeroInitialized {
        /**
         *  The memory passed to getPixels is zero initialized. The SkCodec
         *  may take advantage of this by skipping writing zeroes.
         */
        kYes_ZeroInitialized,
        /**
         *  The memory passed to getPixels has not been initialized to zero,
         *  so the SkCodec must write all zeroes to memory.
         *
         *  This is the default. It will be used if no Options struct is used.
         */
        kNo_ZeroInitialized,
    };

    /**
     *  Additional options to pass to getPixels.
     */
    struct Options {
        Options()
            : fZeroInitialized(kNo_ZeroInitialized)
            , fSubset(nullptr)
            , fFrameIndex(0)
            , fPriorFrame(kNoFrame)
        {}

        ZeroInitialized            fZeroInitialized;
        /**
         *  If not NULL, represents a subset of the original image to decode.
         *  Must be within the bounds returned by getInfo().
         *  If the EncodedFormat is SkEncodedImageFormat::kWEBP (the only one which
         *  currently supports subsets), the top and left values must be even.
         *
         *  In getPixels and incremental decode, we will attempt to decode the
         *  exact rectangular subset specified by fSubset.
         *
         *  In a scanline decode, it does not make sense to specify a subset
         *  top or subset height, since the client already controls which rows
         *  to get and which rows to skip.  During scanline decodes, we will
         *  require that the subset top be zero and the subset height be equal
         *  to the full height.  We will, however, use the values of
         *  subset left and subset width to decode partial scanlines on calls
         *  to getScanlines().
         */
        const SkIRect*             fSubset;

        /**
         *  The frame to decode.
         *
         *  Only meaningful for multi-frame images.
         */
        int                        fFrameIndex;

        /**
         *  If not kNoFrame, the dst already contains the prior frame at this index.
         *
         *  Only meaningful for multi-frame images.
         *
         *  If fFrameIndex needs to be blended with a prior frame (as reported by
         *  getFrameInfo[fFrameIndex].fRequiredFrame), the client can set this to
         *  any non-kRestorePrevious frame in [fRequiredFrame, fFrameIndex) to
         *  indicate that that frame is already in the dst. Options.fZeroInitialized
         *  is ignored in this case.
         *
         *  If set to kNoFrame, the codec will decode any necessary required frame(s) first.
         */
        int                        fPriorFrame;
    };

    /**
     *  Decode into the given pixels, a block of memory of size at
     *  least (info.fHeight - 1) * rowBytes + (info.fWidth *
     *  bytesPerPixel)
     *
     *  Repeated calls to this function should give the same results,
     *  allowing the PixelRef to be immutable.
     *
     *  @param info A description of the format (config, size)
     *         expected by the caller.  This can simply be identical
     *         to the info returned by getInfo().
     *
     *         This contract also allows the caller to specify
     *         different output-configs, which the implementation can
     *         decide to support or not.
     *
     *         A size that does not match getInfo() implies a request
     *         to scale. If the generator cannot perform this scale,
     *         it will return kInvalidScale.
     *
     *         If the info contains a non-null SkColorSpace, the codec
     *         will perform the appropriate color space transformation.
     *
     *         If the caller passes in the SkColorSpace that maps to the
     *         ICC profile reported by getICCProfile(), the color space
     *         transformation is a no-op.
     *
     *         If the caller passes a null SkColorSpace, no color space
     *         transformation will be done.
     *
     *  If a scanline decode is in progress, scanline mode will end, requiring the client to call
     *  startScanlineDecode() in order to return to decoding scanlines.
     *
     *  For certain codecs, reading into a smaller bitmap than the original dimensions may not
     *  produce correct results (e.g. animated webp).
     *
     *  @return Result kSuccess, or another value explaining the type of failure.
     */
    Result getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const Options*);

    /**
     *  Simplified version of getPixels() that uses the default Options.
     */
    Result getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes) {
        return this->getPixels(info, pixels, rowBytes, nullptr);
    }

    Result getPixels(const SkPixmap& pm, const Options* opts = nullptr) {
        return this->getPixels(pm.info(), pm.writable_addr(), pm.rowBytes(), opts);
    }

    /**
     *  Return an image containing the pixels. If the codec's origin is not "upper left",
     *  This will rotate the output image accordingly.
     */
    std::tuple<sk_sp<SkImage>, SkCodec::Result> getImage(const SkImageInfo& info,
                                                         const Options* opts = nullptr);
    std::tuple<sk_sp<SkImage>, SkCodec::Result> getImage();

    /**
     *  If decoding to YUV is supported, this returns true. Otherwise, this
     *  returns false and the caller will ignore output parameter yuvaPixmapInfo.
     *
     * @param  supportedDataTypes Indicates the data type/planar config combinations that are
     *                            supported by the caller. If the generator supports decoding to
     *                            YUV(A), but not as a type in supportedDataTypes, this method
     *                            returns false.
     *  @param yuvaPixmapInfo Output parameter that specifies the planar configuration, subsampling,
     *                        orientation, chroma siting, plane color types, and row bytes.
     */
    bool queryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes& supportedDataTypes,
                       SkYUVAPixmapInfo* yuvaPixmapInfo) const;

    /**
     *  Returns kSuccess, or another value explaining the type of failure.
     *  This always attempts to perform a full decode. To get the planar
     *  configuration without decoding use queryYUVAInfo().
     *
     *  @param yuvaPixmaps  Contains preallocated pixmaps configured according to a successful call
     *                      to queryYUVAInfo().
     */
    Result getYUVAPlanes(const SkYUVAPixmaps& yuvaPixmaps);

    /**
     *  Prepare for an incremental decode with the specified options.
     *
     *  This may require a rewind.
     *
     *  If kIncompleteInput is returned, may be called again after more data has
     *  been provided to the source SkStream.
     *
     *  @param dstInfo Info of the destination. If the dimensions do not match
     *      those of getInfo, this implies a scale.
     *  @param dst Memory to write to. Needs to be large enough to hold the subset,
     *      if present, or the full image as described in dstInfo.
     *  @param options Contains decoding options, including if memory is zero
     *      initialized and whether to decode a subset.
     *  @return Enum representing success or reason for failure.
     */
    Result startIncrementalDecode(const SkImageInfo& dstInfo, void* dst, size_t rowBytes,
            const Options*);

    Result startIncrementalDecode(const SkImageInfo& dstInfo, void* dst, size_t rowBytes) {
        return this->startIncrementalDecode(dstInfo, dst, rowBytes, nullptr);
    }

    /**
     *  Start/continue the incremental decode.
     *
     *  Not valid to call before a call to startIncrementalDecode() returns
     *  kSuccess.
     *
     *  If kIncompleteInput is returned, may be called again after more data has
     *  been provided to the source SkStream.
     *
     *  Unlike getPixels and getScanlines, this does not do any filling. This is
     *  left up to the caller, since they may be skipping lines or continuing the
     *  decode later. In the latter case, they may choose to initialize all lines
     *  first, or only initialize the remaining lines after the first call.
     *
     *  @param rowsDecoded Optional output variable returning the total number of
     *      lines initialized. Only meaningful if this method returns kIncompleteInput.
     *      Otherwise the implementation may not set it.
     *      Note that some implementations may have initialized this many rows, but
     *      not necessarily finished those rows (e.g. interlaced PNG). This may be
     *      useful for determining what rows the client needs to initialize.
     *  @return kSuccess if all lines requested in startIncrementalDecode have
     *      been completely decoded. kIncompleteInput otherwise.
     */
    Result incrementalDecode(int* rowsDecoded = nullptr) {
        if (!fStartedIncrementalDecode) {
            return kInvalidParameters;
        }
        return this->onIncrementalDecode(rowsDecoded);
    }

    /**
     * The remaining functions revolve around decoding scanlines.
     */

    /**
     *  Prepare for a scanline decode with the specified options.
     *
     *  After this call, this class will be ready to decode the first scanline.
     *
     *  This must be called in order to call getScanlines or skipScanlines.
     *
     *  This may require rewinding the stream.
     *
     *  Not all SkCodecs support this.
     *
     *  @param dstInfo Info of the destination. If the dimensions do not match
     *      those of getInfo, this implies a scale.
     *  @param options Contains decoding options, including if memory is zero
     *      initialized.
     *  @return Enum representing success or reason for failure.
     */
    Result startScanlineDecode(const SkImageInfo& dstInfo, const Options* options);

    /**
     *  Simplified version of startScanlineDecode() that uses the default Options.
     */
    Result startScanlineDecode(const SkImageInfo& dstInfo) {
        return this->startScanlineDecode(dstInfo, nullptr);
    }

    /**
     *  Write the next countLines scanlines into dst.
     *
     *  Not valid to call before calling startScanlineDecode().
     *
     *  @param dst Must be non-null, and large enough to hold countLines
     *      scanlines of size rowBytes.
     *  @param countLines Number of lines to write.
     *  @param rowBytes Number of bytes per row. Must be large enough to hold
     *      a scanline based on the SkImageInfo used to create this object.
     *  @return the number of lines successfully decoded.  If this value is
     *      less than countLines, this will fill the remaining lines with a
     *      default value.
     */
    int getScanlines(void* dst, int countLines, size_t rowBytes);

    /**
     *  Skip count scanlines.
     *
     *  Not valid to call before calling startScanlineDecode().
     *
     *  The default version just calls onGetScanlines and discards the dst.
     *  NOTE: If skipped lines are the only lines with alpha, this default
     *  will make reallyHasAlpha return true, when it could have returned
     *  false.
     *
     *  @return true if the scanlines were successfully skipped
     *          false on failure, possible reasons for failure include:
     *              An incomplete input image stream.
     *              Calling this function before calling startScanlineDecode().
     *              If countLines is less than zero or so large that it moves
     *                  the current scanline past the end of the image.
     */
    bool skipScanlines(int countLines);

    /**
     *  The order in which rows are output from the scanline decoder is not the
     *  same for all variations of all image types.  This explains the possible
     *  output row orderings.
     */
    enum SkScanlineOrder {
        /*
         * By far the most common, this indicates that the image can be decoded
         * reliably using the scanline decoder, and that rows will be output in
         * the logical order.
         */
        kTopDown_SkScanlineOrder,

        /*
         * This indicates that the scanline decoder reliably outputs rows, but
         * they will be returned in reverse order.  If the scanline format is
         * kBottomUp, the nextScanline() API can be used to determine the actual
         * y-coordinate of the next output row, but the client is not forced
         * to take advantage of this, given that it's not too tough to keep
         * track independently.
         *
         * For full image decodes, it is safe to get all of the scanlines at
         * once, since the decoder will handle inverting the rows as it
         * decodes.
         *
         * For subset decodes and sampling, it is simplest to get and skip
         * scanlines one at a time, using the nextScanline() API.  It is
         * possible to ask for larger chunks at a time, but this should be used
         * with caution.  As with full image decodes, the decoder will handle
         * inverting the requested rows, but rows will still be delivered
         * starting from the bottom of the image.
         *
         * Upside down bmps are an example.
         */
        kBottomUp_SkScanlineOrder,
    };

    /**
     *  An enum representing the order in which scanlines will be returned by
     *  the scanline decoder.
     *
     *  This is undefined before startScanlineDecode() is called.
     */
    SkScanlineOrder getScanlineOrder() const { return this->onGetScanlineOrder(); }

    /**
     *  Returns the y-coordinate of the next row to be returned by the scanline
     *  decoder.
     *
     *  This will equal fCurrScanline, except in the case of strangely
     *  encoded image types (bottom-up bmps).
     *
     *  Results are undefined when not in scanline decoding mode.
     */
    int nextScanline() const { return this->outputScanline(fCurrScanline); }

    /**
     *  Returns the output y-coordinate of the row that corresponds to an input
     *  y-coordinate.  The input y-coordinate represents where the scanline
     *  is located in the encoded data.
     *
     *  This will equal inputScanline, except in the case of strangely
     *  encoded image types (bottom-up bmps, interlaced gifs).
     */
    int outputScanline(int inputScanline) const;

    /**
     *  Return the number of frames in the image.
     *
     *  May require reading through the stream.
     *
     *  Note that some codecs may be unable to gather `FrameInfo` for all frames
     *  in case of `kIncompleteInput`.  For such codecs `getFrameCount` may
     *  initially report a low frame count.  After the underlying `SkStream`
     *  provides additional data, then calling `getFrameCount` again may return
     *  an updated, increased frame count.
     */
    int getFrameCount() {
        return this->onGetFrameCount();
    }

    // Sentinel value used when a frame index implies "no frame":
    // - FrameInfo::fRequiredFrame set to this value means the frame
    //   is independent.
    // - Options::fPriorFrame set to this value means no (relevant) prior frame
    //   is residing in dst's memory.
    static constexpr int kNoFrame = -1;

    /**
     *  Information about individual frames in a multi-framed image.
     */
    struct FrameInfo {
        /**
         *  The frame that this frame needs to be blended with, or
         *  kNoFrame if this frame is independent (so it can be
         *  drawn over an uninitialized buffer).
         *
         *  Note that this is the *earliest* frame that can be used
         *  for blending. Any frame from [fRequiredFrame, i) can be
         *  used, unless its fDisposalMethod is kRestorePrevious.
         */
        int fRequiredFrame;

        /**
         *  Number of milliseconds to show this frame.
         */
        int fDuration;

        /**
         *  Whether the end marker for this frame is contained in the stream.
         *
         *  Note: this does not guarantee that an attempt to decode will be complete.
         *  There could be an error in the stream.
         */
        bool fFullyReceived;

        /**
         *  This is conservative; it will still return non-opaque if e.g. a
         *  color index-based frame has a color with alpha but does not use it.
         */
        SkAlphaType fAlphaType;

        /**
         *  Whether the updated rectangle contains alpha.
         *
         *  This is conservative; it will still be set to true if e.g. a color
         *  index-based frame has a color with alpha but does not use it. In
         *  addition, it may be set to true, even if the final frame, after
         *  blending, is opaque.
         */
        bool fHasAlphaWithinBounds;

        /**
         *  How this frame should be modified before decoding the next one.
         */
        SkCodecAnimation::DisposalMethod fDisposalMethod;

        /**
         *  How this frame should blend with the prior frame.
         */
        SkCodecAnimation::Blend fBlend;

        /**
         *  The rectangle updated by this frame.
         *
         *  It may be empty, if the frame does not change the image. It will
         *  always be contained by SkCodec::dimensions().
         */
        SkIRect fFrameRect;
    };

    /**
     *  Return info about a single frame.
     *
     *  Does not read through the stream, so it should be called after
     *  getFrameCount() to parse any frames that have not already been parsed.
     *
     *  Only supported by animated (multi-frame) codecs. Note that this is a
     *  property of the codec (the SkCodec subclass), not the image.
     *
     *  To elaborate, some codecs support animation (e.g. GIF). Others do not
     *  (e.g. BMP). Animated codecs can still represent single frame images.
     *  Calling getFrameInfo(0, etc) will return true for a single frame GIF
     *  even if the overall image is not animated (in that the pixels on screen
     *  do not change over time). When incrementally decoding a GIF image, we
     *  might only know that there's a single frame *so far*.
     *
     *  For non-animated SkCodec subclasses, it's sufficient but not necessary
     *  for this method to always return false.
     */
    bool getFrameInfo(int index, FrameInfo* info) const {
        if (index < 0) {
            return false;
        }
        return this->onGetFrameInfo(index, info);
    }

    /**
     *  Return info about all the frames in the image.
     *
     *  May require reading through the stream to determine info about the
     *  frames (including the count).
     *
     *  As such, future decoding calls may require a rewind.
     *
     *  This may return an empty vector for non-animated codecs. See the
     *  getFrameInfo(int, FrameInfo*) comment.
     */
    std::vector<FrameInfo> getFrameInfo();

    static constexpr int kRepetitionCountInfinite = -1;

    /**
     *  Return the number of times to repeat, if this image is animated. This number does not
     *  include the first play through of each frame. For example, a repetition count of 4 means
     *  that each frame is played 5 times and then the animation stops.
     *
     *  It can return kRepetitionCountInfinite, a negative number, meaning that the animation
     *  should loop forever.
     *
     *  May require reading the stream to find the repetition count.
     *
     *  As such, future decoding calls may require a rewind.
     *
     *  `getRepetitionCount` will return `0` in two cases:
     *  1. Still (non-animated) images.
     *  2. Animated images that only play the animation once (i.e. that don't
     *     repeat the animation)
     *  `isAnimated` can be used to disambiguate between these two cases.
     */
    int getRepetitionCount() {
        return this->onGetRepetitionCount();
    }

    /**
     * `isAnimated` returns whether the full input is expected to contain an
     * animated image (i.e. more than 1 image frame).  This can be used to
     * disambiguate the meaning of `getRepetitionCount` returning `0` (see
     * `getRepetitionCount`'s doc comment for more details).
     *
     * Note that in some codecs `getFrameCount()` only returns the number of
     * frames for which all the metadata has been already successfully decoded.
     * Therefore for a partial input `isAnimated()` may return "yes", even
     * though `getFrameCount()` may temporarily return `1` until more of the
     * input is available.
     *
     * When handling partial input, some codecs may not know until later (e.g.
     * until encountering additional image frames) whether the given image has
     * more than one frame.  Such codecs may initially return
     * `IsAnimated::kUnknown` and only later give a definitive "yes" or "no"
     * answer.  GIF format is one example where this may happen.
     *
     * Other codecs may be able to decode the information from the metadata
     * present before the first image frame.  Such codecs should be able to give
     * a definitive "yes" or "no" answer as soon as they are constructed.  PNG
     * format is one example where this happens.
     */
    enum class IsAnimated {
        kYes,
        kNo,
        kUnknown,
    };
    IsAnimated isAnimated() { return this->onIsAnimated(); }

    // Register a decoder at runtime by passing two function pointers:
    //    - peek() to return true if the span of bytes appears to be your encoded format;
    //    - make() to attempt to create an SkCodec from the given stream.
    // Not thread safe.
    static void Register(
            bool                     (*peek)(const void*, size_t),
            std::unique_ptr<SkCodec> (*make)(std::unique_ptr<SkStream>, SkCodec::Result*));

protected:
    const SkEncodedInfo& getEncodedInfo() const { return fEncodedInfo; }

    using XformFormat = skcms_PixelFormat;

    SkCodec(SkEncodedInfo&&,
            XformFormat srcFormat,
            std::unique_ptr<SkStream>,
            SkEncodedOrigin = kTopLeft_SkEncodedOrigin);

    void setSrcXformFormat(XformFormat pixelFormat);

    XformFormat getSrcXformFormat() const {
        return fSrcXformFormat;
    }

    virtual bool onGetGainmapCodec(SkGainmapInfo*, std::unique_ptr<SkCodec>*) { return false; }
    virtual bool onGetGainmapInfo(SkGainmapInfo*) { return false; }

    // TODO(issues.skia.org/363544350): This API only works for JPEG images. Remove this API once
    // it is no longer used.
    virtual bool onGetGainmapInfo(SkGainmapInfo*, std::unique_ptr<SkStream>*) { return false; }

    virtual SkISize onGetScaledDimensions(float /*desiredScale*/) const {
        // By default, scaling is not supported.
        return this->dimensions();
    }

    // FIXME: What to do about subsets??
    /**
     *  Subclasses should override if they support dimensions other than the
     *  srcInfo's.
     */
    virtual bool onDimensionsSupported(const SkISize&) {
        return false;
    }

    virtual SkEncodedImageFormat onGetEncodedFormat() const = 0;

    /**
     * @param rowsDecoded When the encoded image stream is incomplete, this function
     *                    will return kIncompleteInput and rowsDecoded will be set to
     *                    the number of scanlines that were successfully decoded.
     *                    This will allow getPixels() to fill the uninitialized memory.
     */
    virtual Result onGetPixels(const SkImageInfo& info,
                               void* pixels, size_t rowBytes, const Options&,
                               int* rowsDecoded) = 0;

    virtual bool onQueryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes&,
                                 SkYUVAPixmapInfo*) const { return false; }

    virtual Result onGetYUVAPlanes(const SkYUVAPixmaps&) { return kUnimplemented; }

    virtual bool onGetValidSubset(SkIRect* /*desiredSubset*/) const {
        // By default, subsets are not supported.
        return false;
    }

    /**
     *  If the stream was previously read, attempt to rewind.
     *
     *  If the stream needed to be rewound, call onRewind.
     *  @returns true if the codec is at the right position and can be used.
     *      false if there was a failure to rewind.
     *
     *  This is called by getPixels(), getYUV8Planes(), startIncrementalDecode() and
     *  startScanlineDecode(). Subclasses may call if they need to rewind at another time.
     */
    [[nodiscard]] bool rewindIfNeeded();

    /**
     *  Called by onRewind(), attempts to rewind fStream.
     */
    bool rewindStream();

    /**
     *  Called by rewindIfNeeded, if the stream needed to be rewound.
     *
     *  Subclasses should call rewindStream() if they own one, and then
     *  do any set up needed after.
     */
    virtual bool onRewind() {
        if (!this->rewindStream()) {
            return false;
        }
        return true;
    }

    /**
     * Get method for the input stream
     */
    SkStream* stream() {
        return fStream.get();
    }

    /**
     *  The remaining functions revolve around decoding scanlines.
     */

    /**
     *  Most images types will be kTopDown and will not need to override this function.
     */
    virtual SkScanlineOrder onGetScanlineOrder() const { return kTopDown_SkScanlineOrder; }

    const SkImageInfo& dstInfo() const { return fDstInfo; }

    const Options& options() const { return fOptions; }

    /**
     *  Returns the number of scanlines that have been decoded so far.
     *  This is unaffected by the SkScanlineOrder.
     *
     *  Returns -1 if we have not started a scanline decode.
     */
    int currScanline() const { return fCurrScanline; }

    virtual int onOutputScanline(int inputScanline) const;

    /**
     *  Return whether we can convert to dst.
     *
     *  Will be called for the appropriate frame, prior to initializing the colorXform.
     */
    virtual bool conversionSupported(const SkImageInfo& dst, bool srcIsOpaque,
                                     bool needsColorXform);

    // Some classes never need a colorXform e.g.
    // - ICO uses its embedded codec's colorXform
    // - WBMP is just Black/White
    virtual bool usesColorXform() const { return true; }
    void applyColorXform(void* dst, const void* src, int count) const;

    bool colorXform() const { return fXformTime != kNo_XformTime; }
    bool xformOnDecode() const { return fXformTime == kDecodeRow_XformTime; }

    virtual int onGetFrameCount() {
        return 1;
    }

    virtual bool onGetFrameInfo(int, FrameInfo*) const {
        return false;
    }

    virtual int onGetRepetitionCount() {
        return 0;
    }

    virtual IsAnimated onIsAnimated() {
        return IsAnimated::kNo;
    }

private:
    const SkEncodedInfo                fEncodedInfo;
    XformFormat                        fSrcXformFormat;
    std::unique_ptr<SkStream>          fStream;
    bool fNeedsRewind = false;
    const SkEncodedOrigin fOrigin;

    SkImageInfo                        fDstInfo;
    Options                            fOptions;

    enum XformTime {
        kNo_XformTime,
        kPalette_XformTime,
        kDecodeRow_XformTime,
    };
    XformTime                          fXformTime;
    XformFormat                        fDstXformFormat; // Based on fDstInfo.
    skcms_ICCProfile                   fDstProfile;
    skcms_AlphaFormat                  fDstXformAlphaFormat;

    // Only meaningful during scanline decodes.
    int fCurrScanline = -1;

    bool fStartedIncrementalDecode = false;

    // Allows SkAndroidCodec to call handleFrameIndex (potentially decoding a prior frame and
    // clearing to transparent) without SkCodec itself calling it, too.
    bool fUsingCallbackForHandleFrameIndex = false;

    bool initializeColorXform(const SkImageInfo& dstInfo, SkEncodedInfo::Alpha, bool srcIsOpaque);

    /**
     *  Return whether these dimensions are supported as a scale.
     *
     *  The codec may choose to cache the information about scale and subset.
     *  Either way, the same information will be passed to onGetPixels/onStart
     *  on success.
     *
     *  This must return true for a size returned from getScaledDimensions.
     */
    bool dimensionsSupported(const SkISize& dim) {
        return dim == this->dimensions() || this->onDimensionsSupported(dim);
    }

    /**
     *  For multi-framed images, return the object with information about the frames.
     */
    virtual const SkFrameHolder* getFrameHolder() const {
        return nullptr;
    }

    // Callback for decoding a prior frame. The `Options::fFrameIndex` is ignored,
    // being replaced by frameIndex. This allows opts to actually be a subclass of
    // SkCodec::Options which SkCodec itself does not know how to copy or modify,
    // but just passes through to the caller (where it can be reinterpret_cast'd).
    using GetPixelsCallback = std::function<Result(const SkImageInfo&, void* pixels,
                                                   size_t rowBytes, const Options& opts,
                                                   int frameIndex)>;

    /**
     *  Check for a valid Options.fFrameIndex, and decode prior frames if necessary.
     *
     * If GetPixelsCallback is not null, it will be used to decode a prior frame instead
     * of using this SkCodec directly. It may also be used recursively, if that in turn
     * depends on a prior frame. This is used by SkAndroidCodec.
     */
    Result handleFrameIndex(const SkImageInfo&, void* pixels, size_t rowBytes, const Options&,
                            GetPixelsCallback = nullptr);

    // Methods for scanline decoding.
    virtual Result onStartScanlineDecode(const SkImageInfo& /*dstInfo*/,
            const Options& /*options*/) {
        return kUnimplemented;
    }

    virtual Result onStartIncrementalDecode(const SkImageInfo& /*dstInfo*/, void*, size_t,
            const Options&) {
        return kUnimplemented;
    }

    virtual Result onIncrementalDecode(int*) {
        return kUnimplemented;
    }


    virtual bool onSkipScanlines(int /*countLines*/) { return false; }

    virtual int onGetScanlines(void* /*dst*/, int /*countLines*/, size_t /*rowBytes*/) { return 0; }

    /**
     * On an incomplete decode, getPixels() and getScanlines() will call this function
     * to fill any uinitialized memory.
     *
     * @param dstInfo        Contains the destination color type
     *                       Contains the destination alpha type
     *                       Contains the destination width
     *                       The height stored in this info is unused
     * @param dst            Pointer to the start of destination pixel memory
     * @param rowBytes       Stride length in destination pixel memory
     * @param zeroInit       Indicates if memory is zero initialized
     * @param linesRequested Number of lines that the client requested
     * @param linesDecoded   Number of lines that were successfully decoded
     */
    void fillIncompleteImage(const SkImageInfo& dstInfo, void* dst, size_t rowBytes,
            ZeroInitialized zeroInit, int linesRequested, int linesDecoded);

    /**
     *  Return an object which will allow forcing scanline decodes to sample in X.
     *
     *  May create a sampler, if one is not currently being used. Otherwise, does
     *  not affect ownership.
     *
     *  Only valid during scanline decoding or incremental decoding.
     */
    virtual SkSampler* getSampler(bool /*createIfNecessary*/) { return nullptr; }

    friend class DM::CodecSrc;  // for fillIncompleteImage
    friend class PNGCodecGM;    // for fillIncompleteImage
    friend class SkSampledCodec;
    friend class SkIcoCodec;
    friend class SkPngCodec;     // for onGetGainmapCodec
    friend class SkAndroidCodec;  // for handleFrameIndex
    friend class SkCodecPriv;     // for fEncodedInfo
};

namespace SkCodecs {

using DecodeContext = void*;
using IsFormatCallback = bool (*)(const void* data, size_t len);
using MakeFromStreamCallback = std::unique_ptr<SkCodec> (*)(std::unique_ptr<SkStream>,
                                                            SkCodec::Result*,
                                                            DecodeContext);

struct SK_API Decoder {
    // By convention, we use all lowercase letters and go with the primary filename extension.
    // For example "png", "jpg", "ico", "webp", etc
    std::string_view id;
    IsFormatCallback isFormat;
    MakeFromStreamCallback makeFromStream;
};

// Add the decoder to the end of a linked list of decoders, which will be used to identify calls to
// SkCodec::MakeFromStream. If a decoder with the same id already exists, this new decoder
// will replace the existing one (in the same position). This is not thread-safe, so make sure all
// initialization is done before the first call.
void SK_API Register(Decoder d);

/**
 *  Return a SkImage produced by the codec, but attempts to defer image allocation until the
 *  image is actually used/drawn. This deferral allows the system to cache the result, either on the
 *  CPU or on the GPU, depending on where the image is drawn. If memory is low, the cache may
 *  be purged, causing the next draw of the image to have to re-decode.
 *
 *  If alphaType is nullopt, the image's alpha type will be chosen automatically based on the
 *  image format. Transparent images will default to kPremul_SkAlphaType. If alphaType contains
 *  kPremul_SkAlphaType or kUnpremul_SkAlphaType, that alpha type will be used. Forcing opaque
 *  (passing kOpaque_SkAlphaType) is not allowed, and will return nullptr.
 *
 *  @param codec    A non-null codec (e.g. from SkPngDecoder::Decode)
 *  @return         created SkImage, or nullptr
 */
SK_API sk_sp<SkImage> DeferredImage(std::unique_ptr<SkCodec> codec,
                                    std::optional<SkAlphaType> alphaType = std::nullopt);
}

#endif // SkCodec_DEFINED
