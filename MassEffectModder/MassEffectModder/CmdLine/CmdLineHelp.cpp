/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2021 Pawel Kolodziejski
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
        "  --scan --gameid <game id>\n" \
        "     Scan game data.\n" \
        "\n" \
        "  --scan-textures --gameid <game id> [--ipc]\n" \
        "     Scan game data for textures.\n" \
        "\n" \
        "  --update-toc --gameid <game id>\n" \
        "     Update TOC files\n" \
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
        "  [--repack] [--skip-markers] [--ipc] [--alot-mode] [--limit-2k] [--verify]\n" \
        "     Install MEM mods from input directory.\n" \
        "\n" \
        "  --detect-mods --gameid <game id> [--ipc]\n" \
        "     Detect compatible mods.\n" \
        "\n" \
        "  --detect-bad-mods --gameid <game id> [--ipc]\n" \
        "     Detect not compatible mods.\n" \
        "\n" \
        "  --apply-lods-gfx --gameid <game id>\n" \
        "     Update GFX settings.\n" \
        "\n" \
        "  --convert-to-mem --gameid <game id> --input <input dir> --output <output file> [--mark-to-convert] [--ipc]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     input dir: directory to be converted, containing following file extension(s):\n" \
        "        MEM, TPF\n" \
        "        DDS, BMP, TGA, PNG\n" \
        "           input format supported for DDS images:\n" \
        "              DXT1, DXT3, DTX5, ATI2, V8U8, G8, ARGB, RGB, RGBA, BC7\n" \
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
        "              DXT1, DXT3, DTX5, ATI2, V8U8, G8, ARGB, RGB, RGBA, BC5, BC7\n" \
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
        "              DXT1, DXT3, DTX5, ATI2, V8U8, G8, ARGB, RGB, RGBA, BC5, BC7\n" \
        "           input format supported for TGA images:\n" \
        "              uncompressed ARGB/RGB, compressed ARGB/RGB\n" \
        "           input pixel format supported for BMP images:\n" \
        "              uncompressed ARGB/RGB/RGBX\n" \
        "           Image filename must include texture CRC (0xhhhhhhhh)\n" \
        "     output dir: directory where textures converted to DDS are placed\n" \
        "\n" \
        "  --extract-mem --gameid <game id> --input <input dir/file> [--output <output dir>] [--ipc]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     input dir: directory of MEM mod file(s)\n" \
        "     input file: MEM file to be extracted\n" \
        "\n" \
        "  --convert-image --format <output pixel format> [--threshold <dxt1 alpha threshold>] --input <input image> --output <output image>\n" \
        "     input image file types: DDS, BMP, TGA, PNG\n" \
        "           input format supported for DDS images:\n" \
        "              DXT1, DXT3, DTX5, ATI2, V8U8, G8, ARGB, RGB, RGBA, BC5, BC7\n" \
        "           input format supported for TGA images:\n" \
        "              uncompressed ARGB/RGB, compressed ARGB/RGB\n" \
        "           input format supported for BMP images:\n" \
        "              uncompressed ARGB/RGB/RGBX\n" \
        "     output image file type: DDS\n" \
        "     output pixel format: DXT1 (no alpha), DXT1a (alpha), DXT3, DXT5, ATI2, V8U8, G8, ARGB, RGB, RGBA, BC5, BC7\n" \
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
        "  --extract-all-png --gameid <game id> --output <output dir> [--tfc-name <filter name>|--pcc-only|--tfc-only] [--package-path <path>] [--map-crc] [--clear-alpha]\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     output dir: directory where textures converted to PNG are placed\n" \
        "     TFC filter name: it will filter only textures stored in specific TFC file.\n" \
        "     Or option: --pcc-only to extract only textures stored in packages.\n" \
        "     Or option: --tfc-only to extract only textures stored in TFC files.\n" \
        "     Package path: single package mode.\n" \
        "     Map Crc: it will try to find vanilla texture crc from texture map.\n" \
        "     Alpha channel can be cleared in PNG output by \"--clear-alpha\".n" \
        "     Textures are extracted with only top mipmap.\n" \
        "\n" \
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
        "  --set-game-data-path --gameid <game id> --path <path>\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     Set game data path to <path>.\n" \
        "\n" \
        "  --get-game-paths [--ipc]\n" \
        "     Print game paths.\n" \
        "\n" \
        "  --unpack-archive --input <zip/7z/rar file> [--output <output path>] [--flatten-archive-path] [--filter-with-ext <ext>] [--ipc]\n" \
        "     Unpack ZIP/7ZIP/RAR archive file.\n" \
        "\n" \
        "  --list-archive --input <zip/7z/rar file> [--ipc]\n" \
        "     List content of ZIP/7ZIP/RAR archive file.\n" \
        "\n";
#if !defined(_WIN32)
    help +=
        "  --set-game-user-path --gameid <game id> --path <path>\n" \
        "     game id: 1 for ME1, 2 for ME2, 3 for ME3\n" \
        "     Set game user config path to <path>.\n" \
        "\n";
#endif
    help +=
        "\n" \
        "\n" \
        "  Additonal option to enable debug logs level to all commands: --debug-logs\n" \
        "\n";
    PINFO(help);
}
