#pragma once
#ifndef _SCOPE_HANDLE_HH
#define _SCOPE_HANDLE_HH

#include "handleapi.h"

//RAII,用于管理文件句柄，Scope_guard , lock_guard
class ScopeHandle
{
//big five
public:
	ScopeHandle(HANDLE h): _handle(h) {};

	//句柄资源不允许拷贝
	ScopeHandle(const ScopeHandle& sh) = delete;
	ScopeHandle& operator = (const ScopeHandle& sh) = delete;

	//句柄可以转移控制权
	ScopeHandle(ScopeHandle&& sh) noexcept {
		_handle = sh.Release();		
	}

	ScopeHandle& operator = ( ScopeHandle&& sh) noexcept{
		if (this != &sh) {
			_handle = sh.Release();
		}	
		return *this;
	}

	~ScopeHandle() {
		Close();
	}

public:
	
	bool Close() {
		if (!isValid()) {
			true;
		}

		return ::CloseHandle(Release());
	}

	bool isValid() const{
		return _handle != INVALID_HANDLE_VALUE && _handle != nullptr;
	}

	HANDLE get() const{
		return _handle;
	}

	HANDLE Release() {
		HANDLE h = _handle;
		_handle = INVALID_HANDLE_VALUE;
		return h;
	}

private:
	HANDLE _handle;
};

#endif