#include <filesystem>
#include <fstream>
#include <iostream>
#include "ultramodern/ultra64.h"
#include "ultramodern/ultramodern.hpp"

#include "recomp.h"
#include "helpers.hpp"

#define PAK_DEBUG 0
#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof(arr[0]))
#define MAX_FILES 16

typedef struct ControllerPak {
    std::fstream header;
    std::fstream file;
} ControllerPak;

extern std::filesystem::path config_path;

static const char* pak_header_name = "controllerPak_header.sav";

static std::string Pfs_FileName(size_t file_no) {
    return "controllerPak_file_" + std::to_string(file_no) + ".sav";
}

// Older builds kept pak files in the working directory, so carry them over the first time
// the save folder has no pak of its own.
static void Pfs_MigrateFromWorkingDirectory(const std::filesystem::path& save_dir) {
    std::error_code ec;
    std::filesystem::path old_dir = std::filesystem::current_path(ec);
    if (ec || std::filesystem::equivalent(old_dir, save_dir, ec)) {
        return;
    }
    if (!std::filesystem::exists(old_dir / pak_header_name) || std::filesystem::exists(save_dir / pak_header_name)) {
        return;
    }

    std::filesystem::copy_file(old_dir / pak_header_name, save_dir / pak_header_name, ec);
    for (size_t i = 0; i < MAX_FILES; i++) {
        if (std::filesystem::exists(old_dir / Pfs_FileName(i))) {
            std::filesystem::copy_file(old_dir / Pfs_FileName(i), save_dir / Pfs_FileName(i), ec);
        }
    }
}

static std::filesystem::path Pfs_SaveFolder() {
    std::filesystem::path save_file_path = ultramodern::get_save_file_path();
    std::filesystem::path save_dir = save_file_path.empty() ? config_path / "saves" : save_file_path.parent_path();

    static std::filesystem::path prepared_dir;
    if (save_dir != prepared_dir) {
        std::error_code ec;
        std::filesystem::create_directories(save_dir, ec);
        Pfs_MigrateFromWorkingDirectory(save_dir);
        prepared_dir = save_dir;
    }
    return save_dir;
}

static std::filesystem::path Pfs_HeaderPath() {
    return Pfs_SaveFolder() / pak_header_name;
}

static std::filesystem::path Pfs_FilePath(size_t file_no) {
    return Pfs_SaveFolder() / Pfs_FileName(file_no);
}

void Pfs_PakHeader_Write(u32* file_size, u32* game_code, u16* company_code, u8* ext_name, u8* game_name, u8 fileIndex) {
    ControllerPak pak;

    pak.header.open(Pfs_HeaderPath(), std::ios::binary | std::ios::in | std::ios::out);

    if (!pak.header.good()) {
        assert(false);
    }
    if (!pak.header.is_open()) {
        assert(false);
    }

    /* Set file parameters to header */
    u32 seek = fileIndex * 0x20;

    // file_size
    pak.header.seekp(seek + 0x0, std::ios::beg);
    pak.header.write((char*) file_size, 4);
    // game_code
    pak.header.seekp(seek + 0x4, std::ios::beg);
    pak.header.write((char*) game_code, 4);
    // company_code
    pak.header.seekp(seek + 0x08, std::ios::beg);
    pak.header.write((char*) company_code, 2);
    // ext_name
    pak.header.seekp(seek + 0xC, std::ios::beg);
    pak.header.write((char*) ext_name, 4);
    // game_name
    pak.header.seekp(seek + 0x10, std::ios::beg);
    pak.header.write((char*) game_name, 16);

    pak.header.close();
}

void Pfs_PakHeader_Read(u32* file_size, u32* game_code, u16* company_code, u8* ext_name, u8* game_name, u8 fileIndex) {
    ControllerPak pak;

    pak.header.open(Pfs_HeaderPath(), std::ios::binary | std::ios::in | std::ios::out);

    if (!pak.header.good()) {
        assert(false);
    }
    if (!pak.header.is_open()) {
        assert(false);
    }

    /* Set file parameters to header */
    u32 seek = fileIndex * sizeof(OSPfsState);

    // file_size
    pak.header.seekg(seek + 0x0, std::ios::beg);
    pak.header.read((char*) file_size, 4);
    // game_code
    pak.header.seekg(seek + 0x4, std::ios::beg);
    pak.header.read((char*) game_code, 4);
    // company_code
    pak.header.seekg(seek + 0x08, std::ios::beg);
    pak.header.read((char*) company_code, 2);
    // ext_name
    pak.header.seekg(seek + 0xC, std::ios::beg);
    pak.header.read((char*) ext_name, 4);
    // game_name
    pak.header.seekg(seek + 0x10, std::ios::beg);
    pak.header.read((char*) game_name, 16);

    pak.header.close();
}

void Pfs_ByteSwapFile(u8* buffer, size_t size) {
    uint8_t c0, c1, c2, c3;

    for (size_t i = 0; i < size; i += 4) {
        c0 = buffer[i + 0];
        c1 = buffer[i + 1];
        c2 = buffer[i + 2];
        c3 = buffer[i + 3];

        buffer[i + 3] = c0;
        buffer[i + 2] = c1;
        buffer[i + 1] = c2;
        buffer[i + 0] = c3;
    }
}

void ByteSwapCopy(uint8_t* dst, uint8_t* src, size_t size_bytes) {
    for (size_t i = 0; i < size_bytes; i++) {
        dst[i] = src[i ^ 3];
    }
}

// Names are stored in the header file with each 4-byte group reversed, matching the layout
// that older versions produced by copying word-aligned names straight out of rdram.
static void Pfs_ReadGuestName(uint8_t* rdram, gpr addr, u8* out, size_t len) {
    for (size_t i = 0; i < len; i++) {
        out[i ^ 3] = MEM_BU(i, addr);
    }
}

static void Pfs_WriteGuestName(uint8_t* rdram, gpr addr, const u8* in, size_t len) {
    for (size_t i = 0; i < len; i++) {
        MEM_B(i, addr) = in[i ^ 3];
    }
}

static bool Pfs_FileExists(size_t file_no) {
    return std::filesystem::exists(Pfs_FilePath(file_no));
}

static void Pfs_CreateHeaderIfMissing() {
    std::filesystem::path header_path = Pfs_HeaderPath();
    if (!std::filesystem::exists(header_path)) {
        std::ofstream header(header_path, std::ios::binary);
    }
}

// An entry is only in use if its codes are set and its backing file still exists.
static bool Pfs_EntryUsed(size_t file_no, u32 game_code, u16 company_code) {
    return (company_code != 0) && (game_code != 0) && Pfs_FileExists(file_no);
}

extern "C" void osPfsIsPlug_recomp(uint8_t* rdram, recomp_context* ctx) {
    MEM_B(0, ctx->r5) = 0b0001; // *pattern = 1;
    ctx->r2 = 0;                // PFS_NO_ERROR
}

extern "C" void osPfsInit_recomp(uint8_t* rdram, recomp_context* ctx) {
    int32_t queue = _arg<0, int32_t>(rdram, ctx);
    OSPfs* pfs = _arg<1, OSPfs*>(rdram, ctx);
    s32 channel = _arg<2, s32>(rdram, ctx);

    pfs->queue = queue;
    pfs->channel = channel;
    pfs->status = 0x1;

    Pfs_CreateHeaderIfMissing();

    ctx->r2 = 0; // PFS_NO_ERROR
}

extern "C" void osPfsInitPak_recomp(uint8_t* rdram, recomp_context* ctx) {
    int32_t queue = _arg<0, int32_t>(rdram, ctx);
    OSPfs* pfs = _arg<1, OSPfs*>(rdram, ctx);
    s32 channel = _arg<2, s32>(rdram, ctx);

    pfs->queue = queue;
    pfs->channel = channel;
    pfs->status = 0x1;

    Pfs_CreateHeaderIfMissing();

// Test
#if PAK_DEBUG
    u32 file_size_ = 0x01020304;
    u32 game_code_ = 0x05060708;
    u16 company_code_ = 0x0910;
    u8 ext_name_[4] = { 0x11, 0x12, 0x13, 0x14 };
    u8 game_name_[16] = { 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24 };

    Pfs_PakHeader_Write(&file_size_, &game_code_, &company_code_, ext_name_, game_name_, 0);

    Pfs_PakHeader_Read(&file_size_, &game_code_, &company_code_, ext_name_, game_name_, 0);

    printf("TEST\n");
    printf("file_size_: %x\n", file_size_);
    printf("game_code: %x\n", game_code_);
    printf("company_code: %x\n", company_code_);

    printf("ext_name: ");
    for (size_t i = 0; i < 4; i++) {
        printf("%x", ext_name_[i]);
    }
    printf("\n");

    printf("game_name: ");
    for (size_t i = 0; i < 16; i++) {
        printf("%x", game_name_[i]);
    }
    printf("\n");
#endif

    ctx->r2 = 0; // PFS_NO_ERROR
}

extern "C" void osPfsFreeBlocks_recomp(uint8_t* rdram, recomp_context* ctx) {
    s32* bytes_not_used = _arg<1, s32*>(rdram, ctx);

    s32 usedSpace = 0;
    for (size_t i = 0; i < MAX_FILES; i++) {
        u32 file_size = 0;
        u32 game_code = 0;
        u16 company_code = 0;
        u8 ext_name[4] = { 0 };
        u8 game_name[16] = { 0 };

        Pfs_PakHeader_Read(&file_size, &game_code, &company_code, ext_name, game_name, i);

        if (Pfs_EntryUsed(i, game_code, company_code)) {
            usedSpace += file_size >> 8;
        }
    }

    *bytes_not_used = (123 - usedSpace) << 8;

    ctx->r2 = 0; // PFS_NO_ERROR
}

extern "C" void osPfsAllocateFile_recomp(uint8_t* rdram, recomp_context* ctx) {
    u16 company_code = _arg<1, u16>(rdram, ctx);
    u32 game_code = _arg<2, u32>(rdram, ctx);
    gpr game_name_addr = ctx->r7;
    gpr ext_name_addr = (gpr) MEM_W(0x10, ctx->r29);
    u32 file_size = (s32) MEM_W(0x14, ctx->r29);
    s32* file_no = TO_PTR(s32, MEM_W(0x18, ctx->r29));

    if ((company_code == 0) || (game_code == 0)) {
        ctx->r2 = 5; // PFS_ERR_INVALID
        return;
    }

    u8 game_name[16];
    u8 ext_name[4];
    Pfs_ReadGuestName(rdram, game_name_addr, game_name, sizeof(game_name));
    Pfs_ReadGuestName(rdram, ext_name_addr, ext_name, sizeof(ext_name));

    /* Search for a free slot */
    u8 freeFileIndex = MAX_FILES;
    for (size_t i = 0; i < MAX_FILES; i++) {
        u32 file_size_ = 0;
        u32 game_code_ = 0;
        u16 company_code_ = 0;
        u8 ext_name_[4] = { 0 };
        u8 game_name_[16] = { 0 };

        Pfs_PakHeader_Read(&file_size_, &game_code_, &company_code_, ext_name_, game_name_, i);

        if (!Pfs_EntryUsed(i, game_code_, company_code_)) {
            freeFileIndex = i;
            break;
        }
    }

    if (freeFileIndex == MAX_FILES) {
        ctx->r2 = 8; // PFS_DIR_FULL
        return;
    }

    Pfs_PakHeader_Write(&file_size, &game_code, &company_code, ext_name, game_name, freeFileIndex);

    /* Create empty file */

    ControllerPak pak;

    pak.file.open(Pfs_FilePath(freeFileIndex), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

    file_size = (file_size + 31) & ~31;

    u8* zero_block = (u8*) malloc(file_size);
    memset(zero_block, 0, file_size);

    pak.file.seekp(0, std::ios::beg);
    pak.file.write((char*) zero_block, file_size);

    free(zero_block);

    pak.file.close();

    *file_no = freeFileIndex;

    ctx->r2 = 0; // PFS_NO_ERROR
}

extern "C" void osPfsFileState_recomp(uint8_t* rdram, recomp_context* ctx) {
    s32 file_no = _arg<1, s32>(rdram, ctx);
    gpr state_addr = ctx->r6;

    u32 file_size = 0;
    u32 game_code = 0;
    u16 company_code = 0;
    u8 ext_name[4] = { 0 };
    u8 game_name[16] = { 0 };

    // should pass the state of the requested file_no to the incoming state pointer,
    // games call this function 16 times, once per file
    // fills the incoming state with the information inside the header of the pak.

    if (!Pfs_FileExists(file_no)) {
        ctx->r2 = 5;
        return;
    }

    /* Read game info from pak */
    Pfs_PakHeader_Read(&file_size, &game_code, &company_code, ext_name, game_name, file_no);

    // libultra layout: file_size 0x0, game_code 0x4, company_code 0x8, ext_name 0xA, game_name 0xE
    MEM_W(0x0, state_addr) = file_size;
    MEM_W(0x4, state_addr) = game_code;
    MEM_H(0x8, state_addr) = company_code;
    Pfs_WriteGuestName(rdram, state_addr + 0xA, ext_name, sizeof(ext_name));
    Pfs_WriteGuestName(rdram, state_addr + 0xE, game_name, sizeof(game_name));
    MEM_H(0x1E, state_addr) = 0;

    ctx->r2 = 0; // PFS_NO_ERROR
}

extern "C" void osPfsFindFile_recomp(uint8_t* rdram, recomp_context* ctx) {
    u16 company_code = _arg<1, u16>(rdram, ctx);
    u32 game_code = _arg<2, u32>(rdram, ctx);
    s32* file_no = TO_PTR(s32, MEM_W(0x14, ctx->r29));

    u8 game_name[16];
    u8 ext_name[4];
    Pfs_ReadGuestName(rdram, ctx->r7, game_name, sizeof(game_name));
    Pfs_ReadGuestName(rdram, (gpr) MEM_W(0x10, ctx->r29), ext_name, sizeof(ext_name));

    for (size_t i = 0; i < MAX_FILES; i++) {
        u32 file_size_ = 0;
        u32 game_code_ = 0;
        u16 company_code_ = 0;
        u8 ext_name_[4] = { 0 };
        u8 game_name_[16] = { 0 };

        Pfs_PakHeader_Read(&file_size_, &game_code_, &company_code_, ext_name_, game_name_, i);

        if (!Pfs_EntryUsed(i, game_code_, company_code_)) {
            continue;
        } else {
            if ((game_code == game_code_) && (company_code == company_code_)) {
                bool names_match = true;
                for (size_t j = 0; j < ARRAY_COUNT(game_name_); j++) {
                    if (game_name[j] != game_name_[j]) {
                        names_match = false;
                        break;
                    }
                }
                for (size_t j = 0; names_match && j < ARRAY_COUNT(ext_name_); j++) {
                    if (ext_name[j] != ext_name_[j]) {
                        names_match = false;
                        break;
                    }
                }
                if (!names_match) {
                    continue;
                }
                //  File found
                *file_no = i;
                ctx->r2 = 0; // PFS_NO_ERROR
                return;
            }
        }
    }

    // File not found
    ctx->r2 = 5; // PFS_ERR_INVALID
}

extern "C" void osPfsReadWriteFile_recomp(uint8_t* rdram, recomp_context* ctx) {
    s32 file_no = _arg<1, s32>(rdram, ctx);
    u8 flag = _arg<2, u8>(rdram, ctx);
    s32 offset = _arg<3, s32>(rdram, ctx);
    s32 size_in_bytes = (s32) MEM_W(0x10, ctx->r29);
    u8* data_buffer = TO_PTR(u8, MEM_W(0x14, ctx->r29));

    ControllerPak pak;

    std::filesystem::path file_path = Pfs_FilePath(file_no);
    pak.file.open(file_path, std::ios::binary | std::ios::in | std::ios::out);

    if (!std::filesystem::exists(file_path)) {
        ctx->r2 = 5; // PFS_ERR_INVALID
        return;
    }
    if (!pak.file.good()) {
        ctx->r2 = 5; // PFS_ERR_INVALID
        return;
    }
    if (!pak.file.is_open()) {
        ctx->r2 = 5; // PFS_ERR_INVALID
        return;
    }

    u8* swapBuffer = (u8*) malloc(size_in_bytes);
    if (flag == 0) {
        pak.file.seekg(offset, std::ios::beg);
        pak.file.read((char*) swapBuffer, size_in_bytes);
        ByteSwapCopy(data_buffer, swapBuffer, size_in_bytes);
    } else {
        ByteSwapCopy(swapBuffer, data_buffer, size_in_bytes);
        pak.file.seekp(offset, std::ios::beg);
        pak.file.write((char*) swapBuffer, size_in_bytes);
    }
    free(swapBuffer);

    pak.file.close();

    ctx->r2 = 0; // PFS_NO_ERROR
}

extern "C" void osPfsChecker_recomp(uint8_t* rdram, recomp_context* ctx) {
    ctx->r2 = 0; // PFS_NO_ERROR
}

extern "C" void osPfsNumFiles_recomp(uint8_t* rdram, recomp_context* ctx) {
    s32* max_files = _arg<1, s32*>(rdram, ctx);
    s32* files_used = _arg<2, s32*>(rdram, ctx);

    u8 files = 0;
    for (size_t i = 0; i < MAX_FILES; i++) {
        u32 file_size = 0;
        u32 game_code = 0;
        u16 company_code = 0;
        u8 ext_name[4] = { 0 };
        u8 game_name[16] = { 0 };

        Pfs_PakHeader_Read(&file_size, &game_code, &company_code, ext_name, game_name, i);

        if (Pfs_EntryUsed(i, game_code, company_code)) {
            files++;
        }
    }

    *files_used = files;
    *max_files = MAX_FILES;

    ctx->r2 = 0; // PFS_NO_ERROR
}

extern "C" void osPfsDeleteFile_recomp(uint8_t* rdram, recomp_context* ctx) {
    u16 company_code = _arg<1, u16>(rdram, ctx);
    u32 game_code = _arg<2, u32>(rdram, ctx);

    u8 game_name[16];
    u8 ext_name[4];
    Pfs_ReadGuestName(rdram, ctx->r7, game_name, sizeof(game_name));
    Pfs_ReadGuestName(rdram, (gpr) MEM_W(0x10, ctx->r29), ext_name, sizeof(ext_name));

#if PAK_DEBUG
    printf("osPfsDeleteFile_recomp: REQUEST\n");
    printf(" company_code: %x\n", company_code);
    printf(" game_code: %x\n", game_code);

    printf(" ext_name: ");
    for (size_t i = 0; i < 4; i++) {
        printf("%x", ext_name[i]);
    }
    printf("\n");

    printf(" game_name: ");
    for (size_t i = 0; i < 16; i++) {
        printf("%x", game_name[i]);
    }
    printf("\n");
#endif

    if (company_code == 0 || game_code == 0) {
        ctx->r2 = 5; // PFS_ERR_INVALID
        return;
    }

    ControllerPak pak;

    for (int i = 0; i < MAX_FILES; i++) {
        u32 file_size_ = 0;
        u32 game_code_ = 0;
        u16 company_code_ = 0;
        u8 ext_name_[4] = { 0 };
        u8 game_name_[16] = { 0 };

        Pfs_PakHeader_Read(&file_size_, &game_code_, &company_code_, ext_name_, game_name_, i);

#if PAK_DEBUG
        if (i == 0) {
            printf("osPfsDeleteFile_recomp: READ\n");
            printf("company_code: %x\n", company_code_);
            printf("game_code: %x\n", game_code_);

            printf("ext_name_: ");
            for (size_t i = 0; i < 4; i++) {
                printf("%x", ext_name_[i]);
            }
            printf("\n");

            printf("game_name_: ");
            for (size_t i = 0; i < 16; i++) {
                printf("%x", game_name_[i]);
            }
            printf("\n");
        }
#endif

        if (!Pfs_EntryUsed(i, game_code_, company_code_)) {
            continue;
        } else {
            if ((game_code == game_code_) && (company_code == company_code_)) {
                int gncount = 0;
                int encount = 0;

                for (size_t i = 0; i < ARRAY_COUNT(game_name_); i++) {
                    if (game_name[i] == game_name_[i]) {
                        gncount++;
                    }
                }
                for (size_t i = 0; i < ARRAY_COUNT(ext_name_); i++) {
                    if (ext_name[i] == ext_name_[i]) {
                        encount++;
                    }
                }
                if ((gncount != 16) || encount != 4) {
                    continue;
                }
                // File found

                pak.header.open(Pfs_HeaderPath(), std::ios::binary | std::ios::in | std::ios::out);

                if (!pak.header.good()) {
                    assert(false);
                }
                if (!pak.header.is_open()) {
                    assert(false);
                }

                u32 seek = i * sizeof(OSPfsState);

                // Zero out the header for this file.
                u8* zero_block = (u8*) malloc(sizeof(OSPfsState));
                memset(zero_block, 0, sizeof(OSPfsState));
                pak.header.seekp(seek + 0x0, std::ios::beg);
                pak.header.write((char*) zero_block, sizeof(OSPfsState));
                free(zero_block);

                pak.header.close();

                std::error_code ec;
                std::filesystem::remove(Pfs_FilePath(i), ec);

                ctx->r2 = 0; // PFS_NO_ERROR
                return;
            }
        }
    }

    // File not found
    ctx->r2 = 5; // PFS_ERR_INVALID
    return;
}

extern "C" void osPfsRepairId_recomp(uint8_t* rdram, recomp_context* ctx) {
    _return<s32>(ctx, 0); // PFS_NO_ERROR
}
