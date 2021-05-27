//===============================================================================
// Copyright (c) 2021  Pawel Kolodziejski
// Copyright (c) 2007-2016  Advanced Micro Devices, Inc. All rights reserved.
// Copyright (c) 2004-2006 ATI Technologies Inc.
//===============================================================================
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// File : BC7_Library.h
//
// Reference implementation of a multithreaded BC7 block compressor.
//
// Version 0.1
//
//-----------------------------------------------------------------------------

#ifndef _BC7_LIBRARY_H_
#define _BC7_LIBRARY_H_

#include "bc7_definitions.h"
#include "3dquant_vpc.h"
#include "shake.h"

// Number of image components
#define BC7_COMPONENT_COUNT 4

// Number of pixels in a BC7 block
#define BC7_BLOCK_PIXELS   16

typedef enum _BC_ERROR {
    BC_ERROR_NONE,
    BC_ERROR_LIBRARY_NOT_INITIALIZED,
    BC_ERROR_LIBRARY_ALREADY_INITIALIZED,
    BC_ERROR_INVALID_PARAMETERS,
    BC_ERROR_OUT_OF_MEMORY,
} BC_ERROR;

class BC7BlockEncoder;
class BC7BlockDecoder;

//=================================================================================
//
// InitializeBC7Library() - Startup the BC7 library
//
// Must be called before any other library methods are valid
//
BC_ERROR CMP_InitializeBC7Library();

//
// ShutdownBC7Library - Shutdown the BC7 library
//
BC_ERROR CMP_ShutdownBC7Library();

//
// CMP_CreateBC7Encoder()  - Creates an encoder object with the specified quality and settings for BC7 codec
//
// Library must be initialized before calling this function.
//
// Arguments and Settings:
//
//      quality       - Quality of encoding. This value ranges between 0.0 and 1.0. Default is 0.01.
//                      0.0 gives the fastest, lowest quality encoding, 1.0 is the slowest, highest quality encoding
//                      In general even quality level 0.0 will give very good results on the vast majority of images
//                      Higher quality settings may be needed for some difficult images (e.g. normal maps) to give good results
//                      Encoding time will increase significantly at high quality levels. Quality levels around 0.8 will
//                      give very close to the highest possible quality, increasing the level above this will cause large
//                      increases in encoding time for very marginal gains in quality
//
//      performance   - Perfromance of encoding. This value ranges between 0.0 and 1.0. Typical default is 1.0.
//                      Encoding time can be reduced by incresing this value for a given Quality level. Lower values will improve overall quality with
//                        optimal setting been performed at a value of 0.
//
//      restrictColor - This setting is a quality tuning setting which may be necessary for convenience in some applications.
//                      BC7 can be used for encoding data with up to four-components (e.g. ARGB), but the output of a BC7 decoder
//                        is effectively always 4-components, even if the original input contained less
//                      If BC7 is used to encode three-component data (e.g. RGB) then the encoder generally assumes that it doesn't matter what
//                      ends up in the 4th component of the data, however some applications might be written in such a way that they
//                      expect the 4th component to always be 1.0 (this might, for example, allow mixing of textures with and without
//                      alpha channels without special handling). In this example case the default behaviour of the encoder might cause some
//                      unexpected results, as the alpha channel is not guaranteed to always contain exactly 1.0 (since some error may be distributed
//                      into the 4th channel)
//                      If the restrictColor flag is set then for any input blocks where the 4th component is always 1.0 (255) the encoder will
//                      restrict itself to using encodings where the reconstructed 4th component is also always guaranteed to contain 1.0 (255)
//                      This may cause a very slight loss in overall quality measured in absolute RMS error, but this will generally be negligible
//
//      restrictAlpha - This setting is a quality tuning setting which may be necessary for some textures. Some textures may need alpha values
//                      of 1.0 and 0.0 to be exactly represented, but some BC7 block modes distribute error between the colour and alpha
//                      channels (because they have a shared least significant bit in the encoding). This could result in the alpha values
//                      being pulled away from zero or one by the global minimization of the error. If this flag is specified then the encoder
//                      will restrict its behaviour so that for blocks which contain an alpha of zero or one then these values should be
//                      precisely represented
//
//      modeMask      - This is an advanced option.
//                      BC7 can encode blocks using any of 8 different block modes in order to obtain the highest quality (for reference of how each
//                      of these block modes work consult the BC7 specification)
//                      Under some circumstances it is possible that it might be desired to manipulate the encoder to only produce certain modes
//                      Using this setting it is possible to instruct the encoder to only use certain block modes.
//                      This input is a bitmask of permitted modes for the encoder to use - for normal operation it should be set to 0xFF (all modes valid)
//                      The bitmask is arranged such that a setting of 0x1 only allows the encoder to use block mode 0.
//                      0x80 would only permit the use of block mode 7
//                      Restricting the available modes will generally reduce quality, but will also increase encoding speed
//
//      encoder       - Address of a pointer to an encoder.
//                      This function will allocate a BC7BlockEncoder object using new
//
BC_ERROR CMP_CreateBC7Encoder(double quality, bool restrictColour, bool restrictAlpha, CMP_DWORD modeMask, double performance,
                              BC7BlockEncoder **encoder);
//
// CMP_CreateBC7Decoder()  - Creates an decoder object
//
// Library must be initialized before calling this function.
//
BC_ERROR CMP_CreateBC7Decoder(BC7BlockDecoder **decoder);

// CMP_EncodeBC7Block()  - Enqueue a single BC7  block to the library for encoding
//
// Input is expected to be a single 16 element block containing 4 components in the range 0.->255.
// Pixel data in the block should be arranged in row-major order
// For three-component input images the 4th component (BC7_COMP_ALPHA) should be set to 255 for
// all pixels to ensure optimal encoding
//
BC_ERROR CMP_EncodeBC7Block(BC7BlockEncoder *encoder, double in[BC7_BLOCK_PIXELS][BC7_COMPONENT_COUNT], CMP_BYTE *out);

//
// CMP_DecodeBC7Block()  - Decode a BC7 block to an uncompressed output
//
// This function takes a pointer to an encoded BC block as input, decodes it and writes out the result
//
//
BC_ERROR CMP_DecodeBC7Block(BC7BlockDecoder *decoder, CMP_BYTE *in, double out[BC7_BLOCK_PIXELS][BC7_COMPONENT_COUNT]);

//
// CMP_DestroyBC7Encoder()  - Deletes a previously allocated encoder object
//
//
BC_ERROR CMP_DestroyBC7Encoder(BC7BlockEncoder *encoder);

//
// CMP_DestroyBC7Decoder()  - Deletes a previously allocated decoder object
//
//
BC_ERROR CMP_DestroyBC7Decoder(BC7BlockDecoder *deccoder);

#endif  // _BC7_LIBRARY_H_
