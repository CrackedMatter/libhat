#include <libhat/defines.hpp>
#if defined(LIBHAT_X86) || defined(LIBHAT_X86_64)

#include <libhat/system.hpp>

#include <array>
#include <cstdint>
#include <bitset>
#include <vector>
#include <immintrin.h>
#include <cstring>

#ifdef LIBHAT_WINDOWS
    #include <intrin.h>
#endif

#ifdef LIBHAT_UNIX
    #include <cpuid.h>
#endif

#ifndef _XCR_XFEATURE_ENABLED_MASK
    #define _XCR_XFEATURE_ENABLED_MASK 0
#endif

namespace hat {

    static constexpr int CPU_BASIC_INFO = 0;
    static constexpr int CPU_EXTENDED_INFO = static_cast<int>(0x80000000);
    static constexpr int CPU_BRAND_STRING = static_cast<int>(0x80000004);

    // don't re-use the __cpuid name here because linux macro'd it
    #ifdef LIBHAT_WINDOWS
    static void cpuid_impl(int* info, int id) {
        __cpuid(info, id);
    }
    static void cpuidex_impl(int* info, int id, int subId) {
        __cpuidex(info, id, subId);
    }
    #endif
    #ifdef LIBHAT_UNIX
    static void cpuid_impl(int* info, int id) {
        __cpuid(id, info[0], info[1], info[2], info[3]);
    }
    static void cpuidex_impl(int* info, int id, int subId) {
        __cpuid_count(id, subId, info[0], info[1], info[2], info[3]);
    }
    #endif

    system_info_x86::system_info_x86() {
        std::array<int, 4> info{};
        std::vector<std::array<int, 4>> data{};
        std::vector<std::array<int, 4>> extData{};

        // Gather info
        hat::cpuid_impl(info.data(), CPU_BASIC_INFO);
        auto nIds = info[0];

        char vendor[0xC + 1]{};
        memcpy(vendor, &info[1], sizeof(int));
        memcpy(vendor + 4, &info[3], sizeof(int));
        memcpy(vendor + 8, &info[2], sizeof(int));

        for (int i = CPU_BASIC_INFO; i <= nIds; i++) {
            hat::cpuidex_impl(info.data(), i, 0);
            data.push_back(info);
        }
        // Gather extended info
        hat::cpuid_impl(info.data(), CPU_EXTENDED_INFO);
        int nExtIds = info[0];
        for (int i = CPU_EXTENDED_INFO; i <= nExtIds; i++) {
            hat::cpuidex_impl(info.data(), i, 0);
            extData.push_back(info);
        }

        // Read relevant info
        std::bitset<32> f_1_ECX_{};
        std::bitset<32> f_1_EDX_{};
        std::bitset<32> f_7_EBX_{};
        if (nIds >= 1) {
            f_1_ECX_ = (uint32_t) data[1][2];
            f_1_EDX_ = (uint32_t) data[1][3];
        }
        if (nIds >= 7) {
            f_7_EBX_ = (uint32_t) data[7][1];
        }

        // Read extended info
        char brand[0x40 + 1]{};
        if (nExtIds >= CPU_BRAND_STRING) {
            memcpy(brand, extData[2].data(), sizeof(info));
            memcpy(brand + 16, extData[3].data(), sizeof(info));
            memcpy(brand + 32, extData[4].data(), sizeof(info));
        }

        // Check OS capabilities
        bool avxsupport = false;
        bool avx512support = false;
        bool xsave = f_1_ECX_[26];
        bool osxsave = f_1_ECX_[27];
        if (xsave && osxsave) {
            // https://cdrdv2-public.intel.com/671190/253668-sdm-vol-3a.pdf (Page 2-20)
            const std::bitset<64> xcr = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
            avxsupport = xcr[1] && xcr[2]; // xmm and ymm
            avx512support = avxsupport && xcr[5] && xcr[6] && xcr[7]; // opmask and zmm
        }

        this->cpu_vendor = vendor;
        this->cpu_brand = brand;
        this->extensions = {
            .sse      = f_1_EDX_[25],
            .sse2     = f_1_EDX_[26],
            .sse3     = f_1_ECX_[0],
            .ssse3    = f_1_ECX_[9],
            .sse41    = f_1_ECX_[19],
            .sse42    = f_1_ECX_[20],
            .avx      = f_1_ECX_[28] && avxsupport,
            .avx2     = f_7_EBX_[5] && avxsupport,
            .avx512f  = f_7_EBX_[16] && avx512support,
            .avx512bw = f_7_EBX_[30] && avx512support,
            .popcnt   = f_1_ECX_[23],
            .bmi      = f_7_EBX_[3],
        };
    }
}
#endif
