#ifndef __SHARED_DATA_H__
#define __SHARED_DATA_H__

class element_base
{

protected:
	std::string _type;
};

template <typename T>
class Element : public element_base
{

public:

	Element()
	{
		if (Util::is_bool<T>::value)
            _type = "bool";
        else if (  Util::is_char<T>::value)
            _type = "char";
        else if ( Util::is_int16<T>::value)
            _type = "int16";
        else if ( Util::is_int32<T>::value)
            _type = "int32";
        else if ( Util::is_int64<T>::value)
            _type = "int64";
        else if ( Util::is_uchar<T>::value)
            _type = "uchar";
        else if (Util::is_uint16<T>::value)
            _type = "uint16";
        else if (Util::is_uint32<T>::value)
            _type = "uint32";
        else if (Util::is_uint64<T>::value)
            _type = "uint64";
        else if ( Util::is_float<T>::value)
            _type = "float";
        else if (Util::is_double<T>::value)
            _type = "double";
        else if (Util::is_string<T>::value)
            _type = "string";
        else
            _type = "";
	}

	Element(const T& init_value)
		: Element(), _value(init_value)
	{
	}

	const T& get() const
	{
		return _value;
	}

	T& get()
	{
		return _value;
	}

	void set(const T& value)
	{
		_value = value;
	}

private:

	T _value;
};

#endif
