/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2019 Pawel Kolodziejski
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "Helpers/MiscHelpers.h"
#include "Helpers/Logs.h"

void DisplayHelp()
{
    PINFO("\nHelp:");
    PINFO("  --help");
    PINFO("     This help");
    PINFO("");
    PINFO("  --scan --gameid <game id> [--ipc]");
    PINFO("     Scan game data for textures.");
    PINFO("");
    PINFO("  --remove-empty-mips --gameid <game id> [--ipc]");
    PINFO("     Remove empty mips from textures.");
    PINFO("");
    PINFO("  --update-toc");
    PINFO("     Update TOC files in ME3.");
    PINFO("");
    PINFO("  --unpack-dlcs [--ipc]");
    PINFO("     Unpack ME3 DLCs.");
    PINFO("");
    PINFO("  --repack --gameid <game id> [--ipc]");
    PINFO("     Repack ME2 or ME3 packages.");
    PINFO("");
    PINFO("  --check-game-data-after --gameid <game id> [--ipc]");
    PINFO("     Check game data for mods installed after textures installation.");
    PINFO("");
    PINFO("  --check-game-data-mismatch --gameid <game id> [--ipc]");
    PINFO("     Check game data with md5 database.");
    PINFO("     Scan to detect mods");
    PINFO("");
    PINFO("  --check-game-data-vanilla --gameid <game id> [--ipc]");
    PINFO("     Check game data with md5 database.");
    PINFO("");
    PINFO("  --check-for-markers --gameid <game id> [--ipc]");
    PINFO("     Check game data for markers.");
    PINFO("");
    PINFO("  --install-mods --gameid <game id> --input <input dir> [--repack] [--ipc]");
    PINFO("  [--alot-mode] [--limit-2k] [--verify]");
    PINFO("     Install MEM mods from input directory.");
    PINFO("");
    PINFO("  --apply-me1-laa");
    PINFO("     Apply LAA patch to ME1 executable.");
    PINFO("");
    PINFO("  --detect-mods --gameid <game id> [--ipc]");
    PINFO("     Detect compatible mods.");
    PINFO("");
    PINFO("  --detect-bad-mods --gameid <game id> [--ipc]");
    PINFO("     Detect not compatible mods.");
    PINFO("");
    PINFO("  --apply-lods-gfx --gameid <game id> [--soft-shadows-mode] [--meuitm-mode] [--limit-2k]");
    PINFO("     Update LODs and GFX settings.");
    PINFO("");
    PINFO("  --remove-lods --gameid <game id>");
    PINFO("     Remove LODs settings.");
    PINFO("");
    PINFO("  --print-lods --gameid <game id> [--ipc]");
    PINFO("     Print LODs settings.");
    PINFO("");
    PINFO("  --convert-to-mem --gameid <game id> --input <input dir> --output <output file> [--mark-to-convert] [--ipc]");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    PINFO("     input dir: directory to be converted, containing following file extension(s):");
    PINFO("        MEM, MOD, TPF");
    PINFO("        BIN - package export raw data");
    PINFO("           Naming pattern used for package in DLC:");
    PINFO("             D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    PINFO("             example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin");
    PINFO("           Naming pattern used for package in base directory:");
    PINFO("             B<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    PINFO("             example: B23-BioH_EDI_00_Explore.pcc-E5090.bin");
    PINFO("        XDELTA - package export xdelta3 patch data");
    PINFO("           Naming pattern used for package in DLC:");
    PINFO("             D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.xdelta");
    PINFO("             example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.xdelta");
    PINFO("           Naming pattern used for package in base directory:");
    PINFO("             B<pkg filename length>-<pkg filename>-E<pkg export id>.xdelta");
    PINFO("             example: B23-BioH_EDI_00_Explore.pcc-E5090.xdelta");
    PINFO("        DDS, BMP, TGA, PNG");
    PINFO("           input format supported for DDS images:");
    PINFO("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB");
    PINFO("           input format supported for TGA images:");
    PINFO("              uncompressed RGBA/RGB, compressed RGBA/RGB");
    PINFO("           input format supported for BMP images:");
    PINFO("              uncompressed RGBA/RGB/RGBX");
    PINFO("           Image filename must include texture CRC (0xhhhhhhhh)");
    PINFO("     ipc: turn on IPC traces");
    PINFO("");
    PINFO("  --convert-game-image --gameid <game id> --input <input image> <--output output image> [--mark-to-convert]");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    PINFO("     Input file with following extension:");
    PINFO("        DDS, BMP, TGA, PNG");
    PINFO("           input format supported for DDS images:");
    PINFO("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB");
    PINFO("           input format supported for TGA images:");
    PINFO("              uncompressed RGBA/RGB, compressed RGBA/RGB");
    PINFO("           input format supported for BMP images:");
    PINFO("              uncompressed RGBA/RGB/RGBX");
    PINFO("           Image filename must include texture CRC (0xhhhhhhhh)");
    PINFO("     Output file is DDS image");
    PINFO("");
    PINFO("  --convert-game-images --gameid <game id> --input <input dir> --output <output dir> [--mark-to-convert]");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    PINFO("     input dir: directory to be converted, containing following file extension(s):");
    PINFO("        Input files with following extension:");
    PINFO("        DDS, BMP, TGA, PNG");
    PINFO("           input format supported for DDS images:");
    PINFO("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB");
    PINFO("           input format supported for TGA images:");
    PINFO("              uncompressed RGBA/RGB, compressed RGBA/RGB");
    PINFO("           input pixel format supported for BMP images:");
    PINFO("              uncompressed RGBA/RGB/RGBX");
    PINFO("           Image filename must include texture CRC (0xhhhhhhhh)");
    PINFO("     output dir: directory where textures converted to DDS are placed");
    PINFO("");
    PINFO("  --extract-mod --gameid <game id> --input <input dir/file> [--output <output dir>] [--ipc]");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    PINFO("     input dir: directory of ME3Explorer MOD file(s)");
    PINFO("     input file: ME3Explorer MOD file to be extracted");
    PINFO("     Can extract textures and package export raw data");
    PINFO("     Naming pattern used for package in DLC:");
    PINFO("        D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    PINFO("        example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin");
    PINFO("     Naming pattern used for package in base directory:");
    PINFO("        B<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    PINFO("        example: B23-BioH_EDI_00_Explore.pcc-E5090.bin");
    PINFO("");
    PINFO("  --extract-mem --gameid <game id> --input <input dir/file> [--output <output dir>] [--ipc]");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    PINFO("     input dir: directory of MEM mod file(s)");
    PINFO("     input file: MEM file to be extracted");
    PINFO("     Can extract textures and package export raw data");
    PINFO("     Naming pattern used for package in DLC:");
    PINFO("        D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    PINFO("        example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin");
    PINFO("     Naming pattern used for package in base directory:");
    PINFO("        B<pkg filename length>-<pkg filename>-E<pkg export id>.bin");
    PINFO("        example: B23-BioH_EDI_00_Explore.pcc-E5090.bin");
    PINFO("");
    PINFO("  --extract-tpf --gameid <input dir/file> [--output <output dir>] [--ipc]");
    PINFO("     input dir: directory containing the TPF file(s) to be extracted");
    PINFO("     input file: TPF file to be extracted");
    PINFO("     Textures are extracted as they are in the TPF, no additional modifications are made.");
    PINFO("");
    PINFO("  --convert-image --format <output pixel format> [--threshold <dxt1 alpha threshold>] --input <input image> --output <output image>");
    PINFO("     input image file types: DDS, BMP, TGA, PNG");
    PINFO("           input format supported for DDS images:");
    PINFO("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB");
    PINFO("           input format supported for TGA images:");
    PINFO("              uncompressed RGBA/RGB, compressed RGBA/RGB");
    PINFO("           input format supported for BMP images:");
    PINFO("              uncompressed RGBA/RGB/RGBX");
    PINFO("     output image file type: DDS");
    PINFO("     output pixel format: DXT1 (no alpha), DXT1a (alpha), DXT3, DXT5, ATI2, V8U8, G8, RGBA, RGB");
    PINFO("     For DXT1a you have to set the alpha threshold (0-255). 128 is suggested as a default value.");
    PINFO("");
    PINFO("  --extract-all-dds --gameid <game id> --output <output dir> [--tfc-name <filter name|--pcc-only|--tfc-only>]");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    PINFO("     output dir: directory where textures converted to DDS are placed");
    PINFO("     TFC filter name: it will filter only textures stored in specific TFC file.");
    PINFO("     Or option: -pcc-only to extract only textures stored in packages.");
    PINFO("     Or option: -tfc-only to extract only textures stored in TFC files.");
    PINFO("     Textures are extracted as they are in game data, only DDS header is added.");
    PINFO("");
    PINFO("  --extract-all-png --gameid <game id> --output <output dir> [--tfc-name <filter name>|--pcc-only|--tfc-only]");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3");
    PINFO("     output dir: directory where textures converted to PNG are placed");
    PINFO("     TFC filter name: it will filter only textures stored in specific TFC file.");
    PINFO("     Or option: -pcc-only to extract only textures stored in packages.");
    PINFO("     Or option: -tfc-only to extract only textures stored in TFC files.");
    PINFO("     Textures are extracted with only top mipmap.");
    PINFO("");
    PINFO("  --dlc-mod-textures --gameid <game id> --input <mem file>");
    PINFO("  [--tfc-name <tfc name>] [--guid <guid in 16 hex digits>] [--verify]");
    PINFO("     Replace textures from <mem file> and store in new <tfc name> file.");
    PINFO("     New TFC name must be added earlier to PCC files.");
    PINFO("");
    PINFO("  --unpack-archive --input <zip/7z/rar file> [--output <output path>] [--ipc]");
    PINFO("     Unpack ZIP/7ZIP/RAR file.");
    PINFO("");
}
