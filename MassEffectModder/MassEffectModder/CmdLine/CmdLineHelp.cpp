/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2020 Pawel Kolodziejski
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

#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>

void DisplayHelp()
{
    QString help =
        "\nHelp:\n" \
        "  --help\n" \
        "     This help\n" \
        "\n" \
        "  --version [--ipc]\n" \
        "     Display program version\n" \
        "\n" \
        "  --scan --gameid <game id> [--remove-empty-mips] [--ipc]\n" \
        "     Scan game data for textures.\n" \
        "\n" \
        "  --update-toc\n" \
        "     Update TOC files in ME3.\n" \
        "\n" \
        "  --unpack-dlcs [--ipc]\n" \
        "     Unpack ME3 DLCs.\n" \
        "\n" \
        "  --repack --gameid <game id> [--ipc]\n" \
        "     Repack ME2 or ME3 packages.\n" \
        "\n" \
        "  --check-game-data-after --gameid <game id> [--ipc]\n" \
        "     Check game data for mods installed after textures installation.\n" \
        "\n" \
        "  --check-game-data-mismatch --gameid <game id> [--ipc]\n" \
        "     Check game data with md5 database.\n" \
        "     Scan to detect mods\n" \
        "\n" \
        "  --check-game-data-vanilla --gameid <game id> [--ipc]\n" \
        "     Check game data with md5 database.\n" \
        "\n" \
        "  --check-for-markers --gameid <game id> [--ipc]\n" \
        "     Check game data for markers.\n" \
        "\n" \
        "  --install-mods --gameid <game id> --input <input dir> [--cache-amount <percent>]\n" \
        "  [--repack] [--ipc] [--alot-mode] [--limit-2k] [--verify]\n" \
        "     Install MEM mods from input directory.\n" \
        "\n" \
        "  --apply-me1-laa\n" \
        "     Apply LAA patch to ME1 executable.\n" \
        "\n" \
        "  --detect-mods --gameid <game id> [--ipc]\n" \
        "     Detect compatible mods.\n" \
        "\n" \
        "  --detect-bad-mods --gameid <game id> [--ipc]\n" \
        "     Detect not compatible mods.\n" \
        "\n" \
        "  --apply-lods-gfx --gameid <game id> [--soft-shadows-mode] [--meuitm-mode] [--limit-2k]\n" \
        "     Update LODs and GFX settings.\n" \
        "\n" \
        "  --remove-lods --gameid <game id>\n" \
        "     Remove LODs settings.\n" \
        "\n" \
        "  --print-lods --gameid <game id> [--ipc]\n" \
        "     Print LODs settings.\n" \
        "\n" \
        "  --convert-to-mem --gameid <game id> --input <input dir> --output <output file> [--mark-to-convert] [--ipc]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     input dir: directory to be converted, containing following file extension(s):\n" \
        "        MEM, MOD, TPF\n" \
        "        BIN - package export raw data\n" \
        "           Naming pattern used for package in DLC:\n" \
        "             D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n" \
        "             example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin\n" \
        "           Naming pattern used for package in base directory:\n" \
        "             B<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n" \
        "             example: B23-BioH_EDI_00_Explore.pcc-E5090.bin\n" \
        "        XDELTA - package export xdelta3 patch data\n" \
        "           Naming pattern used for package in DLC:\n" \
        "             D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.xdelta\n" \
        "             example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.xdelta\n" \
        "           Naming pattern used for package in base directory:\n" \
        "             B<pkg filename length>-<pkg filename>-E<pkg export id>.xdelta\n" \
        "             example: B23-BioH_EDI_00_Explore.pcc-E5090.xdelta\n" \
        "        DDS, BMP, TGA, PNG\n" \
        "           input format supported for DDS images:\n" \
        "              DXT1, DXT3, DTX5, ATI2, V8U8, G8, ARGB, RGB\n" \
        "           input format supported for TGA images:\n" \
        "              uncompressed ARGB/RGB, compressed ARGB/RGB\n" \
        "           input format supported for BMP images:\n" \
        "              uncompressed ARGB/RGB/RGBX\n" \
        "           Image filename must include texture CRC (0xhhhhhhhh)\n" \
        "        BIK\n" \
        "           Movie filename must include texture CRC (0xhhhhhhhh)\n" \
        "     ipc: turn on IPC traces\n" \
        "\n" \
        "  --convert-game-image --gameid <game id> --input <input image> --output <output image> [--mark-to-convert]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     Input file with following extension:\n" \
        "        DDS, BMP, TGA, PNG\n" \
        "           input format supported for DDS images:\n" \
        "              DXT1, DXT3, DTX5, ATI2, V8U8, G8, ARGB, RGB\n" \
        "           input format supported for TGA images:\n" \
        "              uncompressed ARGB/RGB, compressed ARGB/RGB\n" \
        "           input format supported for BMP images:\n" \
        "              uncompressed ARGB/RGB/RGBX\n" \
        "           Image filename must include texture CRC (0xhhhhhhhh)\n" \
        "     Output file is DDS image\n" \
        "\n" \
        "  --convert-game-images --gameid <game id> --input <input dir> --output <output dir> [--mark-to-convert]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     input dir: directory to be converted, containing following file extension(s):\n" \
        "        Input files with following extension:\n" \
        "        DDS, BMP, TGA, PNG\n" \
        "           input format supported for DDS images:\n" \
        "              DXT1, DXT3, DTX5, ATI2, V8U8, G8, ARGB, RGB\n" \
        "           input format supported for TGA images:\n" \
        "              uncompressed ARGB/RGB, compressed ARGB/RGB\n" \
        "           input pixel format supported for BMP images:\n" \
        "              uncompressed ARGB/RGB/RGBX\n" \
        "           Image filename must include texture CRC (0xhhhhhhhh)\n" \
        "     output dir: directory where textures converted to DDS are placed\n" \
        "\n" \
        "  --extract-mod --gameid <game id> --input <input dir/file> [--output <output dir>] [--ipc]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     input dir: directory of ME3Explorer MOD file(s)\n" \
        "     input file: ME3Explorer MOD file to be extracted\n" \
        "     Can extract textures and package export raw data\n" \
        "     Naming pattern used for package in DLC:\n" \
        "        D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n" \
        "        example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin\n" \
        "     Naming pattern used for package in base directory:\n" \
        "        B<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n" \
        "        example: B23-BioH_EDI_00_Explore.pcc-E5090.bin\n" \
        "\n" \
        "  --extract-mem --gameid <game id> --input <input dir/file> [--output <output dir>] [--ipc]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     input dir: directory of MEM mod file(s)\n" \
        "     input file: MEM file to be extracted\n" \
        "     Can extract textures and package export raw data\n" \
        "     Naming pattern used for package in DLC:\n" \
        "        D<DLC dir length>-<DLC dir>-<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n" \
        "        example: D10-DLC_HEN_PR-23-BioH_EDI_02_Explore.pcc-E6101.bin\n" \
        "     Naming pattern used for package in base directory:\n" \
        "        B<pkg filename length>-<pkg filename>-E<pkg export id>.bin\n" \
        "        example: B23-BioH_EDI_00_Explore.pcc-E5090.bin\n" \
        "\n" \
        "  --extract-tpf --gameid <game id> --input <input dir/file> [--output <output dir>] [--ipc]\n" \
        "     input dir: directory containing the TPF file(s) to be extracted\n" \
        "     input file: TPF file to be extracted\n" \
        "     Textures are extracted as they are in the TPF, no additional modifications are made.\n" \
        "\n" \
        "  --convert-image --format <output pixel format> [--threshold <dxt1 alpha threshold>] --input <input image> --output <output image>\n" \
        "     input image file types: DDS, BMP, TGA, PNG\n" \
        "           input format supported for DDS images:\n" \
        "              DXT1, DXT3, DTX5, ATI2, V8U8, G8, ARGB, RGB\n" \
        "           input format supported for TGA images:\n" \
        "              uncompressed ARGB/RGB, compressed ARGB/RGB\n" \
        "           input format supported for BMP images:\n" \
        "              uncompressed ARGB/RGB/RGBX\n" \
        "     output image file type: DDS\n" \
        "     output pixel format: DXT1 (no alpha), DXT1a (alpha), DXT3, DXT5, ATI2, V8U8, G8, ARGB, RGB\n" \
        "     For DXT1a you have to set the alpha threshold (0-255). 128 is suggested as a default value.\n" \
        "\n" \
        "  --extract-all-dds --gameid <game id> --output <output dir> [--tfc-name <filter name>|--pcc-only|--tfc-only] [--package-path <path>] [--map-crc]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     output dir: directory where textures converted to DDS are placed\n" \
        "     TFC filter name: it will filter only textures stored in specific TFC file.\n" \
        "     Or option: --pcc-only to extract only textures stored in packages.\n" \
        "     Or option: --tfc-only to extract only textures stored in TFC files.\n" \
        "     Package path: single package mode.\n" \
        "     Map Crc: it will try to find vanilla texture crc from texture map.\n" \
        "     Textures are extracted as they are in game data, only DDS header is added.\n" \
        "\n" \
        "  --extract-all-png --gameid <game id> --output <output dir> [--tfc-name <filter name>|--pcc-only|--tfc-only] [--package-path <path>] [--map-crc]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     output dir: directory where textures converted to PNG are placed\n" \
        "     TFC filter name: it will filter only textures stored in specific TFC file.\n" \
        "     Or option: --pcc-only to extract only textures stored in packages.\n" \
        "     Or option: --tfc-only to extract only textures stored in TFC files.\n" \
        "     Package path: single package mode.\n" \
        "     Map Crc: it will try to find vanilla texture crc from texture map.\n" \
        "     Textures are extracted with only top mipmap.\n" \
        "  --extract-all-bik --gameid <game id> --output <output dir> [--tfc-name <filter name>|--pcc-only|--tfc-only] [--package-path <path>] [--map-crc]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     output dir: directory where movie textures are placed\n" \
        "     TFC filter name: it will filter only textures stored in specific TFC file.\n" \
        "     Or option: --pcc-only to extract only textures stored in packages.\n" \
        "     Or option: --tfc-only to extract only textures stored in TFC files.\n" \
        "     Package path: single package mode.\n" \
        "     Map Crc: it will try to find vanilla texture crc from texture map.\n" \
        "     Textures are extracted as they are in game data.\n" \
        "\n" \
        "  --compact-dlc --gameid <game id> --dlc-name <DLC name> [--pull-textures] [--no-compression] [--ipc]\n" \
        "     Compact textures in TFC, repack textures according compression flag.\n" \
        "     DLC name is DLC folder name.\n" \
        "     Pull textures option make DLC as standalone, not depend on external TFC files.\n" \
        "\n" \
        "  --unpack-archive --input <zip/7z/rar file> [--output <output path>] [--flatten-archive-path] [--ipc]\n" \
        "     Unpack ZIP/7ZIP/RAR file.\n" \
        "  --set-game-data-path --gameid <game id> --path <path>\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     Set game data path to <path>.\n" \
        "\n";
#if !defined(_WIN32)
    help +=
        "  --set-game-user-path --gameid <game id> --path <path>\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     Set game user config path to <path>.\n" \
        "\n";
#endif
    help +=
        "  --fix-textures-property --gameid <game id> [--filter <string>] [--ipc]\n" \
        "     Fix missing \"NeverStream\" property for textures\n" \
        "     which has more than 6 mips in package.\n" \
        "     If filter param is provided, match packages by filter \"string\".\n" \
        "\n" \
        "\n" \
        "  Additonal option to enable debug logs level to all commands: --debug-logs\n" \
        "\n";
    PINFO(help);
}
