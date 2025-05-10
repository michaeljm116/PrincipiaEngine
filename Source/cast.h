#pragma once

template <typename T>
inline int to_int(T value) {
    return static_cast<int>(value);
}

template<typename T>
inline unsigned int to_uint(T value) {
	return static_cast<unsigned int>(value);
}

template<typename T>
inline unsigned to_u(T value) {
	return static_cast<unsigned>(value);
}

template <typename T>
inline float to_flt(T value) {
    return static_cast<float>(value);
}

template <typename T>
inline uint32_t to_u32(T value) {
	return static_cast<uint32_t>(value);
}

template <typename T>
inline uint16_t to_u16(T value) {
	return static_cast<uint16_t>(value);
}