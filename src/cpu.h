/*****************************************************************************
 * cpu.h - CPU feature and cache-control helpers.
 *****************************************************************************
 * Wraps the few x86 instructions the driver needs so the same source builds
 * with Visual C++ 6.0 (__asm blocks) and with GCC/mingw-w64 (extended asm).
 * MFENCE and CLFLUSH are emitted as raw bytes because VC6 does not know them.
 */

#ifndef _HDA_CPU_H_
#define _HDA_CPU_H_

// EDX feature flags from CPUID leaf 1.
static __inline ULONG HdaCpuidFeaturesEdx(void)
{
	ULONG edx_feat;
#ifdef _MSC_VER
	__asm {
		pushad
		mov eax, 1
		cpuid
		mov edx_feat, edx
		popad
	}
#else
	ULONG eax_out, ebx_out, ecx_out;
	__asm__ __volatile__("cpuid"
		: "=a" (eax_out), "=b" (ebx_out), "=c" (ecx_out), "=d" (edx_feat)
		: "a" (1UL));
#endif
	return edx_feat;
}

#define HDA_CPUID_EDX_CLFSH (1UL << 19)

// CLFLUSH [p]
static __inline void HdaClflush(ULONG_PTR p)
{
#ifdef _MSC_VER
	__asm {
		mov eax, p
		_emit 0x0F
		_emit 0xAE
		_emit 0x38
	}
#else
	__asm__ __volatile__(".byte 0x0F, 0xAE, 0x38" : : "a" (p) : "memory");
#endif
}

// MFENCE: makes preceding flushes globally visible.
static __inline void HdaMfence(void)
{
#ifdef _MSC_VER
	__asm {
		_emit 0x0F
		_emit 0xAE
		_emit 0xF0
	}
#else
	__asm__ __volatile__(".byte 0x0F, 0xAE, 0xF0" : : : "memory");
#endif
}

#endif // _HDA_CPU_H_
