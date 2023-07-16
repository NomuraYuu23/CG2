#pragma once

template<class T> inline void SafeDeleta(T*& p) {
	delete p;
	p = nullptr;
}
