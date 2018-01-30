#ifndef __SHARED_DATA_H__
#define __SHARED_DATA_H__

#include <memory>

#include "util.h"

/**
 * Interface for data elements shared software
 * components
 */
class element_base
{

public:

	element_base(const std::string& name)
		: _name(name)
	{}

	virtual
	~element_base() {}

	/**
	 * Get the name of this element
	 *
	 * @return The element name
	 */
	std::string get_name() { return _name; }

	/**
	 * Get the type of this element
	 *
	 * @return The element type
	 */
	std::string get_type() { return _type; }

protected:

	/**
	 * The name of this element (informational
	 * only)
	 */
	const std::string _name;
	
	/**
	 * The type of this element
	 */
	std::string _type;
};

/**
 * @class Element
 *
 * Represents an individual data element that can be stored in
 * a \ref SharedData object. Elements are typically created
 * via a \ref SharedData object instead of using this directly
 *
 * @tparam T The type of this element
 */
template <typename T>
class Element : public element_base
{

public:

	/**
	 * Constructor (1)
	 *
	 * @param[in] name     The name of this element
	 * @param[in] init_val An initial value for this element
	 */
	Element(const std::string& name, const T& init_value)
		: element_base(name), _value(init_value)
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

	/**
	 * Constructor (2)
	 *
	 * @param[in] name The name of this element
	 */
	Element(const std::string& name)
		: Element(name, T())
	{
	}

	/**
	 * Get the current value of this element (const overload)
	 *
	 * @return The element's value
	 */
	const T& get() const
	{
		return _value;
	}

	/**
	 * Get the current value of this element
	 *
	 * @return The element's value
	 */
	T& get()
	{
		return _value;
	}

	/**
	 *  Update this element with a new value
	 *
	 *  @param[in] value The new value
	 */
	void set(const T& value)
	{
		_value = value;
	}

private:

	/**
	 * The internal storage
	 */
	T _value;
};

/**
 * @class SharedData
 *
 * A simple data storage engine which facilitates data sharing
 * across software components. Shared elements are created
 * during the intialization phase and accessed at run-time via
 * a unique ID returned by \ref create()
 */
class SharedData
{

public:

	SharedData();

	~SharedData();

	/**
	 * Create a new data element
	 *
	 * @param [in]  name Give this name to your element
	 * @param [out] id   A unique id by which to access
	 *                   this element
	 * @param[in]   val  An (optional) initial value
	 *
	 * @return True on success
	 */
	template <typename T>
	bool create (const std::string& name, size_t& id,
		T val = T())
	{
		const std::string _name =
			Util::to_lower(Util::trim(name));

		AbortIf( _name.size() == 0 , false );

		/*
		 * Verify we aren't encountering this element again:
		 */
		if (_exists(_name, id))
		{
			char msg[256];

			std::snprintf(msg, 256, "duplicate name '%s'\n",
				_name.c_str());

			Abort(false, msg);
		}

		auto element = new Element<T>(name, val);
		AbortIfNot( element, false );

		id = _elements.size();
		_elements.push_back(
			std::unique_ptr<element_base>(element));

		return true;
	}

	/**
	 * Get a stored data element
	 *
	 * @param [in]  id    The unique element ID
	 * @param [out] value It's value
	 *
	 * @return True on success
	 */
	template <typename T>
	bool get(size_t id, T& value) const
	{
		AbortIf(_elements.size() <= id, false);

		auto element = dynamic_cast<
			Element<T>* >(_elements[id].get());

		AbortIfNot(element, false);

		value = element->get();
		return true;
	}

	/**
	 * Assign a value to a data element
	 *
	 * @param [in]  id    The unique element ID
	 * @param [in]  value A new value
	 *
	 * @return True on success
	 */
	template <typename T>
	bool set(size_t id, const T& value) const
	{
		AbortIf( _elements.size() <= id, false );

		auto element = dynamic_cast<
			Element<T>* >( _elements[id].get() );

		AbortIfNot(element, false);

		element->set(value);
		return true;
	}

	SharedData(const SharedData&) = delete;
	SharedData&
	 operator=(const SharedData&) = delete;

private:

	bool _exists(const std::string& name, size_t& id) const;

	/**
	 * The underlying storage
	 */
	std::vector<
		std::unique_ptr<element_base>
		> _elements;
};

#endif
