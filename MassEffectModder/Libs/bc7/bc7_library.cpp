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

#include "bc7_library.h"
#include "bc7_definitions.h"
#include "bc7_encode.h"
#include "bc7_decode.h"
#include "3dquant_vpc.h"
#include "shake.h"

static bool g_LibraryInitialized = false;

//
// One time initialization for the library
//
//
//
BC_ERROR CMP_InitializeBC7Library() {
    if (g_LibraryInitialized) {
        return BC_ERROR_LIBRARY_ALREADY_INITIALIZED;
    }

    // One time initialisation for quantizer and shaker
    Quant_Init();
    g_LibraryInitialized = true;
    return BC_ERROR_NONE;
}


BC_ERROR CMP_CreateBC7Encoder(double quality, bool restrictColour, bool restrictAlpha, CMP_DWORD modeMask, double performance, BC7BlockEncoder **encoder) {
    if (!g_LibraryInitialized) {
        return BC_ERROR_LIBRARY_NOT_INITIALIZED;
    }

    if (!encoder) {
        return BC_ERROR_INVALID_PARAMETERS;
    }

    *encoder = new BC7BlockEncoder(modeMask, true, quality, restrictColour, restrictAlpha, performance);
    if (!encoder) {
        return BC_ERROR_OUT_OF_MEMORY;
    }

    return BC_ERROR_NONE;
}

BC_ERROR CMP_CreateBC7Decoder(BC7BlockDecoder **decoder) {
    if (!g_LibraryInitialized) {
        return BC_ERROR_LIBRARY_NOT_INITIALIZED;
    }

    if (!decoder) {
        return BC_ERROR_INVALID_PARAMETERS;
    }

    *decoder = new BC7BlockDecoder();
    if (!decoder) {
        return BC_ERROR_OUT_OF_MEMORY;
    }

    return BC_ERROR_NONE;
}

//
// Submit a block for encoding
//
//
//
BC_ERROR CMP_EncodeBC7Block(BC7BlockEncoder *encoder, double in[BC7_BLOCK_PIXELS][MAX_DIMENSION_BIG], CMP_BYTE *out) {
    if (!g_LibraryInitialized) {
        return BC_ERROR_LIBRARY_NOT_INITIALIZED;
    }

    if (!encoder || !in || !out) {
        return BC_ERROR_INVALID_PARAMETERS;
    }

    encoder->CompressBlock(in, out);

    return BC_ERROR_NONE;
}

//
// Decode a block and write it to the output
//
//
//
BC_ERROR CMP_DecodeBC7Block(BC7BlockDecoder *decoder, CMP_BYTE *in, double out[BC7_BLOCK_PIXELS][MAX_DIMENSION_BIG]) {
    if (!g_LibraryInitialized) {
        return BC_ERROR_LIBRARY_NOT_INITIALIZED;
    }

    if (!decoder || !in || !out) {
        return BC_ERROR_INVALID_PARAMETERS;
    }

    decoder->DecompressBlock(out, in);

    return BC_ERROR_NONE;
}

//
// Destroys decoder object
//
//
//
BC_ERROR CMP_DestroyBC7Decoder(BC7BlockDecoder *decoder) {
    if (!g_LibraryInitialized) {
        return BC_ERROR_LIBRARY_NOT_INITIALIZED;
    }

    if (!decoder) {
        return BC_ERROR_INVALID_PARAMETERS;
    }

    delete decoder;

    return BC_ERROR_NONE;
}

//
// Destroys encoder object
//
//
//
BC_ERROR CMP_DestroyBC7Encoder(BC7BlockEncoder *encoder) {
    if (!g_LibraryInitialized) {
        return BC_ERROR_LIBRARY_NOT_INITIALIZED;
    }

    if (!encoder) {
        return BC_ERROR_INVALID_PARAMETERS;
    }

    delete encoder;

    return BC_ERROR_NONE;
}

//
// Clean up the library and exit
//
//
//
BC_ERROR CMP_ShutdownBC7Library() {
    if (!g_LibraryInitialized) {
        return BC_ERROR_LIBRARY_NOT_INITIALIZED;
    }
    Quant_DeInit();
    g_LibraryInitialized = false;

    return BC_ERROR_NONE;
}

