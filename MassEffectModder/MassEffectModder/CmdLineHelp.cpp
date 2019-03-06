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
    PINFO("\nHelp:\n");
    PINFO("  --help\n");
    PINFO("     This help\n");
    PINFO("\n");
    PINFO("  --scan --gameid <game id> [--ipc]\n");
    PINFO("     Scan game data for textures.\n");
    PINFO("\n");
    PINFO("  --remove-empty-mips --gameid <game id> [--ipc]\n");
    PINFO("     Remove empty mips from textures.\n");
    PINFO("\n");
    PINFO("  --update-toc\n");
    PINFO("     Update TOC files in ME3.\n");
    PINFO("\n");
    PINFO("  --unpack-dlcs [--ipc]\n");
    PINFO("     Unpack ME3 DLCs.\n");
    PINFO("\n");
    PINFO("  --repack --gameid <game id> [--ipc]\n");
    PINFO("     Repack ME2 or ME3 packages.\n");
    PINFO("\n");
    PINFO("  --check-game-data-after --gameid <game id> [--ipc]\n");
    PINFO("     Check game data for mods installed after textures installation.\n");
    PINFO("\n");
    PINFO("  --check-game-data-mismatch --gameid <game id> [--ipc]\n");
    PINFO("     Check game data with md5 database.\n");
    PINFO("     Scan to detect mods\n");
    PINFO("\n");
    PINFO("  --check-game-data-vanilla --gameid <game id> [--ipc]\n");
    PINFO("     Check game data with md5 database.\n");
    PINFO("\n");
    PINFO("  --check-for-markers --gameid <game id> [--ipc]\n");
    PINFO("     Check game data for markers.\n");
    PINFO("\n");
    PINFO("  --install-mods --gameid <game id> --input <input dir> [--repack] [--ipc]\n");
    PINFO("  [--alot-mode] [--limit-2k] [--verify]\n");
    PINFO("     Install MEM mods from input directory.\n");
    PINFO("\n");
    PINFO("  --apply-me1-laa\n");
    PINFO("     Apply LAA patch to ME1 executable.\n");
    PINFO("\n");
    PINFO("  --detect-mods --gameid <game id> [--ipc]\n");
    PINFO("     Detect compatible mods.\n");
    PINFO("\n");
    PINFO("  --detect-bad-mods --gameid <game id> [--ipc]\n");
    PINFO("     Detect not compatible mods.\n");
    PINFO("\n");
    PINFO("  --apply-lods-gfx --gameid <game id> [--soft-shadows-mode] [--meuitm-mode] [--limit-2k]\n");
    PINFO("     Update LODs and GFX settings.\n");
    PINFO("\n");
    PINFO("  --remove-lods --gameid <game id>\n");
    PINFO("     Remove LODs settings.\n");
    PINFO("\n");
    PINFO("  --print-lods --gameid <game id> [--ipc]\n");
    PINFO("     Print LODs settings.\n");
    PINFO("\n");
    PINFO("  --convert-to-mem --gameid <game id> --input <input dir> --output <output file> [--mark-to-convert] [--ipc]\n");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3\n");
    PINFO("     input dir: directory to be converted, containing following file extension(s):\n");
    PINFO("        MEM, MOD, TPF\n");
    PINFO("        BIN - package export raw data\n");
    PINFO("           Naming pattern used for package in DLC:\n");
    PINFO("             D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n");
    PINFO("             example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin\n");
    PINFO("           Naming pattern used for package in base directory:\n");
    PINFO("             B<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n");
    PINFO("             example: B23-BioH_EDI_00_Explore.pcc-E5090.bin\n");
    PINFO("        XDELTA - package export xdelta3 patch data\n");
    PINFO("           Naming pattern used for package in DLC:\n");
    PINFO("             D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.xdelta\n");
    PINFO("             example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.xdelta\n");
    PINFO("           Naming pattern used for package in base directory:\n");
    PINFO("             B<pkg filename length>-<pkg filename>-E<pkg export id>.xdelta\n");
    PINFO("             example: B23-BioH_EDI_00_Explore.pcc-E5090.xdelta\n");
    PINFO("        DDS, BMP, TGA, PNG\n");
    PINFO("           input format supported for DDS images:\n");
    PINFO("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB\n");
    PINFO("           input format supported for TGA images:\n");
    PINFO("              uncompressed RGBA/RGB, compressed RGBA/RGB\n");
    PINFO("           input format supported for BMP images:\n");
    PINFO("              uncompressed RGBA/RGB/RGBX\n");
    PINFO("           Image filename must include texture CRC (0xhhhhhhhh)\n");
    PINFO("     ipc: turn on IPC traces\n");
    PINFO("\n");
    PINFO("  --convert-game-image --gameid <game id> --input <input image> --output <output image> [--mark-to-convert]\n");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3\n");
    PINFO("     Input file with following extension:\n");
    PINFO("        DDS, BMP, TGA, PNG\n");
    PINFO("           input format supported for DDS images:\n");
    PINFO("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB\n");
    PINFO("           input format supported for TGA images:\n");
    PINFO("              uncompressed RGBA/RGB, compressed RGBA/RGB\n");
    PINFO("           input format supported for BMP images:\n");
    PINFO("              uncompressed RGBA/RGB/RGBX\n");
    PINFO("           Image filename must include texture CRC (0xhhhhhhhh)\n");
    PINFO("     Output file is DDS image\n");
    PINFO("\n");
    PINFO("  --convert-game-images --gameid <game id> --input <input dir> --output <output dir> [--mark-to-convert]\n");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3\n");
    PINFO("     input dir: directory to be converted, containing following file extension(s):\n");
    PINFO("        Input files with following extension:\n");
    PINFO("        DDS, BMP, TGA, PNG\n");
    PINFO("           input format supported for DDS images:\n");
    PINFO("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB\n");
    PINFO("           input format supported for TGA images:\n");
    PINFO("              uncompressed RGBA/RGB, compressed RGBA/RGB\n");
    PINFO("           input pixel format supported for BMP images:\n");
    PINFO("              uncompressed RGBA/RGB/RGBX\n");
    PINFO("           Image filename must include texture CRC (0xhhhhhhhh)\n");
    PINFO("     output dir: directory where textures converted to DDS are placed\n");
    PINFO("\n");
    PINFO("  --extract-mod --gameid <game id> --input <input dir/file> [--output <output dir>] [--ipc]\n");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3\n");
    PINFO("     input dir: directory of ME3Explorer MOD file(s)\n");
    PINFO("     input file: ME3Explorer MOD file to be extracted\n");
    PINFO("     Can extract textures and package export raw data\n");
    PINFO("     Naming pattern used for package in DLC:\n");
    PINFO("        D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n");
    PINFO("        example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin\n");
    PINFO("     Naming pattern used for package in base directory:\n");
    PINFO("        B<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n");
    PINFO("        example: B23-BioH_EDI_00_Explore.pcc-E5090.bin\n");
    PINFO("\n");
    PINFO("  --extract-mem --gameid <game id> --input <input dir/file> [--output <output dir>] [--ipc]\n");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3\n");
    PINFO("     input dir: directory of MEM mod file(s)\n");
    PINFO("     input file: MEM file to be extracted\n");
    PINFO("     Can extract textures and package export raw data\n");
    PINFO("     Naming pattern used for package in DLC:\n");
    PINFO("        D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n");
    PINFO("        example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin\n");
    PINFO("     Naming pattern used for package in base directory:\n");
    PINFO("        B<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n");
    PINFO("        example: B23-BioH_EDI_00_Explore.pcc-E5090.bin\n");
    PINFO("\n");
    PINFO("  --extract-tpf --gameid <input dir/file> [--output <output dir>] [--ipc]\n");
    PINFO("     input dir: directory containing the TPF file(s) to be extracted\n");
    PINFO("     input file: TPF file to be extracted\n");
    PINFO("     Textures are extracted as they are in the TPF, no additional modifications are made.\n");
    PINFO("\n");
    PINFO("  --convert-image --format <output pixel format> [--threshold <dxt1 alpha threshold>] --input <input image> --output <output image>\n");
    PINFO("     input image file types: DDS, BMP, TGA, PNG\n");
    PINFO("           input format supported for DDS images:\n");
    PINFO("              DXT1, DXT3, DTX5, ATI2, V8U8, G8, RGBA, RGB\n");
    PINFO("           input format supported for TGA images:\n");
    PINFO("              uncompressed RGBA/RGB, compressed RGBA/RGB\n");
    PINFO("           input format supported for BMP images:\n");
    PINFO("              uncompressed RGBA/RGB/RGBX\n");
    PINFO("     output image file type: DDS\n");
    PINFO("     output pixel format: DXT1 (no alpha), DXT1a (alpha), DXT3, DXT5, ATI2, V8U8, G8, RGBA, RGB\n");
    PINFO("     For DXT1a you have to set the alpha threshold (0-255). 128 is suggested as a default value.\n");
    PINFO("\n");
    PINFO("  --extract-all-dds --gameid <game id> --output <output dir> [--tfc-name <filter name|--pcc-only|--tfc-only>]\n");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3\n");
    PINFO("     output dir: directory where textures converted to DDS are placed\n");
    PINFO("     TFC filter name: it will filter only textures stored in specific TFC file.\n");
    PINFO("     Or option: -pcc-only to extract only textures stored in packages.\n");
    PINFO("     Or option: -tfc-only to extract only textures stored in TFC files.\n");
    PINFO("     Textures are extracted as they are in game data, only DDS header is added.\n");
    PINFO("\n");
    PINFO("  --extract-all-png --gameid <game id> --output <output dir> [--tfc-name <filter name>|--pcc-only|--tfc-only]\n");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3\n");
    PINFO("     output dir: directory where textures converted to PNG are placed\n");
    PINFO("     TFC filter name: it will filter only textures stored in specific TFC file.\n");
    PINFO("     Or option: -pcc-only to extract only textures stored in packages.\n");
    PINFO("     Or option: -tfc-only to extract only textures stored in TFC files.\n");
    PINFO("     Textures are extracted with only top mipmap.\n");
    PINFO("\n");
    PINFO("  --dlc-mod-textures --gameid <game id> --input <mem file>\n");
    PINFO("  [--tfc-name <tfc name>] [--guid <guid in 16 hex digits>] [--append-tfc] [--verify]\n");
    PINFO("     Replace textures from <mem file> and store in new <tfc name> file.\n");
    PINFO("     New TFC name must be added earlier to PCC files.\n");
    PINFO("\n");
    PINFO("  --unpack-archive --input <zip/7z/rar file> [--output <output path>] [--ipc]\n");
    PINFO("     Unpack ZIP/7ZIP/RAR file.\n");
    PINFO("\n");
#if !defined(_WIN32)
    PINFO("  --set-game-data-path --gameid <game id> --path <path>\n");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3\n");
    PINFO("     Set game data path to <path>.\n");
    PINFO("\n");
    PINFO("  --set-game-user-path --gameid <game id> --path <path>\n");
    PINFO("     game id: 1 for ME1, 2 for ME2, 3 for ME3\n");
    PINFO("     Set game user config path to <path>.\n");
    PINFO("\n");
#endif
    PINFO("\n");
    PINFO("\n");
    PINFO("  Additonal option to enable debug logs level to all commands: --debug-logs\n");
    PINFO("\n");
}
