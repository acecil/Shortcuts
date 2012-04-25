//	
//	(c) 2012 by Andrew Gascoyne-Cecil.
//	
//	This file is part of Shortcuts.
//
//	Shortcuts is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//	
//	Shortcuts is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//	
//	You should have received a copy of the GNU General Public License
//	along with Shortcuts.  If not, see <http://www.gnu.org/licenses/>.
//
#pragma once

#include <string>
#include <map>
#include <stdexcept>
#include <type_traits>
#include <ostream>
#include <sstream>

class Config
{
public:
	Config(std::wstring filename=std::wstring());
	~Config(void);
	
	void Save(std::wstring filename=L"_config.conf"); 

	template<typename T>
	void SetParam(std::wstring name, T value)
	{
		std::wstringstream ss;
		ss << value;
		_params[name] = ss.str();
	}
	template<>
	void SetParam<std::wstring>(std::wstring name, std::wstring value)
	{
		_params[name] = value;
	}
	template<typename T>
	typename std::enable_if<std::is_same<std::wstring, T>::value, T>::type
		GetParam(std::wstring name) const
	{
		auto& it = _params.find(name);
		if( it == _params.end() )
		{
			return std::wstring();
		}

		return it->second;
	}
	template<typename T>
	typename std::enable_if<std::is_reference<decltype(*static_cast<std::wostream *>(0) >> *static_cast<T *>(0) )>::value
		&& !std::is_same<std::wstring, T>::value, T>::type
		GetParam(std::wstring name) const
	{
		auto& it = _params.find(name);
		if( it == _params.end() )
		{
			return T();
		}

		std::wstringstream ss(it->second);
		T val;
		if( !(ss >> val) )
		{
			throw new std::invalid_argument("Parameter cannot be converted to type");
		}

		return val;
	}
	template<typename T>
	typename std::enable_if<!std::is_reference<decltype(*static_cast<std::wostream *>(0) >> *static_cast<T *>(0) )>::value
		&& !std::is_same<std::wstring, T>::value, T>::type
		GetParam(std::wstring name) const
	{
		auto& it = _params.find(name);
		if( it == _params.end() )
		{
			return T();
		}

		return T(it->second);
	}
	template<typename T>
	typename std::enable_if<std::is_same<std::wstring, T>::value, T>::type
		GetSetParam(std::wstring name, std::wstring def)
	{
		auto& it = _params.find(name);
		if( it == _params.end() )
		{
			SetParam(name, def);
			it = _params.find(name);
		}

		return it->second;
	}
	template<typename T>
	typename std::enable_if<std::is_reference<decltype(*static_cast<std::wostream *>(0) >> *static_cast<T *>(0) )>::value
		&& !std::is_same<std::wstring, T>::value, T>::type
		GetSetParam(std::wstring name, T def)
	{
		auto& it = _params.find(name);
		if( it == _params.end() )
		{
			SetParam(name, def);
			it = _params.find(name);
		}

		std::wstringstream ss(it->second);
		T val;
		if( !(ss >> val) )
		{
			throw new std::invalid_argument("Parameter cannot be converted to type");
		}

		return val;
	}
	template<typename T>
	typename std::enable_if<!std::is_reference<decltype(*static_cast<std::wostream *>(0) >> *static_cast<T *>(0) )>::value
		&& !std::is_same<std::wstring, T>::value, T>::type
		GetSetParam(std::wstring name, T def)
	{
		auto& it = _params.find(name);
		if( it == _params.end() )
		{
			SetParam(name, def);
			it = _params.find(name);
		}

		return T(it->second);
	}

	std::map<std::wstring, std::wstring>::iterator begin() { return _params.begin(); }
	std::map<std::wstring, std::wstring>::iterator end() { return _params.end(); }
	std::map<std::wstring, std::wstring>::const_iterator begin() const { return _params.cbegin(); }
	std::map<std::wstring, std::wstring>::const_iterator end() const { return _params.cend(); }
	
private:
	std::map<std::wstring, std::wstring> _params;
};

