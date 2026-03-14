#include <format>
#include <cassert>
#include "Exceptions.h"

namespace Smasher {
	template <class T>
	Expected<T>::Expected(const T& other) :
		m_Error(false)
	{
		if constexpr (std::is_copy_constructible_v<T>) {
			new (&m_ReturnVal.ret) T(other);
		}
		else {
			assert(false && "T is not copy constructible");
		}
	}

	template <class T>
	Expected<T>::Expected(T&& other) :
		m_Error(false)
	{
		if constexpr (std::is_move_constructible_v<T>) {
			new (&m_ReturnVal.ret) T(std::move(other));
		}
		else {
			assert(false && "T is not move constructible");
		}
	}

	template <class T>
	Expected<T>& Expected<T>::operator= (const T& other) {
		if constexpr (std::is_copy_assignable_v<T>) {
			m_ReturnVal.ret = other;
		}
		else {
			assert(false && "T is not copy assignable");
		}
		m_Error = false;
	}

	template <class T>
	Expected<T>& Expected<T>::operator= (T&& other) {
		if constexpr (std::is_move_assignable_v<T>) {
			m_ReturnVal.ret = std::move(other);
		}
		else {
			assert(false && "T is not move assignable");
		}
		m_Error = false;
	}

	// Factory Method for throwing error
	template <class T>
	Expected<T> Expected<T>::Error(ErrorCode code) {
		return Expected<T>(code, Expected<T>::ErrTAG{});
	}

	template<class T>
	Expected<T>::~Expected() {
		if (!m_Error) {
			m_ReturnVal.ret.~T();
		}
		else {
			m_ReturnVal.code.~ErrorCode();
		}
	};

	template<class T>
	Expected<T>::operator bool() const {
		return !HasError();
	}

	template<class T>
	const Smasher::ErrorCode Expected<T>::GetError() const {
		return m_ReturnVal.code;
	}

	template<class T>
	T& Expected<T>::Get() {
		if (m_Error) {
			// Cannot cast, expected value was error
			std::string exceptionMessage = std::format("Error Code {}", m_ReturnVal.code);
			throw Exceptions::ExpectedHasError(exceptionMessage);
		}
		return m_ReturnVal.ret;
	}

	template<class T>
	const bool Expected<T>::HasError() const {
		return m_Error;
	}
}