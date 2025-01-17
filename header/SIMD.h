Int8 FORCEINLINE SI16ToI8(Int16 v)
{
	if (v <= -128)
		return -128;
	if (v >= 127)
		return 127;
	return (Int8)v;
}

Int16 FORCEINLINE SI32ToI16(Int32 v)
{
	if (v <= -32768)
		return -32768;
	if (v >= 32767)
		return 32767;
	return (Int16)v;
}

UInt8 FORCEINLINE SI32ToU8(Int32 v)
{
	if (v <= 0)
		return 0;
	if (v >= 255)
		return 255;
	return (UInt8)v;
}

UInt8 FORCEINLINE SU32ToU8(UInt32 v)
{
	if (v >= 255)
		return 255;
	return (UInt8)v;
}

#if 1 && (defined(CPU_X86_64) || defined(CPU_X86_32))
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
typedef __m128i UInt8x4;
typedef __m128i UInt8x8;
typedef __m128i UInt8x16;
typedef __m128i Int16x4;
typedef __m128i UInt16x4;
typedef __m128i Int16x8;
typedef __m128i UInt16x8;
typedef __m128i Int32x4;

#define PUInt8x4Clear() _mm_setzero_si128()
#define PUInt8x8Clear() _mm_setzero_si128()
#define PInt16x4Clear() _mm_setzero_si128()
#define PInt16x8Clear() _mm_setzero_si128()
#define PInt32x4Clear() _mm_setzero_si128()
#define PUInt8x4SetA(v) _mm_set1_epi8((Int8)(UInt8)v)
#define PUInt8x8SetA(v) _mm_set1_epi8((Int8)(UInt8)v)
#define PUInt8x16SetA(v) _mm_set1_epi8((Int8)(UInt8)v)
#define PUInt16x4SetA(v) _mm_set1_epi16((Int16)v)
#define PInt16x4SetA(v) _mm_set1_epi16(v)
#define PInt16x8SetA(v) _mm_set1_epi16(v)
#define PInt32x4SetA(v) _mm_set1_epi32(v)
#define PUInt32x4SetA(v) _mm_set1_epi32(v)
#define PLoadUInt8x4(ptr) _mm_cvtsi32_si128(*(Int32*)(ptr))
#define PLoadUInt8x8(ptr) _mm_loadl_epi64((__m128i*)(ptr))
#define PLoadUInt8x16(ptr) _mm_loadu_si128((__m128i*)(ptr))
#define PLoadInt16x4(ptr) _mm_loadl_epi64((__m128i*)(ptr))
#define PLoadInt16x8(ptr) _mm_loadu_si128((__m128i*)(ptr))
#define PLoadInt16x8A(ptr) _mm_load_si128((__m128i*)(ptr))
#define PLoadInt32x4(ptr) _mm_loadu_si128((__m128i*)(ptr))
#define PLoadInt32x4A(ptr) _mm_load_si128((__m128i*)(ptr))
#define PMLoadInt16x4(ptr1, ptr2) _mm_set_epi64x(*(Int64*)ptr1, *(Int64*)ptr2)
#define PStoreUInt8x4(ptr, v) *(Int32*)ptr = _mm_cvtsi128_si32(v)
#define PStoreUInt8x8(ptr, v) _mm_storel_epi64((__m128i*)(ptr), v)
#define PStoreUInt8x16(ptr, v) _mm_store_si128((__m128i*)(ptr), v)
#define PStoreInt16x2(ptr, v) *(Int32*)(ptr) = _mm_cvtsi128_si32(v)
#define PStoreInt16x4(ptr, v) _mm_storel_epi64((__m128i*)(ptr), v)
#define PStoreInt16x8(ptr, v) _mm_storeu_si128((__m128i*)(ptr), v)
#define PStoreInt16x8A(ptr, v) _mm_store_si128((__m128i*)(ptr), v)
#define PStoreInt16x8NC(ptr, v) _mm_stream_si128((__m128i*)(ptr), v)
#define PStoreUInt16x4(ptr, v) _mm_storel_epi64((__m128i*)(ptr), v)
#define PStoreInt32x4(ptr, v) _mm_storeu_si128((__m128i*)(ptr), v)
#define PStoreInt32x4A(ptr, v) _mm_store_si128((__m128i*)(ptr), v)
#define PStoreInt32x4NC(ptr, v) _mm_stream_si128((__m128i*)(ptr), v)
#define PCONVU16x4_I(v) (v)
#define PCONVU16x8_I(v) (v)
#define PCONVI8x4_U(v) (v)
#define PCONVI8x8_U(v) (v)
#define PCONVI8x16_U(v) (v)
#define PCONVI16x4_U(v) (v)
#define PCONVI16x8_U(v) (v)
#define PEXTUW4(v, i) (UInt16)_mm_extract_epi16(v, i)
#define PEXTUW8(v, i) (UInt16)_mm_extract_epi16(v, i)
#define PEXTW4(v, i) (Int16)_mm_extract_epi16(v, i)
#define PEXTW8(v, i) (Int16)_mm_extract_epi16(v, i)
#if 0
#define PEXTD4(v, i) _mm_extract_epi32(v, i)
#else
#define PEXTD4(v, i) ((_mm_extract_epi16(v, i << 1) & 0xffff) | (_mm_extract_epi16(v, (i << 1) + 1) << 16))
#endif
#define PINSUW4(v, i, iv) _mm_insert_epi16(v, iv, i)
#define PUNPCKBB8(v1, v2) _mm_unpacklo_epi8(v1, v2)
#define PUNPCKUBB4(v1, v2) _mm_unpacklo_epi8(v1, v2)
#define PUNPCKUBW4(v1, v2) _mm_unpacklo_epi8(v1, v2)
#define PUNPCKUBW8(v1, v2) _mm_unpacklo_epi8(v1, v2)
#define PUNPCKLUBW8(v1, v2) _mm_unpacklo_epi8(v1, v2)
#define PUNPCKHUBW8(v1, v2) _mm_unpackhi_epi8(v1, v2)
#define PUNPCKWW4(v1, v2) _mm_unpacklo_epi16(v1, v2)
#define PUNPCKLWW8(v1, v2) _mm_unpacklo_epi16(v1, v2)
#define PUNPCKHWW8(v1, v2) _mm_unpackhi_epi16(v1, v2)
#define PUNPCKLWD4(v1, v2) _mm_unpacklo_epi16(v1, v2)
#define PUNPCKHWD4(v1, v2) _mm_unpackhi_epi16(v1, v2)
#define PUNPCKWD4(v1, v2) _mm_unpacklo_epi16(v1, v2)
#define PMergeW4(v1, v2) _mm_unpacklo_epi64(v1, v2)
#define PMergeLW4(v1, v2) _mm_unpacklo_epi64(v1, v2)
#define PMergeHW4(v1, v2) _mm_unpackhi_epi64(v1, v2)
#define PMergeSARDW4(v1, v2, cnt) _mm_packs_epi32(_mm_srai_epi32(v1, cnt), _mm_srai_epi32(v2, cnt))
#define PSALW4(v1, v2) _mm_slli_epi16(v1, v2)
#define PSHRW4(v1, v2) _mm_srli_epi16(v1, v2)
#define PSHRW8(v1, v2) _mm_srli_epi16(v1, v2)
#define PSARW4(v1, v2) _mm_srai_epi16(v1, v2)
#define PSARW8(v1, v2) _mm_srai_epi16(v1, v2)
#define PSARD4(v1, v2) _mm_srai_epi32(v1, v2)
#define PSARSDW4(v1, cnt) _mm_packs_epi32(_mm_srai_epi32(v1, cnt), v1)
#define PSARSDW8(v1, v2, cnt) _mm_packs_epi32(_mm_srai_epi32(v1, cnt), _mm_srai_epi32(v2, cnt))
#define PSHRADDWB4(v1, v2, cnt) _mm_packs_epi16(_mm_srli_epi16(_mm_adds_epu16(v1, v2), cnt), v2)
#define PADDUB4(v1, v2) _mm_add_epi8(v1, v2)
#define PADDUB8(v1, v2) _mm_add_epi8(v1, v2)
#define PADDUB16(v1, v2) _mm_add_epi8(v1, v2)
#define PADDW4(v1, v2) _mm_add_epi16(v1, v2)
#define PADDUW4(v1, v2) _mm_add_epi16(v1, v2)
#define PADDW8(v1, v2) _mm_add_epi16(v1, v2)
#define PADDD4(v1, v2) _mm_add_epi32(v1, v2)
#define PADDSUWB4(v1, v2) _mm_packs_epi16(_mm_add_epi16(v1, v2), v2)
#define PSADDW4(v1, v2) _mm_adds_epi16(v1, v2)
#define PSADDUW4(v1, v2) _mm_adds_epu16(v1, v2)
#define PSADDW8(v1, v2) _mm_adds_epi16(v1, v2)
#define PHSADDW8_4(v1, v2) _mm_adds_epi16(_mm_unpacklo_epi64(v1, v2), _mm_unpackhi_epi64(v1, v2))
#define PMADDWD(v1, v2) _mm_madd_epi16(v1, v2)
#define PSUBW4(v1, v2) _mm_sub_epi16(v1, v2)
#define PSUBD4(v1, v2) _mm_sub_epi32(v1, v2)
#define PMULHW4(v1, v2) _mm_mulhi_epi16(v1, v2)
#define PMULHW8(v1, v2) _mm_mulhi_epi16(v1, v2)
#define PMULUHW4(v1, v2) _mm_mulhi_epu16(v1, v2)
#define PMULULW4(v1, v2) _mm_mullo_epi16(v1, v2)
#define PMULM2HW4(v1, v2) _mm_slli_epi16(_mm_mulhi_epi16(v1, v2), 1)
#define PMULM2HW8(v1, v2) _mm_slli_epi16(_mm_mulhi_epi16(v1, v2), 1)
#define PMULLD4(v1, v2) _mm_mullo_epi16(v1, v2)
#define SI16ToI8x4(v1) _mm_packs_epi16(v1, v1)
#define SI16ToI8x8(v1) _mm_packs_epi16(v1, v1)
#define SI16ToI8x16(v1, v2) _mm_packs_epi16(v1, v2)
#define SU16ToU8x4(v1) _mm_packs_epi16(v1, v1)
#define SI32ToU8x8(v1, v2) _mm_packus_epi16(_mm_packs_epi32(v1, v2), v1)
#define SI32ToI16x4(v1) _mm_packs_epi32(v1, v1)
#define SI32ToI16x8(v1, v2) _mm_packs_epi32(v1, v2)

#if defined(__AVX__)
typedef __m256i Int32x8;
#define PResetAVX() _mm256_zeroupper()
#define PInt32x8Clear() _mm256_setzero_si256()
#define PLoadInt32x8(ptr) _mm256_loadu_si256((__m256i*)ptr)
#define PStoreInt32x8(ptr, v) _mm256_store_si256((__m256i*)ptr, v)
#define PStoreInt32x8NC(ptr, v) _mm256_stream_si256((__m256i*)ptr, v)
#else
typedef struct
{
	__m128i vals[2];
} Int32x8;

void FORCEINLINE PResetAVX()
{

}

Int32x8 FORCEINLINE PInt32x8Clear()
{
	Int32x8 ret;
	ret.vals[0] = PInt32x4Clear();
	ret.vals[1] = ret.vals[0];
	return ret;
}

Int32x8 FORCEINLINE PLoadInt32x8(const void *ptr)
{
	Int32x8 ret;
	ret.vals[0] = PLoadInt32x4(ptr);
	ret.vals[1] = PLoadInt32x4(16 + (const UInt8*)ptr);
	return ret;
}

void FORCEINLINE PStoreInt32x8(void *ptr, Int32x8 v)
{
	PStoreInt32x4(ptr, v.vals[0]);
	PStoreInt32x4(16 + (UInt8*)ptr, v.vals[1]);
}

void FORCEINLINE PStoreInt32x8NC(void *ptr, Int32x8 v)
{
	PStoreInt32x4NC(ptr, v.vals[0]);
	PStoreInt32x4NC(16 + (UInt8*)ptr, v.vals[1]);
}

#endif
#elif 1 && (defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(__ARM_ARCH_7) || defined(_M_ARM64))
#if defined(_M_ARM64)
#include <arm64_neon.h>
#else
#include <arm_neon.h>
#endif
typedef uint8x8_t UInt8x4;
typedef uint8x8_t UInt8x8;
typedef uint8x16_t UInt8x16;
typedef int16x4_t Int16x4;
typedef uint16x4_t UInt16x4;
typedef int16x8_t Int16x8;
typedef uint16x8_t UInt16x8;
typedef int32x4_t Int32x4;
//typedef __m256i Int32x8;

#define PUInt8x4Clear() vdup_n_u8(0)
#define PUInt8x8Clear() vdup_n_u8(0)
#define PInt16x4Clear() vdup_n_s16(0)
#define PInt16x8Clear() vdupq_n_s16(0)
#define PInt32x4Clear() vdupq_n_s32(0)
#define PUInt8x4SetA(v) vdup_n_u8(v)
#define PUInt8x8SetA(v) vdup_n_u8(v)
#define PUInt8x16SetA(v) vdupq_n_u8(v)
#define PInt16x4SetA(v) vdup_n_s16(v)
#define PInt16x8SetA(v) vdupq_n_s16(v)
#define PUInt16x4SetA(v) vdup_n_u16(v)
#define PInt32x4SetA(v) vdupq_n_s32(v)
#define PUInt32x4SetA(v) vdupq_n_u32(v)
#if defined(_M_ARM64)
#define PLoadUInt8x4(ptr) vreinterpret_u32_u8(vld1_u32((const UInt32*)ptr))
#define PLoadUInt8x8(ptr) (*(uint8x8_t*)(ptr))
#define PLoadUInt8x16(ptr) (*(uint8x16_t*)(ptr))
#define PLoadInt16x4(ptr) (*(int16x4_t*)(ptr))
#define PLoadInt16x8(ptr) (*(int16x8_t*)(ptr))
#define PLoadInt16x8A(ptr) (*(int16x8_t*)(ptr))
#define PLoadInt32x4(ptr) (*(int32x4_t*)(ptr))
#define PLoadInt32x4A(ptr) (*(int32x4_t*)(ptr))
#define PMLoadInt16x4(ptr1, ptr2) vcombine_s16(*(int16x4_t*)(ptr1), *(int16x4_t*)(ptr2))
#define PStoreUInt8x4(ptr, v) *(UInt32*)(ptr) = vget_lane_u32(vreinterpret_u8_u32(v), 0)
#define PStoreUInt8x8(ptr, v) *(uint8x8_t*)(ptr) = v
#define PStoreUInt8x16(ptr, v) *(uint8x16_t*)(ptr) = v
#define PStoreInt16x2(ptr, v) *(UInt32*)(ptr) = vget_lane_u32(vreinterpret_u8_u32(v), 0)
#define PStoreInt16x4(ptr, v) *(int16x4_t*)(ptr) = v
#define PStoreUInt16x4(ptr, v) *(uint16x4_t*)(ptr) = v
#define PStoreInt16x8(ptr, v) *(int16x8_t*)(ptr) = v
#define PStoreInt16x8A(ptr, v) *(int16x8_t*)(ptr) = v
#define PStoreInt16x8NC(ptr, v) *(int16x8_t*)(ptr) = v
#define PStoreInt32x4(ptr, v) *(int32x4_t*)(ptr) = v
#define PStoreInt32x4NC(ptr, v) *(int32x4_t*)(ptr) = v
#else
#define PLoadUInt8x4(ptr) vreinterpret_u32_u8(vld1_u32((const UInt32*)ptr))
#define PLoadUInt8x8(ptr) (*(volatile uint8x8_t*)(ptr))
#define PLoadUInt8x16(ptr) (*(volatile uint8x16_t*)(ptr))
#define PLoadInt16x4(ptr) (*(volatile int16x4_t*)(ptr))
#define PLoadInt16x8(ptr) (*(volatile int16x8_t*)(ptr))
#define PLoadInt16x8A(ptr) (*(volatile int16x8_t*)(ptr))
#define PLoadInt32x4(ptr) (*(volatile int32x4_t*)(ptr))
#define PLoadInt32x4A(ptr) (*(volatile int32x4_t*)(ptr))
#define PMLoadInt16x4(ptr1, ptr2) vcombine_s16(*(int16x4_t*)(ptr1), *(int16x4_t*)(ptr2))
#define PStoreUInt8x4(ptr, v) *(UInt32*)(ptr) = vget_lane_u32(vreinterpret_u8_u32(v), 0)
#define PStoreUInt8x8(ptr, v) *(uint8x8_t*)(ptr) = v
#define PStoreUInt8x16(ptr, v) *(uint8x16_t*)(ptr) = v
#define PStoreInt16x2(ptr, v) *(UInt32*)(ptr) = vget_lane_u32(vreinterpret_u8_u32(v), 0)
#define PStoreInt16x4(ptr, v) *(int16x4_t*)(ptr) = v
#define PStoreUInt16x4(ptr, v) *(uint16x4_t*)(ptr) = v
#define PStoreInt16x8(ptr, v) *(int16x8_t*)(ptr) = v
#define PStoreInt16x8A(ptr, v) *(int16x8_t*)(ptr) = v
#define PStoreInt16x8NC(ptr, v) *(int16x8_t*)(ptr) = v
#define PStoreInt32x4(ptr, v) *(int32x4_t*)(ptr) = v
#define PStoreInt32x4NC(ptr, v) *(int32x4_t*)(ptr) = v
#endif
#define PCONVI8x4_U(v) vreinterpret_s8_u8(v)
#define PCONVI8x8_U(v) vreinterpret_s8_u8(v)
#define PCONVI8x16_U(v) vreinterpretq_s8_u8(v)
#define PCONVI16x4_U(v) vreinterpret_s16_u16(v)
#define PCONVI16x8_U(v) vreinterpretq_s16_u16(v)
#define PCONVU16x4_I(v) vreinterpret_u16_s16(v)
#define PCONVU16x8_I(v) vreinterpretq_u16_s16(v)
#define PEXTW4(v, i) vget_lane_s16(v, i)
#define PEXTW8(v, i) vgetq_lane_s16(v, i)
#define PEXTUW4(v, i) vget_lane_u16(v, i)
#define PEXTUW8(v, i) vgetq_lane_u16(v, i)
#define PEXTD4(v, i) vgetq_lane_s32(v, i)
#define PINSUW4(v, i, iv) vset_lane_u16(iv, v, i)

UInt8x8 FORCEINLINE PUNPCKBB8(UInt8x4 v1, UInt8x4 v2)
{
	uint8x8x2_t v = vzip_u8(v1, v2);
	return v.val[0];
}

UInt8x8 FORCEINLINE PUNPCKUBB4(UInt8x4 v1, UInt8x4 v2)
{
	uint8x8x2_t v = vzip_u8(v1, v2);
	return v.val[0];
}

UInt16x4 FORCEINLINE PUNPCKUBW4(UInt8x4 v1, UInt8x4 v2)
{
	uint8x8x2_t v = vzip_u8(v1, v2);
	return vreinterpret_u8_u16(v.val[0]);
}

UInt16x8 FORCEINLINE PUNPCKUBW8(UInt8x8 v1, UInt8x8 v2)
{
	uint8x8x2_t v = vzip_u8(v1, v2);
	return vreinterpretq_u8_u16(vcombine_u8(v.val[0], v.val[1]));
}

UInt16x8 FORCEINLINE PUNPCKLUBW8(UInt8x16 v1, UInt8x16 v2)
{
	return vreinterpretq_u8_u16(vzipq_u8(v1, v2).val[0]);
}

UInt16x8 FORCEINLINE PUNPCKHUBW8(UInt8x16 v1, UInt8x16 v2)
{
	return vreinterpretq_u8_u16(vzipq_u8(v1, v2).val[1]);
}

Int16x8 FORCEINLINE PUNPCKWW4(Int16x4 v1, Int16x4 v2)
{
	int16x4x2_t v = vzip_s16(v1, v2);
	return vcombine_s16(v.val[0], v.val[1]);
}

Int16x8 FORCEINLINE PUNPCKLWW8(Int16x8 v1, Int16x8 v2)
{
	return vzipq_s16(v1, v2).val[0];
}

Int16x8 FORCEINLINE PUNPCKHWW8(Int16x8 v1, Int16x8 v2)
{
	return vzipq_s16(v1, v2).val[1];
}
//#define PUNPCKWD4(v1, v2) _mm_unpacklo_epi16(v1, v2)
#define PMergeW4(v1, v2) vcombine_s16(v1, v2)
#define PMergeLW4(v1, v2) vcombine_s16(vget_low_s16(v1), vget_low_s16(v2))
#define PMergeHW4(v1, v2) vcombine_s16(vget_high_s16(v1), vget_high_s16(v2))
#define PMergeSARDW4(v1, v2, cnt) vcombine_s16(vqshrn_n_s32(v1, cnt), vqshrn_n_s32(v2, cnt))
#define PSALW4(v1, cnt) vshl_n_s16(v1, cnt)
#define PSHRW4(v1, cnt) vshr_n_u16(v1, cnt)
#define PSHRW8(v1, cnt) vshrq_n_u16(v1, cnt)
#define PSARW4(v1, cnt) vshr_n_s16(v1, cnt)
#define PSARW8(v1, cnt) vshrq_n_s16(v1, cnt)
#define PSARD4(v1, cnt) vshrq_n_s32(v1, cnt)
#define PSARSDW4(v1, cnt) vqshrn_n_s32(v1, cnt)
#define PSARSDW8(v1, v2, cnt) vcombine_s16(vqshrn_n_s32(v1, cnt), vqshrn_n_s32(v2, cnt))
#define PSHRADDWB4(v1, v2, cnt) vqshrn_n_u16(vcombine_u16(vqadd_u16(v1, v2), v2), cnt)
#define PADDUB4(v1, v2) vadd_u8(v1, v2)
#define PADDUB8(v1, v2) vadd_u8(v1, v2)
#define PADDUB16(v1, v2) vaddq_u8(v1, v2)
#define PADDW4(v1, v2) vadd_s16(v1, v2)
#define PADDUW4(v1, v2) vadd_u16(v1, v2)
#define PADDW8(v1, v2) vaddq_s16(v1, v2)
#define PADDD4(v1, v2) vaddq_s32(v1, v2)
#define PADDSUWB4(v1, v2) vmovn_u16(vcombine_u16(vadd_u16(v1, v2), v1))
#define PSADDW4(v1, v2) vqadd_s16(v1, v2)
#define PSADDUW4(v1, v2) vqadd_u16(v1, v2)
#define PSADDW8(v1, v2) vqaddq_s16(v1, v2)
#define PHSADDW8_4(v1, v2) vcombine_s16(vqadd_s16(vget_low_s16(v1), vget_high_s16(v1)), vqadd_s16(vget_low_s16(v2), vget_high_s16(v2)))
#define PSUBW4(v1, v2) vsub_s16(v1, v2)
#define PMULHW4(v1, v2) vshrn_n_s32(vmull_s16(v1, v2), 16)
Int16x8 FORCEINLINE PMULHW8(Int16x8 v1, Int16x8 v2)
{
	return PSARSDW8(vmull_s16(vget_low_s16(v1), vget_low_s16(v2)), vmull_s16(vget_high_s16(v1), vget_high_s16(v2)), 16);
}
#define PMULUHW4(v1, v2) vshrn_n_u32(vmull_u16(v1, v2), 16)
#define PMULULW4(v1, v2) vmovn_u32(vmull_u16(v1, v2))
#define PMULM2HW4(v1, v2) vqdmulh_s16(v1, v2)
#define PMULM2HW8(v1, v2) vqdmulhq_s16(v1, v2)
Int32x4 FORCEINLINE PMADDWD(Int16x8 v1, Int16x8 v2)
{
	int32x4x2_t v = vuzpq_s32(vmull_s16(vget_low_s16(v1), vget_low_s16(v2)), vmull_s16(vget_high_s16(v1), vget_high_s16(v2)));
	return vaddq_s32(v.val[0], v.val[1]);
}
#define SI16ToI8x4(v1) vqmovn_s16(vcombine_s16(v1, v1))
#define SI16ToI8x8(v1) vqmovn_s16(v1)
#define SI16ToI8x16(v1, v2) vcombine_s16(vqmovn_s16(v1), vqmovn_s16(v2))
#define SI32ToU8x8(v1, v2) vmovn_u16(vreinterpretq_s16_u16(vcombine_s16(vqmovn_s32(v1), vqmovn_s32(v2))))
#define SI32ToI16x4(v1) vqmovn_s32(v1)
#define SI32ToI16x8(v1, v2) vcombine_s16(vqmovn_s32(v1), vqmovn_s32(v2))

/*#define PResetAVX() _mm256_zeroupper()
#define PInt32x8Clear() _mm256_setzero_si256()
#define PLoadInt32x8(ptr) _mm256_loadu_si256((__m256i*)ptr)
#define PStoreInt32x8(ptr, v) _mm256_store_si256((__m256i*)ptr, v)
#define PStoreInt32x8NC(ptr, v) _mm256_stream_si256((__m256i*)ptr, v)
*/
#else
typedef struct
{
	Int8 vals[4];
} Int8x4;

typedef struct
{
	Int8 vals[8];
} Int8x8;

typedef struct
{
	Int8 vals[16];
} Int8x16;

typedef struct
{
	UInt8 vals[4];
} UInt8x4;

typedef struct
{
	UInt8 vals[8];
} UInt8x8;

typedef struct
{
	UInt8 vals[16];
} UInt8x16;

typedef struct
{
	Int16 vals[4];
} Int16x4;

typedef struct
{
	Int16 vals[8];
} Int16x8;

typedef struct
{
	UInt16 vals[8];
} UInt16x4;

typedef struct
{
	UInt16 vals[8];
} UInt16x8;

typedef struct
{
	Int32 vals[4];
} Int32x4;

typedef struct
{
	Int32 vals[8];
} Int32x8;

typedef struct
{
	UInt32 vals[4];
} UInt32x4;

void FORCEINLINE PResetAVX()
{
}

UInt8x4 FORCEINLINE PUInt8x4Clear()
{
	UInt8x4 ret;
	ret.vals[0] = 0;
	ret.vals[1] = 0;
	ret.vals[2] = 0;
	ret.vals[3] = 0;
	return ret;
}

UInt8x8 FORCEINLINE PUInt8x8Clear()
{
	UInt8x8 ret;
	ret.vals[0] = 0;
	ret.vals[1] = 0;
	ret.vals[2] = 0;
	ret.vals[3] = 0;
	ret.vals[4] = 0;
	ret.vals[5] = 0;
	ret.vals[6] = 0;
	ret.vals[7] = 0;
	return ret;
}

Int16x4 FORCEINLINE PInt16x4Clear()
{
	Int16x4 ret;
	ret.vals[0] = 0;
	ret.vals[1] = 0;
	ret.vals[2] = 0;
	ret.vals[3] = 0;
	return ret;
}

Int16x8 FORCEINLINE PInt16x8Clear()
{
	Int16x8 ret;
	ret.vals[0] = 0;
	ret.vals[1] = 0;
	ret.vals[2] = 0;
	ret.vals[3] = 0;
	ret.vals[4] = 0;
	ret.vals[5] = 0;
	ret.vals[6] = 0;
	ret.vals[7] = 0;
	return ret;
}

Int32x4 FORCEINLINE PInt32x4Clear()
{
	Int32x4 ret;
	ret.vals[0] = 0;
	ret.vals[1] = 0;
	ret.vals[2] = 0;
	ret.vals[3] = 0;
	return ret;
}

Int32x8 FORCEINLINE PInt32x8Clear()
{
	Int32x8 ret;
	ret.vals[0] = 0;
	ret.vals[1] = 0;
	ret.vals[2] = 0;
	ret.vals[3] = 0;
	ret.vals[4] = 0;
	ret.vals[5] = 0;
	ret.vals[6] = 0;
	ret.vals[7] = 0;
	return ret;
}

UInt8x4 FORCEINLINE PUInt8x4SetA(UInt8 val)
{
	UInt8x4 ret;
	ret.vals[0] = val;
	ret.vals[1] = val;
	ret.vals[2] = val;
	ret.vals[3] = val;
	return ret;
}

UInt8x8 FORCEINLINE PUInt8x8SetA(UInt8 val)
{
	UInt8x8 ret;
	ret.vals[0] = val;
	ret.vals[1] = val;
	ret.vals[2] = val;
	ret.vals[3] = val;
	ret.vals[4] = val;
	ret.vals[5] = val;
	ret.vals[6] = val;
	ret.vals[7] = val;
	return ret;
}

UInt8x16 FORCEINLINE PUInt8x16SetA(UInt8 val)
{
	UInt8x16 ret;
	ret.vals[0] = val;
	ret.vals[1] = val;
	ret.vals[2] = val;
	ret.vals[3] = val;
	ret.vals[4] = val;
	ret.vals[5] = val;
	ret.vals[6] = val;
	ret.vals[7] = val;
	ret.vals[8] = val;
	ret.vals[9] = val;
	ret.vals[10] = val;
	ret.vals[11] = val;
	ret.vals[12] = val;
	ret.vals[13] = val;
	ret.vals[14] = val;
	ret.vals[15] = val;
	return ret;
}

Int16x4 FORCEINLINE PInt16x4SetA(Int16 val)
{
	Int16x4 ret;
	ret.vals[0] = val;
	ret.vals[1] = val;
	ret.vals[2] = val;
	ret.vals[3] = val;
	return ret;
}

Int16x8 FORCEINLINE PInt16x8SetA(Int16 val)
{
	Int16x8 ret;
	ret.vals[0] = val;
	ret.vals[1] = val;
	ret.vals[2] = val;
	ret.vals[3] = val;
	ret.vals[4] = val;
	ret.vals[5] = val;
	ret.vals[6] = val;
	ret.vals[7] = val;
	return ret;
}

UInt16x4 FORCEINLINE PUInt16x4SetA(UInt16 val)
{
	UInt16x4 ret;
	ret.vals[0] = val;
	ret.vals[1] = val;
	ret.vals[2] = val;
	ret.vals[3] = val;
	return ret;
}

Int32x4 FORCEINLINE PInt32x4SetA(Int32 val)
{
	Int32x4 ret;
	ret.vals[0] = val;
	ret.vals[1] = val;
	ret.vals[2] = val;
	ret.vals[3] = val;
	return ret;
}

UInt32x4 FORCEINLINE PUInt32x4SetA(UInt32 val)
{
	UInt32x4 ret;
	ret.vals[0] = val;
	ret.vals[1] = val;
	ret.vals[2] = val;
	ret.vals[3] = val;
	return ret;
}

UInt8x4 FORCEINLINE PLoadUInt8x4(const void *ptr)
{
	return *(UInt8x4*)ptr;	
}

UInt8x8 FORCEINLINE PLoadUInt8x8(const void *ptr)
{
	return *(UInt8x8*)ptr;	
}

UInt8x16 FORCEINLINE PLoadUInt8x16(const void *ptr)
{
	return *(UInt8x16*)ptr;	
}

Int16x4 FORCEINLINE PLoadInt16x4(const void *ptr)
{
	return *(Int16x4*)ptr;
}

Int16x8 FORCEINLINE PLoadInt16x8(const void *ptr)
{
	return *(Int16x8*)ptr;
}

Int16x8 FORCEINLINE PLoadInt16x8A(const void *ptr)
{
	return *(Int16x8*)ptr;
}

Int32x4 FORCEINLINE PLoadInt32x4(const void *ptr)
{
	return *(Int32x4*)ptr;
}

Int32x4 FORCEINLINE PLoadInt32x4A(const void *ptr)
{
	return *(Int32x4*)ptr;
}

Int32x4 FORCEINLINE PLoadInt32x4A(volatile void *ptr)
{
	volatile Int32x4 *iptr = (volatile Int32x4*)ptr;
	Int32x4 ret;
	ret.vals[0] = iptr->vals[0];
	ret.vals[1] = iptr->vals[1];
	ret.vals[2] = iptr->vals[2];
	ret.vals[3] = iptr->vals[3];
	return ret;
}

Int32x8 FORCEINLINE PLoadInt32x8(const void *ptr)
{
	return *(Int32x8*)ptr;
}

Int16x8 FORCEINLINE PMLoadInt16x4(const void *ptr1, const void *ptr2)
{
	Int16x8 ret;
	*(Int64*)&ret.vals[0] = *(Int64*)ptr1;
	*(Int64*)&ret.vals[4] = *(Int64*)ptr2;
	return ret;
}

void FORCEINLINE PStoreUInt8x4(void *ptr, UInt8x4 v)
{
	*(UInt8x4*)ptr = v;
}

void FORCEINLINE PStoreUInt8x8(void *ptr, UInt8x8 v)
{
	*(UInt8x8*)ptr = v;
}

void FORCEINLINE PStoreUInt8x16(void *ptr, UInt8x16 v)
{
	*(UInt8x16*)ptr = v;
}

void FORCEINLINE PStoreInt16x2(void *ptr, Int16x4 v)
{
	*(Int16*)&((UInt8*)ptr)[0] = v.vals[0];
	*(Int16*)&((UInt8*)ptr)[2] = v.vals[1];
}

void FORCEINLINE PStoreInt16x4(void *ptr, Int16x4 v)
{
	*(Int16x4*)ptr = v;
}

void FORCEINLINE PStoreInt16x8(void *ptr, Int16x8 v)
{
	*(Int16x8*)ptr = v;
}

void FORCEINLINE PStoreInt16x8A(void *ptr, Int16x8 v)
{
	*(Int16x8*)ptr = v;
}

void FORCEINLINE PStoreInt16x8NC(void *ptr, Int16x8 v)
{
	*(Int16x8*)ptr = v;
}

void FORCEINLINE PStoreUInt16x4(void *ptr, UInt16x4 v)
{
	*(UInt16x4*)ptr = v;
}

void FORCEINLINE PStoreInt32x4(void *ptr, Int32x4 v)
{
	*(Int32x4*)ptr = v;
}

void FORCEINLINE PStoreInt32x4NC(void *ptr, Int32x4 v)
{
	*(Int32x4*)ptr = v;
}

void FORCEINLINE PStoreInt32x8(void *ptr, Int32x8 v)
{
	*(Int32x8*)ptr = v;
}

void FORCEINLINE PStoreInt32x8NC(void *ptr, Int32x8 v)
{
	*(Int32x8*)ptr = v;
}

Int16x4 FORCEINLINE PCONVU16x4_I(UInt16x4 val)
{
	return *(Int16x4*)&val;
}

Int16x8 FORCEINLINE PCONVU16x8_I(UInt16x8 val)
{
	return *(Int16x8*)&val;
}

UInt8x4 FORCEINLINE PCONVI8x4_U(Int8x4 val)
{
	return *(UInt8x4*)&val;
}

UInt8x8 FORCEINLINE PCONVI8x8_U(Int8x8 val)
{
	return *(UInt8x8*)&val;
}

UInt8x16 FORCEINLINE PCONVI8x16_U(Int8x16 val)
{
	return *(UInt8x16*)&val;
}

UInt16x4 FORCEINLINE PCONVI16x4_U(Int16x4 val)
{
	return *(UInt16x4*)&val;
}

UInt16x8 FORCEINLINE PCONVI16x8_U(Int16x8 val)
{
	return *(UInt16x8*)&val;
}

Int16 FORCEINLINE PEXTW4(Int16x4 v, UInt16 iVal)
{
	return v.vals[iVal];
}

Int16 FORCEINLINE PEXTW8(Int16x8 v, UInt16 iVal)
{
	return v.vals[iVal];
}

UInt16 FORCEINLINE PEXTUW4(UInt16x4 v, UInt16 iVal)
{
	return v.vals[iVal];
}

UInt16 FORCEINLINE PEXTUW8(UInt16x8 v, UInt16 iVal)
{
	return v.vals[iVal];
}

Int32 FORCEINLINE PEXTD4(Int32x4 v, UInt16 iVal)
{
	return v.vals[iVal];
}

UInt16x4 FORCEINLINE PINSUW4(UInt16x4 val, UInt8 index, UInt16 iVal)
{
	val.vals[index] = iVal;
	return val;
}

UInt8x8 FORCEINLINE PUNPCKUBB4(UInt8x4 val1, UInt8x4 val2)
{
	UInt8x8 ret;
	ret.vals[0] = val1.vals[0];
	ret.vals[1] = val2.vals[0];
	ret.vals[2] = val1.vals[1];
	ret.vals[3] = val2.vals[1];
	ret.vals[4] = val1.vals[2];
	ret.vals[5] = val2.vals[2];
	ret.vals[6] = val1.vals[3];
	ret.vals[7] = val2.vals[3];
	return ret;
}

UInt8x16 FORCEINLINE PUNPCKUBB8(UInt8x8 val1, UInt8x8 val2)
{
	UInt8x16 ret;
	ret.vals[0] = val1.vals[0];
	ret.vals[1] = val2.vals[0];
	ret.vals[2] = val1.vals[1];
	ret.vals[3] = val2.vals[1];
	ret.vals[4] = val1.vals[2];
	ret.vals[5] = val2.vals[2];
	ret.vals[6] = val1.vals[3];
	ret.vals[7] = val2.vals[3];
	ret.vals[8] = val1.vals[4];
	ret.vals[9] = val2.vals[4];
	ret.vals[10] = val1.vals[5];
	ret.vals[11] = val2.vals[5];
	ret.vals[12] = val1.vals[6];
	ret.vals[13] = val2.vals[6];
	ret.vals[14] = val1.vals[7];
	ret.vals[15] = val2.vals[7];
	return ret;
}

UInt16x4 FORCEINLINE PUNPCKUBW4(UInt8x4 val1, UInt8x4 val2)
{
	UInt16x4 ret;
	ret.vals[0] = val1.vals[0] | (val2.vals[0] << 8);
	ret.vals[1] = val1.vals[1] | (val2.vals[1] << 8);
	ret.vals[2] = val1.vals[2] | (val2.vals[2] << 8);
	ret.vals[3] = val1.vals[3] | (val2.vals[3] << 8);
	return ret;
}

UInt16x8 FORCEINLINE PUNPCKUBW8(UInt8x8 val1, UInt8x8 val2)
{
	UInt16x8 ret;
	ret.vals[0] = val1.vals[0] | (val2.vals[0] << 8);
	ret.vals[1] = val1.vals[1] | (val2.vals[1] << 8);
	ret.vals[2] = val1.vals[2] | (val2.vals[2] << 8);
	ret.vals[3] = val1.vals[3] | (val2.vals[3] << 8);
	ret.vals[4] = val1.vals[4] | (val2.vals[4] << 8);
	ret.vals[5] = val1.vals[5] | (val2.vals[5] << 8);
	ret.vals[6] = val1.vals[6] | (val2.vals[6] << 8);
	ret.vals[7] = val1.vals[7] | (val2.vals[7] << 8);
	return ret;
}

UInt16x8 FORCEINLINE PUNPCKLUBW8(UInt8x16 val1, UInt8x16 val2)
{
	UInt16x8 ret;
	ret.vals[0] = val1.vals[0] | (val2.vals[0] << 8);
	ret.vals[1] = val1.vals[1] | (val2.vals[1] << 8);
	ret.vals[2] = val1.vals[2] | (val2.vals[2] << 8);
	ret.vals[3] = val1.vals[3] | (val2.vals[3] << 8);
	ret.vals[4] = val1.vals[4] | (val2.vals[4] << 8);
	ret.vals[5] = val1.vals[5] | (val2.vals[5] << 8);
	ret.vals[6] = val1.vals[6] | (val2.vals[6] << 8);
	ret.vals[7] = val1.vals[7] | (val2.vals[7] << 8);
	return ret;
}

UInt16x8 FORCEINLINE PUNPCKHUBW8(UInt8x16 val1, UInt8x16 val2)
{
	UInt16x8 ret;
	ret.vals[0] = val1.vals[8] | (val2.vals[8] << 8);
	ret.vals[1] = val1.vals[9] | (val2.vals[9] << 8);
	ret.vals[2] = val1.vals[10] | (val2.vals[10] << 8);
	ret.vals[3] = val1.vals[11] | (val2.vals[11] << 8);
	ret.vals[4] = val1.vals[12] | (val2.vals[12] << 8);
	ret.vals[5] = val1.vals[13] | (val2.vals[13] << 8);
	ret.vals[6] = val1.vals[14] | (val2.vals[14] << 8);
	ret.vals[7] = val1.vals[15] | (val2.vals[15] << 8);
	return ret;
}

Int16x4 FORCEINLINE PUNPCKBSW4(UInt8x4 val1, UInt8x4 val2)
{
	Int16x4 ret;
	ret.vals[0] = (Int16)(val1.vals[0] | (val2.vals[0] << 8));
	ret.vals[1] = (Int16)(val1.vals[1] | (val2.vals[1] << 8));
	ret.vals[2] = (Int16)(val1.vals[2] | (val2.vals[2] << 8));
	ret.vals[3] = (Int16)(val1.vals[3] | (val2.vals[3] << 8));
	return ret;
}

Int16x8 FORCEINLINE PUNPCKWW4(Int16x4 val1, Int16x4 val2)
{
	Int16x8 ret;
	ret.vals[0] = val1.vals[0];
	ret.vals[1] = val2.vals[0];
	ret.vals[2] = val1.vals[1];
	ret.vals[3] = val2.vals[1];
	ret.vals[4] = val1.vals[2];
	ret.vals[5] = val2.vals[2];
	ret.vals[6] = val1.vals[3];
	ret.vals[7] = val2.vals[3];
	return ret;
}

Int16x8 FORCEINLINE PUNPCKLWW8(Int16x8 val1, Int16x8 val2)
{
	Int16x8 ret;
	ret.vals[0] = val1.vals[0];
	ret.vals[1] = val2.vals[0];
	ret.vals[2] = val1.vals[1];
	ret.vals[3] = val2.vals[1];
	ret.vals[4] = val1.vals[2];
	ret.vals[5] = val2.vals[2];
	ret.vals[6] = val1.vals[3];
	ret.vals[7] = val2.vals[3];
	return ret;
}

Int16x8 FORCEINLINE PUNPCKHWW8(Int16x8 val1, Int16x8 val2)
{
	Int16x8 ret;
	ret.vals[0] = val1.vals[4];
	ret.vals[1] = val2.vals[4];
	ret.vals[2] = val1.vals[5];
	ret.vals[3] = val2.vals[5];
	ret.vals[4] = val1.vals[6];
	ret.vals[5] = val2.vals[6];
	ret.vals[6] = val1.vals[7];
	ret.vals[7] = val2.vals[7];
	return ret;
}

Int32x4 FORCEINLINE PUNPCKWD4(Int16x4 val1, Int16x4 val2)
{
	Int32x4 ret;
	ret.vals[0] = (((Int32)val2.vals[0]) << 16) | (UInt16)val1.vals[0];
	ret.vals[1] = (((Int32)val2.vals[1]) << 16) | (UInt16)val1.vals[1];
	ret.vals[2] = (((Int32)val2.vals[2]) << 16) | (UInt16)val1.vals[2];
	ret.vals[3] = (((Int32)val2.vals[3]) << 16) | (UInt16)val1.vals[3];
	return ret;
}

Int32x4 FORCEINLINE PUNPCKLWD4(Int16x8 val1, Int16x8 val2)
{
	Int32x4 ret;
	ret.vals[0] = (((Int32)val2.vals[0]) << 16) | (UInt16)val1.vals[0];
	ret.vals[1] = (((Int32)val2.vals[1]) << 16) | (UInt16)val1.vals[1];
	ret.vals[2] = (((Int32)val2.vals[2]) << 16) | (UInt16)val1.vals[2];
	ret.vals[3] = (((Int32)val2.vals[3]) << 16) | (UInt16)val1.vals[3];
	return ret;
}

Int32x4 FORCEINLINE PUNPCKHWD4(Int16x8 val1, Int16x8 val2)
{
	Int32x4 ret;
	ret.vals[0] = (((Int32)val2.vals[4]) << 16) | (UInt16)val1.vals[4];
	ret.vals[1] = (((Int32)val2.vals[5]) << 16) | (UInt16)val1.vals[5];
	ret.vals[2] = (((Int32)val2.vals[6]) << 16) | (UInt16)val1.vals[6];
	ret.vals[3] = (((Int32)val2.vals[7]) << 16) | (UInt16)val1.vals[7];
	return ret;
}

Int16x8 FORCEINLINE PMergeW4(Int16x4 val1, Int16x4 val2)
{
	Int16x8 ret;
	ret.vals[0] = val1.vals[0];
	ret.vals[1] = val1.vals[1];
	ret.vals[2] = val1.vals[2];
	ret.vals[3] = val1.vals[3];
	ret.vals[4] = val2.vals[0];
	ret.vals[5] = val2.vals[1];
	ret.vals[6] = val2.vals[2];
	ret.vals[7] = val2.vals[3];
	return ret;
}

Int16x8 FORCEINLINE PMergeLW4(Int16x8 val1, Int16x8 val2)
{
	val1.vals[4] = val2.vals[0];
	val1.vals[5] = val2.vals[1];
	val1.vals[6] = val2.vals[2];
	val1.vals[7] = val2.vals[3];
	return val1;
}

Int16x8 FORCEINLINE PMergeHW4(Int16x8 val1, Int16x8 val2)
{
	val2.vals[0] = val1.vals[4];
	val2.vals[1] = val1.vals[5];
	val2.vals[2] = val1.vals[6];
	val2.vals[3] = val1.vals[7];
	return val2;
}

Int16x8 FORCEINLINE PMergeSARDW4(Int32x4 val1, Int32x4 val2, UInt8 cnt)
{
	Int16x8 ret;
	ret.vals[0] = (Int16)(val1.vals[0] >> cnt);
	ret.vals[1] = (Int16)(val1.vals[1] >> cnt);
	ret.vals[2] = (Int16)(val1.vals[2] >> cnt);
	ret.vals[3] = (Int16)(val1.vals[3] >> cnt);
	ret.vals[4] = (Int16)(val2.vals[0] >> cnt);
	ret.vals[5] = (Int16)(val2.vals[1] >> cnt);
	ret.vals[6] = (Int16)(val2.vals[2] >> cnt);
	ret.vals[7] = (Int16)(val2.vals[3] >> cnt);
	return ret;
}

Int16x4 FORCEINLINE PSALW4(Int16x4 oriVal, UInt8 cnt)
{
	oriVal.vals[0] <<= cnt;
	oriVal.vals[1] <<= cnt;
	oriVal.vals[2] <<= cnt;
	oriVal.vals[3] <<= cnt;
	return oriVal;
}

UInt16x4 FORCEINLINE PSHRW4(UInt16x4 oriVal, UInt8 cnt)
{
	oriVal.vals[0] >>= cnt;
	oriVal.vals[1] >>= cnt;
	oriVal.vals[2] >>= cnt;
	oriVal.vals[3] >>= cnt;
	return oriVal;
}

UInt16x8 FORCEINLINE PSHRW8(UInt16x8 oriVal, UInt8 cnt)
{
	oriVal.vals[0] >>= cnt;
	oriVal.vals[1] >>= cnt;
	oriVal.vals[2] >>= cnt;
	oriVal.vals[3] >>= cnt;
	oriVal.vals[4] >>= cnt;
	oriVal.vals[5] >>= cnt;
	oriVal.vals[6] >>= cnt;
	oriVal.vals[7] >>= cnt;
	return oriVal;
}

Int16x4 FORCEINLINE PSARW4(Int16x4 oriVal, UInt8 cnt)
{
	oriVal.vals[0] >>= cnt;
	oriVal.vals[1] >>= cnt;
	oriVal.vals[2] >>= cnt;
	oriVal.vals[3] >>= cnt;
	return oriVal;
}

Int16x8 FORCEINLINE PSARW8(Int16x8 oriVal, UInt8 cnt)
{
	oriVal.vals[0] >>= cnt;
	oriVal.vals[1] >>= cnt;
	oriVal.vals[2] >>= cnt;
	oriVal.vals[3] >>= cnt;
	oriVal.vals[4] >>= cnt;
	oriVal.vals[5] >>= cnt;
	oriVal.vals[6] >>= cnt;
	oriVal.vals[7] >>= cnt;
	return oriVal;
}

Int32x4 FORCEINLINE PSARD4(Int32x4 oriVal, UInt8 cnt)
{
	oriVal.vals[0] >>= cnt;
	oriVal.vals[1] >>= cnt;
	oriVal.vals[2] >>= cnt;
	oriVal.vals[3] >>= cnt;
	return oriVal;
}

Int16x4 FORCEINLINE PSARSDW4(Int32x4 oriVal, UInt8 cnt)
{
	Int16x4 ret;
	ret.vals[0] = SI32ToI16(oriVal.vals[0] >> cnt);
	ret.vals[1] = SI32ToI16(oriVal.vals[1] >> cnt);
	ret.vals[2] = SI32ToI16(oriVal.vals[2] >> cnt);
	ret.vals[3] = SI32ToI16(oriVal.vals[3] >> cnt);
	return ret;
}

Int16x8 FORCEINLINE PSARSDW8(Int32x4 v1, Int32x4 v2, UInt8 cnt)
{
	Int16x8 ret;
	ret.vals[0] = SI32ToI16(v1.vals[0] >> cnt);
	ret.vals[1] = SI32ToI16(v1.vals[1] >> cnt);
	ret.vals[2] = SI32ToI16(v1.vals[2] >> cnt);
	ret.vals[3] = SI32ToI16(v1.vals[3] >> cnt);
	ret.vals[4] = SI32ToI16(v2.vals[0] >> cnt);
	ret.vals[5] = SI32ToI16(v2.vals[1] >> cnt);
	ret.vals[6] = SI32ToI16(v2.vals[2] >> cnt);
	ret.vals[7] = SI32ToI16(v2.vals[3] >> cnt);
	return ret;
}

UInt8x4 FORCEINLINE PSHRADDWB4(UInt16x4 v1, UInt16x4 v2, UInt8 cnt)
{
	UInt8x4 ret;
	ret.vals[0] = ((v1.vals[0] + v2.vals[0]) >> cnt) & 0xff;
	ret.vals[1] = ((v1.vals[1] + v2.vals[1]) >> cnt) & 0xff;
	ret.vals[2] = ((v1.vals[2] + v2.vals[2]) >> cnt) & 0xff;
	ret.vals[3] = ((v1.vals[3] + v2.vals[3]) >> cnt) & 0xff;
	return ret;
}

UInt8x4 FORCEINLINE PADDUB4(UInt8x4 val1, UInt8x4 val2)
{
	val1.vals[0] += val2.vals[0];
	val1.vals[1] += val2.vals[1];
	val1.vals[2] += val2.vals[2];
	val1.vals[3] += val2.vals[3];
	return val1;
}

UInt8x8 FORCEINLINE PADDUB8(UInt8x8 val1, UInt8x8 val2)
{
	val1.vals[0] += val2.vals[0];
	val1.vals[1] += val2.vals[1];
	val1.vals[2] += val2.vals[2];
	val1.vals[3] += val2.vals[3];
	val1.vals[4] += val2.vals[4];
	val1.vals[5] += val2.vals[5];
	val1.vals[6] += val2.vals[6];
	val1.vals[7] += val2.vals[7];
	return val1;
}

UInt8x16 FORCEINLINE PADDUB16(UInt8x16 val1, UInt8x16 val2)
{
	val1.vals[0] += val2.vals[0];
	val1.vals[1] += val2.vals[1];
	val1.vals[2] += val2.vals[2];
	val1.vals[3] += val2.vals[3];
	val1.vals[4] += val2.vals[4];
	val1.vals[5] += val2.vals[5];
	val1.vals[6] += val2.vals[6];
	val1.vals[7] += val2.vals[7];
	val1.vals[8] += val2.vals[8];
	val1.vals[9] += val2.vals[9];
	val1.vals[10] += val2.vals[10];
	val1.vals[11] += val2.vals[11];
	val1.vals[12] += val2.vals[12];
	val1.vals[13] += val2.vals[13];
	val1.vals[14] += val2.vals[14];
	val1.vals[15] += val2.vals[15];
	return val1;
}

Int16x4 FORCEINLINE PADDW4(Int16x4 val1, Int16x4 val2)
{
	val1.vals[0] += val2.vals[0];
	val1.vals[1] += val2.vals[1];
	val1.vals[2] += val2.vals[2];
	val1.vals[3] += val2.vals[3];
	return val1;
}

UInt16x4 FORCEINLINE PADDUW4(UInt16x4 val1, UInt16x4 val2)
{
	val1.vals[0] += val2.vals[0];
	val1.vals[1] += val2.vals[1];
	val1.vals[2] += val2.vals[2];
	val1.vals[3] += val2.vals[3];
	return val1;
}

Int16x8 FORCEINLINE PADDW8(Int16x8 val1, Int16x8 val2)
{
	val1.vals[0] += val2.vals[0];
	val1.vals[1] += val2.vals[1];
	val1.vals[2] += val2.vals[2];
	val1.vals[3] += val2.vals[3];
	val1.vals[4] += val2.vals[4];
	val1.vals[5] += val2.vals[5];
	val1.vals[6] += val2.vals[6];
	val1.vals[7] += val2.vals[7];
	return val1;
}

Int32x4 FORCEINLINE PADDD4(Int32x4 val1, Int32x4 val2)
{
	val1.vals[0] += val2.vals[0];
	val1.vals[1] += val2.vals[1];
	val1.vals[2] += val2.vals[2];
	val1.vals[3] += val2.vals[3];
	return val1;
}

UInt8x4 FORCEINLINE PADDSUWB4(UInt16x4 val1, UInt16x4 val2)
{
	UInt8x4 ret;
	ret.vals[0] = (val1.vals[0] + val2.vals[0]) & 0xff;
	ret.vals[1] = (val1.vals[1] + val2.vals[1]) & 0xff;
	ret.vals[2] = (val1.vals[2] + val2.vals[2]) & 0xff;
	ret.vals[3] = (val1.vals[3] + val2.vals[3]) & 0xff;
	return ret;
}

Int16x4 FORCEINLINE PSADDW4(Int16x4 val1, Int16x4 val2)
{
	Int32 v;
	v = val1.vals[0] + (Int32)val2.vals[0];
	val1.vals[0] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[1] + (Int32)val2.vals[1];
	val1.vals[1] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[2] + (Int32)val2.vals[2];
	val1.vals[2] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[3] + (Int32)val2.vals[3];
	val1.vals[3] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	return val1;
}

Int16x8 FORCEINLINE PSADDW8(Int16x8 val1, Int16x8 val2)
{
	Int32 v;
	v = val1.vals[0] + (Int32)val2.vals[0];
	val1.vals[0] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[1] + (Int32)val2.vals[1];
	val1.vals[1] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[2] + (Int32)val2.vals[2];
	val1.vals[2] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[3] + (Int32)val2.vals[3];
	val1.vals[3] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[4] + (Int32)val2.vals[4];
	val1.vals[4] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[5] + (Int32)val2.vals[5];
	val1.vals[5] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[6] + (Int32)val2.vals[6];
	val1.vals[6] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[7] + (Int32)val2.vals[7];
	val1.vals[7] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	return val1;
}

UInt16x4 FORCEINLINE PSADDUW4(UInt16x4 val1, UInt16x4 val2)
{
	UInt32 v;
	v = val1.vals[0] + (UInt32)val2.vals[0];
	val1.vals[0] = (v > 0xffff)?0xffff:(UInt16)v;
	v = val1.vals[1] + (UInt32)val2.vals[1];
	val1.vals[1] = (v > 0xffff)?0xffff:(UInt16)v;
	v = val1.vals[2] + (UInt32)val2.vals[2];
	val1.vals[2] = (v > 0xffff)?0xffff:(UInt16)v;
	v = val1.vals[3] + (UInt32)val2.vals[3];
	val1.vals[3] = (v > 0xffff)?0xffff:(UInt16)v;
	return val1;
}

Int16x8 FORCEINLINE PHSADDW8_4(Int16x8 val1, Int16x8 val2)
{
	Int32 v;
	v = val1.vals[0] + (Int32)val1.vals[4];
	val1.vals[0] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[1] + (Int32)val1.vals[5];
	val1.vals[1] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[2] + (Int32)val1.vals[6];
	val1.vals[2] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val1.vals[3] + (Int32)val1.vals[7];
	val1.vals[3] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val2.vals[0] + (Int32)val2.vals[4];
	val1.vals[4] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val2.vals[1] + (Int32)val2.vals[5];
	val1.vals[5] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val2.vals[2] + (Int32)val2.vals[6];
	val1.vals[6] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	v = val2.vals[3] + (Int32)val2.vals[7];
	val1.vals[7] = (v > 0x7fff)?0x7fff:((v < -0x8000)?-0x8000:(Int16)v);
	return val1;
}

Int16x4 FORCEINLINE PSUBW4(Int16x4 val1, Int16x4 val2)
{
	val1.vals[0] -= val2.vals[0];
	val1.vals[1] -= val2.vals[1];
	val1.vals[2] -= val2.vals[2];
	val1.vals[3] -= val2.vals[3];
	return val1;
}

Int32x4 FORCEINLINE PSUBD4(Int32x4 val1, Int32x4 val2)
{
	val1.vals[0] -= val2.vals[0];
	val1.vals[1] -= val2.vals[1];
	val1.vals[2] -= val2.vals[2];
	val1.vals[3] -= val2.vals[3];
	return val1;
}

Int16x4 FORCEINLINE PMULHW4(Int16x4 val1, Int16x4 val2)
{
	val1.vals[0] = (((Int32)val1.vals[0]) * val2.vals[0]) >> 16;
	val1.vals[1] = (((Int32)val1.vals[1]) * val2.vals[1]) >> 16;
	val1.vals[2] = (((Int32)val1.vals[2]) * val2.vals[2]) >> 16;
	val1.vals[3] = (((Int32)val1.vals[3]) * val2.vals[3]) >> 16;
	return val1;
}

Int16x8 FORCEINLINE PMULHW8(Int16x8 val1, Int16x8 val2)
{
	val1.vals[0] = (((Int32)val1.vals[0]) * val2.vals[0]) >> 16;
	val1.vals[1] = (((Int32)val1.vals[1]) * val2.vals[1]) >> 16;
	val1.vals[2] = (((Int32)val1.vals[2]) * val2.vals[2]) >> 16;
	val1.vals[3] = (((Int32)val1.vals[3]) * val2.vals[3]) >> 16;
	val1.vals[4] = (((Int32)val1.vals[4]) * val2.vals[4]) >> 16;
	val1.vals[5] = (((Int32)val1.vals[5]) * val2.vals[5]) >> 16;
	val1.vals[6] = (((Int32)val1.vals[6]) * val2.vals[6]) >> 16;
	val1.vals[7] = (((Int32)val1.vals[7]) * val2.vals[7]) >> 16;
	return val1;
}

UInt16x4 FORCEINLINE PMULULW4(UInt16x4 val1, UInt16x4 val2)
{
	val1.vals[0] *= val2.vals[0];
	val1.vals[1] *= val2.vals[1];
	val1.vals[2] *= val2.vals[2];
	val1.vals[3] *= val2.vals[3];
	return val1;
}

UInt16x4 FORCEINLINE PMULUHW4(UInt16x4 val1, UInt16x4 val2)
{
	val1.vals[0] = (val1.vals[0] * (UInt32)val2.vals[0]) >> 16;
	val1.vals[1] = (val1.vals[1] * (UInt32)val2.vals[1]) >> 16;
	val1.vals[2] = (val1.vals[2] * (UInt32)val2.vals[2]) >> 16;
	val1.vals[3] = (val1.vals[3] * (UInt32)val2.vals[3]) >> 16;
	return val1;
}

Int16x4 FORCEINLINE PMULM2HW4(Int16x4 val1, Int16x4 val2)
{
	val1.vals[0] = (val1.vals[0] * val2.vals[0]) >> 15;
	val1.vals[1] = (val1.vals[1] * val2.vals[1]) >> 15;
	val1.vals[2] = (val1.vals[2] * val2.vals[2]) >> 15;
	val1.vals[3] = (val1.vals[3] * val2.vals[3]) >> 15;
	return val1;
}

Int16x8 FORCEINLINE PMULM2HW8(Int16x8 val1, Int16x8 val2)
{
	val1.vals[0] = (val1.vals[0] * val2.vals[0]) >> 15;
	val1.vals[1] = (val1.vals[1] * val2.vals[1]) >> 15;
	val1.vals[2] = (val1.vals[2] * val2.vals[2]) >> 15;
	val1.vals[3] = (val1.vals[3] * val2.vals[3]) >> 15;
	val1.vals[4] = (val1.vals[4] * val2.vals[4]) >> 15;
	val1.vals[5] = (val1.vals[5] * val2.vals[5]) >> 15;
	val1.vals[6] = (val1.vals[6] * val2.vals[6]) >> 15;
	val1.vals[7] = (val1.vals[7] * val2.vals[7]) >> 15;
	return val1;
}

Int32x4 FORCEINLINE PMULLD4(Int32x4 val1, Int32x4 val2)
{
	val1.vals[0] *= val2.vals[0];
	val1.vals[1] *= val2.vals[1];
	val1.vals[2] *= val2.vals[2];
	val1.vals[3] *= val2.vals[3];
	return val1;
}

Int32x4 FORCEINLINE PMADDWD(Int16x8 val1, Int16x8 val2)
{
	Int32x4 ret;
	ret.vals[0] = val1.vals[0] * val2.vals[0] + val1.vals[1] * val2.vals[1];
	ret.vals[1] = val1.vals[2] * val2.vals[2] + val1.vals[3] * val2.vals[3];
	ret.vals[2] = val1.vals[4] * val2.vals[4] + val1.vals[5] * val2.vals[5];
	ret.vals[3] = val1.vals[6] * val2.vals[6] + val1.vals[7] * val2.vals[7];
	return ret;
}

UInt8x4 FORCEINLINE PSRADDWB4(UInt16x4 val1, UInt16x4 val2, UInt8 cnt)
{
	UInt8x4 ret;
	ret.vals[0] = SU32ToU8((val1.vals[0] + (UInt32)val2.vals[0]) >> cnt);
	ret.vals[1] = SU32ToU8((val1.vals[1] + (UInt32)val2.vals[1]) >> cnt);
	ret.vals[2] = SU32ToU8((val1.vals[2] + (UInt32)val2.vals[2]) >> cnt);
	ret.vals[3] = SU32ToU8((val1.vals[3] + (UInt32)val2.vals[3]) >> cnt);
	return ret;
}

Int8x4 FORCEINLINE SI16ToI8x4(Int16x4 val)
{
	Int8x4 ret;
	ret.vals[0] = SI16ToI8(val.vals[0]);
	ret.vals[1] = SI16ToI8(val.vals[1]);
	ret.vals[2] = SI16ToI8(val.vals[2]);
	ret.vals[3] = SI16ToI8(val.vals[3]);
	return ret;
}

Int8x8 FORCEINLINE SI16ToI8x8(Int16x8 val)
{
	Int8x8 ret;
	ret.vals[0] = SI16ToI8(val.vals[0]);
	ret.vals[1] = SI16ToI8(val.vals[1]);
	ret.vals[2] = SI16ToI8(val.vals[2]);
	ret.vals[3] = SI16ToI8(val.vals[3]);
	ret.vals[4] = SI16ToI8(val.vals[4]);
	ret.vals[5] = SI16ToI8(val.vals[5]);
	ret.vals[6] = SI16ToI8(val.vals[6]);
	ret.vals[7] = SI16ToI8(val.vals[7]);
	return ret;
}

Int8x16 FORCEINLINE SI16ToI8x16(Int16x8 val, Int16x8 val2)
{
	Int8x16 ret;
	ret.vals[0] = SI16ToI8(val.vals[0]);
	ret.vals[1] = SI16ToI8(val.vals[1]);
	ret.vals[2] = SI16ToI8(val.vals[2]);
	ret.vals[3] = SI16ToI8(val.vals[3]);
	ret.vals[4] = SI16ToI8(val.vals[4]);
	ret.vals[5] = SI16ToI8(val.vals[5]);
	ret.vals[6] = SI16ToI8(val.vals[6]);
	ret.vals[7] = SI16ToI8(val.vals[7]);
	ret.vals[8] = SI16ToI8(val2.vals[0]);
	ret.vals[9] = SI16ToI8(val2.vals[1]);
	ret.vals[10] = SI16ToI8(val2.vals[2]);
	ret.vals[11] = SI16ToI8(val2.vals[3]);
	ret.vals[12] = SI16ToI8(val2.vals[4]);
	ret.vals[13] = SI16ToI8(val2.vals[5]);
	ret.vals[14] = SI16ToI8(val2.vals[6]);
	ret.vals[15] = SI16ToI8(val2.vals[7]);
	return ret;
}

Int16x4 FORCEINLINE SI32ToI16x4(Int32x4 val)
{
	Int16x4 ret;
	ret.vals[0] = SI32ToI16(val.vals[0]);
	ret.vals[1] = SI32ToI16(val.vals[1]);
	ret.vals[2] = SI32ToI16(val.vals[2]);
	ret.vals[3] = SI32ToI16(val.vals[3]);
	return ret;
}

Int16x8 FORCEINLINE SI32ToI16x8(Int32x4 val1, Int32x4 val2)
{
	Int16x8 ret;
	ret.vals[0] = SI32ToI16(val1.vals[0]);
	ret.vals[1] = SI32ToI16(val1.vals[1]);
	ret.vals[2] = SI32ToI16(val1.vals[2]);
	ret.vals[3] = SI32ToI16(val1.vals[3]);
	ret.vals[4] = SI32ToI16(val2.vals[0]);
	ret.vals[5] = SI32ToI16(val2.vals[1]);
	ret.vals[6] = SI32ToI16(val2.vals[2]);
	ret.vals[7] = SI32ToI16(val2.vals[3]);
	return ret;
}

UInt8x8 FORCEINLINE SI32ToU8x8(Int32x4 val1, Int32x4 val2)
{
	UInt8x8 ret;
	ret.vals[0] = SI32ToU8(val1.vals[0]);
	ret.vals[1] = SI32ToU8(val1.vals[1]);
	ret.vals[2] = SI32ToU8(val1.vals[2]);
	ret.vals[3] = SI32ToU8(val1.vals[3]);
	ret.vals[4] = SI32ToU8(val2.vals[0]);
	ret.vals[5] = SI32ToU8(val2.vals[1]);
	ret.vals[6] = SI32ToU8(val2.vals[2]);
	ret.vals[7] = SI32ToU8(val2.vals[3]);
	return ret;
}
#endif
