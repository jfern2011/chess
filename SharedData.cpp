#include "SharedData.h"

/**
 * Constructor
 */
SharedData::SharedData()
	: _elements()
{
}

/**
 * Destructor
 */
SharedData::~SharedData()
{
}

/**
 * Check if an element has been created
 * 
 * @param[in]  name The element name
 * @param[out] id   The element id, if the element exists
 *
 * @return True if the element exists
 */
bool SharedData::_exists(const std::string& name,
	size_t& id ) const
{
	const std::string _name =
		Util::to_lower(Util::trim(name));

	for (size_t i = 0;
		 i < _elements.size(); i++)
	{
		if ( _elements[ i ]->get_name() == _name)
		{
			id = i; return true;
		}
	}

	return false;
}
