#pragma once

namespace dd
{
	using u8 = unsigned char;
	using u16 = unsigned short;
	using u32 = unsigned long;
	using u64 = unsigned long long;
	
	using s8 = signed char;
	using s16 = signed short;
	using s32 = signed long;
	using s64 = signed long long;
	
	using uint = u32;
	using usize = u64;
	using uptr = u64;

	using BitAction = std::function<void(u32 bitIndex)>;

	template <typename T>
	constexpr inline void foreachSetBit(const T* __restrict source, BitAction action) {
		T word = source[0];

		for (u32 i = 0u; word != 0u; ++i) {
			const bool smallestBitIsSet = word & T(1u);
			if (smallestBitIsSet) {
				action(i);
			}

			word >>= 1u;
		}
	}

	template <typename T>
	constexpr inline u8 fillSetBits(const T* __restrict source, u8* __restrict target) {
		T word = source[0];
		u8* dest = target;

		for (u8 i = 0u; word != 0u; ++i) {
			*dest = i;
			dest += word & T(1u);
			word >>= 1u;
		}

		return static_cast<u8>(dest - target);
	}
}