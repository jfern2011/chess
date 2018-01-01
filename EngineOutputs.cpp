#include "EngineOutputs.h"

/**
 * Constructor
 *
 * @param [in] name The name of this output element
 */
element_base::element_base(const std::string& name)
	: _has_changed(false), _name(name)
{
}

/**
 * Destructor
 */
element_base::~element_base()
{
}

/**
 * Get the name of this output element
 *
 * @return The element name
 */
std::string element_base::get_name() const
{
	return _name;
}

/**
 * Constructor
 */
EngineOutputs::EngineOutputs()
	: _elements()
{
}

/**
 * Destructor
 */
EngineOutputs::~EngineOutputs()
{
	for (size_t i = 0; i < _elements.size(); i++)
	{
		delete _elements[i];
	}
}

/**
 * Get the unique ID of an output element
 *
 * @param[in] name The element name
 *
 * @return The element ID, or -1 on error
 */
int EngineOutputs::get_id(const std::string& name) const
{
	for (size_t i = 0; i < _elements.size(); i++)
	{
		auto elem = _elements[i];
		if (elem->get_name() == name)
			return i;
	}

	return -1;
}

/**
 * Get the number of created output elements
 *
 * @return The number of elements
 */
size_t EngineOutputs::size() const
{
	return _elements.size();
}

/**
 * Iterate through all output elements, updating
 * each one
 */
void EngineOutputs::update()
{
	for (size_t i = 0; i < _elements.size(); i++)
		_elements[i]->update();
}

/**
 * Look up the output element with the specified name
 *
 * @param[in] name The name of the element
 *
 * @return  The element with name \a name, or nullptr
 *          on error
 */
const element_base*
	EngineOutputs::operator[](const std::string& name) const
{
	for (auto i = 0; i < _elements.size(); i++)
	{
		auto elem = _elements[i];
		if (elem->get_name() == name)
			return elem;
	}

	char msg[128];
	std::snprintf(msg, 128, "no such element '%s'\n",
		name.c_str());

	Abort(nullptr, msg);
}

/**
 * Get the output element at the specified index
 *
 * @param[in] index Pull from here
 *
 * @return The element at \a index, or a nullptr
 *         on error
 */
const element_base* EngineOutputs::operator[](size_t index) const
{
	AbortIf(_elements.size() <= index, nullptr);
	return _elements[index];
}

/*
EngineOutputs::EngineOutputs()
	: bestmove(0),
	  cpuload(-1),
	  currline(),
	  currmove(0),
	  currmovenumber(-1),
	  depth(-1),
	  hashfull(-1),
	  multipv(-1),
	  nodes(-1),
	  nps(-1),
	  pondermove(0),
	  pv(),
	  refutation(),
	  score(),
	  seldepth(-1),
	  string(""),
	  tbhits(-1),
	  time(-1)
{
}

EngineOutputs::~EngineOutputs()
{
}
*/
